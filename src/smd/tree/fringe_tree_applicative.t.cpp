#include <smd/tree/fringe_tree.hpp>
#include <smd/tree/fringe_tree.hpp>  // Re-inclusion check
#include <smd/tree/fringe_tree_applicative.hpp>

#include <catch2/catch_test_macros.hpp>

#include <vector>

TEST_CASE("FringeTreeApplicativeTest - Invoke")
{
    using Tree = smd::tree::FringeTree<int>;
    auto lhs = Tree::branch(Tree::leaf(1), Tree::leaf(2));
    auto rhs = Tree::branch(Tree::leaf(10), Tree::leaf(20));

    const auto& applicative = smd::applicative_typeclass<Tree>;
    auto summed = applicative.invoke([](int a, int b) { return a + b; }, lhs, rhs);

    CHECK(summed.flatten() == (std::vector<int>{11, 22}));
}

TEST_CASE("FringeTreeApplicativeTest - ApplyEmptyArgumentsOrFunctions")
{
    using Tree = smd::tree::FringeTree<int>;
    const auto& applicative = smd::applicative_typeclass<Tree>;

    auto fs = smd::tree::FringeTree<int(*)(int)>::leaf(+[](int x) { return x + 1; });

    auto empty_args = applicative.apply(fs, Tree::empty());
    CHECK(empty_args.is_empty());

    auto empty_functions = applicative.apply(smd::tree::FringeTree<int(*)(int)>::empty(), Tree::leaf(1));
    CHECK(empty_functions.is_empty());
}

TEST_CASE("FringeTreeApplicativeTest - ApplyDistributesAcrossShapes")
{
    using Tree = smd::tree::FringeTree<int>;
    const auto& applicative = smd::applicative_typeclass<Tree>;

    auto fs_leaf = smd::tree::FringeTree<int(*)(int)>::leaf(+[](int x) { return x * 10; });
    auto args_tree = Tree::branch(Tree::leaf(1), Tree::leaf(2));
    auto distributed = applicative.apply(fs_leaf, args_tree);
    CHECK(distributed.flatten() == (std::vector<int>{10, 20}));

    auto fs_tree = smd::tree::FringeTree<int(*)(int)>::branch(
        smd::tree::FringeTree<int(*)(int)>::leaf(+[](int x) { return x + 2; }),
        smd::tree::FringeTree<int(*)(int)>::leaf(+[](int x) { return x + 3; }));
    auto applied_to_leaf = applicative.apply(fs_tree, Tree::leaf(5));
    CHECK(applied_to_leaf.flatten() == (std::vector<int>{7, 8}));
}
