#include <carp.h>
#include <iostream>


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

    auto [ok, args] = parser.parse(argc, argv);

    if(!ok) {
        std::cout << "\nToo many positional arguments, or unknown options.";
    }

    auto a = args["a"] | carp::required<int>;
    auto b = args["b"] | "zebra";
    auto c = args["c"] | 0;
    auto d = args["d"] | 1.3;
    auto e = args["e"] | 2.5f;

    auto s = args["-s"];
    auto t = args["-t"] | "none";
    auto u = args["-u"] | std::array{0, 0};
    auto v = args["-v"] | std::array{"tiger", "auroch"};

    if (a) {
        std::cout << "\na = " << *a;
    } else {
        std::cout << "\nI need a valid value for 'a'.\n";
        ok = false;
    }

    if (b) {
        std::cout << ", b = " << *b;
    } else {
        std::cout << "\nI need a valid value for 'b'.";
        ok = false;
    }

    if (c) {
        std::cout << ", c = " << *c;
    } else {
        std::cout << "\nI need a valid value for 'c'.";
        ok = false;
    }
    
    if (d) {
        std::cout << ", d = " << *d;
    } else {
        std::cout << "\nI need a valid value for 'd'.";
        ok = false;
    }
    
    if (e) {
        std::cout << ", e = " << *e;
    } else {
        std::cout << "\nI need a valid value for 'e'.";
        ok = false;
    }


    std::cout << std::boolalpha <<", -s = " << s;

    if (t)
        std::cout << "\n-t = " << *t;
    else
        std::cout << "\nmissing -t";

    if (u) {
        std::cout << ", -u = {";
        for (auto a : *u)
            std::cout << a << ", ";
        std::cout << "}";
    } else {
        std::cout << "\nmissing -u";
    }
    if (v) {
        std::cout << ", -v = {";
        for (auto a : *v)
            std::cout << a << ", ";
        std::cout << "}";
    } else
        std::cout << "\nmissing -v";

    if(!ok) {
        std::cout << "\n" << parser.usage(argv[0]);
        return 1;
    }

    return 0;
}