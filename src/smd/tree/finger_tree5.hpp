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

#include <smd/typeclass/monoid.hpp>

#include <cassert>
#include <cstddef>
#include <inplace_vector>
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

template <typename... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};
template <typename... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

// -- Tag operations ----------------------------------------------------------

template <typename Tag>
inline auto tag_id() -> Tag {
    return smd::typeclass::monoid_v<Tag>.identity();
}

template <typename Tag>
inline auto tag_op(const Tag &a, const Tag &b) -> Tag {
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

template <typename T, typename Tag, typename MeasFn>
auto make_leaf(MeasFn &&mf, T value) -> ElemPtr<T, Tag> {
    auto m = mf(value);
    return std::make_shared<const Elem<T, Tag>>(
        Elem<T, Tag>{std::move(m),
                     typename Elem<T, Tag>::Leaf{std::move(value)}});
}

template <typename T, typename Tag>
auto make_node2(ElemPtr<T, Tag> a, ElemPtr<T, Tag> b) -> ElemPtr<T, Tag> {
    auto m = tag_op<Tag>(a->d_measure, b->d_measure);
    return std::make_shared<const Elem<T, Tag>>(
        Elem<T, Tag>{std::move(m),
                     typename Elem<T, Tag>::Node2{std::move(a), std::move(b)}});
}

template <typename T, typename Tag>
auto make_node3(ElemPtr<T, Tag> a, ElemPtr<T, Tag> b, ElemPtr<T, Tag> c)
    -> ElemPtr<T, Tag> {
    auto m = tag_op<Tag>(tag_op<Tag>(a->d_measure, b->d_measure), c->d_measure);
    return std::make_shared<const Elem<T, Tag>>(
        Elem<T, Tag>{std::move(m),
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

// Unwrap a Node elem into a 2-or-3 element digit.
template <typename T, typename Tag>
auto elem_to_digit(const ElemPtr<T, Tag> &ep) -> Digit<T, Tag> {
    using E = Elem<T, Tag>;
    Digit<T, Tag> out;
    std::visit(
        overloaded{
            [&](const typename E::Leaf &) {
                assert(false && "elem_to_digit called on Leaf");
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

template <typename T, typename TAG_TYPE = std::size_t,
          typename MEASURE_POLICY = UnitMeasure5<T, TAG_TYPE>>
class FingerTree5 {
    using Tag = TAG_TYPE;
    using Meas = MEASURE_POLICY;
    using E = ft5::Elem<T, Tag>;
    using EP = ft5::ElemPtr<T, Tag>;
    using Digit = ft5::Digit<T, Tag>;

    using SpinePtr = std::shared_ptr<const FingerTree5>;

    static auto meas_fn() -> Meas { return Meas{}; }

    static auto wrap_leaf(T value) -> EP {
        return ft5::make_leaf<T, Tag>(meas_fn(), std::move(value));
    }

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

    explicit FingerTree5(Repr r) : d_repr(std::move(r)) {}

    static auto make_empty() -> FingerTree5 {
        return FingerTree5(Repr{Empty{}});
    }

    static auto make_single(EP elem) -> FingerTree5 {
        return FingerTree5(Repr{Single{std::move(elem)}});
    }

    static auto make_deep(Digit left, SpinePtr spine, Digit right)
        -> FingerTree5 {
        auto m = ft5::tag_op<Tag>(
            ft5::tag_op<Tag>(ft5::digit_measure(left),
                             spine ? spine->measure() : ft5::tag_id<Tag>()),
            ft5::digit_measure(right));
        return FingerTree5(Repr{std::make_shared<const Deep>(
            Deep{std::move(m), std::move(left), std::move(spine),
                 std::move(right)})});
    }

    // -- Rebalancing helpers used by view ------------------------------------

    static auto digit_to_tree(const Digit &d) -> FingerTree5 {
        assert(!d.empty());
        switch (d.size()) {
        case 1:
            return make_single(d[0]);
        case 2: {
            Digit l, r;
            l.push_back(d[0]);
            r.push_back(d[1]);
            return make_deep(std::move(l), nullptr, std::move(r));
        }
        case 3: {
            Digit l, r;
            l.push_back(d[0]);
            l.push_back(d[1]);
            r.push_back(d[2]);
            return make_deep(std::move(l), nullptr, std::move(r));
        }
        case 4: {
            Digit l, r;
            l.push_back(d[0]);
            l.push_back(d[1]);
            r.push_back(d[2]);
            r.push_back(d[3]);
            return make_deep(std::move(l), nullptr, std::move(r));
        }
        default:
            std::unreachable();
        }
    }

    static auto deep_l(SpinePtr spine, Digit right) -> FingerTree5 {
        if (!spine || spine->is_empty())
            return digit_to_tree(right);
        auto vl = spine->view_l_internal();
        assert(vl.has_value());
        auto new_left = ft5::elem_to_digit<T, Tag>(vl->d_elem);
        SpinePtr new_spine;
        if (!vl->d_rest.is_empty())
            new_spine =
                std::make_shared<const FingerTree5>(std::move(vl->d_rest));
        return make_deep(std::move(new_left), std::move(new_spine),
                         std::move(right));
    }

    static auto deep_r(Digit left, SpinePtr spine) -> FingerTree5 {
        if (!spine || spine->is_empty())
            return digit_to_tree(left);
        auto vr = spine->view_r_internal();
        assert(vr.has_value());
        auto new_right = ft5::elem_to_digit<T, Tag>(vr->d_elem);
        SpinePtr new_spine;
        if (!vr->d_rest.is_empty())
            new_spine =
                std::make_shared<const FingerTree5>(std::move(vr->d_rest));
        return make_deep(std::move(left), std::move(new_spine),
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
                [](const Single &s) -> std::optional<InternalView> {
                    return InternalView{s.d_elem, make_empty()};
                },
                [](const DeepPtr &d) -> std::optional<InternalView> {
                    EP h = d->d_left.front();
                    if (d->d_left.size() > 1) {
                        auto t = ft5::digit_tail<T, Tag>(d->d_left);
                        return InternalView{
                            h, make_deep(std::move(t), d->d_spine,
                                         d->d_right)};
                    }
                    return InternalView{h,
                                        deep_l(d->d_spine, d->d_right)};
                }},
            d_repr);
    }

    auto view_r_internal() const -> std::optional<InternalView> {
        return std::visit(
            ft5::overloaded{
                [](const Empty &) -> std::optional<InternalView> {
                    return std::nullopt;
                },
                [](const Single &s) -> std::optional<InternalView> {
                    return InternalView{s.d_elem, make_empty()};
                },
                [](const DeepPtr &d) -> std::optional<InternalView> {
                    EP l = d->d_right.back();
                    if (d->d_right.size() > 1) {
                        auto i = ft5::digit_init<T, Tag>(d->d_right);
                        return InternalView{
                            l, make_deep(d->d_left, d->d_spine, std::move(i))};
                    }
                    return InternalView{l,
                                        deep_r(d->d_left, d->d_spine)};
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
                    return make_deep(std::move(l), nullptr, std::move(r));
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
                        d->d_left[1], d->d_left[2], d->d_left[3]);
                    SpinePtr sp;
                    if (d->d_spine)
                        sp = std::make_shared<const FingerTree5>(
                            d->d_spine->cons_internal(std::move(node)));
                    else
                        sp = std::make_shared<const FingerTree5>(
                            make_single(std::move(node)));
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
                    return make_deep(std::move(l), nullptr, std::move(r));
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
                        d->d_right[0], d->d_right[1], d->d_right[2]);
                    SpinePtr sp;
                    if (d->d_spine)
                        sp = std::make_shared<const FingerTree5>(
                            d->d_spine->snoc_internal(std::move(node)));
                    else
                        sp = std::make_shared<const FingerTree5>(
                            make_single(std::move(node)));
                    Digit new_right;
                    new_right.push_back(d->d_right[3]);
                    new_right.push_back(std::move(x));
                    return make_deep(d->d_left, std::move(sp),
                                     std::move(new_right));
                }},
            d_repr);
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
    using value_type = T;
    using tag_type = Tag;

    static auto empty() -> FingerTree5 { return make_empty(); }

    static auto leaf(T value) -> FingerTree5 {
        return make_single(wrap_leaf(std::move(value)));
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
            ft5::overloaded{
                [](const Empty &) { return ft5::tag_id<Tag>(); },
                [](const Single &s) -> Tag { return s.d_elem->d_measure; },
                [](const DeepPtr &d) -> Tag { return d->d_measure; }},
            d_repr);
    }

    auto value() const -> const T & {
        assert(is_leaf());
        auto &s = std::get<Single>(d_repr);
        assert(ft5::is_leaf(s.d_elem));
        return ft5::leaf_value(s.d_elem);
    }

    // -- cons / snoc: O(1) amortized -----------------------------------------

    auto cons(T x) const -> FingerTree5 {
        return cons_internal(wrap_leaf(std::move(x)));
    }

    auto snoc(T x) const -> FingerTree5 {
        return snoc_internal(wrap_leaf(std::move(x)));
    }

    // -- view_l / view_r: O(1) amortized -------------------------------------

    struct View {
        T d_value;
        FingerTree5 d_rest;
    };

    auto view_l() const -> std::optional<View> {
        auto iv = view_l_internal();
        if (!iv.has_value())
            return std::nullopt;
        assert(ft5::is_leaf(iv->d_elem));
        return View{ft5::leaf_value(iv->d_elem), std::move(iv->d_rest)};
    }

    auto view_r() const -> std::optional<View> {
        auto iv = view_r_internal();
        if (!iv.has_value())
            return std::nullopt;
        assert(ft5::is_leaf(iv->d_elem));
        return View{ft5::leaf_value(iv->d_elem), std::move(iv->d_rest)};
    }

    auto head() const -> T {
        auto v = view_l();
        assert(v.has_value());
        return std::move(v->d_value);
    }

    auto tail() const -> FingerTree5 {
        auto v = view_l();
        return v.has_value() ? std::move(v->d_rest) : empty();
    }

    auto last() const -> T {
        auto v = view_r();
        assert(v.has_value());
        return std::move(v->d_value);
    }

    auto init() const -> FingerTree5 {
        auto v = view_r();
        return v.has_value() ? std::move(v->d_rest) : empty();
    }

    // -- flatten / for_each: O(n) --------------------------------------------

    auto flatten() const -> std::vector<T> {
        std::vector<T> out;
        flatten_elems(out);
        return out;
    }

    template <typename F>
    void for_each(F &&fn) const {
        for_each_internal(fn);
    }

    static auto from_sequence(std::vector<T> values) -> FingerTree5 {
        auto result = empty();
        for (auto &v : values)
            result = result.snoc(std::move(v));
        return result;
    }
};

} // namespace smd::tree

#endif
