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

template <typename T>
struct MinMeasure {
  auto operator()(const T& value) const -> MinTag<T> { return MinTag<T>{value}; }
};

template <typename T>
struct MaxMeasure {
  auto operator()(const T& value) const -> MaxTag<T> { return MaxTag<T>{value}; }
};

template <typename T>
class FingerTreePriorityQueue {
  using MinTree = FingerTree<T, MinTag<T>, MinMeasure<T>>;
  using MaxTree = FingerTree<T, MaxTag<T>, MaxMeasure<T>>;

  MinTree d_min_tree;
  MaxTree d_max_tree;

  template <typename TREE>
  static auto remove_one_rebuild(const TREE& tree, const T& needle) -> TREE
  {
    auto values = tree.flatten();
    auto it = std::find(values.begin(), values.end(), needle);
    if (it == values.end()) {
      return tree;
    }

    values.erase(it);
    return TREE::from_sequence(std::move(values));
  }

  template <typename TREE>
  static auto has_multiple_instances(const TREE& tree, const T& needle) -> bool
  {
    return std::ranges::count(tree.flatten(), needle) > 1;
  }

  // Lazy split/concat removal path (experimental - use with caution)
  static auto remove_one_split_min(const MinTree& tree, const T& needle)
    -> MinTree
  {
    auto split = tree.split([&needle](const MinTag<T>& prefix) {
      return prefix.d_value.has_value() && prefix.d_value.value() <= needle;
    });

    if (!split.has_value()) {
      return tree;
    }

    return MinTree::concat(split->d_left, split->d_right);
  }

  static auto remove_one_split_max(const MaxTree& tree, const T& needle)
    -> MaxTree
  {
    auto split = tree.split([&needle](const MaxTag<T>& prefix) {
      return prefix.d_value.has_value() && prefix.d_value.value() >= needle;
    });

    if (!split.has_value()) {
      return tree;
    }

    return MaxTree::concat(split->d_left, split->d_right);
  }

 public:
  FingerTreePriorityQueue()
    : d_min_tree(MinTree::empty())
    , d_max_tree(MaxTree::empty())
  {
  }

  static auto from_values(std::vector<T> values) -> FingerTreePriorityQueue
  {
    return FingerTreePriorityQueue{
      MinTree::from_sequence(values),
      MaxTree::from_sequence(std::move(values))
    };
  }

  auto empty() const -> bool { return d_min_tree.is_empty(); }

  auto size() const -> std::size_t { return d_min_tree.breadth(); }

  auto min() const -> std::optional<T>
  {
    auto m = d_min_tree.measure().d_value;
    return m.has_value() ? std::optional<T>{*m} : std::nullopt;
  }

  auto max() const -> std::optional<T>
  {
    auto m = d_max_tree.measure().d_value;
    return m.has_value() ? std::optional<T>{*m} : std::nullopt;
  }

  auto push(T value) const -> FingerTreePriorityQueue
  {
    return FingerTreePriorityQueue{
      d_min_tree.snoc(value),
      d_max_tree.snoc(std::move(value))
    };
  }

  auto pop_min() const -> std::optional<std::pair<T, FingerTreePriorityQueue>>
  {
    auto m = min();
    if (!m.has_value()) {
      return std::nullopt;
    }

    auto new_min_tree = remove_one_rebuild(d_min_tree, *m);
    auto new_max_tree = remove_one_rebuild(d_max_tree, *m);

    return std::pair<T, FingerTreePriorityQueue>{
      *m,
      FingerTreePriorityQueue{std::move(new_min_tree), std::move(new_max_tree)}
    };
  }

  auto pop_max() const -> std::optional<std::pair<T, FingerTreePriorityQueue>>
  {
    auto m = max();
    if (!m.has_value()) {
      return std::nullopt;
    }

    auto rebuilt_max = remove_one_rebuild(d_max_tree, *m);
    auto rebuilt_min = remove_one_rebuild(d_min_tree, *m);

    return std::pair<T, FingerTreePriorityQueue>{
      *m,
      FingerTreePriorityQueue{std::move(rebuilt_min), std::move(rebuilt_max)}
    };
  }

  auto to_vector() const -> std::vector<T> { return d_min_tree.flatten(); }

  // TEST-ONLY: Alternative pop_min using split-based (lazy) removal
  // This method is for debugging/testing the lazy removal path
  auto pop_min_lazy() const -> std::optional<std::pair<T, FingerTreePriorityQueue>>
  {
    auto m = min();
    if (!m.has_value()) {
      return std::nullopt;
    }

    if (has_multiple_instances(d_min_tree, *m)) {
      auto rebuilt_min = remove_one_rebuild(d_min_tree, *m);
      auto rebuilt_max = remove_one_rebuild(d_max_tree, *m);
      return std::pair<T, FingerTreePriorityQueue>{
        *m,
        FingerTreePriorityQueue{std::move(rebuilt_min), std::move(rebuilt_max)}
      };
    }

    auto new_min_tree = remove_one_split_min(d_min_tree, *m);
    auto new_max_tree = remove_one_rebuild(d_max_tree, *m);
    if (new_min_tree.breadth() != new_max_tree.breadth()) {
      return std::nullopt;
    }

    return std::pair<T, FingerTreePriorityQueue>{
      *m,
      FingerTreePriorityQueue{std::move(new_min_tree), std::move(new_max_tree)}
    };
  }

  // TEST-ONLY: isolate MinTree split path without touching MaxTree.
  auto debug_pop_min_split_min_only() const
    -> std::optional<std::pair<T, FingerTreePriorityQueue>>
  {
    auto m = min();
    if (!m.has_value()) {
      return std::nullopt;
    }

    auto new_min_tree = remove_one_split_min(d_min_tree, *m);
    return std::pair<T, FingerTreePriorityQueue>{
      *m,
      FingerTreePriorityQueue{std::move(new_min_tree), d_max_tree}
    };
  }

  // TEST-ONLY: isolate MaxTree rebuild path without touching MinTree.
  auto debug_pop_min_rebuild_max_only() const
    -> std::optional<std::pair<T, FingerTreePriorityQueue>>
  {
    auto m = min();
    if (!m.has_value()) {
      return std::nullopt;
    }

    auto new_max_tree = remove_one_rebuild(d_max_tree, *m);
    return std::pair<T, FingerTreePriorityQueue>{
      *m,
      FingerTreePriorityQueue{d_min_tree, std::move(new_max_tree)}
    };
  }

  // TEST-ONLY: execute both pop_min_lazy removal steps without queue construction.
  auto debug_pop_min_lazy_components_only() const -> std::optional<T>
  {
    auto m = min();
    if (!m.has_value()) {
      return std::nullopt;
    }

    auto new_min_tree = remove_one_split_min(d_min_tree, *m);
    auto new_max_tree = remove_one_rebuild(d_max_tree, *m);
    (void)new_min_tree;
    (void)new_max_tree;
    return m;
  }

  // TEST-ONLY: construct a queue from both lazy-pop components directly.
  auto debug_construct_lazy_min_result() const -> bool
  {
    auto m = min();
    if (!m.has_value()) {
      return false;
    }

    auto new_min_tree = remove_one_split_min(d_min_tree, *m);
    auto new_max_tree = remove_one_rebuild(d_max_tree, *m);
    FingerTreePriorityQueue constructed{
      std::move(new_min_tree), std::move(new_max_tree)};
    return constructed.size() > 0U;
  }

  // TEST-ONLY: Alternative pop_max using split-based (lazy) removal
  auto pop_max_lazy() const -> std::optional<std::pair<T, FingerTreePriorityQueue>>
  {
    auto m = max();
    if (!m.has_value()) {
      return std::nullopt;
    }

    if (has_multiple_instances(d_max_tree, *m)) {
      auto rebuilt_max = remove_one_rebuild(d_max_tree, *m);
      auto rebuilt_min = remove_one_rebuild(d_min_tree, *m);
      return std::pair<T, FingerTreePriorityQueue>{
        *m,
        FingerTreePriorityQueue{std::move(rebuilt_min), std::move(rebuilt_max)}
      };
    }

    auto rebuilt_max = remove_one_split_max(d_max_tree, *m);
    auto rebuilt_min = remove_one_rebuild(d_min_tree, *m);
    if (rebuilt_min.breadth() != rebuilt_max.breadth()) {
      return std::nullopt;
    }

    return std::pair<T, FingerTreePriorityQueue>{
      *m,
      FingerTreePriorityQueue{std::move(rebuilt_min), std::move(rebuilt_max)}
    };
  }

 private:
  FingerTreePriorityQueue(MinTree min_tree, MaxTree max_tree)
    : d_min_tree(std::move(min_tree))
    , d_max_tree(std::move(max_tree))
  {
  }
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

}  // namespace smd::typeclass

#endif

#include <smd/tree/finger_tree_priority_queue_foldable.hpp>
#include <smd/tree/finger_tree_priority_queue_traversable.hpp>
