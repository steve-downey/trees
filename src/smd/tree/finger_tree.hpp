#ifndef INCLUDE_SMD_TREE_FINGER_TREE_HPP
#define INCLUDE_SMD_TREE_FINGER_TREE_HPP

#include <smd/tree/memoized_thunk.hpp>
#include <smd/typeclass/monoid.hpp>

#include <cassert>
#include <concepts>
#include <cstddef>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace smd::tree {

template <typename T>
using Boxed = std::shared_ptr<const T>;

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
  Boxed<T> a;
  Boxed<T> b;
};

template <typename T>
struct Node3 {
  Boxed<T> a;
  Boxed<T> b;
  Boxed<T> c;
};

template <typename T>
using Node = std::variant<Node2<T>, Node3<T>>;

template <typename VALUE_TYPE, typename TAG_TYPE>
struct UnitMeasure {
  auto operator()(const VALUE_TYPE&) const -> TAG_TYPE { return TAG_TYPE{1}; }
};

template <typename T,
          typename TAG_TYPE = std::size_t,
          typename MEASURE_POLICY = UnitMeasure<T, TAG_TYPE> >
class FingerTree {
  static_assert(std::is_default_constructible_v<MEASURE_POLICY>,
                "FingerTree measure policy must be default-constructible");

  using Tag = TAG_TYPE;
  using MeasurePolicy = MEASURE_POLICY;

  struct Segment;
  using SegmentPtr = std::shared_ptr<const Segment>;

  static auto make_segment_thunk(SegmentPtr eager,
                                 std::shared_ptr<const std::vector<T>> values = {},
                                 std::size_t begin = 0,
                                 std::size_t end = 0);
  using SegmentThunk = decltype(make_segment_thunk(
    SegmentPtr{}, std::shared_ptr<const std::vector<T>>{}, std::size_t{}, std::size_t{}));

  static auto tag_identity() -> Tag
  {
    return smd::typeclass::monoid_v<Tag>.identity();
  }

  static auto tag_combine(const Tag& lhs, const Tag& rhs) -> Tag
  {
    return smd::typeclass::monoid_v<Tag>.combine(lhs, rhs);
  }

  static auto tag_value(const T& value) -> Tag
  {
    return MeasurePolicy{}(value);
  }

  static auto tag_range(const std::shared_ptr<const std::vector<T> >& values,
                        std::size_t begin,
                        std::size_t end) -> Tag
  {
    auto result = tag_identity();
    for (auto i = begin; i < end; ++i) {
      result = tag_combine(result, tag_value((*values)[i]));
    }
    return result;
  }

  static auto balanced_depth(std::size_t count) -> std::size_t
  {
    if (count == 0U) {
      return 0U;
    }
    if (count == 1U) {
      return 1U;
    }

    auto left_count = count / 2U;
    auto right_count = count - left_count;
    return std::max(balanced_depth(left_count), balanced_depth(right_count)) + 1U;
  }

  struct SegmentMetadata {
    std::size_t d_size;
    std::size_t d_depth;
    Tag d_tag;
  };

  static auto make_suspended_segment(SegmentMetadata metadata, SegmentThunk thunk) -> SegmentPtr;

  struct Segment {
    virtual ~Segment() = default;

    [[nodiscard]] virtual auto size() const -> std::size_t = 0;
    [[nodiscard]] virtual auto tag() const -> const Tag& = 0;
    [[nodiscard]] virtual auto depth() const -> std::size_t = 0;
    virtual void flatten_into(std::vector<T>& out) const = 0;
    [[nodiscard]] virtual auto pop_left() const -> std::optional<std::pair<T, SegmentPtr>> = 0;
    [[nodiscard]] virtual auto pop_right() const -> std::optional<std::pair<T, SegmentPtr>> = 0;
  };

  struct FlatSegment final : Segment {
    std::shared_ptr<const std::vector<T>> d_values;
    std::size_t d_begin;
    std::size_t d_end;
    Tag d_tag;

    explicit FlatSegment(std::vector<T> values)
      : d_values(std::make_shared<const std::vector<T>>(std::move(values)))
      , d_begin(0)
      , d_end(d_values->size())
      , d_tag(tag_range(d_values, d_begin, d_end))
    {
    }

    FlatSegment(std::shared_ptr<const std::vector<T>> values, std::size_t begin, std::size_t end)
      : d_values(std::move(values))
      , d_begin(begin)
      , d_end(end)
      , d_tag(tag_range(d_values, d_begin, d_end))
    {
    }

    [[nodiscard]] auto size() const -> std::size_t override { return d_end - d_begin; }
    [[nodiscard]] auto tag() const -> const Tag& override { return d_tag; }

    [[nodiscard]] auto depth() const -> std::size_t override
    {
      return size() == 0 ? std::size_t{0} : std::size_t{1};
    }

    void flatten_into(std::vector<T>& out) const override
    {
      out.insert(out.end(),
                 d_values->begin() + static_cast<std::ptrdiff_t>(d_begin),
                 d_values->begin() + static_cast<std::ptrdiff_t>(d_end));
    }

    [[nodiscard]] auto pop_left() const -> std::optional<std::pair<T, SegmentPtr>> override
    {
      if (size() == 0) {
        return std::nullopt;
      }

      SegmentPtr rest;
      if (size() > 1) {
        rest = std::make_shared<const FlatSegment>(d_values, d_begin + 1, d_end);
      }

      return std::pair<T, SegmentPtr>{(*d_values)[d_begin], std::move(rest)};
    }

    [[nodiscard]] auto pop_right() const -> std::optional<std::pair<T, SegmentPtr>> override
    {
      if (size() == 0) {
        return std::nullopt;
      }

      SegmentPtr rest;
      if (size() > 1) {
        rest = std::make_shared<const FlatSegment>(d_values, d_begin, d_end - 1);
      }

      return std::pair<T, SegmentPtr>{(*d_values)[d_end - 1], std::move(rest)};
    }
  };

  static auto seg_size(const SegmentPtr& seg) -> std::size_t
  {
    return seg ? seg->size() : std::size_t{0};
  }

  static auto seg_depth(const SegmentPtr& seg) -> std::size_t
  {
    return seg ? seg->depth() : std::size_t{0};
  }

  static auto seg_tag(const SegmentPtr& seg) -> Tag
  {
    return seg ? seg->tag() : tag_identity();
  }

  static auto make_flat(std::vector<T> values) -> SegmentPtr
  {
    if (values.empty()) {
      return nullptr;
    }
    return std::make_shared<const FlatSegment>(std::move(values));
  }

  static auto make_flat_range(const std::shared_ptr<const std::vector<T>>& values,
                              std::size_t begin,
                              std::size_t end) -> SegmentPtr
  {
    if (begin >= end) {
      return nullptr;
    }
    return std::make_shared<const FlatSegment>(values, begin, end);
  }

  static auto segment_metadata(const SegmentPtr& seg) -> SegmentMetadata
  {
    return SegmentMetadata{seg_size(seg), seg_depth(seg), seg_tag(seg)};
  }

  static auto range_metadata(const std::shared_ptr<const std::vector<T>>& values,
                             std::size_t begin,
                             std::size_t end) -> SegmentMetadata
  {
    auto count = end > begin ? end - begin : 0U;
    return SegmentMetadata{count, balanced_depth(count), tag_range(values, begin, end)};
  }

  static auto concat_metadata(const SegmentMetadata& left,
                              const SegmentMetadata& right) -> SegmentMetadata
  {
    return SegmentMetadata{left.d_size + right.d_size,
                           std::max(left.d_depth, right.d_depth) + std::size_t{1},
                           tag_combine(left.d_tag, right.d_tag)};
  }

  struct MiddleEdge {
    SegmentMetadata d_metadata;
    mutable SegmentThunk d_force;

    [[nodiscard]] auto size() const -> std::size_t { return d_metadata.d_size; }
    [[nodiscard]] auto depth() const -> std::size_t { return d_metadata.d_depth; }
    [[nodiscard]] auto tag() const -> const Tag& { return d_metadata.d_tag; }
    [[nodiscard]] auto force() const -> const SegmentPtr& { return d_force(); }
  };

  static auto make_middle_from_segment(SegmentPtr seg) -> MiddleEdge
  {
    return MiddleEdge{segment_metadata(seg), make_segment_thunk(seg)};
  }

  static auto make_middle_from_range(const std::shared_ptr<const std::vector<T>>& values,
                                     std::size_t begin,
                                     std::size_t end) -> MiddleEdge
  {
    return MiddleEdge{range_metadata(values, begin, end),
                      make_segment_thunk(nullptr, values, begin, end)};
  }

  struct SuspendedSegment final : Segment {
    SegmentMetadata d_metadata;
    mutable SegmentThunk d_force;

    SuspendedSegment(SegmentMetadata metadata, SegmentThunk thunk)
      : d_metadata(std::move(metadata))
      , d_force(std::move(thunk))
    {
    }

    [[nodiscard]] auto size() const -> std::size_t override { return d_metadata.d_size; }
    [[nodiscard]] auto tag() const -> const Tag& override { return d_metadata.d_tag; }
    [[nodiscard]] auto depth() const -> std::size_t override { return d_metadata.d_depth; }

    [[nodiscard]] auto force() const -> const SegmentPtr& { return d_force(); }

    void flatten_into(std::vector<T>& out) const override
    {
      force()->flatten_into(out);
    }

    [[nodiscard]] auto pop_left() const -> std::optional<std::pair<T, SegmentPtr>> override
    {
      return force()->pop_left();
    }

    [[nodiscard]] auto pop_right() const -> std::optional<std::pair<T, SegmentPtr>> override
    {
      return force()->pop_right();
    }
  };

  static auto force_segment(const SegmentPtr& seg) -> SegmentPtr
  {
    if (const auto* suspended = dynamic_cast<const SuspendedSegment*>(seg.get())) {
      return suspended->force();
    }
    return seg;
  }

  static auto make_segment_from_middle(const MiddleEdge& edge) -> SegmentPtr
  {
    if (edge.size() == 0U) {
      return nullptr;
    }
    return make_suspended_segment(edge.d_metadata, edge.d_force);
  }

  static auto make_delayed_concat_segment(const MiddleEdge& left,
                                          SegmentPtr right) -> SegmentPtr
  {
    if (left.size() == 0U) {
      return right;
    }
    if (!right) {
      return make_segment_from_middle(left);
    }

    return make_suspended_segment(
      concat_metadata(left.d_metadata, segment_metadata(right)),
      detail::thunk(
        [left, right = std::move(right)]() mutable -> SegmentPtr {
          return make_concat(left.force(), right);
        }));
  }

  struct ConcatSegment final : Segment {
    SegmentPtr d_left;
    MiddleEdge d_right;
    std::size_t d_size;
    std::size_t d_depth;
    Tag d_tag;

    ConcatSegment(SegmentPtr left, MiddleEdge right)
      : d_left(std::move(left))
      , d_right(std::move(right))
      , d_size(seg_size(d_left) + d_right.size())
      , d_depth(std::max(seg_depth(d_left), d_right.depth()) + std::size_t{1})
      , d_tag(tag_combine(seg_tag(d_left), d_right.tag()))
    {
    }

    [[nodiscard]] auto size() const -> std::size_t override { return d_size; }
    [[nodiscard]] auto tag() const -> const Tag& override { return d_tag; }
    [[nodiscard]] auto depth() const -> std::size_t override { return d_depth; }

    void flatten_into(std::vector<T>& out) const override
    {
      if (d_left) {
        d_left->flatten_into(out);
      }
      if (d_right.size() != 0U) {
        d_right.force()->flatten_into(out);
      }
    }

    [[nodiscard]] auto pop_left() const -> std::optional<std::pair<T, SegmentPtr>> override
    {
      if (!d_left && d_right.size() == 0U) {
        return std::nullopt;
      }

      if (d_left) {
        auto l = d_left->pop_left();
        if (l.has_value()) {
          return std::pair<T, SegmentPtr>{std::move(l->first), make_concat(std::move(l->second), d_right)};
        }
      }

      return d_right.size() != 0U ? d_right.force()->pop_left() : std::nullopt;
    }

    [[nodiscard]] auto pop_right() const -> std::optional<std::pair<T, SegmentPtr>> override
    {
      if (!d_left && d_right.size() == 0U) {
        return std::nullopt;
      }

      if (d_right.size() != 0U) {
        auto r = d_right.force()->pop_right();
        if (r.has_value()) {
          return std::pair<T, SegmentPtr>{std::move(r->first), make_concat(d_left, std::move(r->second))};
        }
      }

      return d_left ? d_left->pop_right() : std::nullopt;
    }
  };

  static auto make_concat(SegmentPtr left, MiddleEdge right) -> SegmentPtr
  {
    if (!left) {
      return make_segment_from_middle(right);
    }
    if (right.size() == 0U) {
      return left;
    }
    return std::make_shared<const ConcatSegment>(std::move(left), std::move(right));
  }

  static auto make_suspended_segment(SegmentMetadata metadata, SegmentThunk thunk) -> SegmentPtr
  {
    if (metadata.d_size == 0U) {
      return nullptr;
    }
    return std::make_shared<const SuspendedSegment>(std::move(metadata), std::move(thunk));
  }

  static auto make_concat(SegmentPtr left, SegmentPtr right) -> SegmentPtr
  {
    auto make_node = [](SegmentPtr lhs, SegmentPtr rhs) -> SegmentPtr {
      if (!lhs) {
        return rhs;
      }
      if (!rhs) {
        return lhs;
      }
      return std::make_shared<const ConcatSegment>(std::move(lhs), make_middle_from_segment(std::move(rhs)));
    };

    auto as_concat = [](const SegmentPtr& seg) -> const ConcatSegment* {
      return dynamic_cast<const ConcatSegment*>(seg.get());
    };

    auto balance = [&](SegmentPtr lhs, SegmentPtr rhs) -> SegmentPtr {
      while (true) {
        auto lhs_depth = seg_depth(lhs);
        auto rhs_depth = seg_depth(rhs);

        if (lhs_depth <= rhs_depth + 1 && rhs_depth <= lhs_depth + 1) {
          return make_node(std::move(lhs), std::move(rhs));
        }

        if (lhs_depth > rhs_depth + 1) {
          lhs = force_segment(lhs);
          const auto* l = as_concat(lhs);
          if (!l) {
            return make_node(std::move(lhs), std::move(rhs));
          }

          if (seg_depth(l->d_left) < seg_depth(l->d_right)) {
            const auto* lr = as_concat(l->d_right.force());
            if (lr) {
              lhs = make_node(l->d_left, lr->d_left);
              rhs = make_delayed_concat_segment(lr->d_right, std::move(rhs));
              continue;
            }
          }

          rhs = make_delayed_concat_segment(l->d_right, std::move(rhs));
          lhs = l->d_left;
          continue;
        }

        rhs = force_segment(rhs);
        const auto* r = as_concat(rhs);
        if (!r) {
          return make_node(std::move(lhs), std::move(rhs));
        }

        if (r->d_right.depth() < seg_depth(r->d_left)) {
          auto forced_left = force_segment(r->d_left);
          const auto* rl = as_concat(forced_left);
          if (rl) {
            lhs = make_node(std::move(lhs), rl->d_left);
            rhs = make_delayed_concat_segment(rl->d_right, make_segment_from_middle(r->d_right));
            continue;
          }
        }

        lhs = make_node(std::move(lhs), r->d_left);
        rhs = make_segment_from_middle(r->d_right);
      }
    };

    if (!left) {
      return right;
    }
    if (!right) {
      return left;
    }

    auto left_depth = seg_depth(left);
    auto right_depth = seg_depth(right);

    if (left_depth > right_depth + 1) {
      auto forced_left = force_segment(left);
      if (const auto* l = dynamic_cast<const ConcatSegment*>(forced_left.get())) {
        return balance(l->d_left, make_delayed_concat_segment(l->d_right, std::move(right)));
      }
    }

    if (right_depth > left_depth + 1) {
      auto forced_right = force_segment(right);
      if (const auto* r = dynamic_cast<const ConcatSegment*>(forced_right.get())) {
        return balance(make_concat(std::move(left), r->d_left), make_segment_from_middle(r->d_right));
      }
    }

    return balance(std::move(left), std::move(right));
  }

  static auto build_balanced(const std::shared_ptr<const std::vector<T>>& values,
                             std::size_t begin,
                             std::size_t end) -> SegmentPtr
  {
    if (begin >= end) {
      return nullptr;
    }
    if (end - begin == 1) {
      return make_flat_range(values, begin, end);
    }

    auto mid = begin + (end - begin) / 2;
    return make_concat(build_balanced(values, begin, mid),
                       make_middle_from_range(values, mid, end));
  }

  static auto make_segment_thunk(SegmentPtr eager,
                                 std::shared_ptr<const std::vector<T>> values,
                                 std::size_t begin,
                                 std::size_t end)
  {
    return detail::thunk(
      [eager = std::move(eager), values = std::move(values), begin, end]() -> SegmentPtr {
        if (eager) {
          return eager;
        }
        return build_balanced(values, begin, end);
      });
  }

  template <typename PREDICATE>
  static auto search_segment(const SegmentPtr& seg,
                             const PREDICATE& predicate,
                             Tag prefix) -> std::optional<T>
  {
    auto current = force_segment(seg);
    if (!current) {
      return std::nullopt;
    }

    if (const auto* flat = dynamic_cast<const FlatSegment*>(current.get())) {
      for (std::size_t i = flat->d_begin; i < flat->d_end; ++i) {
        prefix = tag_combine(prefix, tag_value((*flat->d_values)[i]));
        if (predicate(prefix)) {
          return (*flat->d_values)[i];
        }
      }
      return std::nullopt;
    }

    const auto* concat = dynamic_cast<const ConcatSegment*>(current.get());
    assert(concat != nullptr);

    auto left_prefix = tag_combine(prefix, seg_tag(concat->d_left));
    if (predicate(left_prefix)) {
      return search_segment(concat->d_left, predicate, prefix);
    }

    return search_segment(make_segment_from_middle(concat->d_right), predicate, left_prefix);
  }

  struct SegmentSplit {
    SegmentPtr d_left;
    T d_pivot;
    SegmentPtr d_right;
  };

  template <typename PREDICATE>
  static auto split_segment(const SegmentPtr& seg,
                            const PREDICATE& predicate,
                            Tag prefix) -> std::optional<SegmentSplit>
  {
    auto current = force_segment(seg);
    if (!current) {
      return std::nullopt;
    }

    if (const auto* flat = dynamic_cast<const FlatSegment*>(current.get())) {
      auto running = prefix;
      for (std::size_t i = flat->d_begin; i < flat->d_end; ++i) {
        running = tag_combine(running, tag_value((*flat->d_values)[i]));
        if (predicate(running)) {
          return SegmentSplit{
            make_flat_range(flat->d_values, flat->d_begin, i),
            (*flat->d_values)[i],
            make_flat_range(flat->d_values, i + 1, flat->d_end)};
        }
      }
      return std::nullopt;
    }

    const auto* concat = dynamic_cast<const ConcatSegment*>(current.get());
    assert(concat != nullptr);

    auto left_prefix = tag_combine(prefix, seg_tag(concat->d_left));
    if (predicate(left_prefix)) {
      auto left_split = split_segment(concat->d_left, predicate, prefix);
      if (!left_split.has_value()) {
        return std::nullopt;
      }

      return SegmentSplit{left_split->d_left,
                          left_split->d_pivot,
                          make_concat(left_split->d_right, concat->d_right)};
    }

    auto right_split = split_segment(make_segment_from_middle(concat->d_right), predicate, left_prefix);
    if (!right_split.has_value()) {
      return std::nullopt;
    }

    return SegmentSplit{make_concat(concat->d_left, right_split->d_left),
                        right_split->d_pivot,
                        right_split->d_right};
  }

  static auto split_at_count(const SegmentPtr& seg,
                             std::size_t index) -> std::pair<SegmentPtr, SegmentPtr>
  {
    auto current = force_segment(seg);
    if (!current) {
      return {nullptr, nullptr};
    }

    if (const auto* flat = dynamic_cast<const FlatSegment*>(current.get())) {
      auto size = flat->d_end - flat->d_begin;
      auto pivot = index > size ? size : index;
      return {
        make_flat_range(flat->d_values, flat->d_begin, flat->d_begin + pivot),
        make_flat_range(flat->d_values, flat->d_begin + pivot, flat->d_end)};
    }

    const auto* concat = dynamic_cast<const ConcatSegment*>(current.get());
    assert(concat != nullptr);

    auto left_size = seg_size(concat->d_left);
    if (index < left_size) {
      auto split_left = split_at_count(concat->d_left, index);
      return {split_left.first, make_concat(split_left.second, concat->d_right)};
    }

    if (index == left_size) {
      return {concat->d_left, make_segment_from_middle(concat->d_right)};
    }

    auto split_right = split_at_count(make_segment_from_middle(concat->d_right), index - left_size);
    return {make_concat(concat->d_left, split_right.first), split_right.second};
  }

  SegmentPtr d_root;

  explicit FingerTree(SegmentPtr root)
    : d_root(std::move(root))
  {
  }

  explicit FingerTree(std::vector<T> values)
    : d_root(make_flat(std::move(values)))
  {
  }

  FingerTree() = default;

 public:
  // Current complexity contract (prototype implementation):
  // - O(1): empty, leaf, is_empty/is_leaf/is_branch,
  //         is_empty/is_leaf/is_branch, measure, breadth, depth, value.
  // - O(log n): cons, snoc, append/branch/concat,
  //             view_l/view_r, head/last, tail/init,
  //             search, split, split_at, split_at_index, split_at_measure.
  // - O(n): flatten, from_sequence.
  //
  // This keeps a stable API while leaving room for future asymptotic
  // improvements in search/split without changing call sites.
  //
  // Original finger-tree papers target stronger bounds with measured search:
  // amortized O(1) for end operations, O(log(min(n,m))) concatenation,
  // and O(log n) split/search.

  using value_type = T;
  using tag_type = Tag;

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

  static auto leaf(T value) -> FingerTree
  {
    return FingerTree(std::vector<T>{std::move(value)});
  }

  auto cons(T x) const -> FingerTree
  {
    return FingerTree(make_concat(make_flat(std::vector<T>{std::move(x)}), d_root));
  }

  auto snoc(T x) const -> FingerTree
  {
    return FingerTree(make_concat(d_root, make_flat(std::vector<T>{std::move(x)})));
  }

  auto append(const FingerTree& right) const -> FingerTree
  {
    return FingerTree(make_concat(d_root, right.d_root));
  }

  static auto branch(const FingerTree& left, const FingerTree& right) -> FingerTree
  {
    return left.append(right);
  }

  static auto prepend(T value, const FingerTree& tree) -> FingerTree
  {
    return tree.cons(std::move(value));
  }

  static auto append(const FingerTree& tree, T value) -> FingerTree
  {
    return tree.snoc(std::move(value));
  }

  static auto concat(const FingerTree& left, const FingerTree& right) -> FingerTree
  {
    return left.append(right);
  }

  auto is_empty() const -> bool { return seg_size(d_root) == 0; }
  auto is_leaf() const -> bool { return seg_size(d_root) == 1; }
  auto is_branch() const -> bool { return seg_size(d_root) > 1; }

  auto measure() const -> Tag { return seg_tag(d_root); }

  auto breadth() const -> std::size_t { return seg_size(d_root); }

  auto depth() const -> std::size_t { return seg_depth(d_root); }

  auto value() const -> const T&
  {
    assert(is_leaf());
    const auto* flat = dynamic_cast<const FlatSegment*>(d_root.get());
    assert(flat != nullptr);
    return (*(flat->d_values))[flat->d_begin];
  }

  auto flatten() const -> std::vector<T>
  {
    if (!d_root) {
      return {};
    }

    std::vector<T> out;
    out.reserve(breadth());
    d_root->flatten_into(out);
    return out;
  }

  template <typename PREDICATE>
  auto search(PREDICATE&& predicate) const -> std::optional<T>
  {
    return search_segment(d_root, predicate, tag_identity());
  }

  template <typename PREDICATE>
  auto split(PREDICATE&& predicate) const -> std::optional<Split>
  {
    auto split_result = split_segment(d_root, predicate, tag_identity());
    if (!split_result.has_value()) {
      return std::nullopt;
    }

    return Split{FingerTree(split_result->d_left),
                 std::move(split_result->d_pivot),
                 FingerTree(split_result->d_right)};
  }

  template <typename PREDICATE>
  auto split_at(PREDICATE&& predicate) const -> SplitAt
  {
    auto split_result = split_segment(d_root, predicate, tag_identity());
    if (!split_result.has_value()) {
      return SplitAt{*this, empty()};
    }

    auto right_with_pivot = make_concat(
      make_flat(std::vector<T>{split_result->d_pivot}), split_result->d_right);

    return SplitAt{FingerTree(split_result->d_left), FingerTree(right_with_pivot)};
  }

  auto split_at_index(std::size_t index) const -> SplitAt
  {
    auto clamped = index > breadth() ? breadth() : index;
    auto split_result = split_at_count(d_root, clamped);
    return SplitAt{FingerTree(split_result.first), FingerTree(split_result.second)};
  }

  auto split_at_measure(const Tag& threshold) const -> SplitAt
    requires requires(const Tag& lhs, const Tag& rhs) {
      { lhs >= rhs } -> std::convertible_to<bool>;
    }
  {
    return split_at([&threshold](const Tag& prefix) { return prefix >= threshold; });
  }

  static auto from_sequence(std::vector<T> values) -> FingerTree
  {
    auto shared = std::make_shared<const std::vector<T>>(std::move(values));
    return FingerTree(build_balanced(shared, 0, shared->size()));
  }

  auto view_l() const -> std::optional<View>
  {
    if (!d_root) {
      return std::nullopt;
    }

    auto left = d_root->pop_left();
    if (!left.has_value()) {
      return std::nullopt;
    }

    return View{std::move(left->first), FingerTree(std::move(left->second))};
  }

  auto view_r() const -> std::optional<View>
  {
    if (!d_root) {
      return std::nullopt;
    }

    auto right = d_root->pop_right();
    if (!right.has_value()) {
      return std::nullopt;
    }

    return View{std::move(right->first), FingerTree(std::move(right->second))};
  }

  auto head() const -> T
  {
    auto v = view_l();
    assert(v.has_value());
    return std::move(v->d_value);
  }

  auto tail() const -> FingerTree
  {
    auto v = view_l();
    return v.has_value() ? std::move(v->d_rest) : empty();
  }

  auto last() const -> T
  {
    auto v = view_r();
    assert(v.has_value());
    return std::move(v->d_value);
  }

  auto init() const -> FingerTree
  {
    auto v = view_r();
    return v.has_value() ? std::move(v->d_rest) : empty();
  }
};

}  // namespace smd::tree

#endif
