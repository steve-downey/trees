#include <smd/tree/binary_tree.hpp>
#include <smd/tree/binary_tree.hpp>  // Re-inclusion check
#include <smd/tree/binary_tree_applicative.hpp>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("BinaryTreeApplicativeTest - InvokeAndApply")
{
    using Tree = smd::tree::BinaryTree<int>;
    auto lhs = Tree::from_children_ptrs(
        10,
        Tree::make_ptr(Tree::leaf(1)),
        Tree::make_ptr(Tree::leaf(2)));
    auto rhs = Tree::from_children_ptrs(
        3,
        Tree::make_ptr(Tree::leaf(4)),
        Tree::make_ptr(Tree::leaf(5)));

    const auto& applicative = smd::applicative_typeclass<Tree>;
    auto summed = applicative.invoke([](int a, int b) { return a + b; }, lhs, rhs);

    CHECK(summed.value() == 13);
    REQUIRE(summed.has_left());
    REQUIRE(summed.has_right());
    CHECK(summed.left().value() == 5);
    CHECK(summed.right().value() == 7);

    auto fs = smd::tree::BinaryTree<int(*)(int)>::from_children_ptrs(
        +[](int x) { return x * 2; },
        smd::tree::BinaryTree<int(*)(int)>::make_ptr(
            smd::tree::BinaryTree<int(*)(int)>::leaf(+[](int x) { return x + 1; })),
        {});
    auto applied = applicative.apply(fs, lhs);
    CHECK(applied.value() == 20);
    REQUIRE(applied.has_left());
    CHECK(applied.left().value() == 2);
    CHECK_FALSE(applied.has_right());
}

TEST_CASE("BinaryTreeApplicativeTest - PureFunctionDistributesOverArgumentShape")
{
    using Tree = smd::tree::BinaryTree<int>;
    const auto& applicative = smd::applicative_typeclass<Tree>;

    auto fs = smd::tree::BinaryTree<int(*)(int)>::leaf(+[](int x) { return x + 10; });
    auto xs = Tree::from_children_ptrs(
        1,
        Tree::make_ptr(Tree::leaf(2)),
        Tree::make_ptr(Tree::leaf(3)));

    auto applied = applicative.apply(fs, xs);
    CHECK(applied.value() == 11);
    REQUIRE(applied.has_left());
    REQUIRE(applied.has_right());
    CHECK(applied.left().value() == 12);
    CHECK(applied.right().value() == 13);
}

TEST_CASE("BinaryTreeApplicativeTest - FunctionTreeAppliesPointwiseToLeafArgument")
{
    using Tree = smd::tree::BinaryTree<int>;
    const auto& applicative = smd::applicative_typeclass<Tree>;

    auto fs = smd::tree::BinaryTree<int(*)(int)>::from_children_ptrs(
        +[](int x) { return x * 2; },
        smd::tree::BinaryTree<int(*)(int)>::make_ptr(
            smd::tree::BinaryTree<int(*)(int)>::leaf(+[](int x) { return x + 1; })),
        smd::tree::BinaryTree<int(*)(int)>::make_ptr(
            smd::tree::BinaryTree<int(*)(int)>::leaf(+[](int x) { return x - 1; })));

    auto applied = applicative.apply(fs, Tree::leaf(10));
    CHECK(applied.value() == 20);
    REQUIRE(applied.has_left());
    REQUIRE(applied.has_right());
    CHECK(applied.left().value() == 11);
    CHECK(applied.right().value() == 9);
}

TEST_CASE("BinaryTreeApplicativeTest - PairwiseApplyRequiresMatchingChildren")
{
    using Tree = smd::tree::BinaryTree<int>;
    const auto& applicative = smd::applicative_typeclass<Tree>;

    auto fs = smd::tree::BinaryTree<int(*)(int)>::from_children_ptrs(
        +[](int x) { return x + 100; },
        smd::tree::BinaryTree<int(*)(int)>::make_ptr(
            smd::tree::BinaryTree<int(*)(int)>::leaf(+[](int x) { return x + 1; })),
        {});

    auto xs = Tree::from_children_ptrs(
        1,
        {},
        Tree::make_ptr(Tree::leaf(2)));

    auto applied = applicative.apply(fs, xs);
    CHECK(applied.value() == 101);
    CHECK_FALSE(applied.has_left());
    CHECK_FALSE(applied.has_right());
}
