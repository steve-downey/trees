#include <smd/tree/fix_tree.hpp>
#include <smd/tree/fix_tree_traversable.hpp>

#include <catch2/catch_test_macros.hpp>

#include <optional>

TEST_CASE("FixTreeTraversableTest - TraverseOptionalSuccess")
{
    using Tree = smd::tree::FixTree<int>;
    auto tree = Tree::branch(Tree::leaf(1), Tree::leaf(2));
    const auto& traversable = smd::traversable_typeclass<Tree>;

    auto traversed = traversable.traverse(
        [](int x) -> std::optional<int> {
            return x >= 0 ? std::optional<int>{x + 1} : std::optional<int>{};
        },
        tree);

    REQUIRE(traversed.has_value());
    CHECK(traversed->left().value() == 2);
    CHECK(traversed->right().value() == 3);
}

TEST_CASE("FixTreeTraversableTest - TraverseOptionalFailure")
{
    using Tree = smd::tree::FixTree<int>;
    auto tree = Tree::branch(Tree::leaf(1), Tree::leaf(-2));
    const auto& traversable = smd::traversable_typeclass<Tree>;

    auto traversed = traversable.traverse(
        [](int x) -> std::optional<int> {
            return x >= 0 ? std::optional<int>{x + 1} : std::optional<int>{};
        },
        tree);

    CHECK_FALSE(traversed.has_value());
}

TEST_CASE("FixTreeTraversableTest - ForEachOptionalSuccess")
{
    using Tree = smd::tree::FixTree<int>;
    auto tree = Tree::branch(Tree::leaf(3), Tree::leaf(4));
    const auto& traversable = smd::traversable_typeclass<Tree>;

    auto traversed = traversable.for_each(tree, [](int x) -> std::optional<int> {
        return std::optional<int>{x * 2};
    });

    REQUIRE(traversed.has_value());
    CHECK(traversed->left().value() == 6);
    CHECK(traversed->right().value() == 8);
}
