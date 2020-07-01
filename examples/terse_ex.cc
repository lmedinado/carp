#include <carp.h>
#include <iostream>

template <typename T, size_t ... I>
std::ostream &tuple_insertion_impl(std::ostream &os, T const &t, std::index_sequence<I...>) {
    ((os << std::get<I>(t) << ", "), ...);
    return os;
}

template <typename T>
auto operator<<(std::ostream &os, T const &t)
    -> std::enable_if_t<carp::detail::is_tuple<T>, std::ostream &> {

    os << "{ ";
    tuple_insertion_impl(os, t, std::make_index_sequence<std::tuple_size_v<T>>{});
    return os << "}";
}

int main(int argc, char *argv[]) {

    constexpr auto parser =
        carp::parser({{"a", "'a', a required integer"},
                      {"b", "'b', a string"},
                      {"c", "'c', an integer"},
                      {"d", "'d', a double"},
                      {"e", "'e', a float"},
                      {"-s", "'s', a boolean switch"},
                      {"-t", "'t', a switch taking a string as an extra argument", 1},
                      {"-u", "'u', a switch taking two integers as extra arguments", 2},
                      {"-v", "'v', a switch taking two strings as extra arguments", 2}});

    auto args = parser.parse(argc, argv);

    auto a = args["a"] | carp::required<int>;
    auto b = args["b"] | "zebra";
    auto c = args["c"] | 0;
    auto d = args["d"] | 1.3;
    auto e = args["e"] | 2.5f;

    auto s = args["-s"];
    auto t = args["-t"] | "none";
    auto u = args["-u"] | std::array{0, 0};
    auto v = args["-v"] | std::array{"tiger", "auroch"};

    if(!args.ok) {
        std::cout << "\n" << parser.usage(argv[0]);
        return 1;
    }

    /* past this point, all the optionals can be safely accessed. */

    std::cout << "\na = " << *a << ", ";
    std::cout << "b = " << *b << ", ";
    std::cout << "c = " << *c << ", ";
    std::cout << "d = " << *d << ", ";
    std::cout << "e = " << *e << ", ";

    std::cout << std::boolalpha << "\n-s = " << s << ", ";
    std::cout << "-t = " << *t << ", ";
    std::cout << "-u = " << *u << ", ";
    std::cout << "-v = " << *v << ", ";

    return 0;
}