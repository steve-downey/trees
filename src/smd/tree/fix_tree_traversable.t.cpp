#include <smd/tree/fix_tree.hpp>
#include <smd/tree/fix_tree.hpp>  // Re-inclusion check
#include <smd/tree/fix_tree_traversable.hpp>

#include <catch2/catch_test_macros.hpp>

#include <beman/optional/optional.hpp>

#include <optional>

namespace {

struct NonNegativePlusOne {
    auto operator()(int x) const -> std::optional<int>
    {
        return x >= 0 ? std::optional<int>{x + 1} : std::optional<int>{};
    }
};

struct TimesTwo {
    auto operator()(int x) const -> std::optional<int>
    {
        return std::optional<int>{x * 2};
    }
};

struct MinusTwo {
    auto operator()(int x) const -> std::optional<int>
    {
        return std::optional<int>{x - 2};
    }
};

struct TimesFiveBeman {
    auto operator()(int x) const -> beman::optional::optional<int>
    {
        return beman::optional::optional<int>{x * 5};
    }
};

}  // namespace

TEST_CASE("FixTreeTraversableTest - TraverseOptionalSuccess")
{
    using Tree = smd::tree::FixTree<int>;
    auto tree = Tree::branch(Tree::leaf(1), Tree::leaf(2));
    const auto& traversable = smd::traversable_typeclass<Tree>;

    auto traversed = smd::traverse(NonNegativePlusOne{}, tree);

    REQUIRE(traversed.has_value());
    CHECK(traversed->left().value() == 2);
    CHECK(traversed->right().value() == 3);
}

TEST_CASE("FixTreeTraversableTest - TraverseOptionalFailure")
{
    using Tree = smd::tree::FixTree<int>;
    auto tree = Tree::branch(Tree::leaf(1), Tree::leaf(-2));
    const auto& traversable = smd::traversable_typeclass<Tree>;

    auto traversed = smd::traverse(NonNegativePlusOne{}, tree);

    CHECK_FALSE(traversed.has_value());
}

TEST_CASE("FixTreeTraversableTest - ForEachOptionalSuccess")
{
    using Tree = smd::tree::FixTree<int>;
    auto tree = Tree::branch(Tree::leaf(3), Tree::leaf(4));
    const auto& traversable = smd::traversable_typeclass<Tree>;

    auto traversed = traversable.for_each(tree, TimesTwo{});

    REQUIRE(traversed.has_value());
    CHECK(traversed->left().value() == 6);
    CHECK(traversed->right().value() == 8);
}

TEST_CASE("FixTreeTraversableTest - TraverseLeaf")
{
    using Tree = smd::tree::FixTree<int>;
    auto tree = Tree::leaf(9);
    const auto& traversable = smd::traversable_typeclass<Tree>;

    auto traversed = smd::traverse(MinusTwo{}, tree);

    REQUIRE(traversed.has_value());
    REQUIRE(traversed->is_leaf());
    CHECK(traversed->value() == 7);
}

TEST_CASE("FixTreeTraversableTest - TraverseBemanOptional")
{
    using Tree = smd::tree::FixTree<int>;
    auto tree = Tree::branch(Tree::leaf(2), Tree::leaf(3));
    const auto& traversable = smd::traversable_typeclass<Tree>;

    auto traversed = smd::traverse(TimesFiveBeman{}, tree);

    REQUIRE(traversed.has_value());
    CHECK(traversed->left().value() == 10);
    CHECK(traversed->right().value() == 15);
}
