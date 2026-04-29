#include <smd/tree/finger_tree.hpp>

#include <gtest/gtest.h>

#include <cstddef>
#include <vector>

namespace {

struct Weighted {
    std::size_t d_total;

    friend bool operator==(const Weighted&, const Weighted&) = default;
    friend bool operator>=(const Weighted& lhs, const Weighted& rhs)
    {
        return lhs.d_total >= rhs.d_total;
    }
};

struct WeightedMeasure {
    auto operator()(int value) const -> Weighted
    {
        return Weighted{static_cast<std::size_t>(value * 10)};
    }
};

}  // namespace

namespace smd::typeclass {

template <>
struct Monoid<Weighted> {
    constexpr auto identity() const -> Weighted { return Weighted{0U}; }

    constexpr auto combine(const Weighted& lhs, const Weighted& rhs) const
        -> Weighted
    {
        return Weighted{lhs.d_total + rhs.d_total};
    }
};

}  // namespace smd::typeclass

TEST(FingerTreeTest, EmptyLeafAndPredicates)
{
    using Tree = smd::tree::FingerTree<int>;

    auto empty = Tree::empty();
    EXPECT_TRUE(empty.is_empty());
    EXPECT_FALSE(empty.is_leaf());
    EXPECT_FALSE(empty.is_branch());
    EXPECT_EQ(empty.measure(), 0U);
    EXPECT_EQ(empty.breadth(), 0U);
    EXPECT_EQ(empty.depth(), 0U);
    EXPECT_EQ(empty.flatten(), (std::vector<int>{}));
    EXPECT_FALSE(empty.view_l().has_value());
    EXPECT_FALSE(empty.view_r().has_value());

    auto single = Tree::leaf(42);
    EXPECT_FALSE(single.is_empty());
    EXPECT_TRUE(single.is_leaf());
    EXPECT_FALSE(single.is_branch());
    EXPECT_EQ(single.measure(), 1U);
    EXPECT_EQ(single.value(), 42);
    EXPECT_EQ(single.flatten(), (std::vector<int>{42}));
}

TEST(FingerTreeTest, FromSequenceConsSnocAndMemberAppend)
{
    using Tree = smd::tree::FingerTree<int>;

    auto from = Tree::from_sequence({1, 2, 3});
    EXPECT_EQ(from.flatten(), (std::vector<int>{1, 2, 3}));

    auto with_cons = from.cons(0);
    EXPECT_EQ(with_cons.flatten(), (std::vector<int>{0, 1, 2, 3}));

    auto with_snoc = with_cons.snoc(4);
    EXPECT_EQ(with_snoc.flatten(), (std::vector<int>{0, 1, 2, 3, 4}));

    auto appended_member = from.append(Tree::from_sequence({4, 5}));
    EXPECT_EQ(appended_member.flatten(), (std::vector<int>{1, 2, 3, 4, 5}));
}

TEST(FingerTreeTest, SingletonViewsAndEmptyTailInit)
{
    using Tree = smd::tree::FingerTree<int>;

    auto single = Tree::leaf(7);
    auto left = single.view_l();
    ASSERT_TRUE(left.has_value());
    EXPECT_EQ(left->d_value, 7);
    EXPECT_TRUE(left->d_rest.is_empty());

    auto right = single.view_r();
    ASSERT_TRUE(right.has_value());
    EXPECT_EQ(right->d_value, 7);
    EXPECT_TRUE(right->d_rest.is_empty());

    auto empty = Tree::empty();
    EXPECT_TRUE(empty.tail().is_empty());
    EXPECT_TRUE(empty.init().is_empty());
}

TEST(FingerTreeTest, BasicMeasureDepthFlatten)
{
    using Tree = smd::tree::FingerTree<int>;

    auto tree = Tree::branch(
        Tree::branch(Tree::leaf(1), Tree::leaf(2)),
        Tree::leaf(3));

    EXPECT_EQ(tree.measure(), 3U);
    EXPECT_EQ(tree.breadth(), 3U);
    EXPECT_GE(tree.depth(), 1U);
    EXPECT_EQ(tree.flatten(), (std::vector<int>{1, 2, 3}));
}

TEST(FingerTreeTest, ViewsAndListOps)
{
    using Tree = smd::tree::FingerTree<int>;

    auto tree = Tree::branch(
        Tree::branch(Tree::leaf(1), Tree::leaf(2)),
        Tree::leaf(3));

    auto left_view = tree.view_l();
    ASSERT_TRUE(left_view.has_value());
    EXPECT_EQ(left_view->d_value, 1);
    EXPECT_EQ(left_view->d_rest.flatten(), (std::vector<int>{2, 3}));

    auto right_view = tree.view_r();
    ASSERT_TRUE(right_view.has_value());
    EXPECT_EQ(right_view->d_value, 3);
    EXPECT_EQ(right_view->d_rest.flatten(), (std::vector<int>{1, 2}));

    EXPECT_EQ(tree.head(), 1);
    EXPECT_EQ(tree.last(), 3);
    EXPECT_EQ(tree.tail().flatten(), (std::vector<int>{2, 3}));
    EXPECT_EQ(tree.init().flatten(), (std::vector<int>{1, 2}));
}

TEST(FingerTreeTest, PrependAppendConcat)
{
    using Tree = smd::tree::FingerTree<int>;

    auto tree = Tree::branch(Tree::leaf(1), Tree::leaf(2));

    auto prepended = Tree::prepend(0, tree);
    EXPECT_EQ(prepended.flatten(), (std::vector<int>{0, 1, 2}));

    auto appended = Tree::append(tree, 3);
    EXPECT_EQ(appended.flatten(), (std::vector<int>{1, 2, 3}));

    auto concatenated = Tree::concat(tree, tree);
    EXPECT_EQ(concatenated.flatten(), (std::vector<int>{1, 2, 1, 2}));
}

TEST(FingerTreeTest, MonoidTaggedMeasure)
{
    using Tree = smd::tree::FingerTree<int, Weighted, WeightedMeasure>;

    auto tree = Tree::from_sequence({1, 2, 3});
    EXPECT_EQ(tree.measure(), Weighted{60U});

    auto prepended = Tree::prepend(4, tree);
    EXPECT_EQ(prepended.measure(), Weighted{100U});

    auto concatenated = Tree::concat(tree, Tree::leaf(5));
    EXPECT_EQ(concatenated.measure(), Weighted{110U});
}

TEST(FingerTreeTest, MeasureGuidedSearchAndSplit)
{
    using Tree = smd::tree::FingerTree<int>;

    auto tree = Tree::from_sequence({1, 2, 3, 4, 5});

    auto found = tree.search([](std::size_t prefix) { return prefix >= 3U; });
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(*found, 3);

    auto split = tree.split([](std::size_t prefix) { return prefix >= 3U; });
    ASSERT_TRUE(split.has_value());
    EXPECT_EQ(split->d_left.flatten(), (std::vector<int>{1, 2}));
    EXPECT_EQ(split->d_pivot, 3);
    EXPECT_EQ(split->d_right.flatten(), (std::vector<int>{4, 5}));

    EXPECT_FALSE(tree.search([](std::size_t prefix) { return prefix >= 6U; }).has_value());
    EXPECT_FALSE(tree.split([](std::size_t prefix) { return prefix >= 6U; }).has_value());
}

TEST(FingerTreeTest, MeasureGuidedSearchAndSplitWithCustomTag)
{
    using Tree = smd::tree::FingerTree<int, Weighted, WeightedMeasure>;

    auto tree = Tree::from_sequence({1, 2, 3, 4});

    auto found = tree.search([](Weighted prefix) { return prefix.d_total >= 35U; });
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(*found, 3);

    auto split = tree.split([](Weighted prefix) { return prefix.d_total >= 35U; });
    ASSERT_TRUE(split.has_value());
    EXPECT_EQ(split->d_left.flatten(), (std::vector<int>{1, 2}));
    EXPECT_EQ(split->d_left.measure(), Weighted{30U});
    EXPECT_EQ(split->d_pivot, 3);
    EXPECT_EQ(split->d_right.flatten(), (std::vector<int>{4}));
    EXPECT_EQ(split->d_right.measure(), Weighted{40U});
}

TEST(FingerTreeTest, SplitAtCountBoundary)
{
    using Tree = smd::tree::FingerTree<int>;

    auto tree = Tree::from_sequence({1, 2, 3, 4, 5});

    auto at_three = tree.split_at([](std::size_t prefix) { return prefix >= 3U; });
    EXPECT_EQ(at_three.d_left.flatten(), (std::vector<int>{1, 2}));
    EXPECT_EQ(at_three.d_right.flatten(), (std::vector<int>{3, 4, 5}));

    auto at_one = tree.split_at([](std::size_t prefix) { return prefix >= 1U; });
    EXPECT_TRUE(at_one.d_left.is_empty());
    EXPECT_EQ(at_one.d_right.flatten(), (std::vector<int>{1, 2, 3, 4, 5}));

    auto none = tree.split_at([](std::size_t prefix) { return prefix >= 6U; });
    EXPECT_EQ(none.d_left.flatten(), (std::vector<int>{1, 2, 3, 4, 5}));
    EXPECT_TRUE(none.d_right.is_empty());
}

TEST(FingerTreeTest, SplitAtWeightedBoundary)
{
    using Tree = smd::tree::FingerTree<int, Weighted, WeightedMeasure>;

    auto tree = Tree::from_sequence({1, 2, 3, 4});

    auto split = tree.split_at([](Weighted prefix) { return prefix.d_total >= 35U; });
    EXPECT_EQ(split.d_left.flatten(), (std::vector<int>{1, 2}));
    EXPECT_EQ(split.d_left.measure(), Weighted{30U});
    EXPECT_EQ(split.d_right.flatten(), (std::vector<int>{3, 4}));
    EXPECT_EQ(split.d_right.measure(), Weighted{70U});
}

TEST(FingerTreeTest, SplitAtIndexConvenience)
{
    using Tree = smd::tree::FingerTree<int>;

    auto tree = Tree::from_sequence({1, 2, 3, 4, 5});

    auto at_zero = tree.split_at_index(0U);
    EXPECT_TRUE(at_zero.d_left.is_empty());
    EXPECT_EQ(at_zero.d_right.flatten(), (std::vector<int>{1, 2, 3, 4, 5}));

    auto at_three = tree.split_at_index(3U);
    EXPECT_EQ(at_three.d_left.flatten(), (std::vector<int>{1, 2, 3}));
    EXPECT_EQ(at_three.d_right.flatten(), (std::vector<int>{4, 5}));

    auto beyond = tree.split_at_index(99U);
    EXPECT_EQ(beyond.d_left.flatten(), (std::vector<int>{1, 2, 3, 4, 5}));
    EXPECT_TRUE(beyond.d_right.is_empty());
}

TEST(FingerTreeTest, SplitAtMeasureConvenience)
{
    using CountTree = smd::tree::FingerTree<int>;

    auto count_tree = CountTree::from_sequence({1, 2, 3, 4, 5});
    auto count_split = count_tree.split_at_measure(3U);
    EXPECT_EQ(count_split.d_left.flatten(), (std::vector<int>{1, 2}));
    EXPECT_EQ(count_split.d_right.flatten(), (std::vector<int>{3, 4, 5}));

    using WeightedTree = smd::tree::FingerTree<int, Weighted, WeightedMeasure>;

    auto weighted_tree = WeightedTree::from_sequence({1, 2, 3, 4});
    auto weighted_split = weighted_tree.split_at_measure(Weighted{35U});
    EXPECT_EQ(weighted_split.d_left.flatten(), (std::vector<int>{1, 2}));
    EXPECT_EQ(weighted_split.d_right.flatten(), (std::vector<int>{3, 4}));
}
