#include <smd/tree/fix_tree.hpp>
#include <smd/tree/fix_tree.hpp> // Re-inclusion check
#include <smd/tree/fix_tree_foldable.hpp>
#include <smd/typeclass/foldable.hpp>

#include <catch2/catch_test_macros.hpp>

#include <vector>

namespace {

template <class TREE, const auto &FOLDABLE = smd::foldable_typeclass<TREE>>
auto sum_with_nttp_lookup(const TREE &tree) {
    return FOLDABLE.fold_map([](int x) { return x; }, tree);
}

template <class TREE, const auto &FOLDABLE = smd::foldable_typeclass<TREE>>
auto fold_left_with_nttp_lookup(const TREE &tree) {
    return FOLDABLE.fold_left(tree, 0,
                              [](int acc, int x) { return acc * 10 + x; });
}

template <class TREE, const auto &FOLDABLE = smd::foldable_typeclass<TREE>>
auto fold_right_with_nttp_lookup(const TREE &tree) {
    return FOLDABLE.fold_right(tree, 0,
                               [](int x, int acc) { return x * 10 + acc; });
}

} // namespace

TEST_CASE("FixTreeFoldableTest - Length") {
    using Tree = smd::tree::FixTree<int>;
    auto tree =
        Tree::branch(Tree::leaf(1), Tree::branch(Tree::leaf(2), Tree::leaf(3)));

    const auto &foldable = smd::foldable_typeclass<Tree>;
    CHECK(foldable.length(tree) == 3U);
}

TEST_CASE("FixTreeFoldableTest - FoldMapSum") {
    using Tree = smd::tree::FixTree<int>;
    auto tree =
        Tree::branch(Tree::leaf(1), Tree::branch(Tree::leaf(2), Tree::leaf(3)));

    const auto &foldable = smd::foldable_typeclass<Tree>;
    const auto sum = foldable.fold_map([](int x) { return x; }, tree);
    CHECK(sum == 6);
}

TEST_CASE("FixTreeFoldableTest - FoldMapWithExplicitObject") {
    using Tree = smd::tree::FixTree<int>;
    auto tree =
        Tree::branch(Tree::leaf(1), Tree::branch(Tree::leaf(2), Tree::leaf(3)));

    const auto &foldable = smd::foldable_typeclass<Tree>;
    const auto sum = foldable.fold_map([](int x) { return x; }, tree);
    CHECK(sum == 6);
}

TEST_CASE("FixTreeFoldableTest - FoldMapWithNttpLookup") {
    using Tree = smd::tree::FixTree<int>;
    auto tree =
        Tree::branch(Tree::leaf(1), Tree::branch(Tree::leaf(2), Tree::leaf(3)));

    CHECK(sum_with_nttp_lookup(tree) == 6);
}

TEST_CASE("FixTreeFoldableTest - FoldLeftAndRight") {
    using Tree = smd::tree::FixTree<int>;
    auto tree =
        Tree::branch(Tree::leaf(1), Tree::branch(Tree::leaf(2), Tree::leaf(3)));
    const auto &foldable = smd::foldable_typeclass<Tree>;

    const auto left = foldable.fold_left(
        tree, 0, [](int acc, int x) { return acc * 10 + x; });
    const auto right = foldable.fold_right(
        tree, 0, [](int x, int acc) { return x * 10 + acc; });

    CHECK(left == 123);
    CHECK(right == 60);
}

TEST_CASE("FixTreeFoldableTest - FoldLeftRightWithExplicitObject") {
    using Tree = smd::tree::FixTree<int>;
    auto tree =
        Tree::branch(Tree::leaf(1), Tree::branch(Tree::leaf(2), Tree::leaf(3)));

    const auto &foldable = smd::foldable_typeclass<Tree>;
    const auto left = foldable.fold_left(
        tree, 0, [](int acc, int x) { return acc * 10 + x; });
    const auto right = foldable.fold_right(
        tree, 0, [](int x, int acc) { return x * 10 + acc; });

    CHECK(left == 123);
    CHECK(right == 60);
}

TEST_CASE("FixTreeFoldableTest - FoldLeftRightWithNttpLookup") {
    using Tree = smd::tree::FixTree<int>;
    auto tree =
        Tree::branch(Tree::leaf(1), Tree::branch(Tree::leaf(2), Tree::leaf(3)));

    CHECK(fold_left_with_nttp_lookup(tree) == 123);
    CHECK(fold_right_with_nttp_lookup(tree) == 60);
}

TEST_CASE("FixTreeFoldableTest - PredicatesAndFind") {
    using Tree = smd::tree::FixTree<int>;
    auto tree =
        Tree::branch(Tree::leaf(1), Tree::branch(Tree::leaf(2), Tree::leaf(3)));
    const auto &foldable = smd::foldable_typeclass<Tree>;

    CHECK(foldable.any_of(tree, [](int x) { return x == 2; }));
    CHECK(foldable.all_of(tree, [](int x) { return x > 0; }));
    CHECK_FALSE(foldable.empty(tree));

    auto found = foldable.find_first(tree, [](int x) { return x > 1; });
    REQUIRE(found.has_value());
    CHECK(*found == 2);
}

TEST_CASE("FixTreeTest - CoreConstructionAndAccess") {
    using Tree = smd::tree::FixTree<int>;

    auto l = Tree::leaf(4);
    CHECK(l.is_leaf());
    CHECK(l.value() == 4);

    auto via_node = Tree::node(Tree::leaf(1), Tree::leaf(2));
    CHECK_FALSE(via_node.is_leaf());
    CHECK(via_node.left().is_leaf());
    CHECK(via_node.right().is_leaf());
    CHECK(via_node.left().value() == 1);
    CHECK(via_node.right().value() == 2);

    auto via_branch = Tree::branch(Tree::leaf(7), Tree::leaf(8));
    CHECK_FALSE(via_branch.is_leaf());
    CHECK(via_branch.left().value() == 7);
    CHECK(via_branch.right().value() == 8);
}
