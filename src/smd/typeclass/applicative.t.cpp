#include <smd/typeclass/applicative.hpp>
#include <smd/tree/binary_tree.hpp>
#include <smd/tree/binary_tree_applicative.hpp>

#include <gtest/gtest.h>

#include <optional>

TEST(ApplicativeTypeclassTest, PureOptional)
{
    const auto& applicative = smd::applicative_typeclass<std::optional<int> >;
    auto lifted = applicative.pure(7);
    ASSERT_TRUE(lifted.has_value());
    EXPECT_EQ(*lifted, 7);
}

TEST(ApplicativeTypeclassTest, ApplyOptional)
{
    std::optional<int (*)(int)> function{+[](int x) { return x + 3; }};
    std::optional<int> argument{4};
    const auto& applicative = smd::applicative_typeclass<std::optional<int (*)(int)> >;

    auto result = applicative.apply(function, argument);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 7);
}

TEST(ApplicativeTypeclassTest, InvokeOptional)
{
    std::optional<int> ax{10};
    std::optional<int> ay{5};
    const auto& applicative = smd::applicative_typeclass<std::optional<int> >;

    auto result = applicative.invoke([](int a, int b) { return a - b; }, ax, ay);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 5);
}

TEST(ApplicativeTypeclassTest, InvokeOptionalTernaryUsesPartialApplication)
{
    std::optional<int> ax{2};
    std::optional<int> ay{3};
    std::optional<int> az{4};
    const auto& applicative = smd::applicative_typeclass<std::optional<int> >;

    auto result = applicative.invoke(
        [](int a, int b, int c) { return a * b + c; },
        ax,
        ay,
        az);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 10);
}

TEST(ApplicativeTypeclassTest, ApplyPureOptionalTernary)
{
    // 6e8bde7b-a9f1-4c98-8f1a-807d9ee0a93b
    std::optional<int> ax{2};
    std::optional<int> ay{3};
    std::optional<int> az{4};
    const auto& applicative = smd::applicative_typeclass<std::optional<int> >;

    auto result = applicative.apply_pure(
        [](int a, int b, int c) { return a * b + c; },
        ax,
        ay,
        az);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 10);
    // 6e8bde7b-a9f1-4c98-8f1a-807d9ee0a93b end
}

TEST(ApplicativeTypeclassTest, MapOptional)
{
    std::optional<int> value{21};
    const auto& applicative = smd::applicative_typeclass<std::optional<int> >;

    auto result = applicative.map([](int x) { return x * 2; }, value);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 42);
}

TEST(ApplicativeTypeclassTest, InvokeWithExplicitMap)
{
    std::optional<int> ax{10};
    std::optional<int> ay{5};
    const auto& default_applicative = smd::applicative_typeclass<std::optional<int> >;
    const auto& optional_applicative = smd::applicative_typeclass<std::optional<int> >;

    auto result = default_applicative.invoke_with(
        optional_applicative,
        [](int a, int b) { return a + b; },
        ax,
        ay);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 15);
}

TEST(ApplicativeTypeclassTest, BinaryTreeInvokeAndApply)
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

        EXPECT_EQ(summed.value(), 13);
        ASSERT_TRUE(summed.has_left());
        ASSERT_TRUE(summed.has_right());
        EXPECT_EQ(summed.left().value(), 5);
        EXPECT_EQ(summed.right().value(), 7);

        auto fs = smd::tree::BinaryTree<int(*)(int)>::from_children_ptrs(
            +[](int x) { return x * 2; },
            smd::tree::BinaryTree<int(*)(int)>::make_ptr(
                smd::tree::BinaryTree<int(*)(int)>::leaf(+[](int x) { return x + 1; })),
            {});
        auto applied = applicative.apply(fs, lhs);
        EXPECT_EQ(applied.value(), 20);
        ASSERT_TRUE(applied.has_left());
        EXPECT_EQ(applied.left().value(), 2);
        EXPECT_FALSE(applied.has_right());
}
