#include <smd/tree/fringe_tree.hpp>
#include <smd/tree/fringe_tree_foldable.hpp>
#include <smd/typeclass/foldable.hpp>

#include <gtest/gtest.h>

#include <vector>

TEST(FringeTreeTest, EmptyLeafAndPredicates)
{
    using Tree = smd::tree::FringeTree<int>;

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

TEST(FringeTreeTest, BranchLeftRightAndMemberStyleOperations)
{
    using Tree = smd::tree::FringeTree<int>;

    auto left = Tree::branch(Tree::leaf(1), Tree::leaf(2));
    auto right = Tree::branch(Tree::leaf(3), Tree::leaf(4));
    auto tree = Tree::branch(left, right);

    ASSERT_TRUE(tree.is_branch());
    EXPECT_EQ(tree.left().flatten(), (std::vector<int>{1, 2}));
    EXPECT_EQ(tree.right().flatten(), (std::vector<int>{3, 4}));

    auto prepended = Tree::prepend(0, tree);
    EXPECT_EQ(prepended.flatten(), (std::vector<int>{0, 1, 2, 3, 4}));

    auto appended = Tree::append(tree, 5);
    EXPECT_EQ(appended.flatten(), (std::vector<int>{1, 2, 3, 4, 5}));

    auto concatenated = Tree::concat(left, right);
    EXPECT_EQ(concatenated.flatten(), (std::vector<int>{1, 2, 3, 4}));
}

TEST(FringeTreeTest, SingletonViewsAndEmptyTailInit)
{
    using Tree = smd::tree::FringeTree<int>;

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

TEST(FringeTreeTest, BasicMeasureDepthFlatten)
{
    using Tree = smd::tree::FringeTree<int>;

    auto tree = Tree::branch(
        Tree::branch(Tree::leaf(1), Tree::leaf(2)),
        Tree::leaf(3));

    EXPECT_EQ(tree.measure(), 3U);
    EXPECT_EQ(tree.breadth(), 3U);
    EXPECT_EQ(tree.depth(), 3U);
    EXPECT_EQ(tree.flatten(), (std::vector<int>{1, 2, 3}));
}

TEST(FringeTreeTest, ViewsAndListOps)
{
    using Tree = smd::tree::FringeTree<int>;

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

TEST(FringeTreeTest, PrependAppendConcat)
{
    using Tree = smd::tree::FringeTree<int>;

    auto tree = Tree::branch(Tree::leaf(1), Tree::leaf(2));

    auto prepended = Tree::prepend(0, tree);
    EXPECT_EQ(prepended.flatten(), (std::vector<int>{0, 1, 2}));

    auto appended = Tree::append(tree, 3);
    EXPECT_EQ(appended.flatten(), (std::vector<int>{1, 2, 3}));

    auto concatenated = Tree::concat(tree, tree);
    EXPECT_EQ(concatenated.flatten(), (std::vector<int>{1, 2, 1, 2}));
}

TEST(FringeTreeTest, FoldableIntegration)
{
    using Tree = smd::tree::FringeTree<int>;

    auto tree = Tree::branch(
        Tree::branch(Tree::leaf(1), Tree::leaf(2)),
        Tree::leaf(3));

    const auto& foldable = smd::foldable_typeclass<Tree>;
    EXPECT_EQ(foldable.length(tree), 3U);
    EXPECT_EQ(foldable.fold_map([](int x) { return x; }, tree), 6);
    EXPECT_EQ(foldable.to_vector(tree), (std::vector<int>{1, 2, 3}));
}
