#include <smd/tree/fringe_tree.hpp>
#include <smd/tree/fringe_tree.hpp> // Re-inclusion check
#include <smd/tree/fringe_tree_traversable.hpp>

#include <catch2/catch_test_macros.hpp>

#include <beman/optional/optional.hpp>

#include <optional>
#include <vector>

namespace {

struct PositiveTimesTen {
    auto operator()(int x) const -> std::optional<int> {
        return x > 0 ? std::optional<int>{x * 10} : std::optional<int>{};
    }
};

struct TimesTen {
    auto operator()(int x) const -> std::optional<int> {
        return std::optional<int>{x * 10};
    }
};

struct NonNegativeIdentity {
    auto operator()(int x) const -> std::optional<int> {
        return x >= 0 ? std::optional<int>{x} : std::optional<int>{};
    }
};

struct PlusOne {
    auto operator()(int x) const -> std::optional<int> {
        return std::optional<int>{x + 1};
    }
};

struct TimesTenBeman {
    auto operator()(int x) const -> beman::optional::optional<int> {
        return beman::optional::optional<int>{x * 10};
    }
};

struct PlusSevenBeman {
    auto operator()(int x) const -> beman::optional::optional<int> {
        return beman::optional::optional<int>{x + 7};
    }
};

} // namespace

TEST_CASE("FringeTreeTraversableTest - TraverseOptional") {
    using Tree = smd::tree::FringeTree<int>;
    auto tree =
        Tree::branch(Tree::leaf(1), Tree::branch(Tree::leaf(2), Tree::leaf(3)));

    auto traversed = smd::traverse(PositiveTimesTen{}, tree);

    REQUIRE(traversed.has_value());
    CHECK(traversed->flatten() == (std::vector<int>{10, 20, 30}));
}

TEST_CASE("FringeTreeTraversableTest - TraverseOptionalEmpty") {
    using Tree = smd::tree::FringeTree<int>;
    auto tree = Tree::empty();

    auto traversed = smd::traverse(TimesTen{}, tree);

    REQUIRE(traversed.has_value());
    CHECK(traversed->is_empty());
}

TEST_CASE("FringeTreeTraversableTest - TraverseBemanOptionalEmpty") {
    using Tree = smd::tree::FringeTree<int>;
    auto tree = Tree::empty();

    auto traversed = smd::traverse(TimesTenBeman{}, tree);

    REQUIRE(traversed.has_value());
    CHECK(traversed->is_empty());
}

TEST_CASE("FringeTreeTraversableTest - TraverseOptionalFailure") {
    using Tree = smd::tree::FringeTree<int>;
    auto tree = Tree::branch(Tree::leaf(1), Tree::leaf(-2));

    auto traversed = smd::traverse(NonNegativeIdentity{}, tree);

    CHECK_FALSE(traversed.has_value());
}

TEST_CASE("FringeTreeTraversableTest - TraverseLeaf") {
    using Tree = smd::tree::FringeTree<int>;
    auto tree = Tree::leaf(7);

    auto traversed = smd::traverse(PlusOne{}, tree);

    REQUIRE(traversed.has_value());
    REQUIRE(traversed->is_leaf());
    CHECK(traversed->value() == 8);
}

TEST_CASE("FringeTreeTraversableTest - TraverseBemanOptional") {
    using Tree = smd::tree::FringeTree<int>;
    auto tree = Tree::branch(Tree::leaf(2), Tree::leaf(5));

    auto traversed = smd::traverse(PlusSevenBeman{}, tree);

    REQUIRE(traversed.has_value());
    CHECK(traversed->flatten() == (std::vector<int>{9, 12}));
}
