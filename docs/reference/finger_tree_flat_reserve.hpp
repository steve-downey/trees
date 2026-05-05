#ifndef INCLUDE_SMD_TREE_FINGER_TREE_HPP
#define INCLUDE_SMD_TREE_FINGER_TREE_HPP

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <ranges>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

#if defined(__has_include) && __has_include(<smd/typeclass/monoid.hpp>) \
  && __has_include(<smd/typeclass/typeclass_base.hpp>)
#include <smd/typeclass/monoid.hpp>
#else
namespace smd::typeclass {
template <class VALUE_TYPE>
struct Monoid;
}
#endif

namespace smd::tree {

template <class... TS>
struct overload : TS... {
    using TS::operator()...;
};
template <class... TS>
overload(TS...) -> overload<TS...>;

template <typename T>
struct One {
    T a;
};

template <typename T>
struct Two {
    T a;
    T b;
};

template <typename T>
struct Three {
    T a;
    T b;
    T c;
};

template <typename T>
using Digit = std::variant<One<T>, Two<T>, Three<T>>;

template <typename T>
struct Node2 {
    T a;
    T b;
};

template <typename T>
struct Node3 {
    T a;
    T b;
    T c;
};

template <typename T>
using Node = std::variant<Node2<T>, Node3<T>>;

template <typename T>
struct CountMeasure {
    auto operator()(const T &) const -> std::size_t { return 1U; }
};

template <typename T, typename TAG_TYPE = std::size_t,
          typename MEASURE_POLICY = CountMeasure<T>>
class FingerTree;

template <typename T, typename TAG_TYPE = std::size_t,
          typename MEASURE_POLICY = CountMeasure<T>>
auto boxed(FingerTree<T, TAG_TYPE, MEASURE_POLICY> tree)
    -> std::shared_ptr<const FingerTree<T, TAG_TYPE, MEASURE_POLICY>> {
    return std::make_shared<const FingerTree<T, TAG_TYPE, MEASURE_POLICY>>(
        std::move(tree));
}

template <typename T>
inline auto digit_to_vector(const Digit<T> &d) -> std::vector<T> {
    return std::visit(
        overload{[](const One<T> &one) { return std::vector<T>{one.a}; },
                 [](const Two<T> &two) { return std::vector<T>{two.a, two.b}; },
                 [](const Three<T> &three) {
                     return std::vector<T>{three.a, three.b, three.c};
                 }},
        d);
}

template <typename T>
inline auto node_to_vector(const Node<T> &n) -> std::vector<T> {
    return std::visit(overload{[](const Node2<T> &node) {
                                   return std::vector<T>{node.a, node.b};
                               },
                               [](const Node3<T> &node) {
                                   return std::vector<T>{node.a, node.b,
                                                         node.c};
                               }},
                      n);
}

template <typename T, typename TAG_TYPE, typename MEASURE_POLICY>
class FingerTree {
    std::shared_ptr<const std::vector<T>> d_values;

    static auto make_data(std::vector<T> values)
        -> std::shared_ptr<const std::vector<T>> {
        return std::make_shared<const std::vector<T>>(std::move(values));
    }

    static auto identity_measure() -> TAG_TYPE {
        if constexpr (std::is_same_v<TAG_TYPE, std::size_t>) {
            return std::size_t{0};
        } else {
            return smd::typeclass::Monoid<TAG_TYPE>{}.identity();
        }
    }

    static auto combine_measures(const TAG_TYPE &lhs, const TAG_TYPE &rhs)
        -> TAG_TYPE {
        if constexpr (std::is_same_v<TAG_TYPE, std::size_t>) {
            return lhs + rhs;
        } else {
            return smd::typeclass::Monoid<TAG_TYPE>{}.combine(lhs, rhs);
        }
    }

    static auto element_measure(const T &value) -> TAG_TYPE {
        return MEASURE_POLICY{}(value);
    }

    static auto depth_for_size(std::size_t size) -> std::size_t {
        if (size == 0U) {
            return 0U;
        }

        return static_cast<std::size_t>(std::bit_width(size));
    }

  public:
    struct View {
        T d_value;
        FingerTree d_rest;
    };

    struct Split {
        FingerTree d_left;
        T d_pivot;
        FingerTree d_right;
    };

    struct SplitAt {
        FingerTree d_left;
        FingerTree d_right;
    };

    static auto empty() -> FingerTree { return FingerTree(std::vector<T>{}); }

    static auto leaf(T value) -> FingerTree {
        std::vector<T> values;
        values.push_back(std::move(value));
        return FingerTree(std::move(values));
    }

    auto cons(T x) const -> FingerTree {
        auto values = flatten();
        values.insert(values.begin(), std::move(x));
        return FingerTree(std::move(values));
    }

    auto snoc(T x) const -> FingerTree {
        auto values = flatten();
        values.push_back(std::move(x));
        return FingerTree(std::move(values));
    }

    auto append(const FingerTree &right) const -> FingerTree {
        auto combined = flatten();
        auto right_values = right.flatten();
        combined.insert(combined.end(), right_values.begin(),
                        right_values.end());
        return from_sequence(std::move(combined));
    }

    static auto branch(const FingerTree &left, const FingerTree &right)
        -> FingerTree {
        return left.append(right);
    }

    static auto prepend(T value, const FingerTree &tree) -> FingerTree {
        return tree.cons(std::move(value));
    }

    static auto append(const FingerTree &tree, T value) -> FingerTree {
        return tree.snoc(std::move(value));
    }

    static auto concat(const FingerTree &left, const FingerTree &right)
        -> FingerTree {
        return left.append(right);
    }

    auto is_empty() const -> bool { return d_values->empty(); }
    auto is_leaf() const -> bool { return d_values->size() == 1U; }
    auto is_branch() const -> bool { return d_values->size() > 1U; }

    auto measure() const -> TAG_TYPE {
        auto total = identity_measure();
        for (const auto &value : flatten()) {
            total = combine_measures(total, element_measure(value));
        }
        return total;
    }

    auto breadth() const -> std::size_t { return d_values->size(); }

    auto depth() const -> std::size_t {
        return depth_for_size(d_values->size());
    }

    auto value() const -> const T & {
        assert(is_leaf());
        return d_values->front();
    }

    auto flatten() const -> std::vector<T> { return *d_values; }

    template <typename FUNCTION>
    auto for_each(FUNCTION &&function) const -> void {
        for (const auto &value : flatten()) {
            std::invoke(function, value);
        }
    }

    static auto from_sequence(std::vector<T> values) -> FingerTree {
        return FingerTree(std::move(values));
    }

    template <typename PREDICATE>
    auto search(PREDICATE &&predicate) const -> std::optional<T> {
        auto values = flatten();
        auto prefix = identity_measure();

        for (const auto &value : values) {
            prefix = combine_measures(prefix, element_measure(value));
            if (std::invoke(predicate, prefix)) {
                return value;
            }
        }

        return std::nullopt;
    }

    template <typename PREDICATE>
    auto split(PREDICATE &&predicate) const -> std::optional<Split> {
        auto values = flatten();
        auto prefix = identity_measure();

        for (std::size_t i = 0; i < values.size(); ++i) {
            prefix = combine_measures(prefix, element_measure(values[i]));
            if (std::invoke(predicate, prefix)) {
                std::vector<T> left_values(values.begin(),
                                           values.begin() +
                                               static_cast<std::ptrdiff_t>(i));
                std::vector<T> right_values(
                    values.begin() + static_cast<std::ptrdiff_t>(i + 1),
                    values.end());
                return Split{from_sequence(std::move(left_values)),
                             std::move(values[i]),
                             from_sequence(std::move(right_values))};
            }
        }

        return std::nullopt;
    }

    template <typename PREDICATE>
    auto split_at(PREDICATE &&predicate) const -> SplitAt {
        auto values = flatten();
        auto prefix = identity_measure();

        for (std::size_t i = 0; i < values.size(); ++i) {
            prefix = combine_measures(prefix, element_measure(values[i]));
            if (std::invoke(predicate, prefix)) {
                std::vector<T> left_values(values.begin(),
                                           values.begin() +
                                               static_cast<std::ptrdiff_t>(i));
                std::vector<T> right_values(values.begin() +
                                                static_cast<std::ptrdiff_t>(i),
                                            values.end());
                return SplitAt{from_sequence(std::move(left_values)),
                               from_sequence(std::move(right_values))};
            }
        }

        return SplitAt{from_sequence(std::move(values)), empty()};
    }

    auto split_at_index(std::size_t index) const -> SplitAt {
        auto values = flatten();
        const auto clamped = std::min(index, values.size());
        std::vector<T> left_values(values.begin(),
                                   values.begin() +
                                       static_cast<std::ptrdiff_t>(clamped));
        std::vector<T> right_values(values.begin() +
                                        static_cast<std::ptrdiff_t>(clamped),
                                    values.end());
        return SplitAt{from_sequence(std::move(left_values)),
                       from_sequence(std::move(right_values))};
    }

    auto split_at_measure(const TAG_TYPE &value) const -> SplitAt {
        return split_at(
            [&value](const TAG_TYPE &prefix) { return prefix >= value; });
    }

    auto view_l() const -> std::optional<View> {
        if (is_empty()) {
            return std::nullopt;
        }

        auto rest = flatten();
        auto head_value = std::move(rest.front());
        rest.erase(rest.begin());
        return View{std::move(head_value), FingerTree(std::move(rest))};
    }

    auto view_r() const -> std::optional<View> {
        if (is_empty()) {
            return std::nullopt;
        }

        auto rest = flatten();
        auto tail_value = std::move(rest.back());
        rest.pop_back();
        return View{std::move(tail_value), FingerTree(std::move(rest))};
    }

    auto head() const -> T {
        auto v = view_l();
        assert(v.has_value());
        return std::move(v->d_value);
    }

    auto tail() const -> FingerTree {
        auto v = view_l();
        return v.has_value() ? std::move(v->d_rest) : empty();
    }

    auto last() const -> T {
        auto v = view_r();
        assert(v.has_value());
        return std::move(v->d_value);
    }

    auto init() const -> FingerTree {
        auto v = view_r();
        return v.has_value() ? std::move(v->d_rest) : empty();
    }

  private:
    explicit FingerTree(std::vector<T> values)
        : d_values(make_data(std::move(values))) {}
};

} // namespace smd::tree

#endif
