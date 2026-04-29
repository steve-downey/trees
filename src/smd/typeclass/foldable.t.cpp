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

TEST(FoldableTypeclassTest, LengthOnFixTree)
{
    using Tree = smd::tree::FixTree<int>;
    auto tree = Tree::branch(Tree::leaf(1), Tree::branch(Tree::leaf(2), Tree::leaf(3)));

    const auto& foldable = smd::foldable_typeclass<Tree>;
    EXPECT_EQ(foldable.length(tree), 3U);
}

TEST(FoldableTypeclassTest, FoldMapSumOnFixTree)
{
    using Tree = smd::tree::FixTree<int>;
    auto tree = Tree::branch(Tree::leaf(1), Tree::branch(Tree::leaf(2), Tree::leaf(3)));

    const auto& foldable = smd::foldable_typeclass<Tree>;
    const auto sum = foldable.fold_map([](int x) { return x; }, tree);
    EXPECT_EQ(sum, 6);
}

TEST(FoldableTypeclassTest, FoldMapWithExplicitObject)
{
    using Tree = smd::tree::FixTree<int>;
    auto tree = Tree::branch(Tree::leaf(1), Tree::branch(Tree::leaf(2), Tree::leaf(3)));

    const auto& foldable = smd::foldable_typeclass<Tree>;
    const auto sum = foldable.fold_map([](int x) { return x; }, tree);
    EXPECT_EQ(sum, 6);
}

TEST(FoldableTypeclassTest, FoldMapWithNttpLookup)
{
    using Tree = smd::tree::FixTree<int>;
    auto tree = Tree::branch(Tree::leaf(1), Tree::branch(Tree::leaf(2), Tree::leaf(3)));

    EXPECT_EQ(sum_with_nttp_lookup(tree), 6);
}

TEST(FoldableTypeclassTest, FoldLeftAndRight)
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

TEST(FoldableTypeclassTest, FoldLeftRightWithExplicitObject)
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

TEST(FoldableTypeclassTest, FoldLeftRightWithNttpLookup)
{
    using Tree = smd::tree::FixTree<int>;
    auto tree = Tree::branch(Tree::leaf(1), Tree::branch(Tree::leaf(2), Tree::leaf(3)));

    EXPECT_EQ(fold_left_with_nttp_lookup(tree), 123);
    EXPECT_EQ(fold_right_with_nttp_lookup(tree), 60);
}

TEST(FoldableTypeclassTest, PredicatesAndFind)
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

TEST(FoldableTypeclassTest, ToVectorAndCombineAll)
{
    using IntTree = smd::tree::FixTree<int>;
    auto tree = IntTree::branch(IntTree::leaf(1), IntTree::branch(IntTree::leaf(2), IntTree::leaf(3)));
    const auto& int_foldable = smd::foldable_typeclass<IntTree>;

    const auto as_vector = int_foldable.to_vector(tree);
    EXPECT_EQ(as_vector, (std::vector<int>{1, 2, 3}));

    using VectorTree = smd::tree::FixTree<std::vector<int> >;
    auto vectors = VectorTree::branch(VectorTree::leaf({1, 2}), VectorTree::leaf({3}));
    const auto& vector_foldable = smd::foldable_typeclass<VectorTree>;
    const auto combined = vector_foldable.combine_all(vectors);
    EXPECT_EQ(combined, (std::vector<int>{1, 2, 3}));
}
