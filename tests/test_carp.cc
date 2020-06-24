#include <vector>


#include <carp.h>
#include <catch.hpp>

TEST_CASE("Basic functionality", "[basic]") {
    using std::size;
    char const * const argv[] = { "program", "1", "2", "3", "4" };
    int argc = size(argv);

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

    SECTION("string_views") {
        using std::size;

        auto [ok, args] = parser.parse(argc, argv);

        auto a = args["a"] | "";
        auto b = args["b"] | "";
        auto c = args["c"] | "";
        auto d = args["d"] | "";

        REQUIRE(a == "1");
        REQUIRE(b == "2");
        REQUIRE(c == "3");
        REQUIRE(d == "4");
    }

    SECTION("ints") {

        auto [ok, args] = parser.parse(argc, argv);

        auto a = args["a"] | -1;
        auto b = args["b"] | -2;
        auto c = args["c"] | -3;
        auto d = args["d"] | -4;

        REQUIRE(a == 1);
        REQUIRE(b == 2);
        REQUIRE(c == 3);
        REQUIRE(d == 4);
    }
}
