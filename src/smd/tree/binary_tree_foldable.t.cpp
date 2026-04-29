#include <smd/tree/binary_tree.hpp>
#include <smd/tree/binary_tree_foldable.hpp>
#include <smd/typeclass/foldable.hpp>

#include <gtest/gtest.h>

#include <vector>

TEST(BinaryTreeFoldableTest, InorderFoldAndLength)
{
    using Tree = smd::tree::BinaryTree<int>;
    auto tree = Tree::from_children_ptrs(
      2,
      Tree::make_ptr(Tree::leaf(1)),
      Tree::make_ptr(Tree::from_children_ptrs(
        3,
        {},
        Tree::make_ptr(Tree::leaf(4)))));

    const auto& foldable = smd::foldable_typeclass<Tree>;
    EXPECT_EQ(foldable.length(tree), 4U);

    const auto as_vector = foldable.to_vector(tree);
    EXPECT_EQ(as_vector, (std::vector<int>{1, 2, 3, 4}));

    const auto left = foldable.fold_left(tree, 0, [](int acc, int x) {
        return acc * 10 + x;
    });
    EXPECT_EQ(left, 1234);
}