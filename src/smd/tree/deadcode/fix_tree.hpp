// src/smd/tree/fix_tree.hpp                                          -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_FIX_TREE
#define INCLUDED_SMD_TREE_FIX_TREE
#include <memory>
#include <variant>
namespace smd::tree {
template <class T>
class FixTree {
    struct Leaf {
        T v;
    };
    struct Node {
        std::shared_ptr<FixTree> l, r;
    };
    std::variant<Leaf, Node> d;

  public:
    using value_type = T;

    static FixTree leaf(T v) { return FixTree(Leaf{v}); }
    static FixTree node(FixTree a, FixTree b) {
        return FixTree(
            Node{std::make_shared<FixTree>(a), std::make_shared<FixTree>(b)});
    }
    static FixTree branch(FixTree a, FixTree b) {
        return node(std::move(a), std::move(b));
    }
    bool is_leaf() const { return std::holds_alternative<Leaf>(d); }
    const T &value() const { return std::get<Leaf>(d).v; }
    const FixTree &left() const { return *std::get<Node>(d).l; }
    const FixTree &right() const { return *std::get<Node>(d).r; }

  private:
    FixTree(Leaf l) : d(l) {}
    FixTree(Node n) : d(n) {}
};
} // namespace smd::tree
#endif
