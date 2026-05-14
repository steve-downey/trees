// src/smd/tree/finger_tree3.hpp                                      -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_FINGER_TREE3
#define INCLUDED_SMD_TREE_FINGER_TREE3

// Hinze-Paterson 2-3 finger tree — lazy-spine implementation.
//
// The spine inside each Deep node is a memoized thunk (erased_thunk from
// smd::thunk) rather than an eagerly-evaluated tree.  Digit overflow in
// cons/snoc creates a suspension; the spine is only forced when accessed
// (view_l, view_r, split, flatten).
//
// This gives Okasaki-style persistent amortization: cons/snoc are O(1)
// amortized even when the tree is used persistently (multiple futures
// branching from the same snapshot), because the debit argument applies
// to the lazy suspension chain rather than to the strict structure.
//
// Compile-cost strategy (same as finger_tree2):
// - kMaxDepth NTTP bounds recursive instantiation (default 3)
// - TagPred erases split predicates at the spine boundary
//
// The cached measure inside SpineSusp allows measure() to be O(1)
// without forcing the spine thunk.

#include <smd/thunk/memoize.hpp>
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

// Reuse ft2 detail types (Digit, Node, TagPred, helpers).
// They are pure value types with no spine dependency.

namespace ft3 {

template <typename... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};
template <typename... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

// -- Digit -------------------------------------------------------------------

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

// -- Node --------------------------------------------------------------------

template <typename T, typename Tag>
struct Node2 {
    Tag d_measure;
    T a;
    T b;
};

template <typename T, typename Tag>
struct Node3 {
    Tag d_measure;
    T a;
    T b;
    T c;
};

template <typename T, typename Tag>
using Node = std::variant<Node2<T, Tag>, Node3<T, Tag>>;

// -- TagPred -----------------------------------------------------------------

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
        overloaded{
            [&](const One<T> &x) -> Tag { return mf(x.a); },
            [&](const Two<T> &x) -> Tag {
                return tag_op<Tag>(mf(x.a), mf(x.b));
            },
            [&](const Three<T> &x) -> Tag {
                return tag_op<Tag>(tag_op<Tag>(mf(x.a), mf(x.b)), mf(x.c));
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
        overloaded{
            [](const One<T> &) -> std::optional<Digit<T>> {
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
        overloaded{
            [](const One<T> &) -> std::optional<Digit<T>> {
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
        assert(false);
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
    return std::visit(
        overloaded{[](const Node2<T, Tag> &x) -> Digit<T> {
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

template <typename T, typename Tag, typename MeasFn>
auto make_node2(MeasFn &&mf, T a, T b) -> Node<T, Tag> {
    auto m = tag_op<Tag>(mf(a), mf(b));
    return Node2<T, Tag>{std::move(m), std::move(a), std::move(b)};
}

template <typename T, typename Tag, typename MeasFn>
auto make_node3(MeasFn &&mf, T a, T b, T c) -> Node<T, Tag> {
    auto m = tag_op<Tag>(tag_op<Tag>(mf(a), mf(b)), mf(c));
    return Node3<T, Tag>{std::move(m), std::move(a), std::move(b),
                         std::move(c)};
}

template <typename T, typename Tag, typename MeasFn>
auto nodes_from(MeasFn &&mf, std::vector<T> elems)
    -> std::vector<Node<T, Tag>> {
    std::vector<Node<T, Tag>> result;
    auto n = elems.size();
    std::size_t i = 0;
    while (n - i > 4) {
        result.push_back(make_node3<T, Tag>(mf, std::move(elems[i]),
                                            std::move(elems[i + 1]),
                                            std::move(elems[i + 2])));
        i += 3;
    }
    switch (n - i) {
    case 2:
        result.push_back(make_node2<T, Tag>(mf, std::move(elems[i]),
                                            std::move(elems[i + 1])));
        break;
    case 3:
        result.push_back(make_node3<T, Tag>(mf, std::move(elems[i]),
                                            std::move(elems[i + 1]),
                                            std::move(elems[i + 2])));
        break;
    case 4:
        result.push_back(make_node2<T, Tag>(mf, std::move(elems[i]),
                                            std::move(elems[i + 1])));
        result.push_back(make_node2<T, Tag>(mf, std::move(elems[i + 2]),
                                            std::move(elems[i + 3])));
        break;
    default:
        assert(false);
    }
    return result;
}

} // namespace ft3

// ============================================================================
//                            FINGER_TREE3
// ============================================================================

template <typename T, typename TAG_TYPE>
struct UnitMeasure3 {
    auto operator()(const T &) const -> TAG_TYPE { return TAG_TYPE{1}; }
};

template <typename T, typename TAG_TYPE = std::size_t,
          typename MEASURE_POLICY = UnitMeasure3<T, TAG_TYPE>, int DEPTH = 0>
class FingerTree3 {
    template <typename, typename, typename, int>
    friend class FingerTree3;

    static constexpr int kMaxDepth = 5;

    using Tag = TAG_TYPE;
    using Meas = MEASURE_POLICY;

    template <typename U>
    using One = ft3::One<U>;
    template <typename U>
    using Two = ft3::Two<U>;
    template <typename U>
    using Three = ft3::Three<U>;
    template <typename U>
    using Four = ft3::Four<U>;
    template <typename U>
    using Digit = ft3::Digit<U>;
    template <typename U, typename V>
    using Node = ft3::Node<U, V>;

    using NodeT = Node<T, Tag>;

    struct NodeMeasure {
        auto operator()(const NodeT &n) const -> Tag {
            return ft3::node_measure(n);
        }
    };

    using SpineTree =
        std::conditional_t<(DEPTH < kMaxDepth),
                           FingerTree3<NodeT, Tag, NodeMeasure, DEPTH + 1>,
                           void>;

    static auto meas_fn() -> Meas { return Meas{}; }
    static auto tag_value(const T &v) -> Tag { return meas_fn()(v); }

    // ========================================================================
    //  SPINE SUSPENSION — the lazy heart of this implementation
    // ========================================================================
    //
    // A SpineSusp pairs a cached measure (available without forcing) with a
    // memoized thunk that, when forced, produces the SpineTree.  Forcing
    // happens at most once per SpineSusp instance; the result is cached by
    // the memoize infrastructure via shared_ptr<State>.
    //
    // Copies of a FingerTree3 that share a DeepPtr also share the same
    // SpineSusp, so forcing in one branch benefits all branches.

    struct SpineSusp {
        Tag d_cached_measure;
        smd::thunk::erased_thunk<SpineTree> d_force;
    };

    using LazySpine =
        std::conditional_t<(DEPTH < kMaxDepth),
                           std::optional<SpineSusp>,
                           std::monostate>;

    static auto empty_spine() -> LazySpine {
        if constexpr (DEPTH < kMaxDepth) {
            return std::nullopt;
        } else {
            return {};
        }
    }

    static auto spine_is_empty(const LazySpine &sp) -> bool {
        if constexpr (DEPTH < kMaxDepth) {
            return !sp.has_value();
        } else {
            (void)sp;
            return true;
        }
    }

    static auto spine_measure(const LazySpine &sp) -> Tag {
        if constexpr (DEPTH < kMaxDepth) {
            if (!sp.has_value())
                return ft3::tag_id<Tag>();
            return sp->d_cached_measure;
        } else {
            (void)sp;
            return ft3::tag_id<Tag>();
        }
    }

    static auto spine_force(const LazySpine &sp) -> decltype(auto)
        requires(DEPTH < kMaxDepth)
    {
        assert(sp.has_value());
        return sp->d_force();
    }

    static auto spine_from_tree(auto tree) -> LazySpine
        requires(DEPTH < kMaxDepth)
    {
        if (tree.is_empty())
            return std::nullopt;
        auto m = tree.measure();
        auto thunk = smd::thunk::memoize(
            [t = std::move(tree)]() { return t; });
        return SpineSusp{std::move(m),
                         smd::thunk::erased_thunk<SpineTree>(
                             std::move(thunk))};
    }

    // Create a lazy spine that, when forced, conses a node onto an old spine.
    static auto spine_lazy_cons(LazySpine old_spine, NodeT node) -> LazySpine {
        if constexpr (DEPTH < kMaxDepth) {
            auto nm = ft3::node_measure(node);
            auto new_m = ft3::tag_op<Tag>(nm, spine_measure(old_spine));
            auto thunk = smd::thunk::memoize(
                [old = std::move(old_spine),
                 n = std::move(node)]() -> SpineTree {
                    if (!old.has_value())
                        return SpineTree::leaf(n);
                    return old->d_force().cons(n);
                });
            return SpineSusp{std::move(new_m),
                             smd::thunk::erased_thunk<SpineTree>(
                                 std::move(thunk))};
        } else {
            (void)old_spine;
            (void)node;
            assert(false && "FingerTree3: spine depth exceeded kMaxDepth");
            return {};
        }
    }

    static auto spine_lazy_snoc(LazySpine old_spine, NodeT node) -> LazySpine {
        if constexpr (DEPTH < kMaxDepth) {
            auto nm = ft3::node_measure(node);
            auto new_m = ft3::tag_op<Tag>(spine_measure(old_spine), nm);
            auto thunk = smd::thunk::memoize(
                [old = std::move(old_spine),
                 n = std::move(node)]() -> SpineTree {
                    if (!old.has_value())
                        return SpineTree::leaf(n);
                    return old->d_force().snoc(n);
                });
            return SpineSusp{std::move(new_m),
                             smd::thunk::erased_thunk<SpineTree>(
                                 std::move(thunk))};
        } else {
            (void)old_spine;
            (void)node;
            assert(false && "FingerTree3: spine depth exceeded kMaxDepth");
            return {};
        }
    }

    // -- Representation ------------------------------------------------------

    struct Empty {};

    struct Single {
        Tag d_measure;
        T d_value;
    };

    struct Deep {
        Tag d_measure;
        Digit<T> d_left;
        LazySpine d_spine;
        Digit<T> d_right;
    };

    using DeepPtr = std::shared_ptr<const Deep>;
    using Repr = std::variant<Empty, Single, DeepPtr>;
    Repr d_repr;

    explicit FingerTree3(Repr r) : d_repr(std::move(r)) {}

    // -- Smart constructors --------------------------------------------------

    static auto make_empty() -> FingerTree3 {
        return FingerTree3(Repr{Empty{}});
    }

    static auto make_single(T value) -> FingerTree3 {
        auto m = tag_value(value);
        return FingerTree3(Repr{Single{std::move(m), std::move(value)}});
    }

    static auto make_deep(Digit<T> left, LazySpine spine, Digit<T> right)
        -> FingerTree3 {
        auto m = ft3::tag_op<Tag>(
            ft3::tag_op<Tag>(ft3::digit_measure<T, Tag>(left, meas_fn()),
                             spine_measure(spine)),
            ft3::digit_measure<T, Tag>(right, meas_fn()));
        return FingerTree3(Repr{std::make_shared<const Deep>(
            Deep{std::move(m), std::move(left), std::move(spine),
                 std::move(right)})});
    }

    static auto digit_to_tree(const Digit<T> &d) -> FingerTree3 {
        return std::visit(
            ft3::overloaded{
                [](const One<T> &x) { return make_single(x.a); },
                [](const Two<T> &x) {
                    return make_deep(One<T>{x.a}, empty_spine(), One<T>{x.b});
                },
                [](const Three<T> &x) {
                    return make_deep(Two<T>{x.a, x.b}, empty_spine(),
                                     One<T>{x.c});
                },
                [](const Four<T> &x) {
                    return make_deep(Two<T>{x.a, x.b}, empty_spine(),
                                     Two<T>{x.c, x.d});
                }},
            d);
    }

    // deep_l: left digit exhausted — force spine to pull a node
    static auto deep_l(LazySpine spine, Digit<T> right) -> FingerTree3 {
        if constexpr (DEPTH < kMaxDepth) {
            if (spine_is_empty(spine))
                return digit_to_tree(right);
            const auto &st = spine_force(spine);
            auto vl = st.view_l();
            assert(vl.has_value());
            auto new_left = ft3::node_to_digit(vl->d_value);
            auto new_spine = spine_from_tree(std::move(vl->d_rest));
            return make_deep(std::move(new_left), std::move(new_spine),
                             std::move(right));
        } else {
            return digit_to_tree(right);
        }
    }

    static auto deep_r(Digit<T> left, LazySpine spine) -> FingerTree3 {
        if constexpr (DEPTH < kMaxDepth) {
            if (spine_is_empty(spine))
                return digit_to_tree(left);
            const auto &st = spine_force(spine);
            auto vr = st.view_r();
            assert(vr.has_value());
            auto new_right = ft3::node_to_digit(vr->d_value);
            auto new_spine = spine_from_tree(std::move(vr->d_rest));
            return make_deep(std::move(left), std::move(new_spine),
                             std::move(new_right));
        } else {
            return digit_to_tree(left);
        }
    }

    // -- app3 ----------------------------------------------------------------

    static auto app3(const FingerTree3 &left, std::vector<T> middle,
                     const FingerTree3 &right) -> FingerTree3 {
        if (left.is_empty()) {
            auto r = right;
            for (auto it = middle.rbegin(); it != middle.rend(); ++it)
                r = r.cons(std::move(*it));
            return r;
        }
        if (right.is_empty()) {
            auto r = left;
            for (auto &e : middle)
                r = r.snoc(std::move(e));
            return r;
        }
        if (left.is_leaf()) {
            auto r = right;
            for (auto it = middle.rbegin(); it != middle.rend(); ++it)
                r = r.cons(std::move(*it));
            return r.cons(std::get<Single>(left.d_repr).d_value);
        }
        if (right.is_leaf()) {
            auto r = left;
            for (auto &e : middle)
                r = r.snoc(std::move(e));
            return r.snoc(std::get<Single>(right.d_repr).d_value);
        }

        if constexpr (DEPTH < kMaxDepth) {
            const auto &ld = *std::get<DeepPtr>(left.d_repr);
            const auto &rd = *std::get<DeepPtr>(right.d_repr);

            auto combined = ft3::digit_to_vec(ld.d_right);
            combined.insert(combined.end(),
                            std::make_move_iterator(middle.begin()),
                            std::make_move_iterator(middle.end()));
            {
                auto rl = ft3::digit_to_vec(rd.d_left);
                combined.insert(combined.end(), rl.begin(), rl.end());
            }
            auto ns = ft3::nodes_from<T, Tag>(meas_fn(), std::move(combined));

            // Lazy spine concatenation: create a thunk that, when forced,
            // concatenates the two spines with the nodes in between.
            auto new_spine_m = ft3::tag_op<Tag>(
                ft3::tag_op<Tag>(spine_measure(ld.d_spine),
                                 [&] {
                                     Tag acc = ft3::tag_id<Tag>();
                                     for (auto &n : ns)
                                         acc = ft3::tag_op<Tag>(
                                             acc, ft3::node_measure(n));
                                     return acc;
                                 }()),
                spine_measure(rd.d_spine));

            auto left_sp = ld.d_spine;
            auto right_sp = rd.d_spine;
            auto thunk = smd::thunk::memoize(
                [left_sp = std::move(left_sp), ns = std::move(ns),
                 right_sp = std::move(right_sp)]() -> SpineTree {
                    auto ls = left_sp.has_value()
                                  ? left_sp->d_force()
                                  : SpineTree::empty();
                    auto rs = right_sp.has_value()
                                  ? right_sp->d_force()
                                  : SpineTree::empty();
                    return SpineTree::app3(ls, std::move(ns), rs);
                });

            LazySpine new_spine;
            if (new_spine_m != ft3::tag_id<Tag>()) {
                new_spine = SpineSusp{
                    std::move(new_spine_m),
                    smd::thunk::erased_thunk<SpineTree>(std::move(thunk))};
            }
            return make_deep(ld.d_left, std::move(new_spine), rd.d_right);
        } else {
            auto r = left.flatten();
            r.insert(r.end(), std::make_move_iterator(middle.begin()),
                     std::make_move_iterator(middle.end()));
            auto rv = right.flatten();
            r.insert(r.end(), rv.begin(), rv.end());
            return from_sequence(std::move(r));
        }
    }

    // -- Result types (declared early for use in private helpers) -----------

  public:
    struct View {
        T d_value;
        FingerTree3 d_rest;
    };
    struct Split {
        FingerTree3 d_left;
        T d_pivot;
        FingerTree3 d_right;
    };
    struct SplitAt {
        FingerTree3 d_left;
        FingerTree3 d_right;
    };

  private:
    // -- Split ---------------------------------------------------------------

    struct DigitSplit {
        std::optional<Digit<T>> d_left;
        T d_pivot;
        std::optional<Digit<T>> d_right;
    };

    static auto split_digit(const ft3::TagPred<Tag> &pred, Tag prefix,
                            const Digit<T> &d) -> std::optional<DigitSplit> {
        auto elems = ft3::digit_to_vec(d);
        auto running = prefix;
        for (std::size_t i = 0; i < elems.size(); ++i) {
            running = ft3::tag_op<Tag>(running, tag_value(elems[i]));
            if (pred(running)) {
                std::optional<Digit<T>> ld, rd;
                if (i > 0) {
                    std::vector<T> lv(elems.begin(),
                                      elems.begin() +
                                          static_cast<std::ptrdiff_t>(i));
                    ld = ft3::vec_to_digit(std::move(lv));
                }
                if (i + 1 < elems.size()) {
                    std::vector<T> rv(elems.begin() +
                                          static_cast<std::ptrdiff_t>(i + 1),
                                      elems.end());
                    rd = ft3::vec_to_digit(std::move(rv));
                }
                return DigitSplit{std::move(ld), std::move(elems[i]),
                                  std::move(rd)};
            }
        }
        return std::nullopt;
    }

    auto split_impl(const ft3::TagPred<Tag> &pred, Tag prefix) const
        -> std::optional<Split> {
        if (is_empty())
            return std::nullopt;

        if (is_leaf()) {
            const auto &s = std::get<Single>(d_repr);
            if (pred(ft3::tag_op<Tag>(prefix, s.d_measure)))
                return Split{make_empty(), s.d_value, make_empty()};
            return std::nullopt;
        }

        if constexpr (DEPTH < kMaxDepth) {
            const auto &d = *std::get<DeepPtr>(d_repr);

            auto vl = ft3::tag_op<Tag>(
                prefix, ft3::digit_measure<T, Tag>(d.d_left, meas_fn()));
            if (pred(vl)) {
                auto ds = split_digit(pred, prefix, d.d_left);
                if (!ds.has_value())
                    return std::nullopt;
                auto lt = ds->d_left.has_value() ? digit_to_tree(*ds->d_left)
                                                 : make_empty();
                auto rt = assemble_l(std::move(ds->d_right), d.d_spine,
                                     d.d_right);
                return Split{std::move(lt), std::move(ds->d_pivot),
                             std::move(rt)};
            }

            auto vm = ft3::tag_op<Tag>(vl, spine_measure(d.d_spine));
            if (pred(vm)) {
                if (spine_is_empty(d.d_spine))
                    return std::nullopt;
                // Force spine for split
                const auto &st = spine_force(d.d_spine);
                auto spine_pred = [&](const Tag &t) {
                    return pred(ft3::tag_op<Tag>(vl, t));
                };
                ft3::TagPred<Tag> erased_sp(spine_pred);
                auto ss = st.split_impl(erased_sp, ft3::tag_id<Tag>());
                if (!ss.has_value())
                    return std::nullopt;

                auto node_prefix =
                    ft3::tag_op<Tag>(vl, ss->d_left.measure());
                auto nd = ft3::node_to_digit(ss->d_pivot);
                auto ns = split_digit(pred, node_prefix, nd);
                if (!ns.has_value())
                    return std::nullopt;

                auto sl = spine_from_tree(std::move(ss->d_left));
                auto sr = spine_from_tree(std::move(ss->d_right));
                auto lt = assemble_r(d.d_left, std::move(sl),
                                     std::move(ns->d_left));
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
            return Split{std::move(lt), std::move(ds->d_pivot),
                         std::move(rt)};
        } else {
            auto vec = flatten();
            auto running = prefix;
            for (std::size_t i = 0; i < vec.size(); ++i) {
                running = ft3::tag_op<Tag>(running, tag_value(vec[i]));
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

    static auto assemble_l(std::optional<Digit<T>> left_d, LazySpine spine,
                           Digit<T> right) -> FingerTree3 {
        if (left_d.has_value())
            return make_deep(std::move(*left_d), std::move(spine),
                             std::move(right));
        return deep_l(std::move(spine), std::move(right));
    }

    static auto assemble_r(Digit<T> left, LazySpine spine,
                           std::optional<Digit<T>> right_d) -> FingerTree3 {
        if (right_d.has_value())
            return make_deep(std::move(left), std::move(spine),
                             std::move(*right_d));
        return deep_r(std::move(left), std::move(spine));
    }

    // -- Spine flatten / for_each (force) ------------------------------------

    static void spine_flatten_into(const LazySpine &sp, std::vector<T> &out) {
        if constexpr (DEPTH < kMaxDepth) {
            if (spine_is_empty(sp))
                return;
            const auto &st = spine_force(sp);
            auto nodes = st.flatten();
            for (auto &n : nodes)
                ft3::node_push_into(n, out);
        }
    }

    template <typename F>
    static void spine_for_each(const LazySpine &sp, const F &fn) {
        if constexpr (DEPTH < kMaxDepth) {
            if (spine_is_empty(sp))
                return;
            spine_force(sp).for_each(
                [&](const NodeT &n) { ft3::node_for_each(n, fn); });
        }
    }

  public:
    using value_type = T;
    using tag_type = Tag;

    FingerTree3() : d_repr(Empty{}) {}

    static auto empty() -> FingerTree3 { return make_empty(); }

    static auto leaf(T value) -> FingerTree3 {
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
            ft3::overloaded{
                [](const Empty &) { return ft3::tag_id<Tag>(); },
                [](const Single &s) -> Tag { return s.d_measure; },
                [](const DeepPtr &d) -> Tag { return d->d_measure; }},
            d_repr);
    }

    auto value() const -> const T & {
        assert(is_leaf());
        return std::get<Single>(d_repr).d_value;
    }

    // -- cons: O(1) — digit overflow creates a thunk, no spine work ----------

    auto cons(T x) const -> FingerTree3 {
        return std::visit(
            ft3::overloaded{
                [&](const Empty &) { return make_single(std::move(x)); },
                [&](const Single &s) {
                    return make_deep(One<T>{std::move(x)}, empty_spine(),
                                     One<T>{s.d_value});
                },
                [&](const DeepPtr &d) -> FingerTree3 {
                    return std::visit(
                        ft3::overloaded{
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
                                    Four<T>{std::move(x), dig.a, dig.b,
                                            dig.c},
                                    d->d_spine, d->d_right);
                            },
                            [&](const Four<T> &dig) -> FingerTree3 {
                                auto node = ft3::make_node3<T, Tag>(
                                    meas_fn(), dig.b, dig.c, dig.d);
                                // LAZY: just create a suspension
                                auto sp = spine_lazy_cons(d->d_spine,
                                                          std::move(node));
                                return make_deep(Two<T>{std::move(x), dig.a},
                                                 std::move(sp), d->d_right);
                            }},
                        d->d_left);
                }},
            d_repr);
    }

    // -- snoc: O(1) — symmetric to cons -------------------------------------

    auto snoc(T x) const -> FingerTree3 {
        return std::visit(
            ft3::overloaded{
                [&](const Empty &) { return make_single(std::move(x)); },
                [&](const Single &s) {
                    return make_deep(One<T>{s.d_value}, empty_spine(),
                                     One<T>{std::move(x)});
                },
                [&](const DeepPtr &d) -> FingerTree3 {
                    return std::visit(
                        ft3::overloaded{
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
                                    Four<T>{dig.a, dig.b, dig.c,
                                            std::move(x)});
                            },
                            [&](const Four<T> &dig) -> FingerTree3 {
                                auto node = ft3::make_node3<T, Tag>(
                                    meas_fn(), dig.a, dig.b, dig.c);
                                auto sp = spine_lazy_snoc(d->d_spine,
                                                          std::move(node));
                                return make_deep(d->d_left, std::move(sp),
                                                 Two<T>{dig.d, std::move(x)});
                            }},
                        d->d_right);
                }},
            d_repr);
    }

    // -- view_l: O(1) amortized (forces spine lazily) ------------------------

    auto view_l() const -> std::optional<View> {
        return std::visit(
            ft3::overloaded{
                [](const Empty &) -> std::optional<View> {
                    return std::nullopt;
                },
                [](const Single &s) -> std::optional<View> {
                    return View{s.d_value, make_empty()};
                },
                [](const DeepPtr &d) -> std::optional<View> {
                    auto h = ft3::digit_head(d->d_left);
                    auto t = ft3::digit_tail(d->d_left);
                    if (t.has_value())
                        return View{h, make_deep(std::move(*t), d->d_spine,
                                                 d->d_right)};
                    return View{h, deep_l(d->d_spine, d->d_right)};
                }},
            d_repr);
    }

    auto view_r() const -> std::optional<View> {
        return std::visit(
            ft3::overloaded{
                [](const Empty &) -> std::optional<View> {
                    return std::nullopt;
                },
                [](const Single &s) -> std::optional<View> {
                    return View{s.d_value, make_empty()};
                },
                [](const DeepPtr &d) -> std::optional<View> {
                    auto l = ft3::digit_last(d->d_right);
                    auto i = ft3::digit_init(d->d_right);
                    if (i.has_value())
                        return View{l, make_deep(d->d_left, d->d_spine,
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
    auto tail() const -> FingerTree3 {
        auto v = view_l();
        return v.has_value() ? std::move(v->d_rest) : empty();
    }
    auto last() const -> T {
        auto v = view_r();
        assert(v.has_value());
        return std::move(v->d_value);
    }
    auto init() const -> FingerTree3 {
        auto v = view_r();
        return v.has_value() ? std::move(v->d_rest) : empty();
    }

    auto flatten() const -> std::vector<T> {
        return std::visit(
            ft3::overloaded{
                [](const Empty &) -> std::vector<T> { return {}; },
                [](const Single &s) -> std::vector<T> { return {s.d_value}; },
                [](const DeepPtr &d) -> std::vector<T> {
                    std::vector<T> out;
                    ft3::digit_push_into(d->d_left, out);
                    spine_flatten_into(d->d_spine, out);
                    ft3::digit_push_into(d->d_right, out);
                    return out;
                }},
            d_repr);
    }

    template <typename F>
    void for_each(F &&fn) const {
        std::visit(
            ft3::overloaded{
                [](const Empty &) {},
                [&](const Single &s) { fn(s.d_value); },
                [&](const DeepPtr &d) {
                    ft3::digit_for_each(d->d_left, fn);
                    spine_for_each(d->d_spine, fn);
                    ft3::digit_for_each(d->d_right, fn);
                }},
            d_repr);
    }

    auto append(const FingerTree3 &right) const -> FingerTree3 {
        return app3(*this, {}, right);
    }
    static auto concat(const FingerTree3 &l, const FingerTree3 &r)
        -> FingerTree3 {
        return l.append(r);
    }

    template <typename PRED>
    auto search(PRED &&pred) const -> std::optional<T> {
        auto sp = split(std::forward<PRED>(pred));
        if (!sp.has_value())
            return std::nullopt;
        return std::move(sp->d_pivot);
    }

    template <typename PRED>
    auto split(PRED &&pred) const -> std::optional<Split> {
        ft3::TagPred<Tag> erased(pred);
        return split_impl(erased, ft3::tag_id<Tag>());
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

    static auto from_sequence(std::vector<T> values) -> FingerTree3 {
        auto r = empty();
        for (auto &v : values)
            r = r.snoc(std::move(v));
        return r;
    }
};

} // namespace smd::tree

#endif
