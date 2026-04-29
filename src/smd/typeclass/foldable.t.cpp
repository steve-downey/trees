#include <smd/tree/fix_tree.hpp>
#include <smd/tree/fix_tree_foldable.hpp>
#include <smd/typeclass/foldable.hpp>

#include <gtest/gtest.h>

namespace {

template <class TREE,
          const auto& FOLDABLE = smd::foldable_typeclass<TREE> >
auto sum_with_nttp_lookup(const TREE& tree)
{
    return FOLDABLE.fold_map([](int x) { return x; }, tree);
}

}  // namespace

TEST(FoldableTypeclassTest, LengthOnFixTree)
{
    using Tree = smd::tree::FixTree<int>;
    auto tree = Tree::branch(Tree::leaf(1), Tree::branch(Tree::leaf(2), Tree::leaf(3)));

    EXPECT_EQ(smd::length(tree), 3U);
}

TEST(FoldableTypeclassTest, FoldMapSumOnFixTree)
{
    using Tree = smd::tree::FixTree<int>;
    auto tree = Tree::branch(Tree::leaf(1), Tree::branch(Tree::leaf(2), Tree::leaf(3)));

    const auto sum = smd::fold_map([](int x) { return x; }, tree);
    EXPECT_EQ(sum, 6);
}

TEST(FoldableTypeclassTest, FoldMapWithExplicitObject)
{
    using Tree = smd::tree::FixTree<int>;
    auto tree = Tree::branch(Tree::leaf(1), Tree::branch(Tree::leaf(2), Tree::leaf(3)));

    const auto& foldable = smd::foldable_typeclass<Tree>;
    const auto sum = smd::fold_map(foldable, [](int x) { return x; }, tree);
    EXPECT_EQ(sum, 6);
}

TEST(FoldableTypeclassTest, FoldMapWithNttpLookup)
{
    using Tree = smd::tree::FixTree<int>;
    auto tree = Tree::branch(Tree::leaf(1), Tree::branch(Tree::leaf(2), Tree::leaf(3)));

    EXPECT_EQ(sum_with_nttp_lookup(tree), 6);
}
