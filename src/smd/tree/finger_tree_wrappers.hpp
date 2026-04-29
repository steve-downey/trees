#ifndef INCLUDE_SMD_TREE_FINGER_TREE_WRAPPERS_HPP
#define INCLUDE_SMD_TREE_FINGER_TREE_WRAPPERS_HPP

#include <smd/tree/finger_tree.hpp>
#include <smd/typeclass/monoid.hpp>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace smd::tree {

template <typename T>
class FingerTreeRandomAccess {
  FingerTree<T> d_tree;

 public:
  FingerTreeRandomAccess() = default;

  explicit FingerTreeRandomAccess(FingerTree<T> tree)
    : d_tree(std::move(tree))
  {
  }

  static auto from_sequence(std::vector<T> values) -> FingerTreeRandomAccess
  {
    return FingerTreeRandomAccess(FingerTree<T>::from_sequence(std::move(values)));
  }

  auto size() const -> std::size_t { return d_tree.breadth(); }

  auto empty() const -> bool { return d_tree.is_empty(); }

  auto at(std::size_t index) const -> std::optional<T>
  {
    if (index >= size()) {
      return std::nullopt;
    }
    return d_tree.flatten()[index];
  }

  auto push_back(T value) const -> FingerTreeRandomAccess
  {
    return FingerTreeRandomAccess(d_tree.snoc(std::move(value)));
  }

  auto push_front(T value) const -> FingerTreeRandomAccess
  {
    return FingerTreeRandomAccess(d_tree.cons(std::move(value)));
  }

  auto insert(std::size_t index, T value) const -> FingerTreeRandomAccess
  {
    auto parts = d_tree.split_at_index(index);
    auto middle = FingerTree<T>::leaf(std::move(value));
    return FingerTreeRandomAccess(FingerTree<T>::concat(FingerTree<T>::concat(parts.d_left, middle), parts.d_right));
  }

  auto erase(std::size_t index) const -> FingerTreeRandomAccess
  {
    if (index >= size()) {
      return *this;
    }

    auto left_right = d_tree.split_at_index(index);
    auto drop_rest = left_right.d_right.tail();
    return FingerTreeRandomAccess(FingerTree<T>::concat(left_right.d_left, drop_rest));
  }

  auto update(std::size_t index, T value) const -> FingerTreeRandomAccess
  {
    return erase(index).insert(index, std::move(value));
  }

  auto to_vector() const -> std::vector<T> { return d_tree.flatten(); }
};

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
  FingerTreePriorityQueue() = default;

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

    auto new_max_tree = FingerTree<T, MaxTag<T>, MaxMeasure<T>>::from_sequence(d_max_tree.flatten());
    auto split = new_max_tree.split([&m](const MaxTag<T>& prefix) {
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

 private:
  FingerTreePriorityQueue(MinTree min_tree, MaxTree max_tree)
    : d_min_tree(std::move(min_tree))
    , d_max_tree(std::move(max_tree))
  {
  }
};

template <typename PAYLOAD_TYPE>
struct Interval {
  std::size_t d_start;
  std::size_t d_end;
  PAYLOAD_TYPE d_payload;
};

template <typename PAYLOAD_TYPE>
struct IntervalMaxEndTag {
  std::size_t d_max_end;

  friend bool operator==(const IntervalMaxEndTag&, const IntervalMaxEndTag&) = default;
};

template <typename PAYLOAD_TYPE>
struct IntervalMeasure {
  auto operator()(const Interval<PAYLOAD_TYPE>& interval) const -> IntervalMaxEndTag<PAYLOAD_TYPE>
  {
    return IntervalMaxEndTag<PAYLOAD_TYPE>{interval.d_end};
  }
};

template <typename PAYLOAD_TYPE>
class FingerTreeIntervalIndex {
  using Entry = Interval<PAYLOAD_TYPE>;
  using Tree = FingerTree<Entry, IntervalMaxEndTag<PAYLOAD_TYPE>, IntervalMeasure<PAYLOAD_TYPE>>;

  Tree d_tree;

 public:
  FingerTreeIntervalIndex() = default;

  static auto from_intervals(std::vector<Entry> entries) -> FingerTreeIntervalIndex
  {
    return FingerTreeIntervalIndex{Tree::from_sequence(std::move(entries))};
  }

  auto insert(Entry entry) const -> FingerTreeIntervalIndex
  {
    return FingerTreeIntervalIndex{d_tree.snoc(std::move(entry))};
  }

  auto query_point(std::size_t point) const -> std::vector<PAYLOAD_TYPE>
  {
    std::vector<PAYLOAD_TYPE> out;

    for (const auto& entry : d_tree.flatten()) {
      if (entry.d_start <= point && point < entry.d_end) {
        out.push_back(entry.d_payload);
      }
    }

    return out;
  }

  auto query_overlap(std::size_t start, std::size_t end) const -> std::vector<PAYLOAD_TYPE>
  {
    std::vector<PAYLOAD_TYPE> out;

    for (const auto& entry : d_tree.flatten()) {
      if (entry.d_start < end && start < entry.d_end) {
        out.push_back(entry.d_payload);
      }
    }

    return out;
  }

 private:
  explicit FingerTreeIntervalIndex(Tree tree)
    : d_tree(std::move(tree))
  {
  }
};

struct RopeChunkMeasure {
  auto operator()(const std::string& value) const -> std::size_t { return value.size(); }
};

class FingerTreeRope {
  using Tree = FingerTree<std::string, std::size_t, RopeChunkMeasure>;

  Tree d_tree;

  static auto from_chunks(std::vector<std::string> chunks) -> FingerTreeRope
  {
    return FingerTreeRope{Tree::from_sequence(std::move(chunks))};
  }

  auto split_chars(std::size_t pos) const -> std::pair<FingerTreeRope, FingerTreeRope>
  {
    if (pos == 0) {
      return {FingerTreeRope{}, *this};
    }

    if (pos >= size_bytes()) {
      return {*this, FingerTreeRope{}};
    }

    auto split = d_tree.split([pos](std::size_t prefix) {
      return prefix > pos;
    });

    if (!split.has_value()) {
      return {*this, FingerTreeRope{}};
    }

    auto left_prefix_bytes = split->d_left.measure();
    auto local = pos - left_prefix_bytes;

    const auto& pivot = split->d_pivot;
    assert(local < pivot.size());

    auto left = split->d_left;
    if (local > 0) {
      left = left.snoc(pivot.substr(0, local));
    }

    auto right = split->d_right;
    if (local < pivot.size()) {
      right = right.cons(pivot.substr(local));
    }

    return {FingerTreeRope{std::move(left)}, FingerTreeRope{std::move(right)}};
  }

 public:
  FingerTreeRope()
    : d_tree(Tree::empty())
  {
  }

  static auto from_text(std::string_view text, std::size_t chunk_size = 16) -> FingerTreeRope
  {
    std::vector<std::string> chunks;
    chunks.reserve((text.size() / chunk_size) + 1);

    for (std::size_t i = 0; i < text.size(); i += chunk_size) {
      const auto n = std::min(chunk_size, text.size() - i);
      chunks.emplace_back(text.substr(i, n));
    }

    return from_chunks(std::move(chunks));
  }

  auto size_bytes() const -> std::size_t { return d_tree.measure(); }

  auto to_string() const -> std::string
  {
    std::string out;
    out.reserve(size_bytes());
    for (const auto& chunk : d_tree.flatten()) {
      out += chunk;
    }
    return out;
  }

  auto insert(std::size_t pos, std::string_view text) const -> FingerTreeRope
  {
    auto [left, right] = split_chars(pos);
    auto middle = from_text(text);
    return FingerTreeRope{Tree::concat(Tree::concat(left.d_tree, middle.d_tree), right.d_tree)};
  }

  auto erase(std::size_t pos, std::size_t count) const -> FingerTreeRope
  {
    auto [left, rest] = split_chars(pos);
    auto [_, right] = rest.split_chars(count);
    return FingerTreeRope{Tree::concat(left.d_tree, right.d_tree)};
  }

  auto replace(std::size_t pos, std::size_t count, std::string_view text) const -> FingerTreeRope
  {
    return erase(pos, count).insert(pos, text);
  }

 private:
  explicit FingerTreeRope(Tree tree)
    : d_tree(std::move(tree))
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

template <typename PAYLOAD_TYPE>
struct Monoid<smd::tree::IntervalMaxEndTag<PAYLOAD_TYPE>> {
  auto identity() const -> smd::tree::IntervalMaxEndTag<PAYLOAD_TYPE>
  {
    return {0U};
  }

  auto combine(const smd::tree::IntervalMaxEndTag<PAYLOAD_TYPE>& lhs,
               const smd::tree::IntervalMaxEndTag<PAYLOAD_TYPE>& rhs) const
    -> smd::tree::IntervalMaxEndTag<PAYLOAD_TYPE>
  {
    return {std::max(lhs.d_max_end, rhs.d_max_end)};
  }
};

}  // namespace smd::typeclass

#endif
