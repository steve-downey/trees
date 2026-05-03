// src/smd/tree/binary_tree.t.cpp                                     -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <smd/tree/binary_tree.hpp>
#include <smd/tree/binary_tree.hpp> // Re-inclusion check

#include <catch2/catch_test_macros.hpp>

TEST_CASE("BinaryTreeTest - LeafConstruction") {
    auto t = smd::tree::BinaryTree<int>::leaf(42);
    CHECK(t.value() == 42);
    CHECK_FALSE(t.has_left());
    CHECK_FALSE(t.has_right());
}

TEST_CASE("BinaryTreeTest - NodeConstruction") {
    using Tree = smd::tree::BinaryTree<int>;
    auto t = Tree::node(1, Tree::leaf(2), Tree::leaf(3));
    CHECK(t.value() == 1);
    CHECK(t.has_left());
    CHECK(t.has_right());
    CHECK(t.left().value() == 2);
    CHECK(t.right().value() == 3);
}

TEST_CASE("BinaryTreeTest - DeepTree") {
    using Tree = smd::tree::BinaryTree<int>;
    auto t = Tree::node(1, Tree::node(2, Tree::leaf(4), Tree::leaf(5)),
                        Tree::leaf(3));
    CHECK(t.value() == 1);
    CHECK(t.left().value() == 2);
    CHECK(t.left().left().value() == 4);
    CHECK(t.left().right().value() == 5);
    CHECK_FALSE(t.right().has_left());
    CHECK_FALSE(t.right().has_right());
}
