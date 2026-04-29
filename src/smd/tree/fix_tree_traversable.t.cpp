#include <smd/tree/fix_tree.hpp>
#include <smd/tree/fix_tree_traversable.hpp>

#include <gtest/gtest.h>

#include <optional>

TEST(FixTreeTraversableTest, TraverseOptionalSuccess)
{
    using Tree = smd::tree::FixTree<int>;
    auto tree = Tree::branch(Tree::leaf(1), Tree::leaf(2));
    const auto& traversable = smd::traversable_typeclass<Tree>;

    auto traversed = traversable.traverse(
        [](int x) -> std::optional<int> {
            return x >= 0 ? std::optional<int>{x + 1} : std::optional<int>{};
        },
        tree);

    ASSERT_TRUE(traversed.has_value());
    EXPECT_EQ(traversed->left().value(), 2);
    EXPECT_EQ(traversed->right().value(), 3);
}

TEST(FixTreeTraversableTest, TraverseOptionalFailure)
{
    using Tree = smd::tree::FixTree<int>;
    auto tree = Tree::branch(Tree::leaf(1), Tree::leaf(-2));
    const auto& traversable = smd::traversable_typeclass<Tree>;

    auto traversed = traversable.traverse(
        [](int x) -> std::optional<int> {
            return x >= 0 ? std::optional<int>{x + 1} : std::optional<int>{};
        },
        tree);

    EXPECT_FALSE(traversed.has_value());
}

TEST(FixTreeTraversableTest, ForEachOptionalSuccess)
{
    using Tree = smd::tree::FixTree<int>;
    auto tree = Tree::branch(Tree::leaf(3), Tree::leaf(4));
    const auto& traversable = smd::traversable_typeclass<Tree>;

    auto traversed = traversable.for_each(tree, [](int x) -> std::optional<int> {
        return std::optional<int>{x * 2};
    });

    ASSERT_TRUE(traversed.has_value());
    EXPECT_EQ(traversed->left().value(), 6);
    EXPECT_EQ(traversed->right().value(), 8);
}
