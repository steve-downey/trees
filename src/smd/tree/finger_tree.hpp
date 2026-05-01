// src/smd/tree/finger_tree.hpp                                       -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_FINGER_TREE
#define INCLUDED_SMD_TREE_FINGER_TREE

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
struct Four {
  T a;
  T b;
  T c;
  T d;
};

template <typename T>
using Digit = std::variant<One<T>, Two<T>, Three<T>, Four<T>>;

template <typename T, typename TAG_TYPE>
struct Node2 {
  TAG_TYPE d_measure;
  std::size_t d_leaf_count;
  Boxed<T> a;
  Boxed<T> b;
};

template <typename T, typename TAG_TYPE>
struct Node3 {
  TAG_TYPE d_measure;
  std::size_t d_leaf_count;
  Boxed<T> a;
  Boxed<T> b;
  Boxed<T> c;
};

template <typename T, typename TAG_TYPE>
using Node = std::variant<Node2<T, TAG_TYPE>, Node3<T, TAG_TYPE>>;

template <typename VALUE_TYPE, typename TAG_TYPE>
struct UnitMeasure {
  auto operator()(const VALUE_TYPE&) const -> TAG_TYPE { return TAG_TYPE{1}; }
};

template <typename NODE_T, typename TAG_TYPE>
struct NodeMeasure {
  auto operator()(const NODE_T& node) const -> TAG_TYPE
  {
    return std::visit([](const auto& n) -> TAG_TYPE { return n.d_measure; }, node);
  }
};

namespace detail {

template <typename... Ts>
struct overloaded : Ts... {
  using Ts::operator()...;
};

template <typename... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

}  // namespace detail

template <typename T,
          typename TAG_TYPE = std::size_t,
          typename MEASURE_POLICY = UnitMeasure<T, TAG_TYPE>,
          int DEPTH = 0>
class FingerTree {
  template <typename, typename, typename, int>
  friend class FingerTree;

  static_assert(std::is_default_constructible_v<MEASURE_POLICY>,
                "FingerTree measure policy must be default-constructible");

  static constexpr int kMaxDepth = 10;

  using Tag = TAG_TYPE;
  using MeasurePolicy = MEASURE_POLICY;
  using NodeT = Node<T, Tag>;
  using SpineMeasure = NodeMeasure<NodeT, Tag>;
  struct SpineTerminal {};
  using SpineTree = std::conditional_t<
    (DEPTH < kMaxDepth),
    FingerTree<NodeT, Tag, SpineMeasure, DEPTH + 1>,
    SpineTerminal>;
  using SpinePtr = std::shared_ptr<const SpineTree>;

  // -- Tag operations --

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

  static constexpr auto elem_leaf_count(
    [[maybe_unused]] const T& elem) -> std::size_t
  {
    if constexpr (DEPTH == 0) {
      return 1U;
    } else {
      return std::visit([](const auto& n) { return n.d_leaf_count; }, elem);
    }
  }

  // -- Node construction --

  static auto make_node2(T a, T b) -> NodeT
  {
    auto m = tag_combine(tag_value(a), tag_value(b));
    auto lc = elem_leaf_count(a) + elem_leaf_count(b);
    return Node2<T, Tag>{std::move(m), lc,
                         std::make_shared<const T>(std::move(a)),
                         std::make_shared<const T>(std::move(b))};
  }

  static auto make_node3(T a, T b, T c) -> NodeT
  {
    auto m = tag_combine(tag_combine(tag_value(a), tag_value(b)), tag_value(c));
    auto lc = elem_leaf_count(a) + elem_leaf_count(b) + elem_leaf_count(c);
    return Node3<T, Tag>{std::move(m), lc,
                         std::make_shared<const T>(std::move(a)),
                         std::make_shared<const T>(std::move(b)),
                         std::make_shared<const T>(std::move(c))};
  }

  static auto node_measure(const NodeT& node) -> Tag
  {
    return std::visit([](const auto& n) -> Tag { return n.d_measure; }, node);
  }

  static void node_flatten_into(const NodeT& node, std::vector<T>& out)
  {
    std::visit(detail::overloaded{
      [&](const Node2<T, Tag>& n) {
        out.push_back(*n.a);
        out.push_back(*n.b);
      },
      [&](const Node3<T, Tag>& n) {
        out.push_back(*n.a);
        out.push_back(*n.b);
        out.push_back(*n.c);
      }
    }, node);
  }

  template <typename F>
  static void node_for_each(const NodeT& node, const F& callback)
  {
    std::visit(detail::overloaded{
      [&](const Node2<T, Tag>& n) { callback(*n.a); callback(*n.b); },
      [&](const Node3<T, Tag>& n) {
        callback(*n.a); callback(*n.b); callback(*n.c);
      }
    }, node);
  }

  static auto node_to_digit(const NodeT& node) -> Digit<T>
  {
    return std::visit(detail::overloaded{
      [](const Node2<T, Tag>& n) -> Digit<T> { return Two<T>{*n.a, *n.b}; },
      [](const Node3<T, Tag>& n) -> Digit<T> {
        return Three<T>{*n.a, *n.b, *n.c};
      }
    }, node);
  }

  static auto node_to_list(const NodeT& node) -> std::vector<T>
  {
    std::vector<T> result;
    node_flatten_into(node, result);
    return result;
  }

  // -- Digit helpers --

  static auto digit_measure(const Digit<T>& d) -> Tag
  {
    return std::visit(detail::overloaded{
      [](const One<T>& x) { return tag_value(x.a); },
      [](const Two<T>& x) {
        return tag_combine(tag_value(x.a), tag_value(x.b));
      },
      [](const Three<T>& x) {
        return tag_combine(tag_combine(tag_value(x.a), tag_value(x.b)),
                           tag_value(x.c));
      },
      [](const Four<T>& x) {
        return tag_combine(
          tag_combine(tag_combine(tag_value(x.a), tag_value(x.b)),
                      tag_value(x.c)),
          tag_value(x.d));
      }
    }, d);
  }

  static auto digit_leaf_count(const Digit<T>& d) -> std::size_t
  {
    return std::visit(detail::overloaded{
      [](const One<T>& x) { return elem_leaf_count(x.a); },
      [](const Two<T>& x) {
        return elem_leaf_count(x.a) + elem_leaf_count(x.b);
      },
      [](const Three<T>& x) {
        return elem_leaf_count(x.a) + elem_leaf_count(x.b) + elem_leaf_count(x.c);
      },
      [](const Four<T>& x) {
        return elem_leaf_count(x.a) + elem_leaf_count(x.b)
             + elem_leaf_count(x.c) + elem_leaf_count(x.d);
      }
    }, d);
  }

  static void digit_flatten_into(const Digit<T>& d, std::vector<T>& out)
  {
    std::visit(detail::overloaded{
      [&](const One<T>& x) { out.push_back(x.a); },
      [&](const Two<T>& x) { out.push_back(x.a); out.push_back(x.b); },
      [&](const Three<T>& x) {
        out.push_back(x.a); out.push_back(x.b); out.push_back(x.c);
      },
      [&](const Four<T>& x) {
        out.push_back(x.a); out.push_back(x.b);
        out.push_back(x.c); out.push_back(x.d);
      }
    }, d);
  }

  template <typename F>
  static void digit_for_each(const Digit<T>& d, const F& callback)
  {
    std::visit(detail::overloaded{
      [&](const One<T>& x) { callback(x.a); },
      [&](const Two<T>& x) { callback(x.a); callback(x.b); },
      [&](const Three<T>& x) { callback(x.a); callback(x.b); callback(x.c); },
      [&](const Four<T>& x) {
        callback(x.a); callback(x.b); callback(x.c); callback(x.d);
      }
    }, d);
  }

  static auto digit_to_list(const Digit<T>& d) -> std::vector<T>
  {
    std::vector<T> result;
    digit_flatten_into(d, result);
    return result;
  }

  static auto digit_head(const Digit<T>& d) -> const T&
  {
    return std::visit([](const auto& x) -> const T& { return x.a; }, d);
  }

  static auto digit_last(const Digit<T>& d) -> const T&
  {
    return std::visit(detail::overloaded{
      [](const One<T>& x) -> const T& { return x.a; },
      [](const Two<T>& x) -> const T& { return x.b; },
      [](const Three<T>& x) -> const T& { return x.c; },
      [](const Four<T>& x) -> const T& { return x.d; }
    }, d);
  }

  static auto digit_tail(const Digit<T>& d) -> std::optional<Digit<T>>
  {
    return std::visit(detail::overloaded{
      [](const One<T>&) -> std::optional<Digit<T>> { return std::nullopt; },
      [](const Two<T>& x) -> std::optional<Digit<T>> { return One<T>{x.b}; },
      [](const Three<T>& x) -> std::optional<Digit<T>> {
        return Two<T>{x.b, x.c};
      },
      [](const Four<T>& x) -> std::optional<Digit<T>> {
        return Three<T>{x.b, x.c, x.d};
      }
    }, d);
  }

  static auto digit_init(const Digit<T>& d) -> std::optional<Digit<T>>
  {
    return std::visit(detail::overloaded{
      [](const One<T>&) -> std::optional<Digit<T>> { return std::nullopt; },
      [](const Two<T>& x) -> std::optional<Digit<T>> { return One<T>{x.a}; },
      [](const Three<T>& x) -> std::optional<Digit<T>> {
        return Two<T>{x.a, x.b};
      },
      [](const Four<T>& x) -> std::optional<Digit<T>> {
        return Three<T>{x.a, x.b, x.c};
      }
    }, d);
  }

  // -- Spine helpers --

  static auto spine_measure(const SpinePtr& sp) -> Tag
  {
    if constexpr (DEPTH < kMaxDepth) {
      if (!sp) return tag_identity();
      return sp->measure();
    } else {
      (void)sp;
      return tag_identity();
    }
  }

  static auto spine_breadth(const SpinePtr& sp) -> std::size_t
  {
    if constexpr (DEPTH < kMaxDepth) {
      if (!sp) return 0U;
      return sp->breadth();
    } else {
      (void)sp;
      return 0U;
    }
  }

  static auto spine_depth(const SpinePtr& sp) -> std::size_t
  {
    if constexpr (DEPTH < kMaxDepth) {
      if (!sp) return 0U;
      return sp->depth();
    } else {
      (void)sp;
      return 0U;
    }
  }

  static auto spine_is_empty(const SpinePtr& sp) -> bool
  {
    if constexpr (DEPTH < kMaxDepth) {
      return !sp || sp->is_empty();
    } else {
      (void)sp;
      return true;
    }
  }

  static auto spine_cons(const SpinePtr& spine, NodeT node) -> SpinePtr
  {
    if constexpr (DEPTH < kMaxDepth) {
      if (spine_is_empty(spine)) {
        return std::make_shared<const SpineTree>(
          SpineTree::leaf(std::move(node)));
      }
      return std::make_shared<const SpineTree>(
        spine->cons(std::move(node)));
    } else {
      (void)node;
      return spine;
    }
  }

  static auto spine_snoc(const SpinePtr& spine, NodeT node) -> SpinePtr
  {
    if constexpr (DEPTH < kMaxDepth) {
      if (spine_is_empty(spine)) {
        return std::make_shared<const SpineTree>(
          SpineTree::leaf(std::move(node)));
      }
      return std::make_shared<const SpineTree>(
        spine->snoc(std::move(node)));
    } else {
      (void)node;
      return spine;
    }
  }

  // -- Internal representation --

  struct Empty {};

  struct Single {
    Tag d_measure;
    T d_value;
  };

  struct Deep {
    Tag d_measure;
    std::size_t d_breadth;
    std::size_t d_depth;
    Digit<T> d_left;
    SpinePtr d_spine;
    Digit<T> d_right;
  };

  using DeepPtr = std::shared_ptr<const Deep>;
  using Repr = std::variant<Empty, Single, DeepPtr>;

  Repr d_repr;

  explicit FingerTree(Repr repr)
    : d_repr(std::move(repr))
  {
  }

  // -- Smart constructors --

  static auto make_empty() -> FingerTree { return FingerTree(Repr{Empty{}}); }

  static auto make_single(T value) -> FingerTree
  {
    auto m = tag_value(value);
    return FingerTree(Repr{Single{std::move(m), std::move(value)}});
  }

  static auto make_deep(Digit<T> left, SpinePtr spine, Digit<T> right) -> FingerTree
  {
    auto m = tag_combine(
      tag_combine(digit_measure(left), spine_measure(spine)),
      digit_measure(right));
    auto b = digit_leaf_count(left) + spine_breadth(spine)
           + digit_leaf_count(right);
    auto d = std::size_t{1} + spine_depth(spine);
    return FingerTree(Repr{std::make_shared<const Deep>(
      Deep{std::move(m), b, d,
           std::move(left), std::move(spine), std::move(right)})});
  }

  static auto digit_to_tree(const Digit<T>& d) -> FingerTree
  {
    return std::visit(detail::overloaded{
      [](const One<T>& x) { return make_single(x.a); },
      [](const Two<T>& x) {
        return make_deep(One<T>{x.a}, nullptr, One<T>{x.b});
      },
      [](const Three<T>& x) {
        return make_deep(Two<T>{x.a, x.b}, nullptr, One<T>{x.c});
      },
      [](const Four<T>& x) {
        return make_deep(Two<T>{x.a, x.b}, nullptr, Two<T>{x.c, x.d});
      }
    }, d);
  }

  static auto deep_l(SpinePtr spine, Digit<T> right) -> FingerTree
  {
    if constexpr (DEPTH < kMaxDepth) {
      if (spine_is_empty(spine)) {
        return digit_to_tree(right);
      }
      auto vl = spine->view_l();
      assert(vl.has_value());
      auto new_left = node_to_digit(vl->d_value);
      SpinePtr new_spine;
      if (!vl->d_rest.is_empty()) {
        new_spine = std::make_shared<const SpineTree>(
          std::move(vl->d_rest));
      }
      return make_deep(std::move(new_left), std::move(new_spine),
                        std::move(right));
    } else {
      return digit_to_tree(right);
    }
  }

  static auto deep_r(Digit<T> left, SpinePtr spine) -> FingerTree
  {
    if constexpr (DEPTH < kMaxDepth) {
      if (spine_is_empty(spine)) {
        return digit_to_tree(left);
      }
      auto vr = spine->view_r();
      assert(vr.has_value());
      auto new_right = node_to_digit(vr->d_value);
      SpinePtr new_spine;
      if (!vr->d_rest.is_empty()) {
        new_spine = std::make_shared<const SpineTree>(
          std::move(vr->d_rest));
      }
      return make_deep(std::move(left), std::move(new_spine),
                        std::move(new_right));
    } else {
      return digit_to_tree(left);
    }
  }

  // -- nodes: pack elements into Node2/Node3 sequence --

  static auto nodes_from(std::vector<T> elems) -> std::vector<NodeT>
  {
    std::vector<NodeT> result;
    auto n = elems.size();
    std::size_t i = 0;
    while (n - i > 4) {
      result.push_back(make_node3(std::move(elems[i]),
                                  std::move(elems[i + 1]),
                                  std::move(elems[i + 2])));
      i += 3;
    }
    switch (n - i) {
      case 2:
        result.push_back(make_node2(std::move(elems[i]),
                                    std::move(elems[i + 1])));
        break;
      case 3:
        result.push_back(make_node3(std::move(elems[i]),
                                    std::move(elems[i + 1]),
                                    std::move(elems[i + 2])));
        break;
      case 4:
        result.push_back(make_node2(std::move(elems[i]),
                                    std::move(elems[i + 1])));
        result.push_back(make_node2(std::move(elems[i + 2]),
                                    std::move(elems[i + 3])));
        break;
      default:
        assert(false && "nodes_from: invalid element count");
    }
    return result;
  }

  // -- app3: Hinze-Paterson recursive concatenation --

  static auto app3(const FingerTree& left,
                   std::vector<T> middle,
                   const FingerTree& right) -> FingerTree
  {
    if (left.is_empty()) {
      auto result = right;
      for (auto it = middle.rbegin(); it != middle.rend(); ++it) {
        result = result.cons(std::move(*it));
      }
      return result;
    }
    if (right.is_empty()) {
      auto result = left;
      for (auto& elem : middle) {
        result = result.snoc(std::move(elem));
      }
      return result;
    }
    if (left.is_leaf()) {
      auto result = right;
      for (auto it = middle.rbegin(); it != middle.rend(); ++it) {
        result = result.cons(std::move(*it));
      }
      return result.cons(std::get<Single>(left.d_repr).d_value);
    }
    if (right.is_leaf()) {
      auto result = left;
      for (auto& elem : middle) {
        result = result.snoc(std::move(elem));
      }
      return result.snoc(std::get<Single>(right.d_repr).d_value);
    }

    if constexpr (DEPTH < kMaxDepth) {
      const auto& ld = *std::get<DeepPtr>(left.d_repr);
      const auto& rd = *std::get<DeepPtr>(right.d_repr);

      auto combined = digit_to_list(ld.d_right);
      combined.insert(combined.end(),
                      std::make_move_iterator(middle.begin()),
                      std::make_move_iterator(middle.end()));
      {
        auto right_left = digit_to_list(rd.d_left);
        combined.insert(combined.end(), right_left.begin(),
                        right_left.end());
      }

      auto ns = nodes_from(std::move(combined));

      auto left_spine = ld.d_spine
        ? *ld.d_spine : SpineTree::empty();
      auto right_spine = rd.d_spine
        ? *rd.d_spine : SpineTree::empty();

      auto new_spine = SpineTree::app3(
        left_spine, std::move(ns), right_spine);
      SpinePtr sp;
      if (!new_spine.is_empty()) {
        sp = std::make_shared<const SpineTree>(std::move(new_spine));
      }

      return make_deep(ld.d_left, std::move(sp), rd.d_right);
    } else {
      auto result = left.flatten();
      result.insert(result.end(),
                    std::make_move_iterator(middle.begin()),
                    std::make_move_iterator(middle.end()));
      auto rv = right.flatten();
      result.insert(result.end(), rv.begin(), rv.end());
      return from_sequence(std::move(result));
    }
  }

  // -- Split helpers --

  struct DigitSplit {
    std::optional<Digit<T>> d_left;
    T d_pivot;
    std::optional<Digit<T>> d_right;
  };

  template <typename PREDICATE>
  static auto split_digit(const PREDICATE& predicate,
                          Tag prefix,
                          const Digit<T>& d) -> std::optional<DigitSplit>
  {
    auto elems = digit_to_list(d);
    auto running = prefix;
    for (std::size_t i = 0; i < elems.size(); ++i) {
      running = tag_combine(running, tag_value(elems[i]));
      if (predicate(running)) {
        std::optional<Digit<T>> left_d;
        std::optional<Digit<T>> right_d;
        if (i > 0) {
          std::vector<T> lv(elems.begin(),
                            elems.begin() + static_cast<std::ptrdiff_t>(i));
          left_d = list_to_digit(std::move(lv));
        }
        auto remaining = elems.size() - i - 1;
        if (remaining > 0) {
          std::vector<T> rv(
            elems.begin() + static_cast<std::ptrdiff_t>(i + 1), elems.end());
          right_d = list_to_digit(std::move(rv));
        }
        return DigitSplit{std::move(left_d), std::move(elems[i]),
                          std::move(right_d)};
      }
    }
    return std::nullopt;
  }

  static auto list_to_digit(std::vector<T> elems) -> std::optional<Digit<T>>
  {
    switch (elems.size()) {
      case 0: return std::nullopt;
      case 1: return One<T>{std::move(elems[0])};
      case 2: return Two<T>{std::move(elems[0]), std::move(elems[1])};
      case 3: return Three<T>{std::move(elems[0]), std::move(elems[1]),
                               std::move(elems[2])};
      case 4: return Four<T>{std::move(elems[0]), std::move(elems[1]),
                              std::move(elems[2]), std::move(elems[3])};
      default: assert(false); return std::nullopt;
    }
  }

  // Assemble tree from optional digit + spine + digit
  static auto assemble_left(std::optional<Digit<T>> left_d,
                            SpinePtr spine,
                            Digit<T> right) -> FingerTree
  {
    if (left_d.has_value()) {
      return make_deep(std::move(*left_d), std::move(spine),
                        std::move(right));
    }
    return deep_l(std::move(spine), std::move(right));
  }

  static auto assemble_right(Digit<T> left,
                             SpinePtr spine,
                             std::optional<Digit<T>> right_d) -> FingerTree
  {
    if (right_d.has_value()) {
      return make_deep(std::move(left), std::move(spine),
                        std::move(*right_d));
    }
    return deep_r(std::move(left), std::move(spine));
  }

  // Flatten spine nodes into elements
  static void spine_flatten_into(const SpinePtr& sp, std::vector<T>& out)
  {
    if constexpr (DEPTH < kMaxDepth) {
      if (spine_is_empty(sp)) return;
      auto spine_elems = sp->flatten();
      for (auto& node : spine_elems) {
        node_flatten_into(node, out);
      }
    }
  }

  template <typename F>
  static void spine_for_each(const SpinePtr& sp, const F& callback)
  {
    if constexpr (DEPTH < kMaxDepth) {
      if (spine_is_empty(sp)) return;
      sp->for_each([&](const NodeT& node) { node_for_each(node, callback); });
    }
  }

 public:
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

 private:
  // -- split_impl: measure-guided split --

  template <typename PREDICATE>
  auto split_impl(const PREDICATE& predicate,
                  Tag prefix) const -> std::optional<Split>
  {
    if (is_empty()) return std::nullopt;

    if (is_leaf()) {
      const auto& s = std::get<Single>(d_repr);
      auto p = tag_combine(prefix, s.d_measure);
      if (predicate(p)) {
        return Split{make_empty(), s.d_value, make_empty()};
      }
      return std::nullopt;
    }

    if constexpr (DEPTH < kMaxDepth) {
      const auto& d = *std::get<DeepPtr>(d_repr);

      // Check left digit
      auto vl = tag_combine(prefix, digit_measure(d.d_left));
      if (predicate(vl)) {
        auto ds = split_digit(predicate, prefix, d.d_left);
        if (!ds.has_value()) return std::nullopt;
        auto left_tree = ds->d_left.has_value()
          ? digit_to_tree(*ds->d_left) : make_empty();
        auto right_tree = assemble_left(
          std::move(ds->d_right), d.d_spine, d.d_right);
        return Split{std::move(left_tree), std::move(ds->d_pivot),
                     std::move(right_tree)};
      }

      // Check spine
      auto vm = tag_combine(vl, spine_measure(d.d_spine));
      if (predicate(vm)) {
        if (spine_is_empty(d.d_spine)) return std::nullopt;

        auto spine_split = d.d_spine->split_impl(
          [&](const Tag& t) { return predicate(tag_combine(vl, t)); },
          tag_identity());
        if (!spine_split.has_value()) return std::nullopt;

        auto node_prefix = tag_combine(vl,
          spine_split->d_left.measure());

        auto nd = node_to_digit(spine_split->d_pivot);
        auto node_split = split_digit(predicate, node_prefix, nd);
        if (!node_split.has_value()) return std::nullopt;

        SpinePtr sl;
        if (!spine_split->d_left.is_empty()) {
          sl = std::make_shared<const SpineTree>(
            std::move(spine_split->d_left));
        }
        SpinePtr sr;
        if (!spine_split->d_right.is_empty()) {
          sr = std::make_shared<const SpineTree>(
            std::move(spine_split->d_right));
        }

        auto left_tree = assemble_right(
          d.d_left, std::move(sl), std::move(node_split->d_left));
        auto right_tree = assemble_left(
          std::move(node_split->d_right), std::move(sr), d.d_right);
        return Split{std::move(left_tree), std::move(node_split->d_pivot),
                     std::move(right_tree)};
      }

      // Check right digit
      auto ds = split_digit(predicate, vm, d.d_right);
      if (!ds.has_value()) return std::nullopt;
      auto left_tree = assemble_right(
        d.d_left, d.d_spine, std::move(ds->d_left));
      auto right_tree = ds->d_right.has_value()
        ? digit_to_tree(*ds->d_right) : make_empty();
      return Split{std::move(left_tree), std::move(ds->d_pivot),
                   std::move(right_tree)};
    } else {
      auto vec = flatten();
      auto running = prefix;
      for (std::size_t i = 0; i < vec.size(); ++i) {
        running = tag_combine(running, tag_value(vec[i]));
        if (predicate(running)) {
          std::vector<T> lv(vec.begin(),
            vec.begin() + static_cast<std::ptrdiff_t>(i));
          std::vector<T> rv(
            vec.begin() + static_cast<std::ptrdiff_t>(i + 1), vec.end());
          return Split{from_sequence(std::move(lv)), std::move(vec[i]),
                       from_sequence(std::move(rv))};
        }
      }
      return std::nullopt;
    }
  }

 public:
  FingerTree()
    : d_repr(Empty{})
  {
  }

  static auto empty() -> FingerTree { return make_empty(); }

  static auto leaf(T value) -> FingerTree
  {
    return make_single(std::move(value));
  }

  auto is_empty() const -> bool
  {
    return std::holds_alternative<Empty>(d_repr);
  }

  auto is_leaf() const -> bool
  {
    return std::holds_alternative<Single>(d_repr);
  }

  auto is_branch() const -> bool
  {
    return std::holds_alternative<DeepPtr>(d_repr);
  }

  auto measure() const -> Tag
  {
    return std::visit(detail::overloaded{
      [](const Empty&) { return tag_identity(); },
      [](const Single& s) -> Tag { return s.d_measure; },
      [](const DeepPtr& d) -> Tag { return d->d_measure; }
    }, d_repr);
  }

  auto breadth() const -> std::size_t
  {
    return std::visit(detail::overloaded{
      [](const Empty&) -> std::size_t { return 0U; },
      [](const Single& s) -> std::size_t {
        return elem_leaf_count(s.d_value);
      },
      [](const DeepPtr& d) -> std::size_t { return d->d_breadth; }
    }, d_repr);
  }

  auto depth() const -> std::size_t
  {
    return std::visit(detail::overloaded{
      [](const Empty&) -> std::size_t { return 0U; },
      [](const Single&) -> std::size_t { return 1U; },
      [](const DeepPtr& d) -> std::size_t { return d->d_depth; }
    }, d_repr);
  }

  auto value() const -> const T&
  {
    assert(is_leaf());
    return std::get<Single>(d_repr).d_value;
  }

  auto cons(T x) const -> FingerTree
  {
    return std::visit(detail::overloaded{
      [&](const Empty&) { return make_single(std::move(x)); },
      [&](const Single& s) {
        return make_deep(One<T>{std::move(x)}, nullptr,
                          One<T>{s.d_value});
      },
      [&](const DeepPtr& d) -> FingerTree {
        return std::visit(detail::overloaded{
          [&](const One<T>& dig) {
            return make_deep(Two<T>{std::move(x), dig.a},
                              d->d_spine, d->d_right);
          },
          [&](const Two<T>& dig) {
            return make_deep(Three<T>{std::move(x), dig.a, dig.b},
                              d->d_spine, d->d_right);
          },
          [&](const Three<T>& dig) {
            return make_deep(
              Four<T>{std::move(x), dig.a, dig.b, dig.c},
              d->d_spine, d->d_right);
          },
          [&](const Four<T>& dig) -> FingerTree {
            auto node = make_node3(dig.b, dig.c, dig.d);
            auto new_spine = spine_cons(d->d_spine, std::move(node));
            return make_deep(Two<T>{std::move(x), dig.a},
                              std::move(new_spine), d->d_right);
          }
        }, d->d_left);
      }
    }, d_repr);
  }

  auto snoc(T x) const -> FingerTree
  {
    return std::visit(detail::overloaded{
      [&](const Empty&) { return make_single(std::move(x)); },
      [&](const Single& s) {
        return make_deep(One<T>{s.d_value}, nullptr,
                          One<T>{std::move(x)});
      },
      [&](const DeepPtr& d) -> FingerTree {
        return std::visit(detail::overloaded{
          [&](const One<T>& dig) {
            return make_deep(d->d_left, d->d_spine,
                              Two<T>{dig.a, std::move(x)});
          },
          [&](const Two<T>& dig) {
            return make_deep(d->d_left, d->d_spine,
                              Three<T>{dig.a, dig.b, std::move(x)});
          },
          [&](const Three<T>& dig) {
            return make_deep(
              d->d_left, d->d_spine,
              Four<T>{dig.a, dig.b, dig.c, std::move(x)});
          },
          [&](const Four<T>& dig) -> FingerTree {
            auto node = make_node3(dig.a, dig.b, dig.c);
            auto new_spine = spine_snoc(d->d_spine, std::move(node));
            return make_deep(d->d_left, std::move(new_spine),
                              Two<T>{dig.d, std::move(x)});
          }
        }, d->d_right);
      }
    }, d_repr);
  }

  auto append(const FingerTree& right) const -> FingerTree
  {
    return app3(*this, {}, right);
  }

  static auto branch(const FingerTree& left,
                     const FingerTree& right) -> FingerTree
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

  static auto concat(const FingerTree& left,
                     const FingerTree& right) -> FingerTree
  {
    return left.append(right);
  }

  auto flatten() const -> std::vector<T>
  {
    return std::visit(detail::overloaded{
      [](const Empty&) -> std::vector<T> { return {}; },
      [](const Single& s) -> std::vector<T> { return {s.d_value}; },
      [](const DeepPtr& d) -> std::vector<T> {
        std::vector<T> out;
        out.reserve(d->d_breadth);
        digit_flatten_into(d->d_left, out);
        spine_flatten_into(d->d_spine, out);
        digit_flatten_into(d->d_right, out);
        return out;
      }
    }, d_repr);
  }

  // Call callback(const T&) for each element in sequence order, without heap
  // allocation. Prefer over flatten() when results do not need to outlive the
  // callback loop.
  template <typename F>
  void for_each(F&& callback) const
  {
    std::visit(detail::overloaded{
      [](const Empty&) {},
      [&](const Single& s) { std::invoke(callback, s.d_value); },
      [&](const DeepPtr& d) {
        digit_for_each(d->d_left, callback);
        spine_for_each(d->d_spine, callback);
        digit_for_each(d->d_right, callback);
      }
    }, d_repr);
  }

  auto view_l() const -> std::optional<View>
  {
    return std::visit(detail::overloaded{
      [](const Empty&) -> std::optional<View> { return std::nullopt; },
      [](const Single& s) -> std::optional<View> {
        return View{s.d_value, make_empty()};
      },
      [](const DeepPtr& d) -> std::optional<View> {
        auto h = digit_head(d->d_left);
        auto t = digit_tail(d->d_left);
        if (t.has_value()) {
          return View{h, make_deep(std::move(*t), d->d_spine,
                                    d->d_right)};
        }
        return View{h, deep_l(d->d_spine, d->d_right)};
      }
    }, d_repr);
  }

  auto view_r() const -> std::optional<View>
  {
    return std::visit(detail::overloaded{
      [](const Empty&) -> std::optional<View> { return std::nullopt; },
      [](const Single& s) -> std::optional<View> {
        return View{s.d_value, make_empty()};
      },
      [](const DeepPtr& d) -> std::optional<View> {
        auto l = digit_last(d->d_right);
        auto i = digit_init(d->d_right);
        if (i.has_value()) {
          return View{l, make_deep(d->d_left, d->d_spine,
                                    std::move(*i))};
        }
        return View{l, deep_r(d->d_left, d->d_spine)};
      }
    }, d_repr);
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

  template <typename PREDICATE>
  auto search(PREDICATE&& predicate) const -> std::optional<T>
  {
    auto sp = split(std::forward<PREDICATE>(predicate));
    if (!sp.has_value()) return std::nullopt;
    return std::move(sp->d_pivot);
  }

  template <typename PREDICATE>
  auto split(PREDICATE&& predicate) const -> std::optional<Split>
  {
    return split_impl(predicate, tag_identity());
  }

  template <typename PREDICATE>
  auto split_at(PREDICATE&& predicate) const -> SplitAt
  {
    auto sp = split(std::forward<PREDICATE>(predicate));
    if (!sp.has_value()) {
      return SplitAt{*this, empty()};
    }
    return SplitAt{std::move(sp->d_left),
                   sp->d_right.cons(std::move(sp->d_pivot))};
  }

  auto split_at_index(std::size_t index) const -> SplitAt
  {
    if (index == 0U) {
      return SplitAt{empty(), *this};
    }
    if (index >= breadth()) {
      return SplitAt{*this, empty()};
    }
    auto vec = flatten();
    auto clamped = index > vec.size() ? vec.size() : index;
    std::vector<T> lv(vec.begin(),
      vec.begin() + static_cast<std::ptrdiff_t>(clamped));
    std::vector<T> rv(
      vec.begin() + static_cast<std::ptrdiff_t>(clamped), vec.end());
    return SplitAt{from_sequence(std::move(lv)),
                   from_sequence(std::move(rv))};
  }

  auto split_at_measure(const Tag& threshold) const -> SplitAt
    requires requires(const Tag& lhs, const Tag& rhs) {
      { lhs >= rhs } -> std::convertible_to<bool>;
    }
  {
    return split_at(
      [&threshold](const Tag& prefix) { return prefix >= threshold; });
  }

  static auto from_sequence(std::vector<T> values) -> FingerTree
  {
    auto result = empty();
    for (auto& v : values) {
      result = result.snoc(std::move(v));
    }
    return result;
  }
};

}  // namespace smd::tree

#endif
