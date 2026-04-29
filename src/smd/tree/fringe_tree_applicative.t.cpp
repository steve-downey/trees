#include <smd/tree/fringe_tree.hpp>
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
