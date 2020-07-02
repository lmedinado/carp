#include <carp.h>
#include <iostream>
#include "tuple_print.h"

int main(int argc, char *argv[]) {

    constexpr auto parser = carp::parser({
        {"a", "'a', a required integer"},
        {"b", "'b', a string"},
        {"c", "'c', an integer"},
        {"d", "'d', a double"},
        {"e", "'e', a float"},
        {"-s", "'s', a boolean switch"},
        {"-t", "'t', a switch taking a string as an extra argument", 1},
        {"-u", "'u', a switch taking two integers as extra arguments", 2},
        {"-v", "'v', a switch taking two strings as extra arguments", 2},
        {"-w", "'w', a switch taking a string, an integer and a double as extra arguments", 3},
    });

    auto args = parser.parse(argc, argv);

    if(!args.ok) {
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
    auto w = args["-w"] | std::tuple{"gasket", 4, 1.3};

    /* you can check each optional individually to give an appropriate error message */
    if (!a) 
        std::cout << "\nI need a valid value for 'a'.\n";
    if (!b) 
        std::cout << "\nInvalid value provided for 'b'.";
    if (!c) 
        std::cout << "\nInvalid value provided for 'c'.";
    if (!d) 
        std::cout << "\nInvalid value provided for 'd'.";
    if (!e) 
        std::cout << "\nInvalid value provided for 'e'.";
    if (!t)  
        std::cout << "\nInvalid value provided for -t";
    if (!u)
        std::cout << "\nInvalid value provided for -u";
    if (!v) 
        std::cout << "\nInvalid value provided for -v";
    if (!w) 
        std::cout << "\nInvalid value provided for -w";
    

    if (!args.ok) {
        std::cout << "\n" << parser.usage(argv[0]);
        return 1;
    }
    /* past this point, all the optionals can be safely accessed. */

    std::cout << "\na = " << *a << ", ";
    std::cout << "b = " << *b << ", ";
    std::cout << "c = " << *c << ", ";
    std::cout << "d = " << *d << ", ";
    std::cout << "e = " << *e << ", ";

    if (s) {
        std::cout << "\n-s is set.";
    } else {
        std::cout << "\n-s is unset.";
    }

    std::cout << "\n-t = " << *t << ", ";
    std::cout << "-u = " << *u << ", ";
    std::cout << "-v = " << *v << ", ";
    std::cout << "-w = " << *w << ", ";

    return 0;
}