// src/smd/tree/fringe_tree.hpp                                       -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_FRINGE_TREE
#define INCLUDED_SMD_TREE_FRINGE_TREE

#include <cassert>
#include <cstddef>
#include <memory>
#include <optional>
#include <utility>
#include <variant>
#include <vector>

namespace smd::tree {

/** Persistent balanced binary tree representing a sequence of leaf values.
 *
 * The tree has three variants: Empty (no elements), Leaf (one element), and
 * Branch (two subtrees). Values live exclusively at leaves; branches carry
 * only structure and a cached measure (leaf count). This design supports
 * efficient deque operations (cons/snoc/head/tail/last/init) and O(1) concat
 * via structural sharing.
 * @tparam T element type stored at leaves
 */
template <class T>
class FringeTree {
    struct Empty {};
    struct Leaf {
        T d_value;
    };
    struct Branch {
        std::size_t d_measure;
        std::shared_ptr<FringeTree> d_left;
        std::shared_ptr<FringeTree> d_right;
    };

    std::variant<Empty, Leaf, Branch> d_data;

  public:
    using value_type = T;

    /** A deconstructed view of the front or back element plus the remaining
     * tree. */
    struct View {
        T d_value;
        FringeTree d_rest;
    };

    /** Construct the empty tree (no elements). */
    static auto empty() -> FringeTree { return FringeTree(Empty{}); }

    /** Construct a single-element tree containing @p value. */
    static auto leaf(T value) -> FringeTree {
        return FringeTree(Leaf{std::move(value)});
    }

    /** Construct a branch joining two non-empty subtrees. */
    static auto branch(FringeTree left, FringeTree right) -> FringeTree {
        auto left_ptr = std::make_shared<FringeTree>(std::move(left));
        auto right_ptr = std::make_shared<FringeTree>(std::move(right));
        auto measure = left_ptr->measure() + right_ptr->measure();
        return FringeTree(
            Branch{measure, std::move(left_ptr), std::move(right_ptr)});
    }

    /** True when the tree is empty. */
    auto is_empty() const -> bool {
        return std::holds_alternative<Empty>(d_data);
    }
    /** True when the tree is a single leaf. */
    auto is_leaf() const -> bool {
        return std::holds_alternative<Leaf>(d_data);
    }
    /** True when the tree is an internal branch. */
    auto is_branch() const -> bool {
        return std::holds_alternative<Branch>(d_data);
    }

    /** Number of leaf elements in the tree (cached at branches). */
    auto measure() const -> std::size_t {
        if (is_empty()) {
            return 0U;
        }
        if (is_leaf()) {
            return 1U;
        }
        return std::get<Branch>(d_data).d_measure;
    }

    /** Return the leaf value; precondition: is_leaf(). */
    auto value() const -> const T & {
        assert(is_leaf());
        return std::get<Leaf>(d_data).d_value;
    }

    /** Return the left subtree; precondition: is_branch(). */
    auto left() const -> const FringeTree & {
        assert(is_branch());
        return *std::get<Branch>(d_data).d_left;
    }

    /** Return the right subtree; precondition: is_branch(). */
    auto right() const -> const FringeTree & {
        assert(is_branch());
        return *std::get<Branch>(d_data).d_right;
    }

    /** Synonym for measure() — number of leaf elements. */
    auto breadth() const -> std::size_t { return measure(); }

    /** Maximum depth from root to any leaf. */
    auto depth() const -> std::size_t {
        if (is_empty()) {
            return 0U;
        }
        if (is_leaf()) {
            return 1U;
        }
        const auto l = left().depth();
        const auto r = right().depth();
        return ((l > r) ? l : r) + 1U;
    }

    /** Collect all leaf values into a vector in left-to-right order. */
    auto flatten() const -> std::vector<T> {
        if (is_empty()) {
            return {};
        }
        if (is_leaf()) {
            return {value()};
        }

        auto l = left().flatten();
        auto r = right().flatten();
        l.insert(l.end(), r.begin(), r.end());
        return l;
    }

    /** Visit each leaf value left-to-right, calling @p callback on each. */
    template <typename F>
    void for_each(F &&callback) const {
        if (is_empty()) {
            return;
        }
        if (is_leaf()) {
            callback(value());
            return;
        }
        left().for_each(callback);
        right().for_each(callback);
    }

    /** Concatenate two trees; empty operands are identity elements. */
    static auto concat(const FringeTree &left_tree,
                       const FringeTree &right_tree) -> FringeTree {
        if (left_tree.is_empty()) {
            return right_tree;
        }
        if (right_tree.is_empty()) {
            return left_tree;
        }
        return branch(left_tree, right_tree);
    }

    /** Prepend @p value to @p tree (cons / push-front). */
    static auto prepend(T value, const FringeTree &tree) -> FringeTree {
        return concat(leaf(std::move(value)), tree);
    }

    /** Append @p value to @p tree (snoc / push-back), static form. */
    static auto append(const FringeTree &tree, T value) -> FringeTree {
        return concat(tree, leaf(std::move(value)));
    }

    /** Return a new tree with @p x prepended (deque cons). */
    auto cons(T x) const -> FringeTree {
        return concat(leaf(std::move(x)), *this);
    }

    /** Return a new tree with @p x appended (deque snoc). */
    auto snoc(T x) const -> FringeTree {
        return concat(*this, leaf(std::move(x)));
    }

    /** Return the concatenation of this tree with @p other. */
    auto append(const FringeTree &other) const -> FringeTree {
        return concat(*this, other);
    }

    /** Build a tree from a vector, appending elements left-to-right. */
    static auto from_sequence(std::vector<T> values) -> FringeTree {
        auto result = empty();
        for (auto &v : values) {
            result = result.snoc(std::move(v));
        }
        return result;
    }

    /**
     * @brief Destructure from the left: returns the front element and the rest.
     * @return nullopt if empty; otherwise View{front, tail}
     */
    auto view_l() const -> std::optional<View> {
        if (is_empty()) {
            return std::nullopt;
        }
        if (is_leaf()) {
            return View{value(), empty()};
        }

        auto left_view = left().view_l();
        if (left_view.has_value()) {
            return View{left_view->d_value, concat(left_view->d_rest, right())};
        }

        return right().view_l();
    }

    /**
     * @brief Destructure from the right: returns the back element and the rest.
     * @return nullopt if empty; otherwise View{back, init}
     */
    auto view_r() const -> std::optional<View> {
        if (is_empty()) {
            return std::nullopt;
        }
        if (is_leaf()) {
            return View{value(), empty()};
        }

        auto right_view = right().view_r();
        if (right_view.has_value()) {
            return View{right_view->d_value,
                        concat(left(), right_view->d_rest)};
        }

        return left().view_r();
    }

    /** Return the first (leftmost) leaf value; precondition: non-empty. */
    auto head() const -> T {
        auto v = view_l();
        assert(v.has_value());
        return v->d_value;
    }

    /** Return all but the first leaf; returns empty() when called on empty. */
    auto tail() const -> FringeTree {
        auto v = view_l();
        return v.has_value() ? v->d_rest : empty();
    }

    /** Return the last (rightmost) leaf value; precondition: non-empty. */
    auto last() const -> T {
        auto v = view_r();
        assert(v.has_value());
        return v->d_value;
    }

    /** Return all but the last leaf; returns empty() when called on empty. */
    auto init() const -> FringeTree {
        auto v = view_r();
        return v.has_value() ? v->d_rest : empty();
    }

  private:
    explicit FringeTree(Empty e) : d_data(std::move(e)) {}

    explicit FringeTree(Leaf l) : d_data(std::move(l)) {}

    explicit FringeTree(Branch b) : d_data(std::move(b)) {}
};

} // namespace smd::tree

#endif
