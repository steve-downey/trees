#include <smd/tree/finger_tree_random_access.hpp>

#include <gtest/gtest.h>

#include <optional>
#include <vector>

TEST(FingerTreeRandomAccessTest, WrapperOperations)
{
    using Seq = smd::tree::FingerTreeRandomAccess<int>;

    auto seq = Seq::from_sequence({1, 2, 3});
    ASSERT_TRUE(seq.at(0).has_value());
    EXPECT_EQ(*seq.at(0), 1);
    EXPECT_FALSE(seq.at(99).has_value());

    auto edited = seq.push_back(4).push_front(0).insert(2, 9).update(3, 7).erase(1);
    EXPECT_EQ(edited.to_vector(), (std::vector<int>{0, 9, 7, 3, 4}));
}

TEST(FingerTreeRandomAccessTest, FoldableTypeclass)
{
    using Seq = smd::tree::FingerTreeRandomAccess<int>;

    auto seq = Seq::from_sequence({1, 2, 3, 4});
    const auto& foldable = smd::foldable_typeclass<Seq>;

    EXPECT_EQ(foldable.fold_map([](int value) { return value; }, seq), 10);
    EXPECT_EQ(foldable.length(seq), 4U);
}

TEST(FingerTreeRandomAccessTest, TraversableTypeclass)
{
    using Seq = smd::tree::FingerTreeRandomAccess<int>;

    auto seq = Seq::from_sequence({1, 2, 3});
    const auto& traversable = smd::traversable_typeclass<Seq>;

    auto success = traversable.traverse(
      [](int value) -> std::optional<int> { return value * 10; },
      seq);
    ASSERT_TRUE(success.has_value());
    EXPECT_EQ(success->to_vector(), (std::vector<int>{10, 20, 30}));

    auto failure = traversable.traverse(
      [](int value) -> std::optional<int> {
          if (value == 2) {
              return std::nullopt;
          }
          return value;
      },
      seq);
    EXPECT_FALSE(failure.has_value());
}
