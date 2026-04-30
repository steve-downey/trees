#ifndef INCLUDE_SMD_TREE_FINGER_TREE_INTERVAL_INDEX_HPP
#define INCLUDE_SMD_TREE_FINGER_TREE_INTERVAL_INDEX_HPP

#include <smd/tree/finger_tree.hpp>
#include <smd/typeclass/applicative.hpp>
#include <smd/typeclass/foldable.hpp>
#include <smd/typeclass/monoid.hpp>
#include <smd/typeclass/traversable.hpp>

#include <algorithm>
#include <cstddef>
#include <functional>
#include <type_traits>
#include <utility>
#include <vector>

namespace smd::tree {

template <typename PAYLOAD_TYPE>
struct Interval {
  std::size_t d_start;
  std::size_t d_end;
  PAYLOAD_TYPE d_payload;
};

template <typename PAYLOAD_TYPE>
struct IntervalMaxEndTag {
  std::size_t d_max_end;

  friend bool operator==(const IntervalMaxEndTag&, const IntervalMaxEndTag&) =
    default;
};

template <typename PAYLOAD_TYPE>
struct IntervalMeasure {
  auto operator()(const Interval<PAYLOAD_TYPE>& interval) const
    -> IntervalMaxEndTag<PAYLOAD_TYPE>
  {
    return IntervalMaxEndTag<PAYLOAD_TYPE>{interval.d_end};
  }
};

template <typename PAYLOAD_TYPE>
class FingerTreeIntervalIndex {
  using Entry = Interval<PAYLOAD_TYPE>;
  using Tree =
    FingerTree<Entry, IntervalMaxEndTag<PAYLOAD_TYPE>, IntervalMeasure<PAYLOAD_TYPE>>;

  Tree d_tree;

 public:
  FingerTreeIntervalIndex()
    : d_tree(Tree::empty())
  {
  }

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

    // Use measure-based pruning: skip intervals where max_end <= point
    // These intervals cannot contain the point by definition
    auto parts = d_tree.split_at([point](const IntervalMaxEndTag<PAYLOAD_TYPE>& prefix) {
      return prefix.d_max_end > point;
    });
    
    // Only search within intervals with d_max_end > point
    for (const auto& entry : parts.d_right.flatten()) {
      if (entry.d_start <= point && point < entry.d_end) {
        out.push_back(entry.d_payload);
      }
    }

    return out;
  }

  auto query_overlap(std::size_t start, std::size_t end) const
    -> std::vector<PAYLOAD_TYPE>
  {
    std::vector<PAYLOAD_TYPE> out;

    // Use measure-based pruning: skip intervals where max_end <= start
    // These intervals cannot overlap with [start, end) by definition
    auto parts = d_tree.split_at([start](const IntervalMaxEndTag<PAYLOAD_TYPE>& prefix) {
      return prefix.d_max_end > start;
    });
    
    // Only search within intervals with d_max_end > start
    for (const auto& entry : parts.d_right.flatten()) {
      if (entry.d_start < end && start < entry.d_end) {
        out.push_back(entry.d_payload);
      }
    }

    return out;
  }

  auto entries() const -> std::vector<Entry> { return d_tree.flatten(); }

 private:
  explicit FingerTreeIntervalIndex(Tree tree)
    : d_tree(std::move(tree))
  {
  }
};

}  // namespace smd::tree

namespace smd::typeclass {

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

#include <smd/tree/finger_tree_interval_index_foldable.hpp>
#include <smd/tree/finger_tree_interval_index_traversable.hpp>
