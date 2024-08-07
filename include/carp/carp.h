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
#include <cctype>
#include <cerrno>
#include <charconv>
#include <cstdlib>
#include <iosfwd>
#include <iterator>
#include <limits>
#include <numeric>
#include <optional>
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
    static std::optional<T> get(char const *str, char const *str_end) noexcept {

        if ((str_end - str) > 2 && std::tolower(str[1]) == 'x')
            return std::nullopt;

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

        return (!errno && p == str_end) ? std::optional<T>{result} : std::nullopt;
    }
};

/* specialization available if the corresponding from_chars overload is present. */
template <typename T>
struct str_to_num<T, std::void_t<decltype(std::from_chars(nullptr, nullptr, std::declval<T &>()))>> {
    static std::optional<T> get(char const *str, char const *str_end) noexcept {
        T result;
        auto [p, ec] = std::from_chars(str, str_end, result);
        return (ec == std::errc() && p == str_end) ? std::optional<T>{result} : std::nullopt;
    }
};

template <typename T>
constexpr bool is_tuple = false;

template <typename... Ts>
constexpr bool is_tuple<std::tuple<Ts...>> = true;

template <typename T, size_t N>
constexpr bool is_tuple<std::array<T, N>> = true;
} // namespace detail

template <typename T, typename = void>
struct unwrapper {
    static std::optional<T> get(int argc, char const *const *argv) noexcept {
        return (argc == 1) ? std::optional<T>{argv[0]} : std::nullopt;
    }
};

template <typename T>
struct unwrapper<T, std::enable_if_t<std::is_arithmetic_v<T>>> {
    static std::optional<T> get(int argc, char const *const *argv) noexcept {
        if (argc != 1)
            return std::nullopt;
        auto val = std::string_view(argv[0]);
        return detail::str_to_num<T>::get(val.data(), val.data() + val.size());
    }
};

template <typename T>
struct unwrapper<T, std::enable_if_t<detail::is_tuple<T>>> {
    template <size_t... I>
    static constexpr std::optional<T> get_tuple(char const *const *argv,
                                                std::index_sequence<I...>) noexcept {
        auto opts =
            std::make_tuple(unwrapper<std::tuple_element_t<I, T>>::get(1, argv + I)...);
        bool ok = (!!std::get<I>(opts) && ...);

        return ok ? std::optional<T>{{*std::get<I>(opts)...}} : std::nullopt;
    }

    static std::optional<T> get(int argc, char const *const *argv) noexcept {
        return (static_cast<size_t>(argc) == std::tuple_size_v<T>)
                   ? get_tuple(argv, std::make_index_sequence<std::tuple_size_v<T>>{})
                   : std::nullopt;
    }
};

template <size_t N>
class parser {
private:
    struct labeled_arg {
        std::string_view name;
        int argc = 0;
        char const *const *argv = nullptr;
    };

public:
    struct arg {
        std::string_view name;
        std::string_view desc;

        ptrdiff_t nargs = 1;

        constexpr labeled_arg parse(ptrdiff_t argc, char const *const *&argv) const noexcept {

            argc = std::min(argc, nargs);
            auto cur_argv = argv;
            argv += nargs - 1;

            if (nargs > 1 && is_switch(name)) {
                argc -= 1;
                cur_argv += 1;
            }

            return labeled_arg{name, static_cast<int>(argc), cur_argv};
        }

        constexpr arg() noexcept = default;
        constexpr arg(std::string_view name, std::string_view desc, size_t nargs = 0) noexcept
          : name(name), desc(desc), nargs(1 + nargs) {}
    };

    constexpr parser(arg const (&arguments)[N]) noexcept {
        for (auto i = std::begin(arguments), e = std::end(arguments); i != e; ++i) {
            assert(is_valid(i->name));
            if (!is_switch(i->name))
                args[n_positionals++] = *i;
        }
        for (auto i = std::begin(arguments), e = std::end(arguments); i != e; ++i) {
            if (is_switch(i->name))
                args[n_positionals + n_switches++] = *i;
        }

        size_t i = 0;
        for ([[maybe_unused]] auto &a : args) {
            for (size_t j = ++i; j < N; ++j)
                assert(a.name != args[j].name && "no repeated names.");
        }
    }

    struct parsed_args {
        bool ok = true;
        std::array<labeled_arg, N> args;

        struct arg_proxy {
            labeled_arg const *arg;
            bool &ok;

            template <typename T>
            constexpr auto operator|(T default_value) const noexcept {
                auto result = arg ? unwrapper<T>::get(arg->argc, arg->argv)
                                  : std::move(default_value);
                if (!result)
                    ok = false;
                return result;
            }

            template <typename T>
            constexpr std::optional<T> operator|(std::optional<T> const &) const noexcept {
                auto result = arg ? unwrapper<T>::get(arg->argc, arg->argv) : std::nullopt;
                if (!result)
                    ok = false;
                return result;
            }

            operator bool() const { return !!arg; }

            arg_proxy &operator=(arg_proxy &&) = delete;
        };

        constexpr arg_proxy operator[](std::string_view name) noexcept {
            auto it = std::find_if(args.begin(), args.end(),
                                   [&](auto &r) { return r.name == name; });

            return {it != args.end() ? &(*it) : nullptr, ok};
        }
    };

    [[nodiscard]] parsed_args parse(int argc, char const *const *argv) const noexcept {

        parsed_args res;

        size_t pos_i = 0;
        for (auto it = argv + 1, end = argv + argc; it < end; ++it) {
            auto const word = std::string_view(*it);

            size_t const ai =
                is_switch(word) ? find_switch(word) : pos_i < n_positionals ? pos_i++ : N;

            if (ai < N) {
                res.args[ai] = args[ai].parse(end - it, it);
            } else { /* unrecognized switch or too many positionals */
                res.ok = false;
            }
        }

        return res;
    }

    auto usage(std::string_view program_name, unsigned max_cols = 80) const noexcept {
        return usage_holder{program_name, this, max_cols};
    }

private:
    struct usage_holder {
        std::string_view program_name;
        parser const *p;
        unsigned max_cols;

        usage_holder &operator=(usage_holder &&) = delete;
    };

    template <typename stream>
    friend stream &operator<<(stream &os, const usage_holder &uh) noexcept {
        using std::size;
        auto &p = *uh.p;
        constexpr auto indent = std::string_view{"        "};

        os << "Usage: ";
        auto const last_slash = uh.program_name.find_last_of("/\\");
        os << uh.program_name.substr(last_slash + 1, uh.program_name.size() - last_slash);

        if (p.n_switches > 0)
            os << " [options]";

        for (auto pi = p.args.begin(); pi != p.args.begin() + p.n_positionals; ++pi)
            os << " " << pi->name;

        auto max_size =
            3 + std::accumulate(p.args.begin(), p.args.end(), size_t{0},
                                [](auto c, auto &a) { return std::max(c, size(a.name)); });
        if (p.n_positionals)
            os << "\n\nArguments:";

        for (auto ai = p.args.begin(); ai != p.args.end(); ++ai) {
            if (ai == p.args.begin() + p.n_positionals)
                os << "\n\nOptions:";

            os << "\n" << indent << ai->name;
            std::fill_n(std::ostreambuf_iterator(os), max_size - size(ai->name), ' ');

            size_t const max_per_line = uh.max_cols - max_size - size(indent) - 1;
            for (size_t i = 0; i < size(ai->desc);) {
                size_t eol = std::min(size(ai->desc) - i, max_per_line);

                auto this_line = ai->desc.substr(i, eol);

                size_t n;
                if ((n = this_line.find('\n')) != std::string_view::npos) {
                    this_line = this_line.substr(0, n);
                    eol = n + 1;
                }
                if (eol == max_per_line && (n = this_line.rfind(' ')) != std::string_view::npos) {
                    this_line = this_line.substr(0, eol = n + 1);
                }
                if (i) {
                    os << "\n" << indent;
                    std::fill_n(std::ostreambuf_iterator(os), max_size, ' ');
                }
                os << this_line;
                i += eol;
            }
        }
        return os;
    }

    static constexpr bool is_switch(std::string_view word) noexcept {
        return word.size() >= 2 && word[0] == '-' && !is_digit(word[1]);
    }

    static constexpr bool is_valid(std::string_view word) noexcept {
        return !word.empty() && !is_digit(word[0]) && word.find(' ') == std::string_view::npos;
    }

    static constexpr bool is_digit(char c) { return c >= '0' && c <= '9'; }

    size_t find_switch(std::string_view word) const noexcept {
        return std::distance(begin(args),
                             std::find_if(begin(args) + n_positionals, end(args),
                                          [&](auto &s) { return s.name == word; }));
    }

    size_t n_positionals = 0, n_switches = 0;
    std::array<arg, N> args;
};

template <typename T>
constexpr auto required = std::optional<T>();
} // namespace carp
