#include <smd/tree/fix_tree.hpp>
#include <smd/tree/binary_tree.hpp>
#include <smd/tree/fringe_tree.hpp>
#include <smd/tree/fix_tree_applicative.hpp>
#include <smd/tree/fix_tree_foldable.hpp>
#include <smd/tree/fix_tree_traversable.hpp>
#include <smd/tree/binary_tree_applicative.hpp>
#include <smd/tree/binary_tree_foldable.hpp>
#include <smd/tree/binary_tree_traversable.hpp>
#include <smd/tree/fringe_tree_applicative.hpp>
#include <smd/tree/fringe_tree_foldable.hpp>
#include <smd/tree/fringe_tree_traversable.hpp>
#include <smd/typeclass/traversable.hpp>

#include <gtest/gtest.h>

#include <optional>
#include <vector>

TEST(TraversableTypeclassTest, TraverseOptionalSuccess)
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
    const auto& foldable = smd::foldable_typeclass<Tree>;
    EXPECT_EQ(foldable.length(*traversed), 2U);
}

TEST(TraversableTypeclassTest, TraverseOptionalFailure)
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

TEST(TraversableTypeclassTest, ForEachOptionalSuccess)
{
    using Tree = smd::tree::FixTree<int>;
    auto tree = Tree::branch(Tree::leaf(3), Tree::leaf(4));
    const auto& traversable = smd::traversable_typeclass<Tree>;

    auto traversed = traversable.for_each(tree, [](int x) -> std::optional<int> {
        return std::optional<int>{x * 2};
    });

    ASSERT_TRUE(traversed.has_value());
    const auto& foldable = smd::foldable_typeclass<Tree>;
    EXPECT_EQ(foldable.length(*traversed), 2U);
}

TEST(TraversableTypeclassTest, SequenceAndSequenceWith)
{
    // f1de12e0-2287-4568-98c7-75be4f6f7446
    using TreeOpt = smd::tree::FixTree<std::optional<int> >;
    auto tree = TreeOpt::branch(TreeOpt::leaf(std::optional<int>{1}),
                                TreeOpt::leaf(std::optional<int>{2}));
    const auto& traversable = smd::traversable_typeclass<TreeOpt>;

    auto sequenced = traversable.sequence(tree);
    ASSERT_TRUE(sequenced.has_value());

    auto sequenced_with = traversable.sequence_with(traversable, tree);
    ASSERT_TRUE(sequenced_with.has_value());
    // f1de12e0-2287-4568-98c7-75be4f6f7446 end
}

TEST(TraversableTypeclassTest, BinaryTreeTraverseOptionalPreservesShape)
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

TEST(TraversableTypeclassTest, FringeTreeTraverseOptional)
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
