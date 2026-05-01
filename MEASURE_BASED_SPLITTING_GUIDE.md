# Measure-Based Splitting in FingerTree

## Overview

The `FingerTree` class provides multiple methods for splitting based on **measures** (tagged metadata) rather than just element count. This guide documents these methods, their signatures, and practical examples.

## Methods for Measure-Based Splitting

### 1. `split_at(PREDICATE&& predicate)` — Generic Predicate-Based Split

**Signature:**
```cpp
template <typename PREDICATE>
auto split_at(PREDICATE&& predicate) const -> SplitAt
```

**Returns:** `SplitAt` struct containing:
```cpp
struct SplitAt {
  FingerTree d_left;   // Elements where predicate was false
  FingerTree d_right;  // Elements where predicate became true (including the pivot)
};
```

**Behavior:**
- Searches the tree from left to right
- Accumulates measures (tags) as it traverses
- Returns `SplitAt{entire_tree, empty_tree}` if predicate never becomes true
- The pivot element (first where predicate becomes true) is **included in d_right**

**Predicate Requirements:**
- Takes a `const Tag&` (the accumulated measure prefix)
- Returns `bool` — `true` means "split before this element"
- Called with increasing accumulated measure values

### 2. `split_at_measure(const Tag& threshold)` — Convenience Measure Threshold

**Signature:**
```cpp
auto split_at_measure(const Tag& threshold) const -> SplitAt
  requires requires(const Tag& lhs, const Tag& rhs) {
    { lhs >= rhs } -> std::convertible_to<bool>;
  }
```

**Returns:** `SplitAt` struct

**Behavior:**
- Convenience wrapper around `split_at()`
- Internally calls: `split_at([&threshold](const Tag& prefix) { return prefix >= threshold; })`
- Splits when accumulated measure reaches or exceeds the threshold
- Requires `Tag` type to support `operator>=()`

**Implementation:**
```cpp
auto split_at_measure(const Tag& threshold) const -> SplitAt
{
  return split_at([&threshold](const Tag& prefix) { return prefix >= threshold; });
}
```

### 3. `split(PREDICATE&& predicate)` — Triple Split with Pivot

**Signature:**
```cpp
template <typename PREDICATE>
auto split(PREDICATE&& predicate) const -> std::optional<Split>
```

**Returns:** `std::optional<Split>` containing:
```cpp
struct Split {
  FingerTree d_left;   // Elements before the pivot
  T d_pivot;           // The element where predicate became true (isolated)
  FingerTree d_right;  // Elements after the pivot
};
```

**Behavior:**
- Like `split_at()` but separates the pivot element
- Returns `std::nullopt` if predicate never becomes true
- Useful when you need the exact element causing the split

## Comparison with `split_at_index()`

| Aspect | `split_at_index()` | `split_at_measure()` / `split_at()` |
|--------|---|---|
| **Basis** | Element count (always available) | Accumulated measure/tag values |
| **Parameter** | `std::size_t index` | `Tag` threshold or predicate function |
| **Use Case** | "Get first N elements" | "Get elements until measure reaches threshold" |
| **Efficiency** | O(log N) | O(log N), but measure-aware pruning |
| **Pivot Included** | N/A (pure index split) | Yes, included in d_right |
| **Return Type** | `SplitAt` | `SplitAt` or `std::optional<Split>` |

**Key Difference:**
- `split_at_index(3)` splits after the 3rd element
- `split_at_measure(3U)` splits when accumulated measure reaches 3 (depends on measure policy)

## How Measure-Based Splitting Works Internally

### The Algorithm: `split_segment()`

```cpp
template <typename PREDICATE>
static auto split_segment(const SegmentPtr& seg,
                          const PREDICATE& predicate,
                          Tag prefix) -> std::optional<SegmentSplit>
{
  if (!seg) {
    return std::nullopt;
  }

  // Base case: flat segment (contiguous elements)
  if (const auto* flat = dynamic_cast<const FlatSegment*>(seg.get())) {
    auto running = prefix;
    for (std::size_t i = flat->d_begin; i < flat->d_end; ++i) {
      // Accumulate measure for current element
      running = tag_combine(running, tag_value((*flat->d_values)[i]));

      // Check predicate on accumulated measure
      if (predicate(running)) {
        return SegmentSplit{
          make_flat_range(flat->d_values, flat->d_begin, i),
          (*flat->d_values)[i],  // The pivot
          make_flat_range(flat->d_values, i + 1, flat->d_end)
        };
      }
    }
    return std::nullopt;
  }

  // Recursive case: concatenated segments
  const auto* concat = dynamic_cast<const ConcatSegment*>(seg.get());

  // Combine prefix with left segment's measure
  auto left_prefix = tag_combine(prefix, seg_tag(concat->d_left));

  if (predicate(left_prefix)) {
    // Predicate true after left segment, search within left
    auto left_split = split_segment(concat->d_left, predicate, prefix);
    if (!left_split.has_value()) {
      return std::nullopt;
    }

    return SegmentSplit{
      left_split->d_left,
      left_split->d_pivot,
      make_concat(left_split->d_right, concat->d_right)  // Preserve right
    };
  }

  // Predicate still false after left, search within right
  auto right_split = split_segment(concat->d_right, predicate, left_prefix);
  if (!right_split.has_value()) {
    return std::nullopt;
  }

  return SegmentSplit{
    make_concat(concat->d_left, right_split->d_left),
    right_split->d_pivot,
    right_split->d_right
  };
}
```

**Key Points:**
- Maintains running `prefix` accumulation
- Uses monoid `combine()` to build measures
- Short-circuits recursion when predicate becomes true
- Reconstructs tree with minimal rebuilding

## Code Examples

### Example 1: Count-Based Split (Default Measure)

```cpp
// Default measure: counts elements (UnitMeasure)
using CountTree = smd::tree::FingerTree<int>;

auto tree = CountTree::from_sequence({1, 2, 3, 4, 5});

// Split when count reaches 3 (first 2 elements, then 3-5)
auto split = tree.split_at_measure(3U);

assert(split.d_left.flatten() == std::vector<int>{1, 2});
assert(split.d_right.flatten() == std::vector<int>{3, 4, 5});
```

### Example 2: Weighted Measure Split

```cpp
struct Weighted {
  std::size_t d_total;

  friend bool operator>=(const Weighted& lhs, const Weighted& rhs) {
    return lhs.d_total >= rhs.d_total;
  }
};

struct WeightedMeasure {
  auto operator()(int value) const -> Weighted {
    return Weighted{static_cast<std::size_t>(value * 10)};
  }
};

using WeightedTree = smd::tree::FingerTree<int, Weighted, WeightedMeasure>;

auto tree = WeightedTree::from_sequence({1, 2, 3, 4});  // Weights: 10, 20, 30, 40
auto split = tree.split_at_measure(Weighted{35U});

// 1(10) + 2(20) = 30, then 3(30) makes 60 >= 35
assert(split.d_left.flatten() == std::vector<int>{1, 2});
assert(split.d_right.flatten() == std::vector<int>{3, 4});
```

### Example 3: Interval Pruning (Practical Use Case)

**Problem:** Given intervals stored in a finger tree, prune all intervals where `d_max_end <= threshold`.

```cpp
#include <smd/tree/finger_tree_interval_index.hpp>

using Interval = smd::tree::Interval<std::string>;
using IntervalMaxEndTag = smd::tree::IntervalMaxEndTag<std::string>;
using IntervalTree = smd::tree::FingerTree<
  Interval,
  IntervalMaxEndTag<std::string>,
  smd::tree::IntervalMeasure<std::string>
>;

// Create tree with intervals
auto tree = IntervalTree::from_sequence({
  Interval{0,  5,   "A"},      // max_end = 5
  Interval{3,  10,  "B"},      // max_end = 10
  Interval{8,  12,  "C"},      // max_end = 12
  Interval{10, 15,  "D"},      // max_end = 15
});

// Prune intervals where d_max_end > 10
// Keep only intervals with d_max_end <= 10
std::size_t pruning_threshold = 10;
auto pruned_split = tree.split_at_measure(
  IntervalMaxEndTag<std::string>{pruning_threshold}
);

// pruned_split.d_left contains: A (max_end=5), B (max_end=10)
// pruned_split.d_right contains: C (max_end=12), D (max_end=15)

auto kept_intervals = pruned_split.d_left.flatten();
// Contains: {[0,5,"A"], [3,10,"B"]}
```

**Measure Accumulation Trace:**
```
Interval A: max_end = 5,     accumulated = max(_, 5)   = 5   [< 10, continue]
Interval B: max_end = 10,    accumulated = max(5, 10)  = 10  [>= 10, SPLIT HERE]
Interval C: max_end = 12,    accumulated = max(10, 12) = 12  [in d_right]
Interval D: max_end = 15,    accumulated = max(12, 15) = 15  [in d_right]
```

### Example 4: Custom Predicate for Complex Conditions

```cpp
// Split when cumulative sum of element values exceeds threshold
auto tree = CountTree::from_sequence({1, 2, 3, 4, 5});

std::size_t cumulative_threshold = 7;
auto split = tree.split_at([&cumulative_threshold](std::size_t cumulative_sum) {
  return cumulative_sum > cumulative_threshold;
});

// Cumulative: 1 -> 3 -> 6 -> 10 (exceeds 7, split here)
// Result: d_left = {1,2,3}, d_right = {4,5}
assert(split.d_left.flatten() == std::vector<int>{1, 2, 3});
assert(split.d_right.flatten() == std::vector<int>{4, 5});
```

### Example 5: Using `split()` to Extract Pivot

```cpp
auto tree = CountTree::from_sequence({1, 2, 3, 4, 5});

auto triple_split = tree.split([](std::size_t count) {
  return count == 3;  // Split at exactly 3 elements
});

if (triple_split.has_value()) {
  auto& s = triple_split.value();
  assert(s.d_left.flatten() == std::vector<int>{1, 2});
  assert(s.d_pivot == 3);
  assert(s.d_right.flatten() == std::vector<int>{4, 5});
}
```

## Performance Characteristics

### Time Complexity
- **split_at_measure()**: O(log N)
  - Tree depth is O(log N)
  - Each recursive call eliminates a branch

- **split_at_index()**: O(log N)
  - Similar tree traversal
  - No measure accumulation needed

### Space Complexity
- **O(log N)** for result trees (only top-level structure rebuilt)
- Shared pointers mean most nodes are not copied

### Optimization: Measure-Aware Pruning
When measures support pruning (like interval `d_max_end`), entire subtrees can be skipped:
- If a subtree's measure indicates it cannot satisfy the predicate, the entire subtree is pruned
- Example: If all intervals in a subtree have `d_max_end < threshold`, search stops early

## Related Concepts

### Monoid Operations
Measure-based splitting relies on monoid `combine()` operations:

```cpp
namespace smd::typeclass {
  template <>
  struct Monoid<std::size_t> {
    auto identity() const -> std::size_t { return 0U; }
    auto combine(std::size_t lhs, std::size_t rhs) const -> std::size_t {
      return lhs + rhs;
    }
  };
}
```

### Measure Policy
Custom measures require:
1. A **tag type** (e.g., `Weighted`, `IntervalMaxEndTag`)
2. A **measure policy** (functor computing tag for each element)
3. A **monoid instance** (identity and combine operations)

## Summary

| Method | Purpose | Returns | Use When |
|--------|---------|---------|----------|
| `split_at_measure(tag)` | Split by measure threshold | `SplitAt` | Simple threshold-based splitting |
| `split_at(predicate)` | Split by accumulated measure predicate | `SplitAt` | Complex splitting logic |
| `split(predicate)` | Split and extract pivot element | `std::optional<Split>` | Need the exact splitting element |
| `split_at_index(index)` | Split by element count | `SplitAt` | Pure index-based splitting |

**Key Takeaway:** Measure-based splitting enables efficient, logarithmic-time range queries and filtering operations on finger trees while maintaining the tree's balanced structure.
