#include <smd/tree/fringe_tree.hpp>
#include <smd/tree/fringe_tree_traversable.hpp>

#include <catch2/catch_test_macros.hpp>

#include <beman/optional/optional.hpp>

#include <optional>
#include <vector>

TEST_CASE("FringeTreeTraversableTest - TraverseOptional")
{
    using Tree = smd::tree::FringeTree<int>;
    auto tree = Tree::branch(Tree::leaf(1), Tree::branch(Tree::leaf(2), Tree::leaf(3)));

    const auto& traversable = smd::traversable_typeclass<Tree>;
    auto traversed = traversable.traverse(
        [](int x) -> std::optional<int> {
            return x > 0 ? std::optional<int>{x * 10} : std::optional<int>{};
        },
        tree);

    REQUIRE(traversed.has_value());
    CHECK(traversed->flatten() == (std::vector<int>{10, 20, 30}));
}

TEST_CASE("FringeTreeTraversableTest - TraverseOptionalEmpty")
{
    using Tree = smd::tree::FringeTree<int>;
    auto tree = Tree::empty();

    const auto& traversable = smd::traversable_typeclass<Tree>;
    auto traversed = traversable.traverse(
        [](int x) -> std::optional<int> {
            return std::optional<int>{x * 10};
        },
        tree);

    REQUIRE(traversed.has_value());
    CHECK(traversed->is_empty());
}

TEST_CASE("FringeTreeTraversableTest - TraverseBemanOptionalEmpty")
{
    using Tree = smd::tree::FringeTree<int>;
    auto tree = Tree::empty();

    const auto& traversable = smd::traversable_typeclass<Tree>;
    auto traversed = traversable.traverse(
        [](int x) -> beman::optional::optional<int> {
            return beman::optional::optional<int>{x * 10};
        },
        tree);

    REQUIRE(traversed.has_value());
    CHECK(traversed->is_empty());
}
