#include <smd/tree/finger_tree_wrappers.hpp>

#include <gtest/gtest.h>

#include <string>
#include <vector>

TEST(FingerTreeWrappersTest, RandomAccessWrapper)
{
    using Seq = smd::tree::FingerTreeRandomAccess<int>;

    auto seq = Seq::from_sequence({1, 2, 3});
    ASSERT_TRUE(seq.at(0).has_value());
    EXPECT_EQ(*seq.at(0), 1);
    EXPECT_FALSE(seq.at(99).has_value());

    auto edited = seq.push_back(4).push_front(0).insert(2, 9).update(3, 7).erase(1);
    EXPECT_EQ(edited.to_vector(), (std::vector<int>{0, 9, 7, 3, 4}));
}

TEST(FingerTreeWrappersTest, PriorityQueueWrapper)
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

TEST(FingerTreeWrappersTest, IntervalWrapper)
{
    using Index = smd::tree::FingerTreeIntervalIndex<std::string>;
    using Entry = smd::tree::Interval<std::string>;

    auto idx = Index::from_intervals({
        Entry{0U, 5U, "A"},
        Entry{3U, 10U, "B"},
        Entry{8U, 12U, "C"}
    });

    EXPECT_EQ(idx.query_point(2U), (std::vector<std::string>{"A"}));
    EXPECT_EQ(idx.query_point(4U), (std::vector<std::string>{"A", "B"}));
    EXPECT_EQ(idx.query_overlap(9U, 11U), (std::vector<std::string>{"B", "C"}));
}

TEST(FingerTreeWrappersTest, RopeWrapper)
{
    using Rope = smd::tree::FingerTreeRope;

    auto rope = Rope::from_text("abCDxy", 2)
                    .insert(2, "--")
                    .erase(5, 2)
                    .replace(0, 2, "AB");

    EXPECT_EQ(rope.to_string(), "AB--Cy");
    EXPECT_EQ(rope.size_bytes(), 6U);
}
