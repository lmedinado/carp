/* carp is a constexpr, non-allocating command-line argument parser.
 *
 * Copyright (c) 2019 - present, Leandro Medina de Oliveira
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR \
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * --- Optional exception to the license ---
 *
 * As an exception, if, as a result of your compiling your source code, portions
 * of this Software are embedded into a machine-executable object form of such
 * source code, you may redistribute such embedded portions in such object form
 * without including the above copyright and permission notices.
 */

#pragma once
#include <algorithm>
#include <array>
#include <cassert>
#include <cerrno>
#include <charconv>
#include <cstdlib>
#include <iterator>
#include <numeric>
#include <optional>
#include <ostream>
#include <string>
#include <string_view>
#include <system_error>
#include <tuple>

namespace carp {

namespace detail {

/* Trick to parse numeric types using from_chars when available.
 * Important: the fallback versions assume a null terminator, so
 * this can be dangerous in other contexts. */
template <typename T, typename = void>
struct str_to_num {
    static std::optional<T> get(char const *str, char const *str_end) {
        char *p = nullptr;
        errno = 0;

        T result = [&]() {
            if constexpr (std::is_same_v<T, float>)
                return std::strtof(str, &p);
            if constexpr (std::is_same_v<T, double>)
                return std::strtod(str, &p);
            if constexpr (std::is_same_v<T, long double>)
                return std::strtold(str, &p);

            if constexpr (std::is_integral_v<T>) {
                auto n = [&]() {
                    if constexpr (std::is_signed_v<T>)
                        return std::strtoll(str, &p, 10);
                    else
                        return std::strtoull(str, &p, 10);
                }();

                if (n < std::numeric_limits<T>::min() || n > std::numeric_limits<T>::max())
                    errno = ERANGE;

                return static_cast<T>(n);
            }
        }();

        return (!errno && p != str_end - 1) ? std::optional<T>{result} : std::nullopt;
    }
};

/* specialization available if the corresponding from_chars overload is present. */
template <typename T>
struct str_to_num<T, std::void_t<decltype(std::from_chars(nullptr, nullptr, std::declval<T &>()))>> {
    static std::optional<T> get(char const *str, char const *str_end) {
        T result;
        auto [p, ec] = std::from_chars(str, str_end, result);
        return (ec == std::errc() && p == str_end) ? std::optional<T>{result} : std::nullopt;
    }
};

} // namespace detail

template <typename T, typename = void>
struct unwrapper {
    static std::optional<T> get(int argc, char const *const *argv) {
        if (argc != 1)
            return std::nullopt;

        return std::optional<T>{std::string_view(argv[0])};
    }
};

template <typename T>
struct unwrapper<T, std::enable_if_t<std::is_arithmetic_v<T>>> {
    static std::optional<T> get(int argc, char const *const *argv) {
        if (argc != 1)
            return std::nullopt;
        auto val = std::string_view(argv[0]);
        return detail::str_to_num<T>::get(val.data(), val.data() + val.size());
    }
};

namespace detail {
template <typename T, size_t N, size_t... I>
std::optional<std::array<T, N>> unwrap_array_impl(char const *const *argv,
                                                  std::index_sequence<I...>) {
    bool ok = true;
    std::optional<T> opt;
    std::array<T, N> res{(opt = unwrapper<T>::get(1, &argv[I]), //
                          ok = ok && opt,                       //
                          opt ? *opt : T{})...};

    return ok ? std::optional{res} : std::nullopt;
}
} // namespace detail

template <typename T, size_t N>
struct unwrapper<std::array<T, N>> {
    static std::optional<std::array<T, N>> get(int argc, char const *const *argv) {
        if (static_cast<size_t>(argc) < N)
            return std::nullopt;
        return detail::unwrap_array_impl<T, N>(argv, std::make_index_sequence<N>{});
    }
};

template <size_t N>
class parser {
private:
    struct labeled_arg {
        std::string_view name;
        int argc;
        char const *const *argv;
    };

    struct arg {
        std::string_view name;
        std::string_view desc;

        int nargs = 1;

        constexpr labeled_arg parse(int argc, char const *const *&argv) const {

            argc = std::min(argc, nargs);
            auto cur_argv = argv;

            if (nargs > 1 && is_switch(name)) {
                argc -= 1;
                cur_argv += 1;
            }

            argv += nargs;
            return labeled_arg{name, argc, cur_argv};
        }

    public:
        constexpr arg() = default;
        constexpr arg(std::string_view name, std::string_view desc, unsigned nargs = 0)
          : name(name), desc(desc), nargs(1 + nargs) {}
    };

public:
    constexpr parser(arg(&&arguments)[N]) {
        for (auto &&ai : arguments) {
            assert(is_valid(ai.name));
            if (!is_switch(ai.name))
                args[n_positionals++] = ai;
        }
        for (auto &&ai : arguments) {
            assert(is_valid(ai.name));
            if (is_switch(ai.name))
                args[n_positionals + n_switches++] = ai;
        }

        size_t i = 0;
        for (auto &a : args) {
            for (size_t j = ++i; j < N; ++j)
                assert(a.name != args[j].name && "no repeated names.");
        }
    }

    struct parsed_args {

        std::array<labeled_arg, N> args = {};

        struct arg_proxy {
            labeled_arg const *arg = nullptr;

            template <typename T>
            std::optional<T> operator|(T const &default_value) const {
                return arg ? unwrapper<T>::get(arg->argc, arg->argv) : default_value;
            }

            template <typename T>
            std::optional<T> operator|(std::optional<T> const &) const {
                return arg ? unwrapper<T>::get(arg->argc, arg->argv) : std::nullopt;
            }

            std::string_view operator|(std::string_view default_value) const {
                return arg ? arg->argv[0] : default_value;
            }
            std::string_view operator|(char const *default_value) const {
                return operator|(std::string_view(default_value));
            }

            template <size_t M>
            auto operator|(std::array<char const *, M> default_value)
                -> std::optional<std::array<std::string_view, M>> const {
                return operator|(
                    make_from_tuple<std::array<std::string_view, M>>(default_value));
            }

            operator bool() const { return !!arg; }

            arg_proxy &operator=(arg_proxy &&) = delete;
        };
        constexpr arg_proxy operator[](std::string_view name) const {
            auto it = std::find_if(args.begin(), args.end(),
                                   [&](auto &r) { return r.name == name; });

            return {it != args.end() ? &(*it) : nullptr};
        }
    };

    [[nodiscard]] std::pair<bool, parsed_args> parse(int argc, char const *const * argv) const {

        parsed_args res;

        bool ok = true;
        size_t pos_i = 0;

        for (auto it = argv + 1, end = argv + argc; it < end;) {
            auto const word = std::string_view(*it);

            size_t const ai = is_switch(word) ? find_switch(word) : pos_i++;

            if (ai < N) {
                res.args[ai] = args[ai].parse(end - it, it);
            } else { /* unrecognized switch or too many positionals */
                ++it, ok = false;
            }
        }

        return {ok, res};
    }

    auto usage(std::string_view program_name) const {
        return usage_holder{program_name, make_usage()};
    }

private:
    struct usage_holder {
        template <typename stream>
        friend stream &operator<<(stream &os, const usage_holder &uh) {
            return uh.usage(os, uh.program_name);
        }

        std::string_view program_name;
        decltype(std::declval<parser>().make_usage()) usage;
        usage_holder &operator=(usage_holder &&) = delete;
    };

    constexpr auto make_usage() const {
        return [&](auto &os, std::string_view program_name) -> decltype(os) {
            using std::size;

            os << "  Usage: ";
            auto const last_slash = program_name.find_last_of("/\\");
            os << program_name.substr(last_slash + 1, program_name.size() - last_slash);

            if (n_switches > 0)
                os << " [options]";

            for (auto pi = args.begin(); pi != args.begin() + n_positionals; ++pi)
                os << " " << pi->name;

            auto max_size =
                std::accumulate(args.begin(), args.end(), size_t{0},
                                [](auto c, auto &a) { return std::max(c, size(a.name)); });
            if (n_positionals)
                os << "\n\n  Arguments:";

            for (auto ai = args.begin(); ai != args.end(); ++ai) {
                if (ai == args.begin() + n_positionals)
                    os << "\n\n  Options:";

                os << "\n\t  " << ai->name;
                std::fill_n(std::ostreambuf_iterator(os), max_size + 1 - size(ai->name), ' ');
                os << ai->desc;
            }

            return os << "\n";
        };
    }
    static constexpr bool is_switch(std::string_view word) {
        return word.size() >= 2 && word[0] == '-' && !isdigit(word[1]);
    }

    static constexpr bool is_valid(std::string_view word) {
        return !word.empty() && !isdigit(word[0]) && word.find(' ') == std::string_view::npos;
    }

    static constexpr bool isdigit(char c) { return c >= '0' && c <= '9'; }

    size_t find_switch(std::string_view word) const {
        return std::distance(begin(args),
                             std::find_if(begin(args) + n_positionals, end(args),
                                          [&](auto &s) { return s.name == word; }));
    }

    template <class T, class Tuple, std::size_t... I>
    static constexpr T make_from_tuple_impl(Tuple &&t, std::index_sequence<I...>) {
        return T{std::get<I>(std::forward<Tuple>(t))...};
    }

    template <class T, class Tuple>
    static constexpr T make_from_tuple(Tuple &&t) {
        return make_from_tuple_impl<T>(
            std::forward<Tuple>(t),
            std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<Tuple>>>{});
    }

    size_t n_positionals = 0, n_switches = 0;
    std::array<arg, N> args;
};

template <typename T>
constexpr auto required = std::optional<T>();

} // namespace carp
