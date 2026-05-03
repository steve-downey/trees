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

    struct View {
        T d_value;
        FringeTree d_rest;
    };

    static auto empty() -> FringeTree { return FringeTree(Empty{}); }

    static auto leaf(T value) -> FringeTree {
        return FringeTree(Leaf{std::move(value)});
    }

    static auto branch(FringeTree left, FringeTree right) -> FringeTree {
        auto left_ptr = std::make_shared<FringeTree>(std::move(left));
        auto right_ptr = std::make_shared<FringeTree>(std::move(right));
        auto measure = left_ptr->measure() + right_ptr->measure();
        return FringeTree(
            Branch{measure, std::move(left_ptr), std::move(right_ptr)});
    }

    auto is_empty() const -> bool {
        return std::holds_alternative<Empty>(d_data);
    }
    auto is_leaf() const -> bool {
        return std::holds_alternative<Leaf>(d_data);
    }
    auto is_branch() const -> bool {
        return std::holds_alternative<Branch>(d_data);
    }

    auto measure() const -> std::size_t {
        if (is_empty()) {
            return 0U;
        }
        if (is_leaf()) {
            return 1U;
        }
        return std::get<Branch>(d_data).d_measure;
    }

    auto value() const -> const T & {
        assert(is_leaf());
        return std::get<Leaf>(d_data).d_value;
    }

    auto left() const -> const FringeTree & {
        assert(is_branch());
        return *std::get<Branch>(d_data).d_left;
    }

    auto right() const -> const FringeTree & {
        assert(is_branch());
        return *std::get<Branch>(d_data).d_right;
    }

    auto breadth() const -> std::size_t { return measure(); }

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

    static auto prepend(T value, const FringeTree &tree) -> FringeTree {
        return concat(leaf(std::move(value)), tree);
    }

    static auto append(const FringeTree &tree, T value) -> FringeTree {
        return concat(tree, leaf(std::move(value)));
    }

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

    auto head() const -> T {
        auto v = view_l();
        assert(v.has_value());
        return v->d_value;
    }

    auto tail() const -> FringeTree {
        auto v = view_l();
        return v.has_value() ? v->d_rest : empty();
    }

    auto last() const -> T {
        auto v = view_r();
        assert(v.has_value());
        return v->d_value;
    }

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
