#include <carp.h>
#include <catch.hpp>

TEST_CASE("Basic positional functionality", "[basic_positional]") {

    constexpr auto parser = carp::parser({
        {"a", "'a', a positional argument"},
        {"b", "'b', a positional argument"},
        {"c", "'c', a positional argument"},
        {"d", "'d', a positional argument"},
    });

    char const *const argv[] = {"program", "0", "1", "2", "3"};
    int argc = std::size(argv);

    auto [ok, args] = parser.parse(argc, argv);


    SECTION("string_views") {
        auto a = args["a"] | "";
        auto b = args["b"] | "";
        auto c = args["c"] | "";
        auto d = args["d"] | "";

        REQUIRE((a == "0" && std::is_same_v<std::string_view, decltype(a)>));
        REQUIRE((b == "1" && std::is_same_v<std::string_view, decltype(b)>));
        REQUIRE((c == "2" && std::is_same_v<std::string_view, decltype(c)>));
        REQUIRE((d == "3" && std::is_same_v<std::string_view, decltype(d)>));
    }

    auto do_tests_for = [&](auto t) {
        using T = decltype(t);

        auto a = args["a"] | T{0};
        auto b = args["b"] | T{1};
        auto c = args["c"] | T{2};
        auto d = args["d"] | T{3};

        REQUIRE((*a == T{0} && std::is_same_v<T &, decltype(*a)>));
        REQUIRE((*b == T{1} && std::is_same_v<T &, decltype(*b)>));
        REQUIRE((*c == T{2} && std::is_same_v<T &, decltype(*c)>));
        REQUIRE((*d == T{3} && std::is_same_v<T &, decltype(*d)>));
    };

    SECTION("ints") { do_tests_for(int{}); }
    SECTION("uints") { do_tests_for(unsigned{}); }
    SECTION("longs") { do_tests_for(long{}); }
    SECTION("ulongs") { do_tests_for((unsigned long){}); }
    SECTION("llongs") { do_tests_for((long long){}); }
    SECTION("ullongs") { do_tests_for((unsigned long long){}); }
    SECTION("shorts") { do_tests_for(short{}); }
    SECTION("ushorts") { do_tests_for((unsigned short){}); }
    SECTION("chars") { do_tests_for(char{}); }
    SECTION("schars") { do_tests_for((signed char){}); }
    SECTION("uchars") { do_tests_for((unsigned char){}); }
    SECTION("floats") { do_tests_for(float{}); }
    SECTION("doubles") { do_tests_for(double{}); }
}

TEST_CASE("Basic boolean switch functionality", "[basic_bool_switch]") {
    using std::size;

    constexpr auto parser = carp::parser({
        {"-s", "'s', a switch"},
        {"-t", "'t', a switch"},
        {"-u", "'u', a switch"},
        {"-v", "'v', a switch"},
    });

    auto do_tests_for = [&](bool const(&f)[4], auto... argv_pack) {
        char const *const argv[] = {argv_pack...};
        int argc = size(argv);
        auto [ok, args] = parser.parse(argc, argv);

        REQUIRE(args["-s"] == f[0]);
        REQUIRE(args["-t"] == f[1]);
        REQUIRE(args["-u"] == f[2]);
        REQUIRE(args["-v"] == f[3]);
    };

    SECTION("None set") { do_tests_for({false, false, false, false}, "program"); }

    SECTION("One set") {
        do_tests_for({true, false, false, false}, "program", "-s");
        do_tests_for({false, true, false, false}, "program", "-t");
        do_tests_for({false, false, true, false}, "program", "-u");
        do_tests_for({false, false, false, true}, "program", "-v");
    }

    SECTION("Two set") {
        do_tests_for({true, true, false, false}, "program", "-s", "-t");
        do_tests_for({true, false, true, false}, "program", "-s", "-u");
        do_tests_for({true, false, false, true}, "program", "-s", "-v");

        do_tests_for({true, true, false, false}, "program", "-t", "-s");
        do_tests_for({false, true, true, false}, "program", "-t", "-u");
        do_tests_for({false, true, false, true}, "program", "-t", "-v");

        do_tests_for({true, false, true, false}, "program", "-u", "-s");
        do_tests_for({false, true, true, false}, "program", "-u", "-t");
        do_tests_for({false, false, true, true}, "program", "-u", "-v");

        do_tests_for({true, false, false, true}, "program", "-v", "-s");
        do_tests_for({false, true, false, true}, "program", "-v", "-t");
        do_tests_for({false, false, true, true}, "program", "-v", "-u");
    }

    SECTION("Three set") {
        do_tests_for({true, true, true, false}, "program", "-s", "-t", "-u");
        do_tests_for({true, true, false, true}, "program", "-s", "-t", "-v");
        do_tests_for({true, false, true, true}, "program", "-s", "-u", "-v");
        do_tests_for({false, true, true, true}, "program", "-t", "-u", "-v");
    }

    SECTION("Four set") {
        do_tests_for({true, true, true, true}, "program", "-s", "-t", "-u", "-v");
    }
}
