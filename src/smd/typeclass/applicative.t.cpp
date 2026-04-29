#include <smd/typeclass/applicative.hpp>

#include <catch2/catch_test_macros.hpp>

#include <optional>

TEST_CASE("ApplicativeTypeclassTest - PureOptional")
{
    const auto& applicative = smd::applicative_typeclass<std::optional<int> >;
    auto lifted = applicative.pure(7);
    REQUIRE(lifted.has_value());
    CHECK(*lifted == 7);
}

TEST_CASE("ApplicativeTypeclassTest - ApplyOptional")
{
    std::optional<int (*)(int)> function{+[](int x) { return x + 3; }};
    std::optional<int> argument{4};
    const auto& applicative = smd::applicative_typeclass<std::optional<int (*)(int)> >;

    auto result = applicative.apply(function, argument);
    REQUIRE(result.has_value());
    CHECK(*result == 7);
}

TEST_CASE("ApplicativeTypeclassTest - InvokeOptional")
{
    std::optional<int> ax{10};
    std::optional<int> ay{5};
    const auto& applicative = smd::applicative_typeclass<std::optional<int> >;

    auto result = applicative.invoke([](int a, int b) { return a - b; }, ax, ay);
    REQUIRE(result.has_value());
    CHECK(*result == 5);
}

TEST_CASE("ApplicativeTypeclassTest - InvokeOptionalTernaryUsesPartialApplication")
{
    std::optional<int> ax{2};
    std::optional<int> ay{3};
    std::optional<int> az{4};
    const auto& applicative = smd::applicative_typeclass<std::optional<int> >;

    auto result = applicative.invoke(
        [](int a, int b, int c) { return a * b + c; },
        ax,
        ay,
        az);
    REQUIRE(result.has_value());
    CHECK(*result == 10);
}

TEST_CASE("ApplicativeTypeclassTest - ApplyPureOptionalTernary")
{
    // 6e8bde7b-a9f1-4c98-8f1a-807d9ee0a93b
    std::optional<int> ax{2};
    std::optional<int> ay{3};
    std::optional<int> az{4};
    const auto& applicative = smd::applicative_typeclass<std::optional<int> >;

    auto result = applicative.apply_pure(
        [](int a, int b, int c) { return a * b + c; },
        ax,
        ay,
        az);
    REQUIRE(result.has_value());
    CHECK(*result == 10);
    // 6e8bde7b-a9f1-4c98-8f1a-807d9ee0a93b end
}

TEST_CASE("ApplicativeTypeclassTest - MapOptional")
{
    std::optional<int> value{21};
    const auto& applicative = smd::applicative_typeclass<std::optional<int> >;

    auto result = applicative.map([](int x) { return x * 2; }, value);
    REQUIRE(result.has_value());
    CHECK(*result == 42);
}

TEST_CASE("ApplicativeTypeclassTest - InvokeWithExplicitMap")
{
    std::optional<int> ax{10};
    std::optional<int> ay{5};
    const auto& default_applicative = smd::applicative_typeclass<std::optional<int> >;
    const auto& optional_applicative = smd::applicative_typeclass<std::optional<int> >;

    auto result = default_applicative.invoke_with(
        optional_applicative,
        [](int a, int b) { return a + b; },
        ax,
        ay);
    REQUIRE(result.has_value());
    CHECK(*result == 15);
}
