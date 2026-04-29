#include <smd/typeclass/applicative.hpp>

#include <gtest/gtest.h>

#include <optional>

TEST(ApplicativeTypeclassTest, PureOptional)
{
    const auto& applicative = smd::applicative_typeclass<std::optional<int> >;
    auto lifted = applicative.pure(7);
    ASSERT_TRUE(lifted.has_value());
    EXPECT_EQ(*lifted, 7);
}

TEST(ApplicativeTypeclassTest, ApplyOptional)
{
    std::optional<int (*)(int)> function{+[](int x) { return x + 3; }};
    std::optional<int> argument{4};
    const auto& applicative = smd::applicative_typeclass<std::optional<int (*)(int)> >;

    auto result = applicative.apply(function, argument);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 7);
}

TEST(ApplicativeTypeclassTest, InvokeOptional)
{
    std::optional<int> ax{10};
    std::optional<int> ay{5};
    const auto& applicative = smd::applicative_typeclass<std::optional<int> >;

    auto result = applicative.invoke([](int a, int b) { return a - b; }, ax, ay);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 5);
}

TEST(ApplicativeTypeclassTest, InvokeOptionalTernaryUsesPartialApplication)
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
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 10);
}

TEST(ApplicativeTypeclassTest, ApplyPureOptionalTernary)
{
    std::optional<int> ax{2};
    std::optional<int> ay{3};
    std::optional<int> az{4};
    const auto& applicative = smd::applicative_typeclass<std::optional<int> >;

    auto result = applicative.apply_pure(
        [](int a, int b, int c) { return a * b + c; },
        ax,
        ay,
        az);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 10);
}

TEST(ApplicativeTypeclassTest, MapOptional)
{
    std::optional<int> value{21};
    const auto& applicative = smd::applicative_typeclass<std::optional<int> >;

    auto result = applicative.map([](int x) { return x * 2; }, value);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 42);
}

TEST(ApplicativeTypeclassTest, InvokeWithExplicitMap)
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
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 15);
}
