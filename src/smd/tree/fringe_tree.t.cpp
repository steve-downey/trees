#include <smd/tree/fringe_tree.hpp>
#include <smd/tree/fringe_tree.hpp>  // Re-inclusion check
#include <smd/tree/fringe_tree_foldable.hpp>
#include <smd/typeclass/foldable.hpp>

#include <catch2/catch_test_macros.hpp>

#include <vector>

TEST_CASE("FringeTreeTest - EmptyLeafAndPredicates")
{
    using Tree = smd::tree::FringeTree<int>;

    auto empty = Tree::empty();
    CHECK(empty.is_empty());
    CHECK_FALSE(empty.is_leaf());
    CHECK_FALSE(empty.is_branch());
    CHECK(empty.measure() == 0U);
    CHECK(empty.breadth() == 0U);
    CHECK(empty.depth() == 0U);
    CHECK(empty.flatten() == (std::vector<int>{}));
    CHECK_FALSE(empty.view_l().has_value());
    CHECK_FALSE(empty.view_r().has_value());

    auto single = Tree::leaf(42);
    CHECK_FALSE(single.is_empty());
    CHECK(single.is_leaf());
    CHECK_FALSE(single.is_branch());
    CHECK(single.measure() == 1U);
    CHECK(single.value() == 42);
    CHECK(single.flatten() == (std::vector<int>{42}));
}

TEST_CASE("FringeTreeTest - BranchLeftRightAndMemberStyleOperations")
{
    using Tree = smd::tree::FringeTree<int>;

    auto left = Tree::branch(Tree::leaf(1), Tree::leaf(2));
    auto right = Tree::branch(Tree::leaf(3), Tree::leaf(4));
    auto tree = Tree::branch(left, right);

    REQUIRE(tree.is_branch());
    CHECK(tree.left().flatten() == (std::vector<int>{1, 2}));
    CHECK(tree.right().flatten() == (std::vector<int>{3, 4}));

    auto prepended = Tree::prepend(0, tree);
    CHECK(prepended.flatten() == (std::vector<int>{0, 1, 2, 3, 4}));

    auto appended = Tree::append(tree, 5);
    CHECK(appended.flatten() == (std::vector<int>{1, 2, 3, 4, 5}));

    auto concatenated = Tree::concat(left, right);
    CHECK(concatenated.flatten() == (std::vector<int>{1, 2, 3, 4}));
}

TEST_CASE("FringeTreeTest - SingletonViewsAndEmptyTailInit")
{
    using Tree = smd::tree::FringeTree<int>;

    auto single = Tree::leaf(7);
    auto left = single.view_l();
    REQUIRE(left.has_value());
    CHECK(left->d_value == 7);
    CHECK(left->d_rest.is_empty());

    auto right = single.view_r();
    REQUIRE(right.has_value());
    CHECK(right->d_value == 7);
    CHECK(right->d_rest.is_empty());

    auto empty = Tree::empty();
    CHECK(empty.tail().is_empty());
    CHECK(empty.init().is_empty());
}

TEST_CASE("FringeTreeTest - BasicMeasureDepthFlatten")
{
    using Tree = smd::tree::FringeTree<int>;

    auto tree = Tree::branch(
        Tree::branch(Tree::leaf(1), Tree::leaf(2)),
        Tree::leaf(3));

    CHECK(tree.measure() == 3U);
    CHECK(tree.breadth() == 3U);
    CHECK(tree.depth() == 3U);
    CHECK(tree.flatten() == (std::vector<int>{1, 2, 3}));
}

TEST_CASE("FringeTreeTest - ViewsAndListOps")
{
    using Tree = smd::tree::FringeTree<int>;

    auto tree = Tree::branch(
        Tree::branch(Tree::leaf(1), Tree::leaf(2)),
        Tree::leaf(3));

    auto left_view = tree.view_l();
    REQUIRE(left_view.has_value());
    CHECK(left_view->d_value == 1);
    CHECK(left_view->d_rest.flatten() == (std::vector<int>{2, 3}));

    auto right_view = tree.view_r();
    REQUIRE(right_view.has_value());
    CHECK(right_view->d_value == 3);
    CHECK(right_view->d_rest.flatten() == (std::vector<int>{1, 2}));

    CHECK(tree.head() == 1);
    CHECK(tree.last() == 3);
    CHECK(tree.tail().flatten() == (std::vector<int>{2, 3}));
    CHECK(tree.init().flatten() == (std::vector<int>{1, 2}));
}

TEST_CASE("FringeTreeTest - PrependAppendConcat")
{
    using Tree = smd::tree::FringeTree<int>;

    auto tree = Tree::branch(Tree::leaf(1), Tree::leaf(2));

    auto prepended = Tree::prepend(0, tree);
    CHECK(prepended.flatten() == (std::vector<int>{0, 1, 2}));

    auto appended = Tree::append(tree, 3);
    CHECK(appended.flatten() == (std::vector<int>{1, 2, 3}));

    auto concatenated = Tree::concat(tree, tree);
    CHECK(concatenated.flatten() == (std::vector<int>{1, 2, 1, 2}));
}

TEST_CASE("FringeTreeTest - FoldableIntegration")
{
    using Tree = smd::tree::FringeTree<int>;

    auto tree = Tree::branch(
        Tree::branch(Tree::leaf(1), Tree::leaf(2)),
        Tree::leaf(3));

    const auto& foldable = smd::foldable_typeclass<Tree>;
    CHECK(foldable.length(tree) == 3U);
    CHECK(foldable.fold_map([](int x) { return x; }, tree) == 6);
    CHECK(foldable.to_vector(tree) == (std::vector<int>{1, 2, 3}));
}
