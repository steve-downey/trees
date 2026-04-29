#include <smd/ranges/range_foldable.hpp>

#include <gtest/gtest.h>

#include <ranges>
#include <vector>

TEST(RangeFoldableTest, LengthAndFoldLeftFollowRangeOrder)
{
    auto values = smd::ranges::all(std::views::iota(1, 5));
    const auto& foldable = smd::foldable_typeclass<decltype(values)>;

    EXPECT_EQ(foldable.length(values), 4U);

    const auto folded = foldable.fold_left(values, 0, [](int acc, int value) {
        return acc * 10 + value;
    });
    EXPECT_EQ(folded, 1234);
}

TEST(RangeFoldableTest, ToVectorMaterializesValues)
{
    auto values = smd::ranges::from_vector(std::vector<int>{3, 1, 4});
    const auto& foldable = smd::foldable_typeclass<decltype(values)>;

    EXPECT_EQ(foldable.to_vector(values), (std::vector<int>{3, 1, 4}));
}

TEST(RangeFoldableTest, PredicatesAndFindUseRangeSemantics)
{
    auto values = smd::ranges::all(std::views::iota(1, 6));
    const auto& foldable = smd::foldable_typeclass<decltype(values)>;

    EXPECT_TRUE(foldable.any_of(values, [](int value) { return value == 4; }));
    EXPECT_FALSE(foldable.all_of(values, [](int value) { return value < 5; }));
    EXPECT_FALSE(foldable.empty(values));

    auto found = foldable.find_first(values, [](int value) { return value > 3; });
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(*found, 4);
}
