#ifndef INCLUDE_SMD_TREE_FINGER_TREE_HPP
#define INCLUDE_SMD_TREE_FINGER_TREE_HPP

#include <cassert>
#include <cstddef>
#include <memory>
#include <optional>
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

template <typename T>
class FingerTree {
  struct Segment;
  using SegmentPtr = std::shared_ptr<const Segment>;

  struct Segment {
    virtual ~Segment() = default;

    [[nodiscard]] virtual auto size() const -> std::size_t = 0;
    [[nodiscard]] virtual auto depth() const -> std::size_t = 0;
    virtual void flatten_into(std::vector<T>& out) const = 0;
    [[nodiscard]] virtual auto pop_left() const -> std::optional<std::pair<T, SegmentPtr>> = 0;
    [[nodiscard]] virtual auto pop_right() const -> std::optional<std::pair<T, SegmentPtr>> = 0;
  };

  struct FlatSegment final : Segment {
    std::shared_ptr<const std::vector<T>> d_values;
    std::size_t d_begin;
    std::size_t d_end;

    explicit FlatSegment(std::vector<T> values)
      : d_values(std::make_shared<const std::vector<T>>(std::move(values)))
      , d_begin(0)
      , d_end(d_values->size())
    {
    }

    FlatSegment(std::shared_ptr<const std::vector<T>> values, std::size_t begin, std::size_t end)
      : d_values(std::move(values))
      , d_begin(begin)
      , d_end(end)
    {
    }

    [[nodiscard]] auto size() const -> std::size_t override { return d_end - d_begin; }

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

  static auto make_flat(std::vector<T> values) -> SegmentPtr
  {
    if (values.empty()) {
      return nullptr;
    }
    return std::make_shared<const FlatSegment>(std::move(values));
  }

  struct ConcatSegment final : Segment {
    SegmentPtr d_left;
    SegmentPtr d_right;
    std::size_t d_size;
    std::size_t d_depth;

    ConcatSegment(SegmentPtr left, SegmentPtr right)
      : d_left(std::move(left))
      , d_right(std::move(right))
      , d_size(seg_size(d_left) + seg_size(d_right))
      , d_depth(std::max(seg_depth(d_left), seg_depth(d_right)) + std::size_t{1})
    {
    }

    [[nodiscard]] auto size() const -> std::size_t override { return d_size; }
    [[nodiscard]] auto depth() const -> std::size_t override { return d_depth; }

    void flatten_into(std::vector<T>& out) const override
    {
      if (d_left) {
        d_left->flatten_into(out);
      }
      if (d_right) {
        d_right->flatten_into(out);
      }
    }

    [[nodiscard]] auto pop_left() const -> std::optional<std::pair<T, SegmentPtr>> override
    {
      if (!d_left && !d_right) {
        return std::nullopt;
      }

      if (d_left) {
        auto l = d_left->pop_left();
        if (l.has_value()) {
          return std::pair<T, SegmentPtr>{std::move(l->first), make_concat(std::move(l->second), d_right)};
        }
      }

      return d_right ? d_right->pop_left() : std::nullopt;
    }

    [[nodiscard]] auto pop_right() const -> std::optional<std::pair<T, SegmentPtr>> override
    {
      if (!d_left && !d_right) {
        return std::nullopt;
      }

      if (d_right) {
        auto r = d_right->pop_right();
        if (r.has_value()) {
          return std::pair<T, SegmentPtr>{std::move(r->first), make_concat(d_left, std::move(r->second))};
        }
      }

      return d_left ? d_left->pop_right() : std::nullopt;
    }
  };

  static auto make_concat(SegmentPtr left, SegmentPtr right) -> SegmentPtr
  {
    if (!left) {
      return right;
    }
    if (!right) {
      return left;
    }
    return std::make_shared<const ConcatSegment>(std::move(left), std::move(right));
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
  struct View {
    T d_value;
    FingerTree d_rest;
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

  auto measure() const -> std::size_t { return seg_size(d_root); }

  auto breadth() const -> std::size_t { return measure(); }

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
    out.reserve(measure());
    d_root->flatten_into(out);
    return out;
  }

  static auto from_sequence(std::vector<T> values) -> FingerTree
  {
    return FingerTree(make_flat(std::move(values)));
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
