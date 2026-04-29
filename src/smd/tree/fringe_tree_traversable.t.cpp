#include <smd/tree/fringe_tree.hpp>
#include <smd/tree/fringe_tree_traversable.hpp>

#include <gtest/gtest.h>

#include <beman/optional/optional.hpp>

#include <optional>
#include <vector>

TEST(FringeTreeTraversableTest, TraverseOptional)
{
    using Tree = smd::tree::FringeTree<int>;
    auto tree = Tree::branch(Tree::leaf(1), Tree::branch(Tree::leaf(2), Tree::leaf(3)));

    const auto& traversable = smd::traversable_typeclass<Tree>;
    auto traversed = traversable.traverse(
        [](int x) -> std::optional<int> {
            return x > 0 ? std::optional<int>{x * 10} : std::optional<int>{};
        },
        tree);

    ASSERT_TRUE(traversed.has_value());
    EXPECT_EQ(traversed->flatten(), (std::vector<int>{10, 20, 30}));
}

TEST(FringeTreeTraversableTest, TraverseOptionalEmpty)
{
    using Tree = smd::tree::FringeTree<int>;
    auto tree = Tree::empty();

    const auto& traversable = smd::traversable_typeclass<Tree>;
    auto traversed = traversable.traverse(
        [](int x) -> std::optional<int> {
            return std::optional<int>{x * 10};
        },
        tree);

    ASSERT_TRUE(traversed.has_value());
    EXPECT_TRUE(traversed->is_empty());
}

TEST(FringeTreeTraversableTest, TraverseBemanOptionalEmpty)
{
    using Tree = smd::tree::FringeTree<int>;
    auto tree = Tree::empty();

    const auto& traversable = smd::traversable_typeclass<Tree>;
    auto traversed = traversable.traverse(
        [](int x) -> beman::optional::optional<int> {
            return beman::optional::optional<int>{x * 10};
        },
        tree);

    ASSERT_TRUE(traversed.has_value());
    EXPECT_TRUE(traversed->is_empty());
}