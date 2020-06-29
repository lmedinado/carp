#include <carp.h>
#include <catch.hpp>

using namespace std::literals::string_view_literals;

using unsigned_long = unsigned long;
using long_long = long long;
using unsigned_long_long = unsigned long long;
using unsigned_short = unsigned short;
using signed_char = signed char;
using unsigned_char = unsigned char;
using long_double = long double;

TEST_CASE("Basic positional functionality", "[basic_positional]") {

    constexpr auto parser = carp::parser({
        {"a", "'a', a positional argument"},
        {"b", "'b', a positional argument"},
        {"c", "'c', a positional argument"},
        {"d", "'d', a positional argument"},
    });

    char const *const argv[] = {"program", "0", "1", "2", "3"};
    int argc = std::size(argv);

    auto args = parser.parse(argc, argv);
    REQUIRE(args.ok);

    SECTION("const chars") {
        auto a = args["a"] | "";
        auto b = args["b"] | "";
        auto c = args["c"] | "";
        auto d = args["d"] | "";

        REQUIRE((a && *a == "0"sv && std::is_same_v<char const *&, decltype(*a)>));
        REQUIRE((b && *b == "1"sv && std::is_same_v<char const *&, decltype(*b)>));
        REQUIRE((c && *c == "2"sv && std::is_same_v<char const *&, decltype(*c)>));
        REQUIRE((d && *d == "3"sv && std::is_same_v<char const *&, decltype(*d)>));
    }

    SECTION("string_views") {
        auto a = args["a"] | ""sv;
        auto b = args["b"] | ""sv;
        auto c = args["c"] | ""sv;
        auto d = args["d"] | ""sv;

        REQUIRE((a && *a == "0" && std::is_same_v<std::string_view &, decltype(*a)>));
        REQUIRE((b && *b == "1" && std::is_same_v<std::string_view &, decltype(*b)>));
        REQUIRE((c && *c == "2" && std::is_same_v<std::string_view &, decltype(*c)>));
        REQUIRE((d && *d == "3" && std::is_same_v<std::string_view &, decltype(*d)>));
    }

    auto do_tests_for = [&](auto t) {
        using T = decltype(t);

        auto a = args["a"] | T{0};
        auto b = args["b"] | T{1};
        auto c = args["c"] | T{2};
        auto d = args["d"] | T{3};

        REQUIRE((a && *a == T{0} && std::is_same_v<T &, decltype(*a)>));
        REQUIRE((b && *b == T{1} && std::is_same_v<T &, decltype(*b)>));
        REQUIRE((c && *c == T{2} && std::is_same_v<T &, decltype(*c)>));
        REQUIRE((d && *d == T{3} && std::is_same_v<T &, decltype(*d)>));
    };

    SECTION("ints") { do_tests_for(int{}); }
    SECTION("uints") { do_tests_for(unsigned{}); }
    SECTION("longs") { do_tests_for(long{}); }
    SECTION("ulongs") { do_tests_for(unsigned_long{}); }
    SECTION("llongs") { do_tests_for(long_long{}); }
    SECTION("ullongs") { do_tests_for(unsigned_long_long{}); }
    SECTION("shorts") { do_tests_for(short{}); }
    SECTION("ushorts") { do_tests_for(unsigned_short{}); }
    SECTION("chars") { do_tests_for(char{}); }
    SECTION("schars") { do_tests_for(signed_char{}); }
    SECTION("uchars") { do_tests_for(unsigned_char{}); }
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
        char const *const argv[] = {"program", argv_pack...};
        int argc = size(argv);
        auto args = parser.parse(argc, argv);

        REQUIRE(args.ok);

        REQUIRE(args["-s"] == f[0]);
        REQUIRE(args["-t"] == f[1]);
        REQUIRE(args["-u"] == f[2]);
        REQUIRE(args["-v"] == f[3]);
    };

    SECTION("None set") { do_tests_for({false, false, false, false}); }

    SECTION("One set") {
        do_tests_for({true, false, false, false}, "-s");
        do_tests_for({false, true, false, false}, "-t");
        do_tests_for({false, false, true, false}, "-u");
        do_tests_for({false, false, false, true}, "-v");
    }

    SECTION("Two set") {
        do_tests_for({true, true, false, false}, "-s", "-t");
        do_tests_for({true, false, true, false}, "-s", "-u");
        do_tests_for({true, false, false, true}, "-s", "-v");

        do_tests_for({true, true, false, false}, "-t", "-s");
        do_tests_for({false, true, true, false}, "-t", "-u");
        do_tests_for({false, true, false, true}, "-t", "-v");

        do_tests_for({true, false, true, false}, "-u", "-s");
        do_tests_for({false, true, true, false}, "-u", "-t");
        do_tests_for({false, false, true, true}, "-u", "-v");

        do_tests_for({true, false, false, true}, "-v", "-s");
        do_tests_for({false, true, false, true}, "-v", "-t");
        do_tests_for({false, false, true, true}, "-v", "-u");
    }

    SECTION("Three set") {
        do_tests_for({true, true, true, false}, "-s", "-t", "-u");
        do_tests_for({true, true, false, true}, "-s", "-t", "-v");
        do_tests_for({true, false, true, true}, "-s", "-u", "-v");
        do_tests_for({false, true, true, true}, "-t", "-u", "-v");
    }

    SECTION("Four set") {
        do_tests_for({true, true, true, true}, "-s", "-t", "-u", "-v");
    }
}

TEST_CASE("All together", "[general]") {
    using std::size;

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

    SECTION("must succeed") {

        auto test_permutation = [&](auto... argv_pack) {
            char const *const argv[] = {"program", argv_pack...};
            int argc = size(argv);
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

            REQUIRE(args.ok);
            REQUIRE((a && *a == 10));
            REQUIRE((b && *b == "zaga"sv));
            REQUIRE((c && *c == 6));
            REQUIRE((d && *d == 3.14159));
            REQUIRE((e && *e == 4.3f));
            REQUIRE((s && *(s | "none") == "-s"sv));
            REQUIRE((t && *t == "cartwheel"sv));
            REQUIRE((u && *u == std::array{1, 2}));
            REQUIRE((v && *v == std::array{"asd", "asd"}));
        };

        test_permutation("10",              //
                         "zaga",            //
                         "6",               //
                         "3.14159",         //
                         "4.3",             //
                         "-s",              //
                         "-t", "cartwheel", //
                         "-u", "1", "2",    //
                         "-v", "asd", "asd" //
        );

        test_permutation("-s",               //
                         "-t", "cartwheel",  //
                         "-u", "1", "2",     //
                         "-v", "asd", "asd", //
                         "10",               //
                         "zaga",             //
                         "6",                //
                         "3.14159",          //
                         "4.3"               //
        );

        test_permutation("10",              //
                         "zaga",            //
                         "6",               //
                         "3.14159",         //
                         "4.3",             //
                         "-s",              //
                         "-t", "cartwheel", //
                         "-u", "1", "2",    //
                         "-v", "asd", "asd" //
        );

        test_permutation("10",              //
                         "zaga",            //
                         "6",               //
                         "3.14159",         //
                         "4.3",             //
                         "-s",              //
                         "-t", "cartwheel", //
                         "-u", "1", "2",    //
                         "-v", "asd", "asd" //
        );

        test_permutation("10",               //
                         "-t", "cartwheel",  //
                         "-s",               //
                         "zaga",             //
                         "6",                //
                         "-v", "asd", "asd", //
                         "3.14159",          //
                         "4.3",              //
                         "-u", "1", "2"      //
        );
    }

    SECTION("must fail") {

        auto test_permutation = [&](auto... argv_pack) {
            char const *const argv[] = {"program", argv_pack...};
            int argc = size(argv);
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

            REQUIRE(!args.ok);
            REQUIRE((a && *a == 10));
            REQUIRE((b && *b == "zaga"sv));
            REQUIRE((c && *c == 6));
            REQUIRE((d && *d == 3.14159));
            REQUIRE((e && *e == 4.3f));
            REQUIRE((s && *(s | "none") == "-s"sv));
            REQUIRE((t && *t == "cartwheel"sv));
            REQUIRE((u && *u == std::array{1, 2}));
            REQUIRE((v && *v == std::array{"asd", "asd"}));
        };

        /* one extra arg */
        test_permutation("10",              //
                         "zaga",            //
                         "6",               //
                         "3.14159",         //
                         "4.3",             //
                         "extra",           //
                         "-s",              //
                         "-t", "cartwheel", //
                         "-u", "1", "2",    //
                         "-v", "asd", "asd" //
        );

        test_permutation("-s",               //
                         "-t", "cartwheel",  //
                         "-u", "1", "2",     //
                         "-v", "asd", "asd", //
                         "10",               //
                         "zaga",             //
                         "6",                //
                         "3.14159",          //
                         "4.3",              //
                         "extra"             //
        );

        test_permutation("10",               //
                         "zaga",             //
                         "6",                //
                         "3.14159",          //
                         "4.3",              //
                         "-s",               //
                         "-t", "cartwheel",  //
                         "-u", "1", "2",     //
                         "-v", "asd", "asd", //
                         "extra"             //
        );

        test_permutation("10",              //
                         "zaga",            //
                         "6",               //
                         "3.14159",         //
                         "4.3",             //
                         "-s",              //
                         "extra",           //
                         "-t", "cartwheel", //
                         "-u", "1", "2",    //
                         "-v", "asd", "asd" //
        );

        test_permutation("10",               //
                         "-t", "cartwheel",  //
                         "-s",               //
                         "zaga",             //
                         "6",                //
                         "-v", "asd", "asd", //
                         "3.14159",          //
                         "4.3",              //
                         "-u", "1", "2",     //
                         "extra"             //
        );

        /* two extra args */
        test_permutation("10",              //
                         "zaga",            //
                         "6",               //
                         "3.14159",         //
                         "4.3",             //
                         "extra",           //
                         "extra",           //
                         "-s",              //
                         "-t", "cartwheel", //
                         "-u", "1", "2",    //
                         "-v", "asd", "asd" //
        );

        test_permutation("-s",               //
                         "-t", "cartwheel",  //
                         "-u", "1", "2",     //
                         "-v", "asd", "asd", //
                         "10",               //
                         "zaga",             //
                         "6",                //
                         "3.14159",          //
                         "4.3",              //
                         "extra",            //
                         "extra"             //
        );

        test_permutation("10",               //
                         "zaga",             //
                         "6",                //
                         "3.14159",          //
                         "4.3",              //
                         "-s",               //
                         "-t", "cartwheel",  //
                         "-u", "1", "2",     //
                         "-v", "asd", "asd", //
                         "extra",            //
                         "extra"             //
        );

        test_permutation("10",              //
                         "zaga",            //
                         "6",               //
                         "3.14159",         //
                         "4.3",             //
                         "extra",           //
                         "-s",              //
                         "extra",           //
                         "-t", "cartwheel", //
                         "-u", "1", "2",    //
                         "-v", "asd", "asd" //
        );

        test_permutation("10",               //
                         "-t", "cartwheel",  //
                         "-s",               //
                         "zaga",             //
                         "6",                //
                         "-v", "asd", "asd", //
                         "3.14159",          //
                         "4.3",              //
                         "extra",            //
                         "-u", "1", "2",     //
                         "extra"             //
        );

        /* three extra args */
        test_permutation("10",              //
                         "zaga",            //
                         "6",               //
                         "3.14159",         //
                         "4.3",             //
                         "extra",           //
                         "extra",           //
                         "extra",           //
                         "-s",              //
                         "-t", "cartwheel", //
                         "-u", "1", "2",    //
                         "-v", "asd", "asd" //
        );

        test_permutation("-s",               //
                         "-t", "cartwheel",  //
                         "-u", "1", "2",     //
                         "-v", "asd", "asd", //
                         "10",               //
                         "zaga",             //
                         "6",                //
                         "3.14159",          //
                         "4.3",              //
                         "extra",            //
                         "extra",            //
                         "extra"             //
        );

        test_permutation("10",               //
                         "zaga",             //
                         "6",                //
                         "3.14159",          //
                         "4.3",              //
                         "-s",               //
                         "-t", "cartwheel",  //
                         "-u", "1", "2",     //
                         "-v", "asd", "asd", //
                         "extra",            //
                         "extra",            //
                         "extra"             //
        );

        test_permutation("10",              //
                         "zaga",            //
                         "6",               //
                         "3.14159",         //
                         "4.3",             //
                         "extra",           //
                         "extra",           //
                         "-s",              //
                         "extra",           //
                         "-t", "cartwheel", //
                         "-u", "1", "2",    //
                         "-v", "asd", "asd" //
        );

        test_permutation("10",               //
                         "-t", "cartwheel",  //
                         "-s",               //
                         "zaga",             //
                         "6",                //
                         "-v", "asd", "asd", //
                         "3.14159",          //
                         "4.3",              //
                         "extra",            //
                         "-u", "1", "2",     //
                         "extra",            //
                         "extra"             //
        );

        /* one unknown switch */
        test_permutation("10",              //
                         "zaga",            //
                         "6",               //
                         "3.14159",         //
                         "4.3",             //
                         "-extra",          //
                         "-s",              //
                         "-t", "cartwheel", //
                         "-u", "1", "2",    //
                         "-v", "asd", "asd" //
        );

        test_permutation("-s",               //
                         "-t", "cartwheel",  //
                         "-u", "1", "2",     //
                         "-v", "asd", "asd", //
                         "10",               //
                         "zaga",             //
                         "6",                //
                         "3.14159",          //
                         "4.3",              //
                         "-extra"            //
        );

        test_permutation("10",               //
                         "zaga",             //
                         "6",                //
                         "3.14159",          //
                         "4.3",              //
                         "-s",               //
                         "-t", "cartwheel",  //
                         "-u", "1", "2",     //
                         "-v", "asd", "asd", //
                         "-extra"            //
        );

        test_permutation("10",              //
                         "zaga",            //
                         "6",               //
                         "3.14159",         //
                         "4.3",             //
                         "-s",              //
                         "-extra",          //
                         "-t", "cartwheel", //
                         "-u", "1", "2",    //
                         "-v", "asd", "asd" //
        );

        test_permutation("10",               //
                         "-t", "cartwheel",  //
                         "-s",               //
                         "zaga",             //
                         "6",                //
                         "-v", "asd", "asd", //
                         "3.14159",          //
                         "4.3",              //
                         "-u", "1", "2",     //
                         "-extra"            //
        );

        /* one extra arg, one unknown switch */
        test_permutation("10",              //
                         "zaga",            //
                         "6",               //
                         "3.14159",         //
                         "4.3",             //
                         "-extra",          //
                         "extra",           //
                         "-s",              //
                         "-t", "cartwheel", //
                         "-u", "1", "2",    //
                         "-v", "asd", "asd" //
        );

        test_permutation("-s",               //
                         "-t", "cartwheel",  //
                         "-u", "1", "2",     //
                         "-v", "asd", "asd", //
                         "10",               //
                         "zaga",             //
                         "6",                //
                         "3.14159",          //
                         "4.3",              //
                         "extra",            //
                         "-extra"            //
        );

        test_permutation("10",               //
                         "zaga",             //
                         "6",                //
                         "3.14159",          //
                         "4.3",              //
                         "-s",               //
                         "-t", "cartwheel",  //
                         "-u", "1", "2",     //
                         "-v", "asd", "asd", //
                         "-extra",           //
                         "extra"             //
        );

        test_permutation("10",              //
                         "zaga",            //
                         "6",               //
                         "3.14159",         //
                         "4.3",             //
                         "-extra",          //
                         "-s",              //
                         "extra",           //
                         "-t", "cartwheel", //
                         "-u", "1", "2",    //
                         "-v", "asd", "asd" //
        );

        test_permutation("10",               //
                         "-t", "cartwheel",  //
                         "-s",               //
                         "zaga",             //
                         "6",                //
                         "-v", "asd", "asd", //
                         "3.14159",          //
                         "4.3",              //
                         "extra",            //
                         "-u", "1", "2",     //
                         "-extra"            //
        );
    }
}

/* unwrapper tests */
TEST_CASE("Unwrapping numbers", "[unwrap_nums]") {
    using std::pair;
    using std::size;

    SECTION("Must succeed") {
        auto check = [&](auto t, auto... argv_pack) {
            using T = decltype(t);
            char const *const argv[] = {argv_pack...};
            int argc = size(argv);

            const auto result = carp::unwrapper<T>::get(argc, argv);
            REQUIRE((result && *result == t && std::is_same_v<T const &, decltype(*result)>));
        };

        SECTION("Small signed integers") {

            auto do_tests_for = [&](auto t) {
                using T = decltype(t);
                for (auto [xs, xv] : {
                         pair("-100", T{-100}),
                         pair("-2", T{-2}),
                         pair("-1", T{-1}),
                         pair("0", T{0}),
                         pair("1", T{1}),
                         pair("2", T{2}),
                         pair("100", T{100}),
                     }) {
                    check(xv, xs);
                }
            };

            SECTION("ints") { do_tests_for(int{}); }
            SECTION("longs") { do_tests_for(long{}); }
            SECTION("llongs") { do_tests_for(long_long{}); }
            SECTION("shorts") { do_tests_for(short{}); }
            SECTION("schars") { do_tests_for(signed_char{}); }
            SECTION("floats") { do_tests_for(float{}); }
            SECTION("doubles") { do_tests_for(double{}); }
        }

        SECTION("Small unsigned integers") {

            auto do_tests_for = [&](auto t) {
                using T = decltype(t);
                for (auto [xs, xv] : {
                         pair("0", T{0}),
                         pair("1", T{1}),
                         pair("2", T{2}),
                         pair("100", T{100}),
                         pair("200", T{200}),
                     }) {
                    check(xv, xs);
                }
            };

            SECTION("uints") { do_tests_for(unsigned{}); }
            SECTION("ulongs") { do_tests_for(unsigned_long{}); }
            SECTION("ullongs") { do_tests_for(unsigned_long_long{}); }
            SECTION("ushorts") { do_tests_for(unsigned_short{}); }
            SECTION("uchars") { do_tests_for(unsigned_char{}); }
        }

        SECTION("Small floats") {

            auto do_tests_for = [&](auto t) {
                using T = decltype(t);
                for (auto [xs, xv] : {
                         pair("0.", T{0.}),
                         pair("0.0", T{0.}),
                         pair(".1", T{0.1}),
                         pair("1.2", T{1.2}),
                         pair("2.4", T{2.4}),
                         pair("100.0", T{100}),
                         pair("2000000.0", T{2000000}),
                     }) {
                    check(xv, xs);
                }
            };

            SECTION("floats") { do_tests_for(float{}); }
            SECTION("doubles") { do_tests_for(double{}); }
        }
    }

    SECTION("Must fail") {
        auto check = [&](auto t, auto... argv_pack) {
            using T = decltype(t);
            char const *const argv[] = {argv_pack...};
            int argc = size(argv);

            const auto result = carp::unwrapper<T>::get(argc, argv);
            REQUIRE((!result && std::is_same_v<T const &, decltype(*result)>));
        };

        SECTION("Signed integers") {

            auto do_tests_for = [&](auto t) {
                using T = decltype(t);
                for (auto x : {
                         "-100u",
                         "-2u",
                         "-u1",
                         "u0",
                         "1u",
                         "u2",
                         "1u00",
                         "as",
                         "100.",
                         ".1",
                         "0xff1",
                         "abc",
                         "u2",
                         "200000000000000000000000000000000000000000000",
                     }) {
                    check(T{}, x);
                }
            };

            SECTION("ints") { do_tests_for(int{}); }
            SECTION("longs") { do_tests_for(long{}); }
            SECTION("llongs") { do_tests_for(long_long{}); }
            SECTION("shorts") { do_tests_for(short{}); }
            SECTION("schars") { do_tests_for(signed_char{}); }
        }

        SECTION("Unsigned integers") {

            auto do_tests_for = [&](auto t) {
                using T = decltype(t);
                for (auto x : {
                         "-100",
                         "-2",
                         "-1",
                         "-100u",
                         "-2u",
                         "-u1",
                         "u0",
                         "1u",
                         "u2",
                         "1u00",
                         "as",
                         "100.",
                         ".1",
                         "0xff1",
                         "abc",
                         "u2",
                         "200000000000000000000000000000000000000000000",
                     }) {
                    check(T{}, x);
                }
            };

            SECTION("uints") { do_tests_for(unsigned{}); }
            SECTION("ulongs") { do_tests_for(unsigned_long{}); }
            SECTION("ullongs") { do_tests_for(unsigned_long_long{}); }
            SECTION("ushorts") { do_tests_for(unsigned_short{}); }
            SECTION("uchars") { do_tests_for(unsigned_char{}); }
        }

        SECTION("floats") {

            auto do_tests_for = [&](auto t) {
                using T = decltype(t);
                for (auto x : {
                         "-100u", "-2u",  "-u1",  "u0",     "1u",
                         "u2",    "1u00", "as",   "0x100.", ".1x",
                         "0xff1", "abc",  "u2",   "0.c",    "0.f0",
                         "f.1",   "1g.2", "a2.4", "100.0a", "c2000000.0",
                     }) {
                    check(T{}, x);
                }
            };

            SECTION("floats") { do_tests_for(float{}); }
            SECTION("doubles") { do_tests_for(double{}); }

            SECTION("Large floats") {
                check(float{}, "200000000000000000000000000000000000000000000");
                check(float{},
                      "999999999999999999999999999999999999999999999999999999999999999");
                check(float{},
                      ("1" + std::to_string(std::numeric_limits<float>::max())).c_str());
            }
            SECTION("Large doubles") {
                check(double{},
                      "999999999999999999999999999999999999999999999999999999999999999"
                      "999999999999999999999999999999999999999999999999999999999999999"
                      "999999999999999999999999999999999999999999999999999999999999999"
                      "999999999999999999999999999999999999999999999999999999999999999"
                      "999999999999999999999999999999999999999999999999999999999999999"
                      "999999999999999999999999999999999999999999999999999999999999999"
                      "999999999999999999999999999999999999999999999999999999999999999"
                      "999999999999999999999999999999999999999999999999999999999999999"
                      "999999999999999999999999999999999999999999999999999999999999999"
                      "999999999999999999999999999999999999999999999999999999999999999"
                      "999999999999999999999999999999999999999999999999999999999999999");
                check(double{},
                      ("1" + std::to_string(std::numeric_limits<double>::max())).c_str());
            }
        }
    }
}


template <typename T, size_t... I>
constexpr auto make_tuple_from_array_impl(T array, std::index_sequence<I...>) {
    return std::make_tuple(std::get<I>(array)...);
}

template <typename T>
constexpr auto make_tuple_from_array(T array) {
    return make_tuple_from_array_impl(array, std::make_index_sequence<std::tuple_size_v<T>>{});
}

/* unwrapper tests */
TEST_CASE("Unwrapping arrays", "[unwrap_arrays]") {
    using std::pair;
    using std::size;

    SECTION("Must succeed") {
        auto check = [&](auto t, auto... argv_pack) {
            using T = decltype(t);
            char const *const argv[] = {argv_pack...};
            int argc = size(argv);

            const auto result = carp::unwrapper<T>::get(argc, argv);
            REQUIRE((result && *result == t && std::is_same_v<T const &, decltype(*result)>));

            /* might as well test tuple while we're at it */
            auto tup_t = make_tuple_from_array(t);
            using tup_T = decltype(tup_t);
            const auto t_result = carp::unwrapper<tup_T>::get(argc, argv);
            REQUIRE((t_result && *t_result == tup_t &&
                     std::is_same_v<tup_T const &, decltype(*t_result)>));
        };

        SECTION("integers") {

            auto do_tests_for = [&](auto t) {
                using T = decltype(t);

                check(std::array{T{1}}, "1");
                check(std::array{T{1}, T{2}}, "1", "2");
                check(std::array{T{1}, T{2}, T{3}}, "1", "2", "3");
                check(std::array{T{1}, T{2}, T{3}, T{4}}, "1", "2", "3", "4");
            };

            SECTION("ints") { do_tests_for(int{}); }
            SECTION("uints") { do_tests_for(unsigned{}); }
            SECTION("longs") { do_tests_for(long{}); }
            SECTION("ulongs") { do_tests_for(unsigned_long{}); }
            SECTION("llongs") { do_tests_for(long_long{}); }
            SECTION("ullongs") { do_tests_for(unsigned_long_long{}); }
            SECTION("shorts") { do_tests_for(short{}); }
            SECTION("ushorts") { do_tests_for(unsigned_short{}); }
            SECTION("chars") { do_tests_for(char{}); }
            SECTION("schars") { do_tests_for(signed_char{}); }
            SECTION("uchars") { do_tests_for(unsigned_char{}); }
            SECTION("floats") { do_tests_for(float{}); }
            SECTION("doubles") { do_tests_for(double{}); }
        }

        SECTION("floats") {

            auto do_tests_for = [&](auto t) {
                using T = decltype(t);

                check(std::array{T{1.1}}, "1.1");
                check(std::array{T{1.1}, T{1.2}}, "1.1", "1.2");
                check(std::array{T{1.1}, T{1.2}, T{1.3}}, "1.1", "1.2", "1.3");
                check(std::array{T{1.1}, T{1.2}, T{1.3}, T{1.4}}, "1.1", "1.2", "1.3", "1.4");
            };

            SECTION("floats") { do_tests_for(float{}); }
            SECTION("doubles") { do_tests_for(double{}); }
        }

        SECTION("strings") {

            check(std::array{"e1"}, "e1");
            check(std::array{"e1", "e2"}, "e1", "e2");
            check(std::array{"e1", "e2", "e3"}, "e1", "e2", "e3");
            check(std::array{"e1", "e2", "e3", "e4"}, "e1", "e2", "e3", "e4");
        }
    }

    SECTION("Must fail") {
        auto check = [&](auto t, auto... argv_pack) {
            using T = decltype(t);
            char const *const argv[] = {argv_pack...};
            int argc = size(argv);

            const auto result = carp::unwrapper<T>::get(argc, argv);
            REQUIRE((!result && std::is_same_v<T const &, decltype(*result)>));

            /* might as well test tuple while we're at it */
            auto tup_t = make_tuple_from_array(t);
            using tup_T = decltype(tup_t);
            const auto t_result = carp::unwrapper<tup_T>::get(argc, argv);
            REQUIRE((!t_result && std::is_same_v<tup_T const &, decltype(*t_result)>));
        };

        SECTION("integers") {

            auto do_tests_for = [&](auto t) {
                using T = decltype(t);

                check(std::array{T{}}, "1x");
                check(std::array{T{}}, "0x1");
                check(std::array{T{}, T{}}, "1", "0x2");
                check(std::array{T{}, T{}}, "1", "2x");
                check(std::array{T{}, T{}}, "0x1", "2");
                check(std::array{T{}, T{}}, "1u", "2");
                check(std::array{T{}, T{}, T{}}, "0x1", "2", "3");
                check(std::array{T{}, T{}, T{}}, "1", "0x2", "3");
                check(std::array{T{}, T{}, T{}}, "1", "2x", "3");
                check(std::array{T{}, T{}, T{}}, "1", "2", "0x3");
                check(std::array{T{}, T{}, T{}}, "1", "2", "3x");
                check(std::array{T{}, T{}, T{}}, "1", "2",
                      "300000000000000000000000000000000000000000000000000");
                check(std::array{T{}, T{}, T{}, T{}}, "1x", "2", "3", "4");
                check(std::array{T{}, T{}, T{}, T{}}, "0x1", "2", "3", "4");
                check(std::array{T{}, T{}, T{}, T{}}, "1", "2x", "3", "4");
                check(std::array{T{}, T{}, T{}, T{}}, "1", "0x2", "3", "4");
                check(std::array{T{}, T{}, T{}, T{}}, "1", "2", "3x", "4");
                check(std::array{T{}, T{}, T{}, T{}}, "1", "2", "0x3", "4");
                check(std::array{T{}, T{}, T{}, T{}}, "1", "2", "3", "4x");
                check(std::array{T{}, T{}, T{}, T{}}, "1", "2", "3", "0x4");
            };

            SECTION("ints") { do_tests_for(int{}); }
            SECTION("uints") { do_tests_for(unsigned{}); }
            SECTION("longs") { do_tests_for(long{}); }
            SECTION("ulongs") { do_tests_for(unsigned_long{}); }
            SECTION("llongs") { do_tests_for(long_long{}); }
            SECTION("ullongs") { do_tests_for(unsigned_long_long{}); }
            SECTION("shorts") { do_tests_for(short{}); }
            SECTION("ushorts") { do_tests_for(unsigned_short{}); }
            SECTION("chars") { do_tests_for(char{}); }
            SECTION("schars") { do_tests_for(signed_char{}); }
            SECTION("uchars") { do_tests_for(unsigned_char{}); }
        }

        SECTION("floats") {

            auto do_tests_for = [&](auto t) {
                using T = decltype(t);

                check(std::array{T{}}, "1x");
                check(std::array{T{}}, "0x1");
                check(std::array{T{}, T{}}, "1", "0x2");
                check(std::array{T{}, T{}}, "1", "2x");
                check(std::array{T{}, T{}}, "0x1", "2");
                check(std::array{T{}, T{}}, "1u", "2");
                check(std::array{T{}, T{}, T{}}, "0x1", "2", "3");
                check(std::array{T{}, T{}, T{}}, "1", "0x2", "3");
                check(std::array{T{}, T{}, T{}}, "1", "2x", "3");
                check(std::array{T{}, T{}, T{}}, "1", "2", "0x3");
                check(std::array{T{}, T{}, T{}}, "1", "2", "3x");
                check(std::array{T{}, T{}, T{}, T{}}, "1x", "2", "3", "4");
                check(std::array{T{}, T{}, T{}, T{}}, "0x1", "2", "3", "4");
                check(std::array{T{}, T{}, T{}, T{}}, "1", "2x", "3", "4");
                check(std::array{T{}, T{}, T{}, T{}}, "1", "0x2", "3", "4");
                check(std::array{T{}, T{}, T{}, T{}}, "1", "2", "3x", "4");
                check(std::array{T{}, T{}, T{}, T{}}, "1", "2", "0x3", "4");
                check(std::array{T{}, T{}, T{}, T{}}, "1", "2", "3", "4x");
                check(std::array{T{}, T{}, T{}, T{}}, "1", "2", "3", "0x4");
            };

            SECTION("floats") { do_tests_for(float{}); }
            SECTION("doubles") { do_tests_for(double{}); }
            SECTION("Large floats") {
                check(std::array{float{}},
                      "200000000000000000000000000000000000000000000");
                check(std::array{float{}},
                      "999999999999999999999999999999999999999999999999999999999999999");
                check(std::array{float{}},
                      ("1" + std::to_string(std::numeric_limits<float>::max())).c_str());

                check(std::array{float{}, float{}}, "1",
                      "200000000000000000000000000000000000000000000");
                check(std::array{float{}, float{}},
                      "999999999999999999999999999999999999999999999999999999999999999", "1");
                check(std::array{float{}},
                      ("1" + std::to_string(std::numeric_limits<float>::max())).c_str());
            }
            SECTION("Large doubles") {
                check(std::array{double{}, double{}}, "1",
                      "999999999999999999999999999999999999999999999999999999999999999"
                      "999999999999999999999999999999999999999999999999999999999999999"
                      "999999999999999999999999999999999999999999999999999999999999999"
                      "999999999999999999999999999999999999999999999999999999999999999"
                      "999999999999999999999999999999999999999999999999999999999999999"
                      "999999999999999999999999999999999999999999999999999999999999999"
                      "999999999999999999999999999999999999999999999999999999999999999"
                      "999999999999999999999999999999999999999999999999999999999999999"
                      "999999999999999999999999999999999999999999999999999999999999999"
                      "999999999999999999999999999999999999999999999999999999999999999"
                      "999999999999999999999999999999999999999999999999999999999999999");
                check(std::array{double{}, double{}},
                      ("1" + std::to_string(std::numeric_limits<double>::max())).c_str(), "1");
            }
        }
    }
}
TEST_CASE("Unwrapping tuples", "[unwrap_tuples]") {
    using std::pair;
    using std::size;

    SECTION("Must succeed") {
        auto check = [&](auto t, auto... argv_pack) {
            using T = decltype(t);
            char const *const argv[] = {argv_pack...};
            int argc = size(argv);

            const auto result = carp::unwrapper<T>::get(argc, argv);
            REQUIRE((result && *result == t && std::is_same_v<T const &, decltype(*result)>));
        };

        check(std::tuple{1, 2u, 3ll, 4ull, short{5}}, "1", "2", "3", "4", "5");
        check(std::tuple{1, "abc", 3.0, 4.4f, signed_char{-5}}, "1", "abc", "3.", "4.4", "-5");
    }
}

TEST_CASE("Parsing numbers", "[numeric]") {
    using carp::detail::str_to_num;
    using std::pair;

    auto check = [](auto x, auto t) {
        using T = decltype(t);
        auto result = carp::detail::str_to_num<T>::get(x.data(), x.data() + x.size());
        REQUIRE((result && *result == t && std::is_same_v<T &, decltype(*result)>));
    };

    SECTION("Small signed integers") {

        auto do_tests_for = [&](auto t) {
            using T = decltype(t);
            for (auto [xs, xv] : {
                     pair("-100"sv, T{-100}),
                     pair("-2"sv, T{-2}),
                     pair("-1"sv, T{-1}),
                     pair("0"sv, T{0}),
                     pair("1"sv, T{1}),
                     pair("2"sv, T{2}),
                     pair("100"sv, T{100}),
                 }) {
                check(xs, xv);
            }
        };

        SECTION("ints") { do_tests_for(int{}); }
        SECTION("longs") { do_tests_for(long{}); }
        SECTION("llongs") { do_tests_for(long_long{}); }
        SECTION("shorts") { do_tests_for(short{}); }
        SECTION("schars") { do_tests_for(signed_char{}); }
        SECTION("floats") { do_tests_for(float{}); }
        SECTION("doubles") { do_tests_for(double{}); }
    }

    SECTION("Small unsigned integers") {

        auto do_tests_for = [&](auto t) {
            using T = decltype(t);
            for (auto [xs, xv] : {
                     pair("0"sv, T{0}),
                     pair("1"sv, T{1}),
                     pair("2"sv, T{2}),
                     pair("100"sv, T{100}),
                     pair("200"sv, T{200}),
                 }) {
                check(xs, xv);
            }
        };

        SECTION("uints") { do_tests_for(unsigned{}); }
        SECTION("ulongs") { do_tests_for(unsigned_long{}); }
        SECTION("ullongs") { do_tests_for(unsigned_long_long{}); }
        SECTION("ushorts") { do_tests_for(unsigned_short{}); }
        SECTION("uchars") { do_tests_for(unsigned_char{}); }
    }

    SECTION("Small floats") {

        auto do_tests_for = [&](auto t) {
            using T = decltype(t);
            for (auto [xs, xv] : {
                     pair("0."sv, T{0.}),
                     pair("0.0"sv, T{0.}),
                     pair(".1"sv, T{0.1}),
                     pair("1.2"sv, T{1.2}),
                     pair("2.4"sv, T{2.4}),
                     pair("100.0"sv, T{100}),
                     pair("2000000.0"sv, T{2000000}),
                 }) {
                check(xs, xv);
            }
        };

        SECTION("floats") { do_tests_for(float{}); }
        SECTION("doubles") { do_tests_for(double{}); }
    }

    SECTION("Overflow", "[overflow]") {

        auto overflow_check = [](auto x, auto t, auto big) {
            using T = decltype(t);
            using big_type = decltype(big);
            auto result = str_to_num<T>::get(x.data(), x.data() + x.size());
            REQUIRE((!result ||
                     (std::numeric_limits<T>::max() == std::numeric_limits<big_type>::max() &&
                      std::numeric_limits<T>::min() == std::numeric_limits<big_type>::min())));
            REQUIRE(std::is_same_v<T &, decltype(*result)>);
        };

        SECTION("Signed integer overflow", "[int_overflow]") {
            using big_type = long long;
            auto pbig = std::to_string(std::numeric_limits<big_type>::max());
            auto nbig = std::to_string(std::numeric_limits<big_type>::min());

            for (auto big : {pbig, nbig}) {
                overflow_check(pbig, signed_char{}, big_type{});
                overflow_check(pbig, short{}, big_type{});
                overflow_check(pbig, int{}, big_type{});
                overflow_check(pbig, long{}, big_type{});
                overflow_check(pbig, long_long{}, big_type{});
                overflow_check(pbig + "0", big_type{}, big_type{});
            }
        }

        SECTION("Unsigned integer overflow", "[uint_overflow]") {
            using big_type = unsigned long long;
            auto pbig = std::to_string(std::numeric_limits<big_type>::max());
            auto nbig = std::to_string(std::numeric_limits<big_type>::min());

            for (auto big : {pbig, nbig}) {
                overflow_check(pbig, unsigned_char{}, big_type{});
                overflow_check(pbig, unsigned_short{}, big_type{});
                overflow_check(pbig, unsigned{}, big_type{});
                overflow_check(pbig, unsigned_long{}, big_type{});
                overflow_check(pbig, long_long{}, big_type{});
                overflow_check(pbig + "0", big_type{}, big_type{});
            }
        }
    }
}