#include <smd/tree/fringe_tree.hpp>
#include <smd/tree/fringe_tree_foldable.hpp>
#include <smd/typeclass/foldable.hpp>

#include <gtest/gtest.h>

#include <vector>

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
