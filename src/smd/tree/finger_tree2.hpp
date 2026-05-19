// src/smd/tree/finger_tree2.hpp                                      -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_FINGER_TREE2
#define INCLUDED_SMD_TREE_FINGER_TREE2

// Hinze-Paterson 2-3 finger tree — second-generation implementation.
//
// Compile-cost strategy vs first generation (finger_tree.hpp):
// 1. kMaxDepth = 3 (4 instantiations, not 11)
// 2. Split predicates type-erased at spine boundary via TagPred
//    (prevents per-predicate instantiation across depth levels)
// 3. Flat-vector fallback at kMaxDepth (no further recursion)
//
// The hidden DEPTH NTTP is an implementation detail; the public type is
// FingerTree2<T, TAG_TYPE, MEASURE_POLICY> with DEPTH defaulted to 0.

#include <smd/typeclass/monoid.hpp>

#include <cassert>
#include <cstddef>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace smd::tree {

// ============================================================================
//                              DETAIL TYPES
// ============================================================================

namespace ft2 {

template <typename... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};
template <typename... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

// -- Digit: 1-4 element buffer at each end of a Deep node --------------------

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

// -- Node: 2-3 elements with cached measure ----------------------------------

template <typename T, typename Tag>
struct Node2 {
    Tag d_measure;
    std::size_t d_leaf_count;
    T a;
    T b;
};

template <typename T, typename Tag>
struct Node3 {
    Tag d_measure;
    std::size_t d_leaf_count;
    T a;
    T b;
    T c;
};

template <typename T, typename Tag>
using Node = std::variant<Node2<T, Tag>, Node3<T, Tag>>;

// -- TagPred: type-erased monotone predicate on Tag --------------------------
// Modelled on erased_thunk.  Non-owning: the wrapped callable must outlive
// the TagPred.  Used so the spine's split is not a template over the
// predicate type.

template <typename Tag>
class TagPred {
    struct Base {
        virtual ~Base() = default;
        virtual auto test(const Tag &) const -> bool = 0;
    };

    template <typename F>
    struct Model final : Base {
        const F &d_fn;
        explicit Model(const F &fn) : d_fn(fn) {}
        auto test(const Tag &t) const -> bool override { return d_fn(t); }
    };

    static constexpr std::size_t kBufSize = 32;
    alignas(Base) unsigned char d_buf[kBufSize];
    Base *d_ptr;

  public:
    template <typename F>
        requires(!std::is_same_v<std::remove_cvref_t<F>, TagPred>)
    explicit TagPred(const F &fn) {
        static_assert(sizeof(Model<F>) <= kBufSize);
        static_assert(alignof(Model<F>) <= alignof(Base));
        d_ptr = ::new (static_cast<void *>(d_buf)) Model<F>(fn);
    }

    TagPred(const TagPred &) = delete;
    TagPred &operator=(const TagPred &) = delete;
    ~TagPred() { d_ptr->~Base(); }

    auto operator()(const Tag &t) const -> bool { return d_ptr->test(t); }
};

// -- Tag operations ----------------------------------------------------------

template <typename Tag>
inline auto tag_id() -> Tag {
    return smd::typeclass::monoid_v<Tag>.identity();
}

template <typename Tag>
inline auto tag_op(const Tag &a, const Tag &b) -> Tag {
    return smd::typeclass::monoid_v<Tag>.combine(a, b);
}

// -- Digit helpers -----------------------------------------------------------

template <typename T, typename Tag, typename MeasFn>
auto digit_measure(const Digit<T> &d, MeasFn &&mf) -> Tag {
    return std::visit(
        overloaded{[&](const One<T> &x) -> Tag { return mf(x.a); },
                   [&](const Two<T> &x) -> Tag {
                       return tag_op<Tag>(mf(x.a), mf(x.b));
                   },
                   [&](const Three<T> &x) -> Tag {
                       return tag_op<Tag>(tag_op<Tag>(mf(x.a), mf(x.b)),
                                          mf(x.c));
                   },
                   [&](const Four<T> &x) -> Tag {
                       return tag_op<Tag>(
                           tag_op<Tag>(tag_op<Tag>(mf(x.a), mf(x.b)), mf(x.c)),
                           mf(x.d));
                   }},
        d);
}

template <typename T>
auto digit_head(const Digit<T> &d) -> const T & {
    return std::visit([](const auto &x) -> const T & { return x.a; }, d);
}

template <typename T>
auto digit_last(const Digit<T> &d) -> const T & {
    return std::visit(
        overloaded{[](const One<T> &x) -> const T & { return x.a; },
                   [](const Two<T> &x) -> const T & { return x.b; },
                   [](const Three<T> &x) -> const T & { return x.c; },
                   [](const Four<T> &x) -> const T & { return x.d; }},
        d);
}

template <typename T>
auto digit_tail(const Digit<T> &d) -> std::optional<Digit<T>> {
    return std::visit(
        overloaded{[](const One<T> &) -> std::optional<Digit<T>> {
                       return std::nullopt;
                   },
                   [](const Two<T> &x) -> std::optional<Digit<T>> {
                       return One<T>{x.b};
                   },
                   [](const Three<T> &x) -> std::optional<Digit<T>> {
                       return Two<T>{x.b, x.c};
                   },
                   [](const Four<T> &x) -> std::optional<Digit<T>> {
                       return Three<T>{x.b, x.c, x.d};
                   }},
        d);
}

template <typename T>
auto digit_init(const Digit<T> &d) -> std::optional<Digit<T>> {
    return std::visit(
        overloaded{[](const One<T> &) -> std::optional<Digit<T>> {
                       return std::nullopt;
                   },
                   [](const Two<T> &x) -> std::optional<Digit<T>> {
                       return One<T>{x.a};
                   },
                   [](const Three<T> &x) -> std::optional<Digit<T>> {
                       return Two<T>{x.a, x.b};
                   },
                   [](const Four<T> &x) -> std::optional<Digit<T>> {
                       return Three<T>{x.a, x.b, x.c};
                   }},
        d);
}

template <typename T>
void digit_push_into(const Digit<T> &d, std::vector<T> &out) {
    std::visit(overloaded{[&](const One<T> &x) { out.push_back(x.a); },
                          [&](const Two<T> &x) {
                              out.push_back(x.a);
                              out.push_back(x.b);
                          },
                          [&](const Three<T> &x) {
                              out.push_back(x.a);
                              out.push_back(x.b);
                              out.push_back(x.c);
                          },
                          [&](const Four<T> &x) {
                              out.push_back(x.a);
                              out.push_back(x.b);
                              out.push_back(x.c);
                              out.push_back(x.d);
                          }},
               d);
}

template <typename T>
auto digit_to_vec(const Digit<T> &d) -> std::vector<T> {
    std::vector<T> out;
    digit_push_into(d, out);
    return out;
}

template <typename T, typename F>
void digit_for_each(const Digit<T> &d, const F &fn) {
    std::visit(overloaded{[&](const One<T> &x) { fn(x.a); },
                          [&](const Two<T> &x) {
                              fn(x.a);
                              fn(x.b);
                          },
                          [&](const Three<T> &x) {
                              fn(x.a);
                              fn(x.b);
                              fn(x.c);
                          },
                          [&](const Four<T> &x) {
                              fn(x.a);
                              fn(x.b);
                              fn(x.c);
                              fn(x.d);
                          }},
               d);
}

template <typename T>
auto vec_to_digit(std::vector<T> v) -> std::optional<Digit<T>> {
    switch (v.size()) {
    case 0:
        return std::nullopt;
    case 1:
        return One<T>{std::move(v[0])};
    case 2:
        return Two<T>{std::move(v[0]), std::move(v[1])};
    case 3:
        return Three<T>{std::move(v[0]), std::move(v[1]), std::move(v[2])};
    case 4:
        return Four<T>{std::move(v[0]), std::move(v[1]), std::move(v[2]),
                       std::move(v[3])};
    default:
        assert(false && "vec_to_digit: size must be 0-4");
        return std::nullopt;
    }
}

// -- Node helpers ------------------------------------------------------------

template <typename T, typename Tag>
auto node_measure(const Node<T, Tag> &n) -> Tag {
    return std::visit([](const auto &x) -> Tag { return x.d_measure; }, n);
}

template <typename T, typename Tag>
auto node_to_digit(const Node<T, Tag> &n) -> Digit<T> {
    return std::visit(overloaded{[](const Node2<T, Tag> &x) -> Digit<T> {
                                     return Two<T>{x.a, x.b};
                                 },
                                 [](const Node3<T, Tag> &x) -> Digit<T> {
                                     return Three<T>{x.a, x.b, x.c};
                                 }},
                      n);
}

template <typename T, typename Tag>
void node_push_into(const Node<T, Tag> &n, std::vector<T> &out) {
    std::visit(overloaded{[&](const Node2<T, Tag> &x) {
                              out.push_back(x.a);
                              out.push_back(x.b);
                          },
                          [&](const Node3<T, Tag> &x) {
                              out.push_back(x.a);
                              out.push_back(x.b);
                              out.push_back(x.c);
                          }},
               n);
}

template <typename T, typename Tag, typename F>
void node_for_each(const Node<T, Tag> &n, const F &fn) {
    std::visit(overloaded{[&](const Node2<T, Tag> &x) {
                              fn(x.a);
                              fn(x.b);
                          },
                          [&](const Node3<T, Tag> &x) {
                              fn(x.a);
                              fn(x.b);
                              fn(x.c);
                          }},
               n);
}

template <typename T, typename Tag>
auto node_leaf_count(const Node<T, Tag> &n) -> std::size_t {
    return std::visit([](const auto &x) { return x.d_leaf_count; }, n);
}

template <typename T, typename Tag, typename MeasFn, typename LeafFn>
auto make_node2(MeasFn &&mf, LeafFn &&lf, T a, T b) -> Node<T, Tag> {
    auto m = tag_op<Tag>(mf(a), mf(b));
    auto lc = lf(a) + lf(b);
    return Node2<T, Tag>{std::move(m), lc, std::move(a), std::move(b)};
}

template <typename T, typename Tag, typename MeasFn, typename LeafFn>
auto make_node3(MeasFn &&mf, LeafFn &&lf, T a, T b, T c) -> Node<T, Tag> {
    auto m = tag_op<Tag>(tag_op<Tag>(mf(a), mf(b)), mf(c));
    auto lc = lf(a) + lf(b) + lf(c);
    return Node3<T, Tag>{std::move(m), lc, std::move(a), std::move(b),
                         std::move(c)};
}

template <typename T, typename Tag, typename MeasFn, typename LeafFn>
auto nodes_from(MeasFn &&mf, LeafFn &&lf, std::vector<T> elems)
    -> std::vector<Node<T, Tag>> {
    std::vector<Node<T, Tag>> result;
    auto n = elems.size();
    std::size_t i = 0;
    while (n - i > 4) {
        result.push_back(make_node3<T, Tag>(mf, lf, std::move(elems[i]),
                                            std::move(elems[i + 1]),
                                            std::move(elems[i + 2])));
        i += 3;
    }
    switch (n - i) {
    case 2:
        result.push_back(make_node2<T, Tag>(mf, lf, std::move(elems[i]),
                                            std::move(elems[i + 1])));
        break;
    case 3:
        result.push_back(make_node3<T, Tag>(mf, lf, std::move(elems[i]),
                                            std::move(elems[i + 1]),
                                            std::move(elems[i + 2])));
        break;
    case 4:
        result.push_back(make_node2<T, Tag>(mf, lf, std::move(elems[i]),
                                            std::move(elems[i + 1])));
        result.push_back(make_node2<T, Tag>(mf, lf, std::move(elems[i + 2]),
                                            std::move(elems[i + 3])));
        break;
    default:
        assert(false && "nodes_from: invalid count");
    }
    return result;
}

} // namespace ft2

// ============================================================================
//                            FINGER_TREE2
// ============================================================================

template <typename T, typename TAG_TYPE>
struct UnitMeasure2 {
    auto operator()(const T &) const -> TAG_TYPE { return TAG_TYPE{1}; }
};

template <typename T, typename TAG_TYPE = std::size_t,
          typename MEASURE_POLICY = UnitMeasure2<T, TAG_TYPE>, int DEPTH = 0>
class FingerTree2 {
    template <typename, typename, typename, int>
    friend class FingerTree2;

    static constexpr int kMaxDepth = 5;

    using Tag = TAG_TYPE;
    using Meas = MEASURE_POLICY;

    template <typename U>
    using One = ft2::One<U>;
    template <typename U>
    using Two = ft2::Two<U>;
    template <typename U>
    using Three = ft2::Three<U>;
    template <typename U>
    using Four = ft2::Four<U>;
    template <typename U>
    using Digit = ft2::Digit<U>;
    template <typename U, typename V>
    using Node = ft2::Node<U, V>;

    using NodeT = Node<T, Tag>;

    struct NodeMeasure {
        auto operator()(const NodeT &n) const -> Tag {
            return ft2::node_measure(n);
        }
    };

    using SpineTree =
        std::conditional_t<(DEPTH < kMaxDepth),
                           FingerTree2<NodeT, Tag, NodeMeasure, DEPTH + 1>,
                           void>;

    using SpinePtr =
        std::conditional_t<(DEPTH < kMaxDepth),
                           std::shared_ptr<const SpineTree>, std::monostate>;

    static auto meas_fn() -> Meas { return Meas{}; }
    static auto tag_value(const T &v) -> Tag { return meas_fn()(v); }

    // -- Spine access (compile-time dispatch) --------------------------------

    static auto spine_empty() -> SpinePtr {
        if constexpr (DEPTH < kMaxDepth) {
            return nullptr;
        } else {
            return {};
        }
    }

    static auto spine_is_empty(const SpinePtr &sp) -> bool {
        if constexpr (DEPTH < kMaxDepth) {
            return !sp || sp->is_empty();
        } else {
            (void)sp;
            return true;
        }
    }

    static auto spine_measure(const SpinePtr &sp) -> Tag {
        if constexpr (DEPTH < kMaxDepth) {
            if (!sp)
                return ft2::tag_id<Tag>();
            return sp->measure();
        } else {
            (void)sp;
            return ft2::tag_id<Tag>();
        }
    }

    static auto spine_cons(const SpinePtr &sp, NodeT node) -> SpinePtr {
        if constexpr (DEPTH < kMaxDepth) {
            if (spine_is_empty(sp)) {
                return std::make_shared<const SpineTree>(
                    SpineTree::leaf(std::move(node)));
            }
            return std::make_shared<const SpineTree>(sp->cons(std::move(node)));
        } else {
            (void)sp;
            (void)node;
            assert(false && "FingerTree2: spine depth exceeded kMaxDepth");
            return {};
        }
    }

    static auto spine_snoc(const SpinePtr &sp, NodeT node) -> SpinePtr {
        if constexpr (DEPTH < kMaxDepth) {
            if (spine_is_empty(sp)) {
                return std::make_shared<const SpineTree>(
                    SpineTree::leaf(std::move(node)));
            }
            return std::make_shared<const SpineTree>(sp->snoc(std::move(node)));
        } else {
            (void)sp;
            (void)node;
            assert(false && "FingerTree2: spine depth exceeded kMaxDepth");
            return {};
        }
    }

    // -- Internal representation ---------------------------------------------

    struct Empty {};

    struct Single {
        Tag d_measure;
        T d_value;
    };

    struct Deep {
        Tag d_measure;
        Digit<T> d_left;
        SpinePtr d_spine;
        Digit<T> d_right;
    };

    using DeepPtr = std::shared_ptr<const Deep>;
    using Repr = std::variant<Empty, Single, DeepPtr>;
    Repr d_repr;

    explicit FingerTree2(Repr r) : d_repr(std::move(r)) {}

    // -- Smart constructors --------------------------------------------------

    static auto make_empty() -> FingerTree2 {
        return FingerTree2(Repr{Empty{}});
    }

    static auto make_single(T value) -> FingerTree2 {
        auto m = tag_value(value);
        return FingerTree2(Repr{Single{std::move(m), std::move(value)}});
    }

    static auto make_deep(Digit<T> left, SpinePtr spine, Digit<T> right)
        -> FingerTree2 {
        auto m = ft2::tag_op<Tag>(
            ft2::tag_op<Tag>(ft2::digit_measure<T, Tag>(left, meas_fn()),
                             spine_measure(spine)),
            ft2::digit_measure<T, Tag>(right, meas_fn()));
        return FingerTree2(Repr{std::make_shared<const Deep>(
            Deep{std::move(m), std::move(left), std::move(spine),
                 std::move(right)})});
    }

    static auto digit_to_tree(const Digit<T> &d) -> FingerTree2 {
        return std::visit(
            ft2::overloaded{
                [](const One<T> &x) { return make_single(x.a); },
                [](const Two<T> &x) {
                    return make_deep(One<T>{x.a}, spine_empty(), One<T>{x.b});
                },
                [](const Three<T> &x) {
                    return make_deep(Two<T>{x.a, x.b}, spine_empty(),
                                     One<T>{x.c});
                },
                [](const Four<T> &x) {
                    return make_deep(Two<T>{x.a, x.b}, spine_empty(),
                                     Two<T>{x.c, x.d});
                }},
            d);
    }

    static auto deep_l(SpinePtr spine, Digit<T> right) -> FingerTree2 {
        if constexpr (DEPTH < kMaxDepth) {
            if (spine_is_empty(spine)) {
                return digit_to_tree(right);
            }
            auto vl = spine->view_l();
            assert(vl.has_value());
            auto new_left = ft2::node_to_digit(vl->d_value);
            SpinePtr new_spine;
            if (!vl->d_rest.is_empty()) {
                new_spine =
                    std::make_shared<const SpineTree>(std::move(vl->d_rest));
            }
            return make_deep(std::move(new_left), std::move(new_spine),
                             std::move(right));
        } else {
            return digit_to_tree(right);
        }
    }

    static auto deep_r(Digit<T> left, SpinePtr spine) -> FingerTree2 {
        if constexpr (DEPTH < kMaxDepth) {
            if (spine_is_empty(spine)) {
                return digit_to_tree(left);
            }
            auto vr = spine->view_r();
            assert(vr.has_value());
            auto new_right = ft2::node_to_digit(vr->d_value);
            SpinePtr new_spine;
            if (!vr->d_rest.is_empty()) {
                new_spine =
                    std::make_shared<const SpineTree>(std::move(vr->d_rest));
            }
            return make_deep(std::move(left), std::move(new_spine),
                             std::move(new_right));
        } else {
            return digit_to_tree(left);
        }
    }

    // -- leaf count per element at this depth --------------------------------

    static constexpr auto elem_leaf_count([[maybe_unused]] const T &elem)
        -> std::size_t {
        if constexpr (DEPTH == 0) {
            return 1U;
        } else {
            return std::visit([](const auto &n) { return n.d_leaf_count; },
                              elem);
        }
    }

    static auto leaf_count_fn() {
        return [](const T &e) { return elem_leaf_count(e); };
    }

    // -- digit leaf count at this level --------------------------------------

    static auto digit_leaf_count(const Digit<T> &d) -> std::size_t {
        std::size_t total = 0;
        ft2::digit_for_each(d,
                            [&](const T &e) { total += elem_leaf_count(e); });
        return total;
    }

    // -- nodes_from for this level -------------------------------------------

    static auto local_nodes(std::vector<T> elems) -> std::vector<NodeT> {
        return ft2::nodes_from<T, Tag>(meas_fn(), leaf_count_fn(),
                                       std::move(elems));
    }

    // -- app3: Hinze-Paterson recursive concatenation -------------------------

    static auto app3(const FingerTree2 &left, std::vector<T> middle,
                     const FingerTree2 &right) -> FingerTree2 {
        if (left.is_empty()) {
            auto result = right;
            for (auto it = middle.rbegin(); it != middle.rend(); ++it)
                result = result.cons(std::move(*it));
            return result;
        }
        if (right.is_empty()) {
            auto result = left;
            for (auto &elem : middle)
                result = result.snoc(std::move(elem));
            return result;
        }
        if (left.is_leaf()) {
            auto result = right;
            for (auto it = middle.rbegin(); it != middle.rend(); ++it)
                result = result.cons(std::move(*it));
            return result.cons(std::get<Single>(left.d_repr).d_value);
        }
        if (right.is_leaf()) {
            auto result = left;
            for (auto &elem : middle)
                result = result.snoc(std::move(elem));
            return result.snoc(std::get<Single>(right.d_repr).d_value);
        }

        if constexpr (DEPTH < kMaxDepth) {
            const auto &ld = *std::get<DeepPtr>(left.d_repr);
            const auto &rd = *std::get<DeepPtr>(right.d_repr);

            auto combined = ft2::digit_to_vec(ld.d_right);
            combined.insert(combined.end(),
                            std::make_move_iterator(middle.begin()),
                            std::make_move_iterator(middle.end()));
            {
                auto rl = ft2::digit_to_vec(rd.d_left);
                combined.insert(combined.end(), rl.begin(), rl.end());
            }
            auto ns = local_nodes(std::move(combined));

            auto left_spine = ld.d_spine ? *ld.d_spine : SpineTree::empty();
            auto right_spine = rd.d_spine ? *rd.d_spine : SpineTree::empty();
            auto new_spine =
                SpineTree::app3(left_spine, std::move(ns), right_spine);
            SpinePtr sp;
            if (!new_spine.is_empty())
                sp = std::make_shared<const SpineTree>(std::move(new_spine));
            return make_deep(ld.d_left, std::move(sp), rd.d_right);
        } else {
            auto result = left.flatten();
            result.insert(result.end(), std::make_move_iterator(middle.begin()),
                          std::make_move_iterator(middle.end()));
            auto rv = right.flatten();
            result.insert(result.end(), rv.begin(), rv.end());
            return from_sequence(std::move(result));
        }
    }

    // -- Result types (declared early for use in private helpers) -----------

  public:
    // Default constructor: variant default-initialises to Empty.
    FingerTree2() = default;

    struct View {
        T d_value;
        FingerTree2 d_rest;
    };

    struct Split {
        FingerTree2 d_left;
        T d_pivot;
        FingerTree2 d_right;
    };

    struct SplitAt {
        FingerTree2 d_left;
        FingerTree2 d_right;
    };

  private:
    // -- Split helpers -------------------------------------------------------

    struct DigitSplit {
        std::optional<Digit<T>> d_left;
        T d_pivot;
        std::optional<Digit<T>> d_right;
    };

    static auto split_digit(const ft2::TagPred<Tag> &pred, Tag prefix,
                            const Digit<T> &d) -> std::optional<DigitSplit> {
        auto elems = ft2::digit_to_vec(d);
        auto running = prefix;
        for (std::size_t i = 0; i < elems.size(); ++i) {
            running = ft2::tag_op<Tag>(running, tag_value(elems[i]));
            if (pred(running)) {
                std::optional<Digit<T>> left_d;
                std::optional<Digit<T>> right_d;
                if (i > 0) {
                    std::vector<T> lv(elems.begin(),
                                      elems.begin() +
                                          static_cast<std::ptrdiff_t>(i));
                    left_d = ft2::vec_to_digit(std::move(lv));
                }
                if (i + 1 < elems.size()) {
                    std::vector<T> rv(elems.begin() +
                                          static_cast<std::ptrdiff_t>(i + 1),
                                      elems.end());
                    right_d = ft2::vec_to_digit(std::move(rv));
                }
                return DigitSplit{std::move(left_d), std::move(elems[i]),
                                  std::move(right_d)};
            }
        }
        return std::nullopt;
    }

    // -- split_impl: uses TagPred (not a template on the predicate) ----------

    auto split_impl(const ft2::TagPred<Tag> &pred, Tag prefix) const
        -> std::optional<Split> {
        if (is_empty())
            return std::nullopt;

        if (is_leaf()) {
            const auto &s = std::get<Single>(d_repr);
            if (pred(ft2::tag_op<Tag>(prefix, s.d_measure)))
                return Split{make_empty(), s.d_value, make_empty()};
            return std::nullopt;
        }

        if constexpr (DEPTH < kMaxDepth) {
            const auto &d = *std::get<DeepPtr>(d_repr);

            auto vl = ft2::tag_op<Tag>(
                prefix, ft2::digit_measure<T, Tag>(d.d_left, meas_fn()));
            if (pred(vl)) {
                auto ds = split_digit(pred, prefix, d.d_left);
                if (!ds.has_value())
                    return std::nullopt;
                auto lt = ds->d_left.has_value() ? digit_to_tree(*ds->d_left)
                                                 : make_empty();
                auto rt =
                    assemble_l(std::move(ds->d_right), d.d_spine, d.d_right);
                return Split{std::move(lt), std::move(ds->d_pivot),
                             std::move(rt)};
            }

            auto vm = ft2::tag_op<Tag>(vl, spine_measure(d.d_spine));
            if (pred(vm)) {
                if (spine_is_empty(d.d_spine))
                    return std::nullopt;

                auto spine_pred = [&](const Tag &t) {
                    return pred(ft2::tag_op<Tag>(vl, t));
                };
                ft2::TagPred<Tag> erased_sp(spine_pred);
                auto ss = d.d_spine->split_impl(erased_sp, ft2::tag_id<Tag>());
                if (!ss.has_value())
                    return std::nullopt;

                auto node_prefix = ft2::tag_op<Tag>(vl, ss->d_left.measure());
                auto nd = ft2::node_to_digit(ss->d_pivot);
                auto ns = split_digit(pred, node_prefix, nd);
                if (!ns.has_value())
                    return std::nullopt;

                SpinePtr sl;
                if (!ss->d_left.is_empty())
                    sl = std::make_shared<const SpineTree>(
                        std::move(ss->d_left));
                SpinePtr sr;
                if (!ss->d_right.is_empty())
                    sr = std::make_shared<const SpineTree>(
                        std::move(ss->d_right));

                auto lt =
                    assemble_r(d.d_left, std::move(sl), std::move(ns->d_left));
                auto rt = assemble_l(std::move(ns->d_right), std::move(sr),
                                     d.d_right);
                return Split{std::move(lt), std::move(ns->d_pivot),
                             std::move(rt)};
            }

            auto ds = split_digit(pred, vm, d.d_right);
            if (!ds.has_value())
                return std::nullopt;
            auto lt = assemble_r(d.d_left, d.d_spine, std::move(ds->d_left));
            auto rt = ds->d_right.has_value() ? digit_to_tree(*ds->d_right)
                                              : make_empty();
            return Split{std::move(lt), std::move(ds->d_pivot), std::move(rt)};
        } else {
            auto vec = flatten();
            auto running = prefix;
            for (std::size_t i = 0; i < vec.size(); ++i) {
                running = ft2::tag_op<Tag>(running, tag_value(vec[i]));
                if (pred(running)) {
                    std::vector<T> lv(vec.begin(),
                                      vec.begin() +
                                          static_cast<std::ptrdiff_t>(i));
                    std::vector<T> rv(vec.begin() +
                                          static_cast<std::ptrdiff_t>(i + 1),
                                      vec.end());
                    return Split{from_sequence(std::move(lv)),
                                 std::move(vec[i]),
                                 from_sequence(std::move(rv))};
                }
            }
            return std::nullopt;
        }
    }

    static auto assemble_l(std::optional<Digit<T>> left_d, SpinePtr spine,
                           Digit<T> right) -> FingerTree2 {
        if (left_d.has_value())
            return make_deep(std::move(*left_d), std::move(spine),
                             std::move(right));
        return deep_l(std::move(spine), std::move(right));
    }

    static auto assemble_r(Digit<T> left, SpinePtr spine,
                           std::optional<Digit<T>> right_d) -> FingerTree2 {
        if (right_d.has_value())
            return make_deep(std::move(left), std::move(spine),
                             std::move(*right_d));
        return deep_r(std::move(left), std::move(spine));
    }

    // -- spine flatten -------------------------------------------------------

    static void spine_flatten_into(const SpinePtr &sp, std::vector<T> &out) {
        if constexpr (DEPTH < kMaxDepth) {
            if (spine_is_empty(sp))
                return;
            auto nodes = sp->flatten();
            for (auto &n : nodes)
                ft2::node_push_into(n, out);
        }
    }

    template <typename F>
    static void spine_for_each(const SpinePtr &sp, const F &fn) {
        if constexpr (DEPTH < kMaxDepth) {
            if (spine_is_empty(sp))
                return;
            sp->for_each([&](const NodeT &n) { ft2::node_for_each(n, fn); });
        }
    }

  public:
    using value_type = T;
    using tag_type = Tag;

    static auto empty() -> FingerTree2 { return make_empty(); }

    static auto leaf(T value) -> FingerTree2 {
        return make_single(std::move(value));
    }

    auto is_empty() const -> bool {
        return std::holds_alternative<Empty>(d_repr);
    }

    auto is_leaf() const -> bool {
        return std::holds_alternative<Single>(d_repr);
    }

    auto is_branch() const -> bool {
        return std::holds_alternative<DeepPtr>(d_repr);
    }

    auto measure() const -> Tag {
        return std::visit(
            ft2::overloaded{
                [](const Empty &) { return ft2::tag_id<Tag>(); },
                [](const Single &s) -> Tag { return s.d_measure; },
                [](const DeepPtr &d) -> Tag { return d->d_measure; }},
            d_repr);
    }

    auto value() const -> const T & {
        assert(is_leaf());
        return std::get<Single>(d_repr).d_value;
    }

    auto breadth() const -> std::size_t {
        if constexpr (DEPTH == 0 && std::is_same_v<Tag, std::size_t> &&
                      std::is_same_v<Meas, UnitMeasure2<T, Tag>>) {
            return measure();
        } else {
            std::size_t count = 0;
            for_each([&count](const T &) { ++count; });
            return count;
        }
    }

    // -- cons: O(1) amortized ------------------------------------------------

    auto cons(T x) const -> FingerTree2 {
        return std::visit(
            ft2::overloaded{
                [&](const Empty &) { return make_single(std::move(x)); },
                [&](const Single &s) {
                    return make_deep(One<T>{std::move(x)}, spine_empty(),
                                     One<T>{s.d_value});
                },
                [&](const DeepPtr &d) -> FingerTree2 {
                    return std::visit(
                        ft2::overloaded{
                            [&](const One<T> &dig) {
                                return make_deep(Two<T>{std::move(x), dig.a},
                                                 d->d_spine, d->d_right);
                            },
                            [&](const Two<T> &dig) {
                                return make_deep(
                                    Three<T>{std::move(x), dig.a, dig.b},
                                    d->d_spine, d->d_right);
                            },
                            [&](const Three<T> &dig) {
                                return make_deep(
                                    Four<T>{std::move(x), dig.a, dig.b, dig.c},
                                    d->d_spine, d->d_right);
                            },
                            [&](const Four<T> &dig) -> FingerTree2 {
                                auto node = ft2::make_node3<T, Tag>(
                                    meas_fn(), leaf_count_fn(), dig.b, dig.c,
                                    dig.d);
                                auto sp =
                                    spine_cons(d->d_spine, std::move(node));
                                return make_deep(Two<T>{std::move(x), dig.a},
                                                 std::move(sp), d->d_right);
                            }},
                        d->d_left);
                }},
            d_repr);
    }

    // -- snoc: O(1) amortized ------------------------------------------------

    auto snoc(T x) const -> FingerTree2 {
        return std::visit(
            ft2::overloaded{
                [&](const Empty &) { return make_single(std::move(x)); },
                [&](const Single &s) {
                    return make_deep(One<T>{s.d_value}, spine_empty(),
                                     One<T>{std::move(x)});
                },
                [&](const DeepPtr &d) -> FingerTree2 {
                    return std::visit(
                        ft2::overloaded{
                            [&](const One<T> &dig) {
                                return make_deep(d->d_left, d->d_spine,
                                                 Two<T>{dig.a, std::move(x)});
                            },
                            [&](const Two<T> &dig) {
                                return make_deep(
                                    d->d_left, d->d_spine,
                                    Three<T>{dig.a, dig.b, std::move(x)});
                            },
                            [&](const Three<T> &dig) {
                                return make_deep(
                                    d->d_left, d->d_spine,
                                    Four<T>{dig.a, dig.b, dig.c, std::move(x)});
                            },
                            [&](const Four<T> &dig) -> FingerTree2 {
                                auto node = ft2::make_node3<T, Tag>(
                                    meas_fn(), leaf_count_fn(), dig.a, dig.b,
                                    dig.c);
                                auto sp =
                                    spine_snoc(d->d_spine, std::move(node));
                                return make_deep(d->d_left, std::move(sp),
                                                 Two<T>{dig.d, std::move(x)});
                            }},
                        d->d_right);
                }},
            d_repr);
    }

    // -- view_l: O(1) amortized ----------------------------------------------

    auto view_l() const -> std::optional<View> {
        return std::visit(
            ft2::overloaded{[](const Empty &) -> std::optional<View> {
                                return std::nullopt;
                            },
                            [](const Single &s) -> std::optional<View> {
                                return View{s.d_value, make_empty()};
                            },
                            [](const DeepPtr &d) -> std::optional<View> {
                                auto h = ft2::digit_head(d->d_left);
                                auto t = ft2::digit_tail(d->d_left);
                                if (t.has_value())
                                    return View{h, make_deep(std::move(*t),
                                                             d->d_spine,
                                                             d->d_right)};
                                return View{h, deep_l(d->d_spine, d->d_right)};
                            }},
            d_repr);
    }

    // -- view_r: O(1) amortized ----------------------------------------------

    auto view_r() const -> std::optional<View> {
        return std::visit(
            ft2::overloaded{[](const Empty &) -> std::optional<View> {
                                return std::nullopt;
                            },
                            [](const Single &s) -> std::optional<View> {
                                return View{s.d_value, make_empty()};
                            },
                            [](const DeepPtr &d) -> std::optional<View> {
                                auto l = ft2::digit_last(d->d_right);
                                auto i = ft2::digit_init(d->d_right);
                                if (i.has_value())
                                    return View{l,
                                                make_deep(d->d_left, d->d_spine,
                                                          std::move(*i))};
                                return View{l, deep_r(d->d_left, d->d_spine)};
                            }},
            d_repr);
    }

    auto head() const -> T {
        auto v = view_l();
        assert(v.has_value());
        return std::move(v->d_value);
    }

    auto tail() const -> FingerTree2 {
        auto v = view_l();
        return v.has_value() ? std::move(v->d_rest) : empty();
    }

    auto last() const -> T {
        auto v = view_r();
        assert(v.has_value());
        return std::move(v->d_value);
    }

    auto init() const -> FingerTree2 {
        auto v = view_r();
        return v.has_value() ? std::move(v->d_rest) : empty();
    }

    // -- flatten: O(n) -------------------------------------------------------

    auto flatten() const -> std::vector<T> {
        return std::visit(
            ft2::overloaded{
                [](const Empty &) -> std::vector<T> { return {}; },
                [](const Single &s) -> std::vector<T> { return {s.d_value}; },
                [](const DeepPtr &d) -> std::vector<T> {
                    std::vector<T> out;
                    ft2::digit_push_into(d->d_left, out);
                    spine_flatten_into(d->d_spine, out);
                    ft2::digit_push_into(d->d_right, out);
                    return out;
                }},
            d_repr);
    }

    // -- for_each: O(n) no allocation ----------------------------------------

    template <typename F>
    void for_each(F &&fn) const {
        std::visit(ft2::overloaded{[](const Empty &) {},
                                   [&](const Single &s) { fn(s.d_value); },
                                   [&](const DeepPtr &d) {
                                       ft2::digit_for_each(d->d_left, fn);
                                       spine_for_each(d->d_spine, fn);
                                       ft2::digit_for_each(d->d_right, fn);
                                   }},
                   d_repr);
    }

    // -- append / concat: O(log min(n,m)) ------------------------------------

    auto append(const FingerTree2 &right) const -> FingerTree2 {
        return app3(*this, {}, right);
    }

    static auto concat(const FingerTree2 &left, const FingerTree2 &right)
        -> FingerTree2 {
        return left.append(right);
    }

    // -- split: O(log n) -----------------------------------------------------

    template <typename PRED>
    auto search(PRED &&pred) const -> std::optional<T> {
        auto sp = split(std::forward<PRED>(pred));
        if (!sp.has_value())
            return std::nullopt;
        return std::move(sp->d_pivot);
    }

    template <typename PRED>
    auto split(PRED &&pred) const -> std::optional<Split> {
        ft2::TagPred<Tag> erased(pred);
        return split_impl(erased, ft2::tag_id<Tag>());
    }

    template <typename PRED>
    auto split_at(PRED &&pred) const -> SplitAt {
        auto sp = split(std::forward<PRED>(pred));
        if (!sp.has_value())
            return SplitAt{*this, empty()};
        return SplitAt{std::move(sp->d_left),
                       sp->d_right.cons(std::move(sp->d_pivot))};
    }

    auto split_at_measure(const Tag &threshold) const -> SplitAt
        requires requires(const Tag &a, const Tag &b) {
            { a >= b } -> std::convertible_to<bool>;
        }
    {
        return split_at(
            [&threshold](const Tag &prefix) { return prefix >= threshold; });
    }

    static auto from_sequence(std::vector<T> values) -> FingerTree2 {
        auto result = empty();
        for (auto &v : values)
            result = result.snoc(std::move(v));
        return result;
    }
};

} // namespace smd::tree

#endif
