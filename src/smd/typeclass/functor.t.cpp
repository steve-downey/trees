#include <smd/typeclass/functor.hpp>

#include <gtest/gtest.h>

#include <optional>
#include <vector>

TEST(FunctorTypeclassTest, OptionalBreathing)
{
    std::optional<int> value{5};
    auto mapped = smd::fmap([](int x) { return x + 1; }, value);

    ASSERT_TRUE(mapped.has_value());
    EXPECT_EQ(*mapped, 6);
}

TEST(FunctorTypeclassTest, ReplaceVector)
{
    std::vector<int> input{1, 2, 3};
    auto replaced = smd::replace(input, 9);

    EXPECT_EQ(replaced, (std::vector<int>{9, 9, 9}));
}
