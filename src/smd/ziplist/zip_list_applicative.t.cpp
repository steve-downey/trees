#include <smd/ziplist/zip_list_applicative.hpp>
#include <smd/ziplist/zip_list_applicative.hpp> // Re-inclusion check

#include <catch2/catch_test_macros.hpp>
#include <smd/typeclass/applicative.hpp>
#include <smd/typeclass/test/test_support.hpp>
#include <smd/ziplist/zip_list.hpp>

#include <functional>
#include <vector>

TEST_CASE("ZipListApplicativeTest - PureBreathing") {
    const auto &applicative = smd::applicative_typeclass<smd::zip_list<int>>;
    auto lifted = applicative.pure(9);

    CHECK(lifted.is_repeating());
    REQUIRE(lifted.repeated.has_value());
    CHECK(*lifted.repeated == 9);
}

TEST_CASE("ZipListApplicativeTest - ApplyZips") {
    smd::zip_list<int (*)(int)> functions{{
        +[](int x) { return x + 1; },
        +[](int x) { return x * 2; },
        +[](int x) { return x - 3; },
    }};
    smd::zip_list<int> arguments{{10, 10}};
    const auto &applicative =
        smd::applicative_typeclass<smd::zip_list<int (*)(int)>>;

    auto result = applicative.apply(functions, arguments);
    CHECK(result.data == (std::vector<int>{11, 20}));
}

TEST_CASE("ZipListApplicativeTest - PureBroadcastsAcrossFiniteInput") {
    const auto &applicative = smd::applicative_typeclass<smd::zip_list<int>>;
    smd::zip_list<int> xs{{1, 2, 3}};

    auto result =
        applicative.ap(applicative.pure(+[](int x) { return x + 10; }), xs);

    CHECK(result.data == (std::vector<int>{11, 12, 13}));
}

TEST_CASE("ZipListApplicativeTest - IdentityLawOnFiniteInput") {
    const auto &applicative = smd::applicative_typeclass<smd::zip_list<int>>;
    smd::zip_list<int> xs{{4, 5, 6}};

    auto result =
        applicative.ap(applicative.pure(+[](int x) { return x; }), xs);

    CHECK(result.data == xs.data);
}

TEST_CASE("ZipListApplicativeTest - BothPureProducesRepeatingResult") {
    const auto &applicative = smd::applicative_typeclass<smd::zip_list<int>>;

    auto result = applicative.ap(applicative.pure(+[](int x) { return x * 2; }),
                                 applicative.pure(7));

    CHECK(result.is_repeating());
    REQUIRE(result.repeated.has_value());
    CHECK(*result.repeated == 14);
}

TEST_CASE("ZipListApplicativeTest - InvokeZipsMultipleArguments") {
    smd::zip_list<int> xs{{1, 2, 3}};
    smd::zip_list<int> ys{{10, 20}};
    smd::zip_list<int> zs{{100, 200, 300, 400}};
    const auto &applicative = smd::applicative_typeclass<smd::zip_list<int>>;

    auto result = applicative.invoke(
        [](int x, int y, int z) { return x + y + z; }, xs, ys, zs);

    CHECK(result.data == (std::vector<int>{111, 222}));
}

TEST_CASE("ZipListApplicativeTest - InvokeWithPureAndFiniteArguments") {
    const auto &applicative = smd::applicative_typeclass<smd::zip_list<int>>;
    smd::zip_list<int> ys{{10, 20, 30}};

    auto result =
        applicative.invoke([](int x, int y, int z) { return x + y + z; },
                           applicative.pure(1), ys, applicative.pure(100));

    CHECK(result.data == (std::vector<int>{111, 121, 131}));
}

TEST_CASE("ZipListApplicativeTest - InterchangeLaw") {
    const auto &applicative = smd::applicative_typeclass<smd::zip_list<int>>;

    smd::zip_list<std::function<int(int)>> functions{{
        [](int x) { return x + 1; },
        [](int x) { return x * 3; },
        [](int x) { return x - 2; },
    }};
    const int value = 7;

    auto lhs = applicative.ap(functions, applicative.pure(value));
    auto rhs = applicative.ap(
        applicative.pure([](const std::function<int(int)> &function) {
            return function(value);
        }),
        functions);

    CHECK(lhs.data == rhs.data);
}

TEST_CASE("ZipListApplicativeTest - CompositionLaw") {
    const auto &applicative = smd::applicative_typeclass<smd::zip_list<int>>;

    smd::zip_list<std::function<int(int)>> u{{
        [](int x) { return x + 10; },
        [](int x) { return x * 2; },
    }};
    smd::zip_list<std::function<int(int)>> v{{
        [](int x) { return x - 3; },
        [](int x) { return x + 4; },
    }};
    smd::zip_list<int> w{{5, 6, 7}};

    auto compose = [](const std::function<int(int)> &f) {
        return [f](const std::function<int(int)> &g) {
            return [f, g](int x) { return f(g(x)); };
        };
    };

    auto lhs = applicative.ap(
        applicative.ap(applicative.ap(applicative.pure(compose), u), v), w);

    auto rhs = applicative.ap(u, applicative.ap(v, w));

    CHECK(lhs.data == rhs.data);
}

TEST_CASE("ZipListApplicativeTest - IdentityHomomorphismAndInvokeViaHarness") {
    CHECK(smd::typeclass::test::check_applicative_identity_law(
        smd::zip_list<int>{{4, 5, 6}}));
    CHECK(smd::typeclass::test::check_applicative_homomorphism_law<
          smd::zip_list<int>>(+[](int x) { return x + 9; }, 3));
    CHECK(smd::typeclass::test::check_applicative_invoke_binary_law(
        [](int a, int b) { return a * 10 + b; }, smd::zip_list<int>{{1, 2, 3}},
        smd::zip_list<int>{{7, 8, 9}}));
}
