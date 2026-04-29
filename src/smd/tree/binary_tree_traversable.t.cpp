#include <smd/tree/binary_tree.hpp>
#include <smd/tree/binary_tree_traversable.hpp>

#include <gtest/gtest.h>

#include <optional>

TEST(BinaryTreeTraversableTest, TraverseOptionalPreservesShape)
{
    using Tree = smd::tree::BinaryTree<int>;
    auto tree = Tree::from_children_ptrs(
        2,
        Tree::make_ptr(Tree::leaf(1)),
        Tree::make_ptr(Tree::from_children_ptrs(3, {}, Tree::make_ptr(Tree::leaf(4)))));

    const auto& traversable = smd::traversable_typeclass<Tree>;
    auto traversed = traversable.traverse(
        [](int x) -> std::optional<int> {
            return x > 0 ? std::optional<int>{x * 10} : std::optional<int>{};
        },
        tree);

    ASSERT_TRUE(traversed.has_value());
    EXPECT_EQ(traversed->value(), 20);
    ASSERT_TRUE(traversed->has_left());
    EXPECT_EQ(traversed->left().value(), 10);
    ASSERT_TRUE(traversed->has_right());
    EXPECT_EQ(traversed->right().value(), 30);
    EXPECT_FALSE(traversed->right().has_left());
    ASSERT_TRUE(traversed->right().has_right());
    EXPECT_EQ(traversed->right().right().value(), 40);
}

TEST(BinaryTreeTraversableTest, TraverseOptionalDoesNotDuplicateRootEffect)
{
    using Tree = smd::tree::BinaryTree<int>;
    auto tree = Tree::from_children_ptrs(
        2,
        {},
        Tree::make_ptr(Tree::leaf(5)));

    int invocations = 0;
    const auto& traversable = smd::traversable_typeclass<Tree>;
    auto traversed = traversable.traverse(
        [&](int x) -> std::optional<int> {
            ++invocations;
            return std::optional<int>{x * 10};
        },
        tree);

    ASSERT_TRUE(traversed.has_value());
    EXPECT_EQ(invocations, 2);
    EXPECT_EQ(traversed->value(), 20);
    EXPECT_FALSE(traversed->has_left());
    ASSERT_TRUE(traversed->has_right());
    EXPECT_EQ(traversed->right().value(), 50);
}