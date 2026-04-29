#include <smd/tree/fix_tree.hpp>
#include <smd/tree/fix_tree_foldable.hpp>
#include <smd/typeclass/foldable.hpp>

#include <gtest/gtest.h>

#include <vector>

namespace {

template <class TREE,
          const auto& FOLDABLE = smd::foldable_typeclass<TREE> >
auto sum_with_nttp_lookup(const TREE& tree)
{
    return FOLDABLE.fold_map([](int x) { return x; }, tree);
}

template <class TREE,
          const auto& FOLDABLE = smd::foldable_typeclass<TREE> >
auto fold_left_with_nttp_lookup(const TREE& tree)
{
    return FOLDABLE.fold_left(tree, 0, [](int acc, int x) {
        return acc * 10 + x;
    });
}

template <class TREE,
          const auto& FOLDABLE = smd::foldable_typeclass<TREE> >
auto fold_right_with_nttp_lookup(const TREE& tree)
{
    return FOLDABLE.fold_right(tree, 0, [](int x, int acc) {
        return x * 10 + acc;
    });
}

}  // namespace

TEST(FixTreeFoldableTest, Length)
{
    using Tree = smd::tree::FixTree<int>;
    auto tree = Tree::branch(Tree::leaf(1), Tree::branch(Tree::leaf(2), Tree::leaf(3)));

    const auto& foldable = smd::foldable_typeclass<Tree>;
    EXPECT_EQ(foldable.length(tree), 3U);
}

TEST(FixTreeFoldableTest, FoldMapSum)
{
    using Tree = smd::tree::FixTree<int>;
    auto tree = Tree::branch(Tree::leaf(1), Tree::branch(Tree::leaf(2), Tree::leaf(3)));

    const auto& foldable = smd::foldable_typeclass<Tree>;
    const auto sum = foldable.fold_map([](int x) { return x; }, tree);
    EXPECT_EQ(sum, 6);
}

TEST(FixTreeFoldableTest, FoldMapWithExplicitObject)
{
    using Tree = smd::tree::FixTree<int>;
    auto tree = Tree::branch(Tree::leaf(1), Tree::branch(Tree::leaf(2), Tree::leaf(3)));

    const auto& foldable = smd::foldable_typeclass<Tree>;
    const auto sum = foldable.fold_map([](int x) { return x; }, tree);
    EXPECT_EQ(sum, 6);
}

TEST(FixTreeFoldableTest, FoldMapWithNttpLookup)
{
    using Tree = smd::tree::FixTree<int>;
    auto tree = Tree::branch(Tree::leaf(1), Tree::branch(Tree::leaf(2), Tree::leaf(3)));

    EXPECT_EQ(sum_with_nttp_lookup(tree), 6);
}

TEST(FixTreeFoldableTest, FoldLeftAndRight)
{
    using Tree = smd::tree::FixTree<int>;
    auto tree = Tree::branch(Tree::leaf(1), Tree::branch(Tree::leaf(2), Tree::leaf(3)));
    const auto& foldable = smd::foldable_typeclass<Tree>;

    const auto left = foldable.fold_left(tree, 0, [](int acc, int x) {
        return acc * 10 + x;
    });
    const auto right = foldable.fold_right(tree, 0, [](int x, int acc) {
        return x * 10 + acc;
    });

    EXPECT_EQ(left, 123);
    EXPECT_EQ(right, 60);
}

TEST(FixTreeFoldableTest, FoldLeftRightWithExplicitObject)
{
    using Tree = smd::tree::FixTree<int>;
    auto tree = Tree::branch(Tree::leaf(1), Tree::branch(Tree::leaf(2), Tree::leaf(3)));

    const auto& foldable = smd::foldable_typeclass<Tree>;
    const auto left = foldable.fold_left(tree, 0, [](int acc, int x) {
        return acc * 10 + x;
    });
    const auto right = foldable.fold_right(tree, 0, [](int x, int acc) {
        return x * 10 + acc;
    });

    EXPECT_EQ(left, 123);
    EXPECT_EQ(right, 60);
}

TEST(FixTreeFoldableTest, FoldLeftRightWithNttpLookup)
{
    using Tree = smd::tree::FixTree<int>;
    auto tree = Tree::branch(Tree::leaf(1), Tree::branch(Tree::leaf(2), Tree::leaf(3)));

    EXPECT_EQ(fold_left_with_nttp_lookup(tree), 123);
    EXPECT_EQ(fold_right_with_nttp_lookup(tree), 60);
}

TEST(FixTreeFoldableTest, PredicatesAndFind)
{
    using Tree = smd::tree::FixTree<int>;
    auto tree = Tree::branch(Tree::leaf(1), Tree::branch(Tree::leaf(2), Tree::leaf(3)));
    const auto& foldable = smd::foldable_typeclass<Tree>;

    EXPECT_TRUE(foldable.any_of(tree, [](int x) { return x == 2; }));
    EXPECT_TRUE(foldable.all_of(tree, [](int x) { return x > 0; }));
    EXPECT_FALSE(foldable.empty(tree));

    auto found = foldable.find_first(tree, [](int x) { return x > 1; });
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(*found, 2);
}