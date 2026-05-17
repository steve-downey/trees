// src/smd/tree/finger_tree5.hpp                                      -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_FINGER_TREE5
#define INCLUDED_SMD_TREE_FINGER_TREE5

// Hinze-Paterson 2-3 finger tree — uniform-elem, unified-digit implementation.
//
// Like finger_tree4, the spine is the same FingerTree5 type at every depth
// (shared_ptr<const FingerTree5>), and Elem<T,Tag> is the self-recursive
// variant<Leaf, Node2, Node3> carrying its cached measure.
//
// The structural change vs finger_tree4 is the digit representation:
// std::inplace_vector<ElemPtr<T,Tag>, 4> with an explicit size, replacing
// finger_tree4's variant<One, Two, Three, Four>.  Cons/snoc/view/split/concat
// branch on size() rather than dispatching through a four-way std::visit at
// every level.

#include <smd/fixpoint/overloaded.hpp>
#include <smd/typeclass/dual_monoid.hpp>
#include <smd/typeclass/monoid.hpp>

#include <algorithm>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <inplace_vector>
#include <iterator>
#include <limits>
#include <memory>
#include <optional>
#include <utility>
#include <variant>
#include <vector>

namespace smd::tree {

// ============================================================================
//                              DETAIL TYPES
// ============================================================================

namespace ft5 {

using smd::fixpoint::overloaded;

// -- Tag operations ----------------------------------------------------------

template <typename Tag>
constexpr auto tag_id() -> Tag {
    return smd::typeclass::monoid_v<Tag>.identity();
}

template <typename Tag>
constexpr auto tag_op(const Tag &a, const Tag &b) -> Tag {
    return smd::typeclass::monoid_v<Tag>.combine(a, b);
}

// -- Elem: self-recursive element type ---------------------------------------

template <typename T, typename Tag>
struct Elem;

template <typename T, typename Tag>
using ElemPtr = std::shared_ptr<const Elem<T, Tag>>;

template <typename T, typename Tag>
struct Elem {
    Tag d_measure;

    struct Leaf {
        T d_value;
    };
    struct Node2 {
        ElemPtr<T, Tag> a;
        ElemPtr<T, Tag> b;
    };
    struct Node3 {
        ElemPtr<T, Tag> a;
        ElemPtr<T, Tag> b;
        ElemPtr<T, Tag> c;
    };

    std::variant<Leaf, Node2, Node3> d_data;
};

template <typename T, typename Tag>
auto elem_measure(const ElemPtr<T, Tag> &ep) -> Tag {
    return ep->d_measure;
}

template <typename T, typename Tag>
auto is_leaf(const ElemPtr<T, Tag> &ep) -> bool {
    return std::holds_alternative<typename Elem<T, Tag>::Leaf>(ep->d_data);
}

template <typename T, typename Tag>
auto leaf_value(const ElemPtr<T, Tag> &ep) -> const T & {
    return std::get<typename Elem<T, Tag>::Leaf>(ep->d_data).d_value;
}

template <typename T, typename Tag, typename Alloc, typename MeasFn>
auto make_leaf(const Alloc& alloc, MeasFn &&mf, T value) -> ElemPtr<T, Tag> {
    // value is taken by-value: measured first (lvalue), then moved into Leaf.
    using EA = typename std::allocator_traits<Alloc>::template rebind_alloc<Elem<T, Tag>>;
    EA ea(alloc);
    auto m = mf(value);
    return std::allocate_shared<const Elem<T, Tag>>(
        ea, Elem<T, Tag>{std::move(m),
                         typename Elem<T, Tag>::Leaf{std::move(value)}});
}

template <typename T, typename Tag, typename Alloc>
auto make_node2(const Alloc& alloc, ElemPtr<T, Tag> a, ElemPtr<T, Tag> b)
    -> ElemPtr<T, Tag> {
    using EA = typename std::allocator_traits<Alloc>::template rebind_alloc<Elem<T, Tag>>;
    EA ea(alloc);
    auto m = tag_op<Tag>(a->d_measure, b->d_measure);
    return std::allocate_shared<const Elem<T, Tag>>(
        ea, Elem<T, Tag>{std::move(m),
                         typename Elem<T, Tag>::Node2{std::move(a), std::move(b)}});
}

template <typename T, typename Tag, typename Alloc>
auto make_node3(const Alloc& alloc, ElemPtr<T, Tag> a, ElemPtr<T, Tag> b,
                ElemPtr<T, Tag> c) -> ElemPtr<T, Tag> {
    using EA = typename std::allocator_traits<Alloc>::template rebind_alloc<Elem<T, Tag>>;
    EA ea(alloc);
    auto m = tag_op<Tag>(tag_op<Tag>(a->d_measure, b->d_measure), c->d_measure);
    return std::allocate_shared<const Elem<T, Tag>>(
        ea, Elem<T, Tag>{std::move(m),
                         typename Elem<T, Tag>::Node3{std::move(a), std::move(b),
                                                      std::move(c)}});
}

template <typename T, typename Tag>
void flatten_elem(const ElemPtr<T, Tag> &ep, std::vector<T> &out) {
    using E = Elem<T, Tag>;
    std::visit(
        overloaded{
            [&](const typename E::Leaf &lf) { out.push_back(lf.d_value); },
            [&](const typename E::Node2 &n) {
                flatten_elem(n.a, out);
                flatten_elem(n.b, out);
            },
            [&](const typename E::Node3 &n) {
                flatten_elem(n.a, out);
                flatten_elem(n.b, out);
                flatten_elem(n.c, out);
            }},
        ep->d_data);
}

template <typename T, typename Tag, typename F>
void for_each_elem(const ElemPtr<T, Tag> &ep, const F &fn) {
    using E = Elem<T, Tag>;
    std::visit(
        overloaded{
            [&](const typename E::Leaf &lf) { fn(lf.d_value); },
            [&](const typename E::Node2 &n) {
                for_each_elem(n.a, fn);
                for_each_elem(n.b, fn);
            },
            [&](const typename E::Node3 &n) {
                for_each_elem(n.a, fn);
                for_each_elem(n.b, fn);
                for_each_elem(n.c, fn);
            }},
        ep->d_data);
}

// -- Digit: 1..4 ElemPtrs in an inplace_vector -------------------------------
//
// Invariant: when a Digit appears inside a Deep node, 1 <= size() <= 4.
// An empty Digit is only a transient construction artifact in helpers.

template <typename T, typename Tag>
using Digit = std::inplace_vector<ElemPtr<T, Tag>, 4>;

template <typename T, typename Tag>
auto digit_measure(const Digit<T, Tag> &d) -> Tag {
    Tag m = tag_id<Tag>();
    for (const auto &ep : d)
        m = tag_op<Tag>(m, ep->d_measure);
    return m;
}

template <typename T, typename Tag>
auto digit_with_pushed_front(const Digit<T, Tag> &d, ElemPtr<T, Tag> x)
    -> Digit<T, Tag> {
    assert(d.size() < 4);
    Digit<T, Tag> out;
    out.push_back(std::move(x));
    for (const auto &e : d)
        out.push_back(e);
    return out;
}

template <typename T, typename Tag>
auto digit_with_pushed_back(const Digit<T, Tag> &d, ElemPtr<T, Tag> x)
    -> Digit<T, Tag> {
    assert(d.size() < 4);
    Digit<T, Tag> out;
    for (const auto &e : d)
        out.push_back(e);
    out.push_back(std::move(x));
    return out;
}

template <typename T, typename Tag>
auto digit_tail(const Digit<T, Tag> &d) -> Digit<T, Tag> {
    assert(!d.empty());
    Digit<T, Tag> out;
    for (std::size_t i = 1; i < d.size(); ++i)
        out.push_back(d[i]);
    return out;
}

template <typename T, typename Tag>
auto digit_init(const Digit<T, Tag> &d) -> Digit<T, Tag> {
    assert(!d.empty());
    Digit<T, Tag> out;
    for (std::size_t i = 0; i + 1 < d.size(); ++i)
        out.push_back(d[i]);
    return out;
}

// Group a flat sequence of ElemPtrs into Node2/Node3 nodes for the next
// spine level.  Input size must be >= 2.
//
// Input capacity is bounded by 12 (4 right-digit + 4 middle + 4 left-digit
// in app3's Deep-Deep path).  Output has at most 4 nodes from 12 inputs;
// capacity 6 gives a safety margin.
template <typename T, typename Tag, typename Alloc>
auto nodes_from(const Alloc& alloc, std::inplace_vector<ElemPtr<T, Tag>, 12> elems)
    -> std::inplace_vector<ElemPtr<T, Tag>, 6> {
    std::inplace_vector<ElemPtr<T, Tag>, 6> result;
    auto n = elems.size();
    std::size_t i = 0;
    while (n - i > 4) {
        result.push_back(
            make_node3<T, Tag>(alloc, std::move(elems[i]), std::move(elems[i + 1]),
                               std::move(elems[i + 2])));
        i += 3;
    }
    switch (n - i) {
    case 2:
        result.push_back(
            make_node2<T, Tag>(alloc, std::move(elems[i]), std::move(elems[i + 1])));
        break;
    case 3:
        result.push_back(
            make_node3<T, Tag>(alloc, std::move(elems[i]), std::move(elems[i + 1]),
                               std::move(elems[i + 2])));
        break;
    case 4:
        result.push_back(
            make_node2<T, Tag>(alloc, std::move(elems[i]), std::move(elems[i + 1])));
        result.push_back(make_node2<T, Tag>(alloc, std::move(elems[i + 2]),
                                            std::move(elems[i + 3])));
        break;
    default:
        assert(false && "nodes_from: invalid count");
        std::unreachable();
    }
    return result;
}

// Unwrap a Node elem into a 2-or-3 element digit.
template <typename T, typename Tag>
auto elem_to_digit(const ElemPtr<T, Tag> &ep) -> Digit<T, Tag> {
    using E = Elem<T, Tag>;
    Digit<T, Tag> out;
    std::visit(
        overloaded{
            [&](const typename E::Leaf &) {
                assert(false && "elem_to_digit called on Leaf");
                std::unreachable();
            },
            [&](const typename E::Node2 &n) {
                out.push_back(n.a);
                out.push_back(n.b);
            },
            [&](const typename E::Node3 &n) {
                out.push_back(n.a);
                out.push_back(n.b);
                out.push_back(n.c);
            }},
        ep->d_data);
    return out;
}

} // namespace ft5

// ============================================================================
//                            FINGER_TREE5
// ============================================================================

template <typename T, typename TAG_TYPE>
struct UnitMeasure5 {
    auto operator()(const T &) const -> TAG_TYPE { return TAG_TYPE{1}; }
};

// ReversedMeasure5 wraps an existing measure policy and lifts its output into
// DualMonoid<Tag>.  Used by FingerTree5::reversed() to build a tree whose
// prefix measures accumulate under the flipped combine order.
template <typename Policy>
struct ReversedMeasure5 {
    Policy d_inner;
    template <typename T>
    auto operator()(const T &x) const {
        return smd::typeclass::DualMonoid{d_inner(x)};
    }
};

// Forward declaration so FingerTree5 can declare begin()/end() members
// whose return type is FingerTree5Iterator.  The full definition is in
// finger_tree5_iterator.hpp, included at the bottom of this file.
template <typename T, typename Tag, typename MP,
          typename Alloc = std::allocator<std::byte>>
class FingerTree5Iterator;

template <typename T, typename TAG_TYPE = std::size_t,
          typename MEASURE_POLICY = UnitMeasure5<T, TAG_TYPE>,
          typename ALLOCATOR = std::allocator<std::byte>>
class FingerTree5 {
    template <typename, typename, typename, typename>
    friend class FingerTree5Iterator; // iterator needs access to Deep, Repr, etc.

    using Tag = TAG_TYPE;
    using Meas = MEASURE_POLICY;
    using E = ft5::Elem<T, Tag>;
    using EP = ft5::ElemPtr<T, Tag>;
    using Digit = ft5::Digit<T, Tag>;

    using SpinePtr = std::shared_ptr<const FingerTree5>;

    static auto meas_fn() -> Meas { return Meas{}; }

    // -- Representation ------------------------------------------------------

    struct Empty {};

    struct Single {
        EP d_elem;
    };

    struct Deep {
        Tag d_measure;
        Digit d_left;
        SpinePtr d_spine;
        Digit d_right;
    };

    using DeepPtr = std::shared_ptr<const Deep>;
    using Repr = std::variant<Empty, Single, DeepPtr>;
    Repr d_repr;
    [[no_unique_address]] ALLOCATOR d_alloc;

    // Private constructor used by factory helpers; sets both repr and alloc.
    FingerTree5(Repr r, const ALLOCATOR& alloc)
        : d_repr(std::move(r)), d_alloc(alloc) {}

    explicit FingerTree5(Repr r) : d_repr(std::move(r)) {}

    // -- Allocation helpers --------------------------------------------------

    // Allocate a new SpinePtr.  The spine FingerTree5 shell (a small variant)
    // uses make_shared (default allocator); the Deep and Elem nodes inside it
    // were already allocated with d_alloc.  Full allocator threading into the
    // SpinePtr control block requires extended-constructor support and is
    // deferred.
    auto make_spine_ptr(FingerTree5 t) const -> SpinePtr {
        return std::make_shared<const FingerTree5>(std::move(t));
    }

    auto wrap_leaf(T value) const -> EP {
        return ft5::make_leaf<T, Tag>(d_alloc, meas_fn(), std::move(value));
    }

    static auto make_empty() -> FingerTree5 { return {}; }

    static auto make_single(EP elem) -> FingerTree5 {
        return FingerTree5(Repr{Single{std::move(elem)}});
    }

    // Allocator-aware make_deep: uses alloc for the Deep allocation.
    static auto make_deep(const ALLOCATOR& alloc, Digit left, SpinePtr spine,
                          Digit right) -> FingerTree5 {
        using DA = typename std::allocator_traits<ALLOCATOR>::
            template rebind_alloc<Deep>;
        DA da(alloc);
        auto m = ft5::tag_op<Tag>(
            ft5::tag_op<Tag>(ft5::digit_measure(left),
                             spine ? spine->measure() : ft5::tag_id<Tag>()),
            ft5::digit_measure(right));
        return FingerTree5(
            Repr{std::allocate_shared<const Deep>(
                da, Deep{std::move(m), std::move(left), std::move(spine),
                         std::move(right)})},
            alloc);
    }

    // Convenience overload for call sites that already have an allocator in scope.
    auto make_deep(Digit left, SpinePtr spine, Digit right) const
        -> FingerTree5 {
        return make_deep(d_alloc, std::move(left), std::move(spine),
                         std::move(right));
    }

    // -- Rebalancing helpers used by view ------------------------------------

    static auto make_digit(std::initializer_list<EP> elems) -> Digit {
        Digit d;
        for (const auto &e : elems)
            d.push_back(e);
        return d;
    }

    static auto digit_to_tree(const ALLOCATOR& alloc,
                               const Digit &d) -> FingerTree5 {
        assert(!d.empty());
        switch (d.size()) {
        case 1:
            return make_single(d[0]);
        case 2:
            return make_deep(alloc, make_digit({d[0]}), SpinePtr{}, make_digit({d[1]}));
        case 3:
            return make_deep(alloc, make_digit({d[0], d[1]}), SpinePtr{}, make_digit({d[2]}));
        case 4:
            return make_deep(alloc, make_digit({d[0], d[1]}), SpinePtr{}, make_digit({d[2], d[3]}));
        default:
            std::unreachable();
        }
    }

    static auto deep_l(const ALLOCATOR& alloc, SpinePtr spine,
                       Digit right) -> FingerTree5 {
        if (!spine || spine->is_empty())
            return digit_to_tree(alloc, right);
        auto vl = spine->view_l_internal();
        assert(vl.has_value());
        auto new_left = ft5::elem_to_digit<T, Tag>(vl->d_elem);
        SpinePtr new_spine;
        if (!vl->d_rest.is_empty())
            new_spine = std::make_shared<const FingerTree5>(std::move(vl->d_rest));
        return make_deep(alloc, std::move(new_left), std::move(new_spine),
                         std::move(right));
    }

    static auto deep_r(const ALLOCATOR& alloc, Digit left,
                       SpinePtr spine) -> FingerTree5 {
        if (!spine || spine->is_empty())
            return digit_to_tree(alloc, left);
        auto vr = spine->view_r_internal();
        assert(vr.has_value());
        auto new_right = ft5::elem_to_digit<T, Tag>(vr->d_elem);
        SpinePtr new_spine;
        if (!vr->d_rest.is_empty())
            new_spine = std::make_shared<const FingerTree5>(std::move(vr->d_rest));
        return make_deep(alloc, std::move(left), std::move(new_spine),
                         std::move(new_right));
    }

    // -- Internal view (works on ElemPtrs) -----------------------------------

    struct InternalView {
        EP d_elem;
        FingerTree5 d_rest;
    };

    auto view_l_internal() const -> std::optional<InternalView> {
        return std::visit(
            ft5::overloaded{
                [](const Empty &) -> std::optional<InternalView> {
                    return std::nullopt;
                },
                [this](const Single &s) -> std::optional<InternalView> {
                    return InternalView{s.d_elem, {}};
                },
                [this](const DeepPtr &d) -> std::optional<InternalView> {
                    EP h = d->d_left.front();
                    if (d->d_left.size() > 1) {
                        auto t = ft5::digit_tail<T, Tag>(d->d_left);
                        return InternalView{
                            h, make_deep(d_alloc, std::move(t), d->d_spine,
                                         d->d_right)};
                    }
                    return InternalView{h,
                                        deep_l(d_alloc, d->d_spine, d->d_right)};
                }},
            d_repr);
    }

    auto view_r_internal() const -> std::optional<InternalView> {
        return std::visit(
            ft5::overloaded{
                [](const Empty &) -> std::optional<InternalView> {
                    return std::nullopt;
                },
                [this](const Single &s) -> std::optional<InternalView> {
                    return InternalView{s.d_elem, {}};
                },
                [this](const DeepPtr &d) -> std::optional<InternalView> {
                    EP l = d->d_right.back();
                    if (d->d_right.size() > 1) {
                        auto i = ft5::digit_init<T, Tag>(d->d_right);
                        return InternalView{
                            l, make_deep(d_alloc, d->d_left, d->d_spine,
                                         std::move(i))};
                    }
                    return InternalView{l,
                                        deep_r(d_alloc, d->d_left, d->d_spine)};
                }},
            d_repr);
    }

    // -- Internal cons/snoc (works on ElemPtrs) ------------------------------

    auto cons_internal(EP x) const -> FingerTree5 {
        return std::visit(
            ft5::overloaded{
                [&](const Empty &) { return make_single(std::move(x)); },
                [&](const Single &s) {
                    Digit l;
                    l.push_back(std::move(x));
                    Digit r;
                    r.push_back(s.d_elem);
                    return make_deep(std::move(l), SpinePtr{}, std::move(r));
                },
                [&](const DeepPtr &d) -> FingerTree5 {
                    if (d->d_left.size() < 4) {
                        return make_deep(
                            ft5::digit_with_pushed_front<T, Tag>(
                                d->d_left, std::move(x)),
                            d->d_spine, d->d_right);
                    }
                    // Left digit is full: take last 3 as a Node3, push down spine.
                    auto node = ft5::make_node3<T, Tag>(
                        d_alloc, d->d_left[1], d->d_left[2], d->d_left[3]);
                    SpinePtr sp;
                    if (d->d_spine)
                        sp = make_spine_ptr(
                            d->d_spine->cons_internal(std::move(node)));
                    else
                        sp = make_spine_ptr(make_single(std::move(node)));
                    Digit new_left;
                    new_left.push_back(std::move(x));
                    new_left.push_back(d->d_left[0]);
                    return make_deep(std::move(new_left), std::move(sp),
                                     d->d_right);
                }},
            d_repr);
    }

    auto snoc_internal(EP x) const -> FingerTree5 {
        return std::visit(
            ft5::overloaded{
                [&](const Empty &) { return make_single(std::move(x)); },
                [&](const Single &s) {
                    Digit l;
                    l.push_back(s.d_elem);
                    Digit r;
                    r.push_back(std::move(x));
                    return make_deep(std::move(l), SpinePtr{}, std::move(r));
                },
                [&](const DeepPtr &d) -> FingerTree5 {
                    if (d->d_right.size() < 4) {
                        return make_deep(
                            d->d_left, d->d_spine,
                            ft5::digit_with_pushed_back<T, Tag>(
                                d->d_right, std::move(x)));
                    }
                    // Right digit is full: take first 3 as a Node3, push down spine.
                    auto node = ft5::make_node3<T, Tag>(
                        d_alloc, d->d_right[0], d->d_right[1], d->d_right[2]);
                    SpinePtr sp;
                    if (d->d_spine)
                        sp = make_spine_ptr(
                            d->d_spine->snoc_internal(std::move(node)));
                    else
                        sp = make_spine_ptr(make_single(std::move(node)));
                    Digit new_right;
                    new_right.push_back(d->d_right[3]);
                    new_right.push_back(std::move(x));
                    return make_deep(d->d_left, std::move(sp),
                                     std::move(new_right));
                }},
            d_repr);
    }

    // -- Internal split ------------------------------------------------------
    //
    // Predicate is templated end-to-end: no std::function allocation per
    // recursive call.  The predicate flows by const reference; the running
    // prefix is threaded as a runtime value, so a single Pred type satisfies
    // the entire descent.

    struct InternalSplit {
        FingerTree5 d_left;
        EP d_pivot;
        FingerTree5 d_right;
    };

    struct DigitSplit {
        std::optional<Digit> d_left;
        EP d_pivot;
        std::optional<Digit> d_right;
    };

    template <typename PRED>
    static auto split_digit(const PRED &pred, Tag prefix, const Digit &d)
        -> std::optional<DigitSplit> {
        Tag running = prefix;
        for (std::size_t i = 0; i < d.size(); ++i) {
            running = ft5::tag_op<Tag>(running, d[i]->d_measure);
            if (pred(running)) {
                std::optional<Digit> left_d;
                std::optional<Digit> right_d;
                if (i > 0) {
                    Digit ld;
                    for (std::size_t k = 0; k < i; ++k)
                        ld.push_back(d[k]);
                    left_d = std::move(ld);
                }
                if (i + 1 < d.size()) {
                    Digit rd;
                    for (std::size_t k = i + 1; k < d.size(); ++k)
                        rd.push_back(d[k]);
                    right_d = std::move(rd);
                }
                return DigitSplit{std::move(left_d), d[i], std::move(right_d)};
            }
        }
        return std::nullopt;
    }

    template <typename PRED>
    static auto split_within_elem(const PRED &pred, Tag prefix,
                                  const EP &node_ep) -> DigitSplit {
        auto children = ft5::elem_to_digit<T, Tag>(node_ep);
        auto ds = split_digit(pred, prefix, children);
        assert(ds.has_value() &&
               "split_within_elem: predicate never triggered");
        return std::move(*ds);
    }

    static auto assemble_l(const ALLOCATOR& alloc, std::optional<Digit> left_d,
                           SpinePtr spine, Digit right) -> FingerTree5 {
        if (left_d.has_value())
            return make_deep(alloc, std::move(*left_d), std::move(spine),
                             std::move(right));
        return deep_l(alloc, std::move(spine), std::move(right));
    }

    static auto assemble_r(const ALLOCATOR& alloc, Digit left, SpinePtr spine,
                           std::optional<Digit> right_d) -> FingerTree5 {
        if (right_d.has_value())
            return make_deep(alloc, std::move(left), std::move(spine),
                             std::move(*right_d));
        return deep_r(alloc, std::move(left), std::move(spine));
    }

    template <typename PRED>
    auto split_impl(const PRED &pred, Tag prefix) const
        -> std::optional<InternalSplit> {
        if (is_empty())
            return std::nullopt;

        if (auto *s = std::get_if<Single>(&d_repr)) {
            if (pred(ft5::tag_op<Tag>(prefix, s->d_elem->d_measure)))
                return InternalSplit{make_empty(), s->d_elem, make_empty()};
            return std::nullopt;
        }

        const auto &d = *std::get<DeepPtr>(d_repr);

        auto vl = ft5::tag_op<Tag>(prefix, ft5::digit_measure(d.d_left));
        if (pred(vl)) {
            auto ds = split_digit(pred, prefix, d.d_left);
            if (!ds.has_value())
                return std::nullopt;
            auto lt = ds->d_left.has_value() ? digit_to_tree(d_alloc, *ds->d_left)
                                              : FingerTree5{};
            auto rt = assemble_l(d_alloc, std::move(ds->d_right), d.d_spine,
                                 d.d_right);
            return InternalSplit{std::move(lt), std::move(ds->d_pivot),
                                 std::move(rt)};
        }

        auto spine_m =
            d.d_spine ? d.d_spine->measure() : ft5::tag_id<Tag>();
        auto vm = ft5::tag_op<Tag>(vl, spine_m);
        if (pred(vm)) {
            if (!d.d_spine || d.d_spine->is_empty())
                return std::nullopt;

            auto ss = d.d_spine->split_impl(pred, vl);
            if (!ss.has_value())
                return std::nullopt;

            auto node_prefix =
                ft5::tag_op<Tag>(vl, ss->d_left.measure());
            auto ns = split_within_elem(pred, node_prefix, ss->d_pivot);

            SpinePtr sl;
            if (!ss->d_left.is_empty())
                sl = make_spine_ptr(std::move(ss->d_left));
            SpinePtr sr;
            if (!ss->d_right.is_empty())
                sr = make_spine_ptr(std::move(ss->d_right));

            auto lt = assemble_r(d_alloc, d.d_left, std::move(sl),
                                 std::move(ns.d_left));
            auto rt = assemble_l(d_alloc, std::move(ns.d_right), std::move(sr),
                                 d.d_right);
            return InternalSplit{std::move(lt), std::move(ns.d_pivot),
                                 std::move(rt)};
        }

        auto ds = split_digit(pred, vm, d.d_right);
        if (!ds.has_value())
            return std::nullopt;
        auto lt = assemble_r(d_alloc, d.d_left, d.d_spine,
                             std::move(ds->d_left));
        auto rt = ds->d_right.has_value() ? digit_to_tree(d_alloc, *ds->d_right)
                                           : FingerTree5{};
        return InternalSplit{std::move(lt), std::move(ds->d_pivot),
                             std::move(rt)};
    }

    // -- app3: Hinze-Paterson concatenation ----------------------------------

    static auto app3(const ALLOCATOR& alloc, const FingerTree5 &left,
                     std::inplace_vector<EP, 6> middle,
                     const FingerTree5 &right) -> FingerTree5 {
        if (left.is_empty()) {
            auto result = right;
            for (auto it = middle.rbegin(); it != middle.rend(); ++it)
                result = result.cons_internal(std::move(*it));
            return result;
        }
        if (right.is_empty()) {
            auto result = left;
            for (auto &elem : middle)
                result = result.snoc_internal(std::move(elem));
            return result;
        }
        if (auto *ls = std::get_if<Single>(&left.d_repr)) {
            auto result = right;
            for (auto it = middle.rbegin(); it != middle.rend(); ++it)
                result = result.cons_internal(std::move(*it));
            return result.cons_internal(ls->d_elem);
        }
        if (auto *rs = std::get_if<Single>(&right.d_repr)) {
            auto result = left;
            for (auto &elem : middle)
                result = result.snoc_internal(std::move(elem));
            return result.snoc_internal(rs->d_elem);
        }

        const auto &ld = *std::get<DeepPtr>(left.d_repr);
        const auto &rd = *std::get<DeepPtr>(right.d_repr);

        // combined ≤ 4 (right digit) + 4 (middle) + 4 (left digit) = 12
        std::inplace_vector<EP, 12> combined;
        for (const auto &ep : ld.d_right)
            combined.push_back(ep);
        for (auto &ep : middle)
            combined.push_back(std::move(ep));
        for (const auto &ep : rd.d_left)
            combined.push_back(ep);
        auto ns = ft5::nodes_from<T, Tag>(alloc, std::move(combined));

        auto left_spine =
            ld.d_spine ? *ld.d_spine : FingerTree5{};
        auto right_spine =
            rd.d_spine ? *rd.d_spine : FingerTree5{};
        auto new_spine =
            FingerTree5::app3(alloc, left_spine, std::move(ns), right_spine);
        SpinePtr sp;
        if (!new_spine.is_empty())
            sp = std::make_shared<const FingerTree5>(std::move(new_spine));
        return make_deep(alloc, ld.d_left, std::move(sp), rd.d_right);
    }

    // -- Bottom-up construction from a flat ElemPtr sequence -----------------
    //
    // Builds a balanced finger tree from a pre-allocated sequence of ElemPtrs
    // without going through repeated snoc.  The snoc path allocates O(N) Deep
    // and SpinePtr objects; this path allocates only O(log N) of each.
    //
    // The N-element input is split: 2 elements go to the left digit, 2 to the
    // right digit, and the remaining N-4 elements are grouped into Node2/Node3
    // objects which are fed into a recursive call.  The recursion depth is
    // O(log_3 N), and total Elem allocations are 1.5N vs 2N for repeated snoc.
    //
    // Small cases (N ≤ 8) are placed directly into left/right digits with an
    // empty spine, avoiding the node-grouping pass entirely.

    static auto from_elems_impl(const ALLOCATOR& alloc,
                                std::vector<EP>  elems) -> FingerTree5 {
        const auto n = elems.size();
        if (n == 0) return {};
        if (n == 1) return make_single(std::move(elems[0]));

        if (n <= 8) {
            const auto lsz = std::min(n / 2, std::size_t{4});
            Digit left_d, right_d;
            for (std::size_t i = 0; i < lsz; ++i)
                left_d.push_back(std::move(elems[i]));
            for (std::size_t i = lsz; i < n; ++i)
                right_d.push_back(std::move(elems[i]));
            return make_deep(alloc, std::move(left_d), SpinePtr{},
                             std::move(right_d));
        }

        // n >= 9: 2 left, 2 right, group middle (n-4) into nodes.
        Digit left_d, right_d;
        left_d.push_back(std::move(elems[0]));
        left_d.push_back(std::move(elems[1]));
        right_d.push_back(std::move(elems[n - 2]));
        right_d.push_back(std::move(elems[n - 1]));

        std::vector<EP> nodes;
        nodes.reserve((n - 4 + 2) / 3);
        std::size_t i   = 2;
        const auto  end = n - 2;
        while (end - i > 4) {
            nodes.push_back(ft5::make_node3<T, Tag>(
                alloc, std::move(elems[i]), std::move(elems[i + 1]),
                std::move(elems[i + 2])));
            i += 3;
        }
        switch (end - i) {
        case 2:
            nodes.push_back(ft5::make_node2<T, Tag>(
                alloc, std::move(elems[i]), std::move(elems[i + 1])));
            break;
        case 3:
            nodes.push_back(ft5::make_node3<T, Tag>(
                alloc, std::move(elems[i]), std::move(elems[i + 1]),
                std::move(elems[i + 2])));
            break;
        case 4:
            nodes.push_back(ft5::make_node2<T, Tag>(
                alloc, std::move(elems[i]), std::move(elems[i + 1])));
            nodes.push_back(ft5::make_node2<T, Tag>(
                alloc, std::move(elems[i + 2]), std::move(elems[i + 3])));
            break;
        default:
            std::unreachable();
        }
        auto spine = from_elems_impl(alloc, std::move(nodes));
        SpinePtr sp = spine.is_empty()
                          ? SpinePtr{}
                          : std::make_shared<const FingerTree5>(std::move(spine));
        return make_deep(alloc, std::move(left_d), std::move(sp),
                         std::move(right_d));
    }

    // -- Internal flatten / for_each -----------------------------------------

    void flatten_elems(std::vector<T> &out) const {
        std::visit(
            ft5::overloaded{
                [](const Empty &) {},
                [&](const Single &s) { ft5::flatten_elem(s.d_elem, out); },
                [&](const DeepPtr &d) {
                    for (const auto &ep : d->d_left)
                        ft5::flatten_elem(ep, out);
                    if (d->d_spine)
                        d->d_spine->flatten_elems(out);
                    for (const auto &ep : d->d_right)
                        ft5::flatten_elem(ep, out);
                }},
            d_repr);
    }

    template <typename F>
    void for_each_internal(const F &fn) const {
        std::visit(
            ft5::overloaded{
                [](const Empty &) {},
                [&](const Single &s) { ft5::for_each_elem(s.d_elem, fn); },
                [&](const DeepPtr &d) {
                    for (const auto &ep : d->d_left)
                        ft5::for_each_elem(ep, fn);
                    if (d->d_spine)
                        d->d_spine->for_each_internal(fn);
                    for (const auto &ep : d->d_right)
                        ft5::for_each_elem(ep, fn);
                }},
            d_repr);
    }

    // ========================================================================
    //                          PUBLIC INTERFACE
    // ========================================================================
  public:
    // -----------------------------------------------------------------------
    // Container / ReversibleContainer named requirements
    // -----------------------------------------------------------------------

    using value_type             = T;
    using tag_type               = Tag;         // non-standard, FT5-specific
    using reference              = const T &;   // immutable: all access is const
    using const_reference        = const T &;
    using allocator_type         = ALLOCATOR;
    using iterator               = FingerTree5Iterator<T, TAG_TYPE, MEASURE_POLICY, ALLOCATOR>;
    using const_iterator         = iterator;    // immutable: no mutable iteration
    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using difference_type        = std::ptrdiff_t;
    using size_type              = std::size_t;

    // -- Construction --------------------------------------------------------

    // Default constructor: variant value-initialises to its first alternative
    // (Empty), so FingerTree5{} is a valid empty tree — no factory call needed.
    FingerTree5() = default;
    explicit FingerTree5(const ALLOCATOR& alloc) : d_alloc(alloc) {}

    FingerTree5(const FingerTree5&)            = default;
    FingerTree5(FingerTree5&&)                 = default;

    // Copy assignment: propagate allocator only if the trait says so.
    // For pmr::polymorphic_allocator, propagation is false — the existing
    // allocator is kept and only the tree structure is copied.
    auto operator=(const FingerTree5& other) & -> FingerTree5& {
        d_repr = other.d_repr;
        if constexpr (std::allocator_traits<ALLOCATOR>::
                          propagate_on_container_copy_assignment::value)
            d_alloc = other.d_alloc;
        return *this;
    }

    auto operator=(FingerTree5&& other) & noexcept -> FingerTree5& {
        d_repr = std::move(other.d_repr);
        if constexpr (std::allocator_traits<ALLOCATOR>::
                          propagate_on_container_move_assignment::value)
            d_alloc = std::move(other.d_alloc);
        return *this;
    }

    [[nodiscard]] auto get_allocator() const noexcept -> allocator_type {
        return d_alloc;
    }

    [[nodiscard]] static auto leaf(T value) -> FingerTree5 {
        return make_single(ft5::make_leaf<T, Tag>(ALLOCATOR{}, meas_fn(),
                                                  std::move(value)));
    }

    // -- Container query members ---------------------------------------------

    [[nodiscard]] auto empty() const noexcept -> bool {
        return std::holds_alternative<Empty>(d_repr);
    }

    // is_empty() is an alias kept for backward compatibility.
    [[nodiscard]] auto is_empty() const noexcept -> bool { return empty(); }

    // size() is O(1) for the default unit measure; O(N) for custom measures.
    [[nodiscard]] auto size() const -> size_type {
        if constexpr (std::same_as<Tag, std::size_t> &&
                      std::same_as<Meas, UnitMeasure5<T, std::size_t>>) {
            return measure();
        } else {
            size_type n = 0;
            for_each([&](const T &) { ++n; });
            return n;
        }
    }

    [[nodiscard]] auto max_size() const noexcept -> size_type {
        return std::numeric_limits<size_type>::max();
    }

    void swap(FingerTree5 &other) noexcept { std::swap(d_repr, other.d_repr); }

    friend void swap(FingerTree5 &a, FingerTree5 &b) noexcept { a.swap(b); }

    // element-wise equality; short-circuits on size for unit-measure trees
    // Hidden friend: element-wise equality via flatten() avoids requiring the
    // iterator to be complete at class-body parse time.
    friend auto operator==(const FingerTree5 &lhs,
                           const FingerTree5 &rhs) -> bool {
        if constexpr (std::same_as<Tag, std::size_t> &&
                      std::same_as<Meas, UnitMeasure5<T, std::size_t>>) {
            if (lhs.measure() != rhs.measure()) return false;
        }
        return lhs.flatten() == rhs.flatten();
    }

    // front / back (SequenceContainer-style convenience; no mutation)
    [[nodiscard]] auto front() const -> const_reference { return head_ref(); }
    [[nodiscard]] auto back()  const -> const_reference { return last_ref(); }

    [[nodiscard]] auto is_leaf() const -> bool {
        return std::holds_alternative<Single>(d_repr);
    }

    [[nodiscard]] auto is_branch() const -> bool {
        return std::holds_alternative<DeepPtr>(d_repr);
    }

    [[nodiscard]] auto measure() const -> Tag {
        return std::visit(
            ft5::overloaded{
                [](const Empty &) { return ft5::tag_id<Tag>(); },
                [](const Single &s) -> Tag { return s.d_elem->d_measure; },
                [](const DeepPtr &d) -> Tag { return d->d_measure; }},
            d_repr);
    }

    [[nodiscard]] auto value() const -> const T & {
        assert(is_leaf());
        auto &s = std::get<Single>(d_repr);
        assert(ft5::is_leaf(s.d_elem));
        return ft5::leaf_value(s.d_elem);
    }

    // -- spine_depth: O(log N) -----------------------------------------------
    //
    // Returns the number of nested spine levels (0 for Empty/Single, 1 for a
    // Deep with no spine, 1 + spine->spine_depth() otherwise).
    //
    // Structural invariant: spine_depth() <= floor(log2(N)) for any N-element
    // tree.  Proof sketch: an element at spine level d is a Node covering >= 2^d
    // leaves, so a non-empty spine at level d requires N >= 2^d.
    //
    // Consequence: every recursive call chain in this class — flatten_elems,
    // for_each_internal, view_l/r spine-borrowing, and the shared_ptr destructor
    // cascade — is bounded by O(log N) frames and cannot stack-overflow for any
    // tree that can physically exist.  This bound is tested by
    // "FingerTree5 - SpineDepthIsLogarithmic" and exercised at N=10'000 by
    // "FingerTree5 - LargeTreeRecursionNoStackOverflow".

    [[nodiscard]] auto spine_depth() const -> std::size_t {
        return std::visit(
            ft5::overloaded{
                [](const Empty &) -> std::size_t { return 0; },
                [](const Single &) -> std::size_t { return 0; },
                [](const DeepPtr &d) -> std::size_t {
                    if (!d->d_spine || d->d_spine->is_empty())
                        return 1;
                    return 1 + d->d_spine->spine_depth();
                }},
            d_repr);
    }

    // -- begin / end: O(1) or O(N) depending on measure ----------------------
    //
    // Returns a bidirectional iterator to the first / past-the-end element.
    // Defined out-of-line below (after finger_tree5_iterator.hpp is included).
    // For the default measure <T, std::size_t, UnitMeasure5> the size is
    // computed in O(1) via measure(); for custom measures it is O(N).

    auto begin() const -> iterator;
    auto end()   const -> iterator;

    // Definitions are out-of-line in finger_tree5_iterator.hpp (iterator must
    // be a complete type for std::reverse_iterator instantiation).
    auto cbegin()  const -> const_iterator;
    auto cend()    const -> const_iterator;
    auto rbegin()  const -> reverse_iterator;
    auto rend()    const -> reverse_iterator;
    auto crbegin() const -> const_reverse_iterator;
    auto crend()   const -> const_reverse_iterator;

    // -- cons / snoc: O(1) amortized -----------------------------------------

    [[nodiscard]] auto cons(T x) const -> FingerTree5 {
        return cons_internal(wrap_leaf(std::move(x)));
    }

    [[nodiscard]] auto snoc(T x) const -> FingerTree5 {
        return snoc_internal(wrap_leaf(std::move(x)));
    }

    // -- view_l / view_r: O(1) amortized -------------------------------------

    struct View {
        T d_value;
        FingerTree5 d_rest;
    };

    [[nodiscard]] auto view_l() const -> std::optional<View> {
        auto iv = view_l_internal();
        if (!iv.has_value())
            return std::nullopt;
        assert(ft5::is_leaf(iv->d_elem));
        return View{ft5::leaf_value(iv->d_elem), std::move(iv->d_rest)};
    }

    [[nodiscard]] auto view_r() const -> std::optional<View> {
        auto iv = view_r_internal();
        if (!iv.has_value())
            return std::nullopt;
        assert(ft5::is_leaf(iv->d_elem));
        return View{ft5::leaf_value(iv->d_elem), std::move(iv->d_rest)};
    }

    [[nodiscard]] auto head() const -> T {
        auto v = view_l();
        assert(v.has_value());
        return std::move(v->d_value);
    }

    // head_ref / last_ref: O(1) amortized, return const T& into the leaf.
    // Safe while *this is alive (the leaf is structurally shared with the
    // tree).  Avoids copying T for read-only access of large element types.
    [[nodiscard]] auto head_ref() const -> const T & {
        auto iv = view_l_internal();
        assert(iv.has_value() && ft5::is_leaf(iv->d_elem));
        return ft5::leaf_value(iv->d_elem);
    }

    [[nodiscard]] auto tail() const -> FingerTree5 {
        auto v = view_l();
        return v.has_value() ? std::move(v->d_rest) : FingerTree5{};
    }

    [[nodiscard]] auto last() const -> T {
        auto v = view_r();
        assert(v.has_value());
        return std::move(v->d_value);
    }

    [[nodiscard]] auto last_ref() const -> const T & {
        auto iv = view_r_internal();
        assert(iv.has_value() && ft5::is_leaf(iv->d_elem));
        return ft5::leaf_value(iv->d_elem);
    }

    [[nodiscard]] auto init() const -> FingerTree5 {
        auto v = view_r();
        return v.has_value() ? std::move(v->d_rest) : FingerTree5{};
    }

    // -- flatten / for_each: O(n) --------------------------------------------

    [[nodiscard]] auto flatten() const -> std::vector<T> {
        std::vector<T> out;
        // Reserve when measure() equals element count (unit measure case).
        // For custom measures (rope byte count, interval max-end, etc.) this
        // condition is false and we fall back to push_back reallocation.
        if constexpr (std::same_as<Tag, std::size_t> &&
                      std::same_as<Meas, UnitMeasure5<T, std::size_t>>) {
            out.reserve(measure());
        }
        flatten_elems(out);
        return out;
    }

    template <typename F>
    void for_each(F &&fn) const {
        for_each_internal(fn);
    }

    // Build from a vector using the default allocator (static; use the
    // allocator-extended overload below for a custom allocator).
    [[nodiscard]] static auto from_sequence(std::vector<T> values) -> FingerTree5 {
        return from_sequence(std::move(values), ALLOCATOR{});
    }

    // Allocator-extended from_sequence: all node allocations use alloc.
    [[nodiscard]] static auto from_sequence(std::vector<T> values,
                                            const ALLOCATOR& alloc) -> FingerTree5 {
        if (values.empty()) return {};
        auto mf = meas_fn();
        std::vector<EP> elems;
        elems.reserve(values.size());
        for (auto &v : values)
            elems.push_back(ft5::make_leaf<T, Tag>(alloc, mf, std::move(v)));
        return from_elems_impl(alloc, std::move(elems));
    }

    // -- append / concat: O(log min(n, m)) -----------------------------------

    [[nodiscard]] auto append(const FingerTree5 &right) const -> FingerTree5 {
        return app3(d_alloc, *this, {}, right);
    }

    [[nodiscard]] static auto concat(const FingerTree5 &left,
                                     const FingerTree5 &right) -> FingerTree5 {
        return left.append(right);
    }

    // -- split: O(log n) -----------------------------------------------------

    struct Split {
        FingerTree5 d_left;
        T d_pivot;
        FingerTree5 d_right;
    };

    struct SplitAt {
        FingerTree5 d_left;
        FingerTree5 d_right;
    };

    template <typename PRED>
    [[nodiscard]] auto search(PRED &&pred) const -> std::optional<T> {
        auto sp = split(std::forward<PRED>(pred));
        if (!sp.has_value())
            return std::nullopt;
        return std::move(sp->d_pivot);
    }

    template <typename PRED>
    [[nodiscard]] auto split(PRED &&pred) const -> std::optional<Split> {
        auto is = split_impl(pred, ft5::tag_id<Tag>());
        if (!is.has_value())
            return std::nullopt;
        assert(ft5::is_leaf(is->d_pivot));
        return Split{std::move(is->d_left),
                     ft5::leaf_value(is->d_pivot),
                     std::move(is->d_right)};
    }

    template <typename PRED>
    [[nodiscard]] auto split_at(PRED &&pred) const -> SplitAt {
        auto sp = split(std::forward<PRED>(pred));
        if (!sp.has_value())
            return SplitAt{*this, {}};
        return SplitAt{std::move(sp->d_left),
                       sp->d_right.cons(std::move(sp->d_pivot))};
    }

    [[nodiscard]] auto split_at_measure(const Tag &threshold) const -> SplitAt
        requires requires(const Tag &a, const Tag &b) {
            { a >= b } -> std::convertible_to<bool>;
        }
    {
        return split_at(
            [&threshold](const Tag &prefix) { return prefix >= threshold; });
    }

    // -- reversed: O(N) eager rebuild ----------------------------------------
    //
    // Returns a new tree containing the same elements in reverse order.
    // The tag type of the result is DualMonoid<Tag>; prefix-accumulated
    // measures on the reversed tree grow from the new left edge (old right
    // edge) under the flipped combine order.
    //
    // Implementation is Option A from the design doc: flatten, reverse, rebuild.
    // Cost: O(N) time and O(N) space.  A lazy structural reverse is possible
    // only for commutative monoids (cached measures would otherwise be wrong);
    // the eager rebuild is always correct.

    [[nodiscard]] auto reversed() const
        -> FingerTree5<T, smd::typeclass::DualMonoid<Tag>, ReversedMeasure5<Meas>>
    {
        using DT = FingerTree5<T, smd::typeclass::DualMonoid<Tag>, ReversedMeasure5<Meas>>;
        auto v = flatten();
        std::reverse(v.begin(), v.end());
        return DT::from_sequence(std::move(v));
    }
};

} // namespace smd::tree

// Iterator support: include after the class so the iterator can reference the
// complete FingerTree5 type.  This brings FingerTree5Iterator into scope and
// defines the free begin/end overloads AND the out-of-line definitions of
// FingerTree5::begin() / FingerTree5::end() declared in the class body above.
#include <smd/tree/finger_tree5_iterator.hpp>

#endif
