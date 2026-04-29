#include <smd/tree/fix_tree.hpp>
#include <smd/tree/fix_tree_applicative.hpp>
#include <smd/tree/fix_tree_foldable.hpp>
#include <smd/tree/fix_tree_traversable.hpp>
#include <smd/typeclass/traversable.hpp>

#include <gtest/gtest.h>

#include <optional>

TEST(TraversableTypeclassTest, TraverseOptionalSuccess)
{
    using Tree = smd::tree::FixTree<int>;
    auto tree = Tree::branch(Tree::leaf(1), Tree::leaf(2));

    auto traversed = smd::traverse(
        [](int x) -> std::optional<int> {
            return x >= 0 ? std::optional<int>{x + 1} : std::optional<int>{};
        },
        tree);

    ASSERT_TRUE(traversed.has_value());
    EXPECT_EQ(smd::length(*traversed), 2U);
}

TEST(TraversableTypeclassTest, TraverseOptionalFailure)
{
    using Tree = smd::tree::FixTree<int>;
    auto tree = Tree::branch(Tree::leaf(1), Tree::leaf(-2));

    auto traversed = smd::traverse(
        [](int x) -> std::optional<int> {
            return x >= 0 ? std::optional<int>{x + 1} : std::optional<int>{};
        },
        tree);

    EXPECT_FALSE(traversed.has_value());
}
