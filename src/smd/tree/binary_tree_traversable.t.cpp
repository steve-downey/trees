#include <smd/tree/binary_tree.hpp>
#include <smd/tree/binary_tree.hpp> // Re-inclusion check
#include <smd/tree/binary_tree_traversable.hpp>

#include <catch2/catch_test_macros.hpp>

#include <optional>

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

struct PlusOne {
    auto operator()(int x) const -> std::optional<int> {
        return std::optional<int>{x + 1};
    }
};

struct NonNegativeIdentity {
    auto operator()(int x) const -> std::optional<int> {
        return x >= 0 ? std::optional<int>{x} : std::optional<int>{};
    }
};

} // namespace

TEST_CASE("BinaryTreeTraversableTest - TraverseOptionalPreservesShape") {
    using Tree = smd::tree::BinaryTree<int>;
    auto tree =
        Tree::from_children_ptrs(2, Tree::make_ptr(Tree::leaf(1)),
                                 Tree::make_ptr(Tree::from_children_ptrs(
                                     3, {}, Tree::make_ptr(Tree::leaf(4)))));

    auto traversed = smd::traverse(PositiveTimesTen{}, tree);

    REQUIRE(traversed.has_value());
    CHECK(traversed->value() == 20);
    REQUIRE(traversed->has_left());
    CHECK(traversed->left().value() == 10);
    REQUIRE(traversed->has_right());
    CHECK(traversed->right().value() == 30);
    CHECK_FALSE(traversed->right().has_left());
    REQUIRE(traversed->right().has_right());
    CHECK(traversed->right().right().value() == 40);
}

TEST_CASE(
    "BinaryTreeTraversableTest - TraverseOptionalDoesNotDuplicateRootEffect") {
    using Tree = smd::tree::BinaryTree<int>;
    auto tree = Tree::from_children_ptrs(2, {}, Tree::make_ptr(Tree::leaf(5)));

    int invocations = 0;
    auto traversed = smd::traverse(
        [&](int x) -> std::optional<int> {
            ++invocations;
            return TimesTen{}(x);
        },
        tree);

    REQUIRE(traversed.has_value());
    CHECK(invocations == 2);
    CHECK(traversed->value() == 20);
    CHECK_FALSE(traversed->has_left());
    REQUIRE(traversed->has_right());
    CHECK(traversed->right().value() == 50);
}

TEST_CASE("BinaryTreeTraversableTest - TraverseOptionalLeaf") {
    using Tree = smd::tree::BinaryTree<int>;
    auto tree = Tree::leaf(9);

    auto traversed = smd::traverse(PlusOne{}, tree);

    REQUIRE(traversed.has_value());
    CHECK(traversed->value() == 10);
    CHECK_FALSE(traversed->has_left());
    CHECK_FALSE(traversed->has_right());
}

TEST_CASE("BinaryTreeTraversableTest - TraverseOptionalLeftOnly") {
    using Tree = smd::tree::BinaryTree<int>;
    auto tree = Tree::from_children_ptrs(2, Tree::make_ptr(Tree::leaf(3)), {});

    auto traversed = smd::traverse(TimesTen{}, tree);

    REQUIRE(traversed.has_value());
    CHECK(traversed->value() == 20);
    REQUIRE(traversed->has_left());
    CHECK_FALSE(traversed->has_right());
    CHECK(traversed->left().value() == 30);
}

TEST_CASE("BinaryTreeTraversableTest - TraverseOptionalFailure") {
    using Tree = smd::tree::BinaryTree<int>;
    auto tree = Tree::from_children_ptrs(2, Tree::make_ptr(Tree::leaf(-1)), {});

    auto traversed = smd::traverse(NonNegativeIdentity{}, tree);

    CHECK_FALSE(traversed.has_value());
}
