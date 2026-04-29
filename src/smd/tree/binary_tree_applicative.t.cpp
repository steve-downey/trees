#include <smd/tree/binary_tree.hpp>
#include <smd/tree/binary_tree_applicative.hpp>

#include <gtest/gtest.h>

TEST(BinaryTreeApplicativeTest, InvokeAndApply)
{
    using Tree = smd::tree::BinaryTree<int>;
    auto lhs = Tree::from_children_ptrs(
        10,
        Tree::make_ptr(Tree::leaf(1)),
        Tree::make_ptr(Tree::leaf(2)));
    auto rhs = Tree::from_children_ptrs(
        3,
        Tree::make_ptr(Tree::leaf(4)),
        Tree::make_ptr(Tree::leaf(5)));

    const auto& applicative = smd::applicative_typeclass<Tree>;
    auto summed = applicative.invoke([](int a, int b) { return a + b; }, lhs, rhs);

    EXPECT_EQ(summed.value(), 13);
    ASSERT_TRUE(summed.has_left());
    ASSERT_TRUE(summed.has_right());
    EXPECT_EQ(summed.left().value(), 5);
    EXPECT_EQ(summed.right().value(), 7);

    auto fs = smd::tree::BinaryTree<int(*)(int)>::from_children_ptrs(
        +[](int x) { return x * 2; },
        smd::tree::BinaryTree<int(*)(int)>::make_ptr(
            smd::tree::BinaryTree<int(*)(int)>::leaf(+[](int x) { return x + 1; })),
        {});
    auto applied = applicative.apply(fs, lhs);
    EXPECT_EQ(applied.value(), 20);
    ASSERT_TRUE(applied.has_left());
    EXPECT_EQ(applied.left().value(), 2);
    EXPECT_FALSE(applied.has_right());
}
