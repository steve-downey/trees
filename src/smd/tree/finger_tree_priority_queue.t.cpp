#include <smd/tree/finger_tree_priority_queue.hpp>

#include <gtest/gtest.h>

#include <optional>
#include <vector>

TEST(FingerTreePriorityQueueTest, WrapperOperations)
{
    using Queue = smd::tree::FingerTreePriorityQueue<int>;

    auto q = Queue::from_values({5, 2, 8, 2, 7});
    ASSERT_TRUE(q.min().has_value());
    ASSERT_TRUE(q.max().has_value());
    EXPECT_EQ(*q.min(), 2);
    EXPECT_EQ(*q.max(), 8);

    auto min_pop = q.pop_min();
    ASSERT_TRUE(min_pop.has_value());
    EXPECT_EQ(min_pop->first, 2);
    ASSERT_TRUE(min_pop->second.min().has_value());
    EXPECT_EQ(*min_pop->second.min(), 2);

    auto max_pop = min_pop->second.pop_max();
    ASSERT_TRUE(max_pop.has_value());
    EXPECT_EQ(max_pop->first, 8);
    ASSERT_TRUE(max_pop->second.max().has_value());
    EXPECT_EQ(*max_pop->second.max(), 7);
}

TEST(FingerTreePriorityQueueTest, FoldableTypeclass)
{
    using Queue = smd::tree::FingerTreePriorityQueue<int>;

    auto q = Queue::from_values({5, 2, 8, 2, 7});
    const auto& foldable = smd::foldable_typeclass<Queue>;

    EXPECT_EQ(foldable.fold_map([](int value) { return value; }, q), 24);
    EXPECT_EQ(foldable.length(q), 5U);
}

TEST(FingerTreePriorityQueueTest, TraversableTypeclass)
{
    using Queue = smd::tree::FingerTreePriorityQueue<int>;

    auto q = Queue::from_values({5, 2, 8});
    const auto& traversable = smd::traversable_typeclass<Queue>;

    auto success = traversable.traverse(
      [](int value) -> std::optional<int> { return value * 10; },
      q);
    ASSERT_TRUE(success.has_value());
    EXPECT_EQ(success->to_vector(), (std::vector<int>{50, 20, 80}));
    ASSERT_TRUE(success->min().has_value());
    ASSERT_TRUE(success->max().has_value());
    EXPECT_EQ(*success->min(), 20);
    EXPECT_EQ(*success->max(), 80);

    auto failure = traversable.traverse(
      [](int value) -> std::optional<int> {
          if (value == 8) {
              return std::nullopt;
          }
          return value;
      },
      q);
    EXPECT_FALSE(failure.has_value());
}
