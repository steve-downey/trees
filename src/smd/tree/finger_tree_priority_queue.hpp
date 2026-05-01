// src/smd/tree/finger_tree_priority_queue.hpp                        -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_FINGER_TREE_PRIORITY_QUEUE
#define INCLUDED_SMD_TREE_FINGER_TREE_PRIORITY_QUEUE

#include <smd/tree/finger_tree.hpp>
#include <smd/typeclass/applicative.hpp>
#include <smd/typeclass/foldable.hpp>
#include <smd/typeclass/monoid.hpp>
#include <smd/typeclass/traversable.hpp>

#include <algorithm>
#include <cstddef>
#include <functional>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

namespace smd::tree {

template <typename T>
struct MinTag {
  std::optional<T> d_value;

  friend bool operator==(const MinTag&, const MinTag&) = default;
};

template <typename T>
struct MaxTag {
  std::optional<T> d_value;

  friend bool operator==(const MaxTag&, const MaxTag&) = default;
};

// Combined measure tracking both min and max in a single tree pass.
template <typename T>
struct PriorityTag {
  MinTag<T> d_min;
  MaxTag<T> d_max;

  friend bool operator==(const PriorityTag&, const PriorityTag&) = default;
};

template <typename T>
struct PriorityMeasure {
  auto operator()(const T& value) const -> PriorityTag<T>
  {
    return PriorityTag<T>{MinTag<T>{value}, MaxTag<T>{value}};
  }
};

template <typename T>
class FingerTreePriorityQueue {
  using Tree = FingerTree<T, PriorityTag<T>, PriorityMeasure<T>>;

  Tree d_tree;

  explicit FingerTreePriorityQueue(Tree tree)
    : d_tree(std::move(tree))
  {
  }

 public:
  FingerTreePriorityQueue()
    : d_tree(Tree::empty())
  {
  }

  static auto from_values(std::vector<T> values) -> FingerTreePriorityQueue
  {
    return FingerTreePriorityQueue{Tree::from_sequence(std::move(values))};
  }

  auto empty() const -> bool { return d_tree.is_empty(); }

  auto size() const -> std::size_t { return d_tree.breadth(); }

  auto min() const -> std::optional<T>
  {
    auto m = d_tree.measure().d_min.d_value;
    return m.has_value() ? std::optional<T>{*m} : std::nullopt;
  }

  auto max() const -> std::optional<T>
  {
    auto m = d_tree.measure().d_max.d_value;
    return m.has_value() ? std::optional<T>{*m} : std::nullopt;
  }

  auto push(T value) const -> FingerTreePriorityQueue
  {
    return FingerTreePriorityQueue{d_tree.snoc(std::move(value))};
  }

  // O(log n): prefix min is non-increasing; predicate flips true at the first
  // element whose value equals the global min and stays true thereafter.
  auto pop_min() const -> std::optional<std::pair<T, FingerTreePriorityQueue>>
  {
    auto tag = d_tree.measure();
    if (!tag.d_min.d_value.has_value()) {
      return std::nullopt;
    }
    T global_min = *tag.d_min.d_value;
    auto sp = d_tree.split([global_min](const PriorityTag<T>& p) {
      return p.d_min.d_value.has_value() && *p.d_min.d_value <= global_min;
    });
    if (!sp.has_value()) {
      return std::nullopt;
    }
    return std::pair<T, FingerTreePriorityQueue>{
      sp->d_pivot,
      FingerTreePriorityQueue{Tree::concat(sp->d_left, sp->d_right)}};
  }

  // O(log n): prefix max is non-decreasing; predicate flips true at the first
  // element whose value equals the global max and stays true thereafter.
  auto pop_max() const -> std::optional<std::pair<T, FingerTreePriorityQueue>>
  {
    auto tag = d_tree.measure();
    if (!tag.d_max.d_value.has_value()) {
      return std::nullopt;
    }
    T global_max = *tag.d_max.d_value;
    auto sp = d_tree.split([global_max](const PriorityTag<T>& p) {
      return p.d_max.d_value.has_value() && *p.d_max.d_value >= global_max;
    });
    if (!sp.has_value()) {
      return std::nullopt;
    }
    return std::pair<T, FingerTreePriorityQueue>{
      sp->d_pivot,
      FingerTreePriorityQueue{Tree::concat(sp->d_left, sp->d_right)}};
  }

  auto to_vector() const -> std::vector<T> { return d_tree.flatten(); }
};

}  // namespace smd::tree

namespace smd::typeclass {

template <typename T>
struct Monoid<smd::tree::MinTag<T>> {
  auto identity() const -> smd::tree::MinTag<T> { return {std::nullopt}; }

  auto combine(const smd::tree::MinTag<T>& lhs,
               const smd::tree::MinTag<T>& rhs) const -> smd::tree::MinTag<T>
  {
    if (!lhs.d_value.has_value()) {
      return rhs;
    }
    if (!rhs.d_value.has_value()) {
      return lhs;
    }

    return lhs.d_value.value() <= rhs.d_value.value() ? lhs : rhs;
  }
};

template <typename T>
struct Monoid<smd::tree::MaxTag<T>> {
  auto identity() const -> smd::tree::MaxTag<T> { return {std::nullopt}; }

  auto combine(const smd::tree::MaxTag<T>& lhs,
               const smd::tree::MaxTag<T>& rhs) const -> smd::tree::MaxTag<T>
  {
    if (!lhs.d_value.has_value()) {
      return rhs;
    }
    if (!rhs.d_value.has_value()) {
      return lhs;
    }

    return lhs.d_value.value() >= rhs.d_value.value() ? lhs : rhs;
  }
};

template <typename T>
struct Monoid<smd::tree::PriorityTag<T>> {
  auto identity() const -> smd::tree::PriorityTag<T>
  {
    return {Monoid<smd::tree::MinTag<T>>{}.identity(),
            Monoid<smd::tree::MaxTag<T>>{}.identity()};
  }

  auto combine(const smd::tree::PriorityTag<T>& lhs,
               const smd::tree::PriorityTag<T>& rhs) const
    -> smd::tree::PriorityTag<T>
  {
    return {Monoid<smd::tree::MinTag<T>>{}.combine(lhs.d_min, rhs.d_min),
            Monoid<smd::tree::MaxTag<T>>{}.combine(lhs.d_max, rhs.d_max)};
  }
};

}  // namespace smd::typeclass

namespace smd {

template <class T>
struct FingerTreePriorityQueueFoldableImpl {
  template <class F>
  auto fold_map(this auto&&,
                F&& function,
                const smd::tree::FingerTreePriorityQueue<T>& queue)
    -> remove_cvref_t<std::invoke_result_t<F, const T&>>
  {
    using Result = remove_cvref_t<std::invoke_result_t<F, const T&>>;
    return std::ranges::fold_left(
        queue.to_vector(),
        smd::typeclass::monoid_v<Result>.identity(),
        [&](Result acc, const auto& value) {
          return smd::typeclass::monoid_v<Result>.combine(
              std::move(acc), std::invoke(function, value));
        });
  }
};

template <class T>
struct FingerTreePriorityQueueFoldableMap
  : Foldable<FingerTreePriorityQueueFoldableImpl<T>> {
  using FingerTreePriorityQueueFoldableImpl<T>::fold_map;
};

template <class T>
inline constexpr auto foldable_typeclass<smd::tree::FingerTreePriorityQueue<T>> =
  FingerTreePriorityQueueFoldableMap<T>{};

template <class T>
struct FingerTreePriorityQueueTraversableImpl {
  using element_type = T;

  template <class APPLICATIVE, class F>
  auto traverse(this auto&&,
                const APPLICATIVE& applicative,
                F&& function,
                const smd::tree::FingerTreePriorityQueue<T>& queue)
  {
    using Context = remove_cvref_t<std::invoke_result_t<F, const T&>>;
    using U = smd::applicative_value_t<Context>;

    auto accumulated = applicative.pure(std::vector<U>{});

    for (const auto& value : queue.to_vector()) {
      auto lifted = std::invoke(function, value);
      accumulated = applicative.invoke(
        [](std::vector<U> values, U element) {
          values.push_back(std::move(element));
          return values;
        },
        std::move(accumulated),
        std::move(lifted));
    }

    return applicative.invoke(
      [](std::vector<U> values) {
        return smd::tree::FingerTreePriorityQueue<U>::from_values(
          std::move(values));
      },
      std::move(accumulated));
  }
};

template <class T>
struct FingerTreePriorityQueueTraversableMap
  : Traversable<FingerTreePriorityQueueTraversableImpl<T>> {
  using FingerTreePriorityQueueTraversableImpl<T>::traverse;
};

template <class T>
inline constexpr auto traversable_typeclass<smd::tree::FingerTreePriorityQueue<T>> =
  FingerTreePriorityQueueTraversableMap<T>{};

}  // namespace smd

#endif
