#include <carp.h>
#include <iostream>


int main(int argc, char *argv[]) {

    constexpr auto parser =
        carp::parser({{"a", "'a', a positional argument"},
                      {"b", "'b', a positional argument"},
                      {"c", "'c', a positional argument"},
                      {"d", "'d', a positional argument"},
                      {"e", "'e', a positional argument"},
                      {"-asdasd", "'asdasd', a switch", 1},
                      {"-arr", "'arr', a switch with args", 2},
                      {"-arrm", "'arrm', a switch with stringy args", 2},
                      {"-f-no-exceptions", "'f-no-exceptions', a switch"}});

    auto [ok, args] = parser.parse(argc, argv);

    std::cout << "\nnow doing argc/argv\n";
    std::cout << "\ndone, ok = " << std::boolalpha << ok << "\n\n";
    std::cout << parser.usage(argv[0]);

    auto a = args["a"] | 0;
    auto b = args["b"] | 1.3;
    auto c = args["c"] | 2.5f;
    auto d = args["d"] | "zebra";
    auto e = args["e"] | carp::required<int>;
    auto asdasd = args["-asdasd"] | 0;
    auto arr = args["-arr"] | std::array{0, 0};
    auto arrm = args["-arrm"] | std::array{"asds", "asdasd"};
    auto fnoexceptions = args["-f-no-exceptions"] | "none";

    if (a)
        std::cout << "\na = " << *a;
    else
        std::cout << "\nmissing a.";
    std::cout << ", b = " << *b;
    std::cout << ", c = " << *c;
    std::cout << ", d = " << d;
    if (e)
        std::cout << ", e = " << *e;
    else
        std::cout << "\nmissing e.";
    if (asdasd)
        std::cout << "\n-asdasd = " << *asdasd;
    else
        std::cout << "\nmissing -asdasd";

    if (arr) {
        std::cout << ", -arr = {";
        for (auto a : *arr)
            std::cout << a << ", ";
        std::cout << "}";
    } else
        std::cout << "\nmissing -arr";
    if (arrm) {
        std::cout << ", -arrm = {";
        for (auto a : *arrm)
            std::cout << a << ", ";
        std::cout << "}";
    } else
        std::cout << "\nmissing -arrm";

    std::cout << ", -f-no-exceptions = " << fnoexceptions;
    std::cout << ", -f-no-exceptions = " << args["-f-no-exceptions"];

   // std::cout << "\n\x1b[38;2;255;100;0mTRUECOLOR\x1b[0m\n";
    return 0;
}