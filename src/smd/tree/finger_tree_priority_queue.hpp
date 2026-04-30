#ifndef INCLUDE_SMD_TREE_FINGER_TREE_PRIORITY_QUEUE_HPP
#define INCLUDE_SMD_TREE_FINGER_TREE_PRIORITY_QUEUE_HPP

#include <smd/tree/finger_tree.hpp>
#include <smd/typeclass/applicative.hpp>
#include <smd/typeclass/foldable.hpp>
#include <smd/typeclass/monoid.hpp>
#include <smd/typeclass/traversable.hpp>

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

  static auto remove_one(const MinTree& tree, const T& needle) -> MinTree
  {
    auto split = tree.split([&needle](const MinTag<T>& prefix) {
      return prefix.d_value.has_value() && prefix.d_value.value() == needle;
    });

    if (!split.has_value()) {
      return tree;
    }

    return MinTree::concat(split->d_left, split->d_right);
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

    auto new_min_tree = remove_one(d_min_tree, *m);
    auto new_max_tree = MaxTree::from_sequence(new_min_tree.flatten());

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

    auto split = d_max_tree.split([&m](const MaxTag<T>& prefix) {
      return prefix.d_value.has_value() && prefix.d_value.value() == *m;
    });

    if (!split.has_value()) {
      return std::nullopt;
    }

    auto rebuilt_max = MaxTree::concat(split->d_left, split->d_right);
    auto rebuilt_min = MinTree::from_sequence(rebuilt_max.flatten());

    return std::pair<T, FingerTreePriorityQueue>{
      *m,
      FingerTreePriorityQueue{std::move(rebuilt_min), std::move(rebuilt_max)}
    };
  }

  auto to_vector() const -> std::vector<T> { return d_min_tree.flatten(); }

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
