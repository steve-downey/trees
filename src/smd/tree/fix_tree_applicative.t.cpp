#include <smd/tree/fix_tree.hpp>
#include <smd/tree/fix_tree_applicative.hpp>

#include <gtest/gtest.h>

TEST(FixTreeApplicativeTest, InvokeDistributesLeafOverShape)
{
    using Tree = smd::tree::FixTree<int>;
    auto scalar = Tree::leaf(10);
    auto shaped = Tree::node(Tree::leaf(1), Tree::leaf(2));

    const auto& applicative = smd::applicative_typeclass<Tree>;
    auto summed = applicative.invoke(
        [](int lhs, int rhs) { return lhs + rhs; },
        scalar,
        shaped);

    ASSERT_FALSE(summed.is_leaf());
    EXPECT_EQ(summed.left().value(), 11);
    EXPECT_EQ(summed.right().value(), 12);
}
