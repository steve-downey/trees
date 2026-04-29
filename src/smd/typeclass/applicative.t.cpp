#include <smd/typeclass/applicative.hpp>

#include <gtest/gtest.h>

#include <optional>

TEST(ApplicativeTypeclassTest, PureOptional)
{
    auto lifted = smd::pure<std::optional<int> >(7);
    ASSERT_TRUE(lifted.has_value());
    EXPECT_EQ(*lifted, 7);
}

TEST(ApplicativeTypeclassTest, ApplyOptional)
{
    std::optional<int (*)(int)> function{+[](int x) { return x + 3; }};
    std::optional<int> argument{4};

    auto result = smd::apply(function, argument);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 7);
}

TEST(ApplicativeTypeclassTest, InvokeOptional)
{
    std::optional<int> ax{10};
    std::optional<int> ay{5};

    auto result = smd::invoke([](int a, int b) { return a - b; }, ax, ay);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 5);
}
