// src/smd/tree/finger_tree4.hpp                                      -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_FINGER_TREE4
#define INCLUDED_SMD_TREE_FINGER_TREE4

// Hinze-Paterson 2-3 finger tree — uniform-elem (fixpoint-style) implementation.
//
// The element type Elem<T, Tag> is self-recursive: an Elem is either a Leaf
// holding a T, or a Node2/Node3 holding 2-3 child ElemPtrs.  This lets the
// spine be the SAME FingerTree4 type at every depth — runtime recursion via
// shared_ptr<const FingerTree4>, not compile-time recursion via a DEPTH NTTP.
//
// Consequences vs finger_tree2 / finger_tree3:
//   - One template instantiation per (T, Tag, Measure) triple, any depth
//   - No kMaxDepth limit, no silent data loss
//   - No TagPred type erasure needed (Tag is uniform across levels)
//   - Extra shared_ptr indirection per Elem child (pointer chase per tree level)

#include <smd/typeclass/monoid.hpp>

#include <cassert>
#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <utility>
#include <variant>
#include <vector>

namespace smd::tree {

// ============================================================================
//                              DETAIL TYPES
// ============================================================================

namespace ft4 {

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

// Get children of a Node elem as a vector.
template <typename T, typename Tag>
auto elem_children(const ElemPtr<T, Tag> &ep) -> std::vector<ElemPtr<T, Tag>> {
    using E = Elem<T, Tag>;
    return std::visit(
        overloaded{
            [](const typename E::Leaf &) -> std::vector<ElemPtr<T, Tag>> {
                assert(false && "elem_children called on Leaf");
                return {};
            },
            [](const typename E::Node2 &n) -> std::vector<ElemPtr<T, Tag>> {
                return {n.a, n.b};
            },
            [](const typename E::Node3 &n) -> std::vector<ElemPtr<T, Tag>> {
                return {n.a, n.b, n.c};
            }},
        ep->d_data);
}

// Recursively flatten an Elem to its leaf values.
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

// -- Digit: 1-4 ElemPtrs ----------------------------------------------------

template <typename T, typename Tag>
using EP = ElemPtr<T, Tag>;

template <typename T, typename Tag>
struct One {
    EP<T, Tag> a;
};
template <typename T, typename Tag>
struct Two {
    EP<T, Tag> a;
    EP<T, Tag> b;
};
template <typename T, typename Tag>
struct Three {
    EP<T, Tag> a;
    EP<T, Tag> b;
    EP<T, Tag> c;
};
template <typename T, typename Tag>
struct Four {
    EP<T, Tag> a;
    EP<T, Tag> b;
    EP<T, Tag> c;
    EP<T, Tag> d;
};

template <typename T, typename Tag>
using Digit =
    std::variant<One<T, Tag>, Two<T, Tag>, Three<T, Tag>, Four<T, Tag>>;

// -- Digit helpers -----------------------------------------------------------

template <typename T, typename Tag>
auto digit_measure(const Digit<T, Tag> &d) -> Tag {
    return std::visit(
        overloaded{
            [](const One<T, Tag> &x) -> Tag { return x.a->d_measure; },
            [](const Two<T, Tag> &x) -> Tag {
                return tag_op<Tag>(x.a->d_measure, x.b->d_measure);
            },
            [](const Three<T, Tag> &x) -> Tag {
                return tag_op<Tag>(
                    tag_op<Tag>(x.a->d_measure, x.b->d_measure),
                    x.c->d_measure);
            },
            [](const Four<T, Tag> &x) -> Tag {
                return tag_op<Tag>(
                    tag_op<Tag>(tag_op<Tag>(x.a->d_measure, x.b->d_measure),
                                x.c->d_measure),
                    x.d->d_measure);
            }},
        d);
}

template <typename T, typename Tag>
auto digit_head(const Digit<T, Tag> &d) -> const EP<T, Tag> & {
    return std::visit(
        [](const auto &x) -> const EP<T, Tag> & { return x.a; }, d);
}

template <typename T, typename Tag>
auto digit_last(const Digit<T, Tag> &d) -> const EP<T, Tag> & {
    return std::visit(
        overloaded{
            [](const One<T, Tag> &x) -> const EP<T, Tag> & { return x.a; },
            [](const Two<T, Tag> &x) -> const EP<T, Tag> & { return x.b; },
            [](const Three<T, Tag> &x) -> const EP<T, Tag> & { return x.c; },
            [](const Four<T, Tag> &x) -> const EP<T, Tag> & { return x.d; }},
        d);
}

template <typename T, typename Tag>
auto digit_tail(const Digit<T, Tag> &d) -> std::optional<Digit<T, Tag>> {
    return std::visit(
        overloaded{
            [](const One<T, Tag> &) -> std::optional<Digit<T, Tag>> {
                return std::nullopt;
            },
            [](const Two<T, Tag> &x) -> std::optional<Digit<T, Tag>> {
                return One<T, Tag>{x.b};
            },
            [](const Three<T, Tag> &x) -> std::optional<Digit<T, Tag>> {
                return Two<T, Tag>{x.b, x.c};
            },
            [](const Four<T, Tag> &x) -> std::optional<Digit<T, Tag>> {
                return Three<T, Tag>{x.b, x.c, x.d};
            }},
        d);
}

template <typename T, typename Tag>
auto digit_init(const Digit<T, Tag> &d) -> std::optional<Digit<T, Tag>> {
    return std::visit(
        overloaded{
            [](const One<T, Tag> &) -> std::optional<Digit<T, Tag>> {
                return std::nullopt;
            },
            [](const Two<T, Tag> &x) -> std::optional<Digit<T, Tag>> {
                return One<T, Tag>{x.a};
            },
            [](const Three<T, Tag> &x) -> std::optional<Digit<T, Tag>> {
                return Two<T, Tag>{x.a, x.b};
            },
            [](const Four<T, Tag> &x) -> std::optional<Digit<T, Tag>> {
                return Three<T, Tag>{x.a, x.b, x.c};
            }},
        d);
}

template <typename T, typename Tag>
void digit_push_into(const Digit<T, Tag> &d,
                     std::vector<ElemPtr<T, Tag>> &out) {
    std::visit(
        overloaded{[&](const One<T, Tag> &x) { out.push_back(x.a); },
                   [&](const Two<T, Tag> &x) {
                       out.push_back(x.a);
                       out.push_back(x.b);
                   },
                   [&](const Three<T, Tag> &x) {
                       out.push_back(x.a);
                       out.push_back(x.b);
                       out.push_back(x.c);
                   },
                   [&](const Four<T, Tag> &x) {
                       out.push_back(x.a);
                       out.push_back(x.b);
                       out.push_back(x.c);
                       out.push_back(x.d);
                   }},
        d);
}

template <typename T, typename Tag>
auto digit_to_vec(const Digit<T, Tag> &d) -> std::vector<ElemPtr<T, Tag>> {
    std::vector<ElemPtr<T, Tag>> out;
    digit_push_into(d, out);
    return out;
}

template <typename T, typename Tag, typename F>
void digit_for_each(const Digit<T, Tag> &d, const F &fn) {
    std::visit(
        overloaded{[&](const One<T, Tag> &x) { fn(x.a); },
                   [&](const Two<T, Tag> &x) {
                       fn(x.a);
                       fn(x.b);
                   },
                   [&](const Three<T, Tag> &x) {
                       fn(x.a);
                       fn(x.b);
                       fn(x.c);
                   },
                   [&](const Four<T, Tag> &x) {
                       fn(x.a);
                       fn(x.b);
                       fn(x.c);
                       fn(x.d);
                   }},
        d);
}

template <typename T, typename Tag>
auto vec_to_digit(std::vector<ElemPtr<T, Tag>> v)
    -> std::optional<Digit<T, Tag>> {
    switch (v.size()) {
    case 0:
        return std::nullopt;
    case 1:
        return One<T, Tag>{std::move(v[0])};
    case 2:
        return Two<T, Tag>{std::move(v[0]), std::move(v[1])};
    case 3:
        return Three<T, Tag>{std::move(v[0]), std::move(v[1]),
                             std::move(v[2])};
    case 4:
        return Four<T, Tag>{std::move(v[0]), std::move(v[1]),
                            std::move(v[2]), std::move(v[3])};
    default:
        assert(false && "vec_to_digit: size must be 0-4");
        return std::nullopt;
    }
}

// Elem-to-digit: unwrap a Node elem into a digit of its children.
template <typename T, typename Tag>
auto elem_to_digit(const ElemPtr<T, Tag> &ep) -> Digit<T, Tag> {
    using E = Elem<T, Tag>;
    return std::visit(
        overloaded{
            [](const typename E::Leaf &) -> Digit<T, Tag> {
                assert(false && "elem_to_digit called on Leaf");
                return One<T, Tag>{{}};
            },
            [](const typename E::Node2 &n) -> Digit<T, Tag> {
                return Two<T, Tag>{n.a, n.b};
            },
            [](const typename E::Node3 &n) -> Digit<T, Tag> {
                return Three<T, Tag>{n.a, n.b, n.c};
            }},
        ep->d_data);
}

// nodes_from: group a vector of ElemPtrs into Node2/Node3 ElemPtrs.
template <typename T, typename Tag>
auto nodes_from(std::vector<ElemPtr<T, Tag>> elems)
    -> std::vector<ElemPtr<T, Tag>> {
    std::vector<ElemPtr<T, Tag>> result;
    auto n = elems.size();
    std::size_t i = 0;
    while (n - i > 4) {
        result.push_back(
            make_node3<T, Tag>(std::move(elems[i]), std::move(elems[i + 1]),
                               std::move(elems[i + 2])));
        i += 3;
    }
    switch (n - i) {
    case 2:
        result.push_back(
            make_node2<T, Tag>(std::move(elems[i]), std::move(elems[i + 1])));
        break;
    case 3:
        result.push_back(
            make_node3<T, Tag>(std::move(elems[i]), std::move(elems[i + 1]),
                               std::move(elems[i + 2])));
        break;
    case 4:
        result.push_back(
            make_node2<T, Tag>(std::move(elems[i]), std::move(elems[i + 1])));
        result.push_back(make_node2<T, Tag>(std::move(elems[i + 2]),
                                            std::move(elems[i + 3])));
        break;
    default:
        assert(false && "nodes_from: invalid count");
    }
    return result;
}

} // namespace ft4

// ============================================================================
//                            FINGER_TREE4
// ============================================================================

template <typename T, typename TAG_TYPE>
struct UnitMeasure4 {
    auto operator()(const T &) const -> TAG_TYPE { return TAG_TYPE{1}; }
};

template <typename T, typename TAG_TYPE = std::size_t,
          typename MEASURE_POLICY = UnitMeasure4<T, TAG_TYPE>>
class FingerTree4 {
    using Tag = TAG_TYPE;
    using Meas = MEASURE_POLICY;
    using E = ft4::Elem<T, Tag>;
    using EP = ft4::ElemPtr<T, Tag>;

    template <typename U>
    using One = ft4::One<U, Tag>;
    template <typename U>
    using Two = ft4::Two<U, Tag>;
    template <typename U>
    using Three = ft4::Three<U, Tag>;
    template <typename U>
    using Four = ft4::Four<U, Tag>;
    using Digit = ft4::Digit<T, Tag>;

    using SpinePtr = std::shared_ptr<const FingerTree4>;

    static auto meas_fn() -> Meas { return Meas{}; }

    static auto wrap_leaf(T value) -> EP {
        return ft4::make_leaf<T, Tag>(meas_fn(), std::move(value));
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

    explicit FingerTree4(Repr r) : d_repr(std::move(r)) {}

    // -- Smart constructors --------------------------------------------------

    static auto make_empty() -> FingerTree4 {
        return FingerTree4(Repr{Empty{}});
    }

    static auto make_single(EP elem) -> FingerTree4 {
        return FingerTree4(Repr{Single{std::move(elem)}});
    }

    static auto make_deep(Digit left, SpinePtr spine, Digit right)
        -> FingerTree4 {
        auto m = ft4::tag_op<Tag>(
            ft4::tag_op<Tag>(ft4::digit_measure(left),
                             spine ? spine->measure() : ft4::tag_id<Tag>()),
            ft4::digit_measure(right));
        return FingerTree4(Repr{std::make_shared<const Deep>(
            Deep{std::move(m), std::move(left), std::move(spine),
                 std::move(right)})});
    }

    static auto digit_to_tree(const Digit &d) -> FingerTree4 {
        return std::visit(
            ft4::overloaded{
                [](const One<T> &x) { return make_single(x.a); },
                [](const Two<T> &x) {
                    return make_deep(One<T>{x.a}, nullptr, One<T>{x.b});
                },
                [](const Three<T> &x) {
                    return make_deep(Two<T>{x.a, x.b}, nullptr, One<T>{x.c});
                },
                [](const Four<T> &x) {
                    return make_deep(Two<T>{x.a, x.b}, nullptr,
                                     Two<T>{x.c, x.d});
                }},
            d);
    }

    static auto deep_l(SpinePtr spine, Digit right) -> FingerTree4 {
        if (!spine || spine->is_empty())
            return digit_to_tree(right);
        auto vl = spine->view_l_internal();
        assert(vl.has_value());
        auto new_left = ft4::elem_to_digit(vl->d_elem);
        SpinePtr new_spine;
        if (!vl->d_rest.is_empty())
            new_spine =
                std::make_shared<const FingerTree4>(std::move(vl->d_rest));
        return make_deep(std::move(new_left), std::move(new_spine),
                         std::move(right));
    }

    static auto deep_r(Digit left, SpinePtr spine) -> FingerTree4 {
        if (!spine || spine->is_empty())
            return digit_to_tree(left);
        auto vr = spine->view_r_internal();
        assert(vr.has_value());
        auto new_right = ft4::elem_to_digit(vr->d_elem);
        SpinePtr new_spine;
        if (!vr->d_rest.is_empty())
            new_spine =
                std::make_shared<const FingerTree4>(std::move(vr->d_rest));
        return make_deep(std::move(left), std::move(new_spine),
                         std::move(new_right));
    }

    // -- Internal view (works on ElemPtrs) -----------------------------------

    struct InternalView {
        EP d_elem;
        FingerTree4 d_rest;
    };

    auto view_l_internal() const -> std::optional<InternalView> {
        return std::visit(
            ft4::overloaded{
                [](const Empty &) -> std::optional<InternalView> {
                    return std::nullopt;
                },
                [](const Single &s) -> std::optional<InternalView> {
                    return InternalView{s.d_elem, make_empty()};
                },
                [](const DeepPtr &d) -> std::optional<InternalView> {
                    auto h = ft4::digit_head(d->d_left);
                    auto t = ft4::digit_tail(d->d_left);
                    if (t.has_value())
                        return InternalView{
                            h, make_deep(std::move(*t), d->d_spine,
                                         d->d_right)};
                    return InternalView{h,
                                        deep_l(d->d_spine, d->d_right)};
                }},
            d_repr);
    }

    auto view_r_internal() const -> std::optional<InternalView> {
        return std::visit(
            ft4::overloaded{
                [](const Empty &) -> std::optional<InternalView> {
                    return std::nullopt;
                },
                [](const Single &s) -> std::optional<InternalView> {
                    return InternalView{s.d_elem, make_empty()};
                },
                [](const DeepPtr &d) -> std::optional<InternalView> {
                    auto l = ft4::digit_last(d->d_right);
                    auto i = ft4::digit_init(d->d_right);
                    if (i.has_value())
                        return InternalView{
                            l, make_deep(d->d_left, d->d_spine,
                                         std::move(*i))};
                    return InternalView{l,
                                        deep_r(d->d_left, d->d_spine)};
                }},
            d_repr);
    }

    // -- Internal cons/snoc (works on ElemPtrs) ------------------------------

    auto cons_internal(EP x) const -> FingerTree4 {
        return std::visit(
            ft4::overloaded{
                [&](const Empty &) { return make_single(std::move(x)); },
                [&](const Single &s) {
                    return make_deep(One<T>{std::move(x)}, nullptr,
                                     One<T>{s.d_elem});
                },
                [&](const DeepPtr &d) -> FingerTree4 {
                    return std::visit(
                        ft4::overloaded{
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
                            [&](const Four<T> &dig) -> FingerTree4 {
                                auto node = ft4::make_node3<T, Tag>(
                                    dig.b, dig.c, dig.d);
                                SpinePtr sp;
                                if (d->d_spine)
                                    sp = std::make_shared<const FingerTree4>(
                                        d->d_spine->cons_internal(
                                            std::move(node)));
                                else
                                    sp = std::make_shared<const FingerTree4>(
                                        make_single(std::move(node)));
                                return make_deep(Two<T>{std::move(x), dig.a},
                                                 std::move(sp), d->d_right);
                            }},
                        d->d_left);
                }},
            d_repr);
    }

    auto snoc_internal(EP x) const -> FingerTree4 {
        return std::visit(
            ft4::overloaded{
                [&](const Empty &) { return make_single(std::move(x)); },
                [&](const Single &s) {
                    return make_deep(One<T>{s.d_elem}, nullptr,
                                     One<T>{std::move(x)});
                },
                [&](const DeepPtr &d) -> FingerTree4 {
                    return std::visit(
                        ft4::overloaded{
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
                            [&](const Four<T> &dig) -> FingerTree4 {
                                auto node = ft4::make_node3<T, Tag>(
                                    dig.a, dig.b, dig.c);
                                SpinePtr sp;
                                if (d->d_spine)
                                    sp = std::make_shared<const FingerTree4>(
                                        d->d_spine->snoc_internal(
                                            std::move(node)));
                                else
                                    sp = std::make_shared<const FingerTree4>(
                                        make_single(std::move(node)));
                                return make_deep(d->d_left, std::move(sp),
                                                 Two<T>{dig.d, std::move(x)});
                            }},
                        d->d_right);
                }},
            d_repr);
    }

    // -- app3: Hinze-Paterson recursive concatenation -------------------------

    static auto app3(const FingerTree4 &left, std::vector<EP> middle,
                     const FingerTree4 &right) -> FingerTree4 {
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

        auto combined = ft4::digit_to_vec(ld.d_right);
        combined.insert(combined.end(),
                        std::make_move_iterator(middle.begin()),
                        std::make_move_iterator(middle.end()));
        {
            auto rl = ft4::digit_to_vec(rd.d_left);
            combined.insert(combined.end(), rl.begin(), rl.end());
        }
        auto ns = ft4::nodes_from<T, Tag>(std::move(combined));

        auto left_spine =
            ld.d_spine ? *ld.d_spine : FingerTree4::empty();
        auto right_spine =
            rd.d_spine ? *rd.d_spine : FingerTree4::empty();
        auto new_spine =
            FingerTree4::app3(left_spine, std::move(ns), right_spine);
        SpinePtr sp;
        if (!new_spine.is_empty())
            sp = std::make_shared<const FingerTree4>(std::move(new_spine));
        return make_deep(ld.d_left, std::move(sp), rd.d_right);
    }

    // -- Internal split ------------------------------------------------------

    using Pred = std::function<bool(const Tag &)>;

    struct InternalSplit {
        FingerTree4 d_left;
        EP d_pivot;
        FingerTree4 d_right;
    };

    struct DigitSplit {
        std::optional<Digit> d_left;
        EP d_pivot;
        std::optional<Digit> d_right;
    };

    static auto split_digit(const Pred &pred, Tag prefix, const Digit &d)
        -> std::optional<DigitSplit> {
        auto elems = ft4::digit_to_vec(d);
        auto running = prefix;
        for (std::size_t i = 0; i < elems.size(); ++i) {
            running =
                ft4::tag_op<Tag>(running, elems[i]->d_measure);
            if (pred(running)) {
                std::optional<Digit> left_d;
                std::optional<Digit> right_d;
                if (i > 0) {
                    std::vector<EP> lv(
                        elems.begin(),
                        elems.begin() + static_cast<std::ptrdiff_t>(i));
                    left_d = ft4::vec_to_digit<T, Tag>(std::move(lv));
                }
                if (i + 1 < elems.size()) {
                    std::vector<EP> rv(
                        elems.begin() + static_cast<std::ptrdiff_t>(i + 1),
                        elems.end());
                    right_d = ft4::vec_to_digit<T, Tag>(std::move(rv));
                }
                return DigitSplit{std::move(left_d), std::move(elems[i]),
                                  std::move(right_d)};
            }
        }
        return std::nullopt;
    }

    // Search among children of a Node elem to find the child where pred triggers.
    static auto split_within_elem(const Pred &pred, Tag prefix,
                                  const EP &node_ep) -> DigitSplit {
        auto children = ft4::elem_children(node_ep);
        auto running = prefix;
        for (std::size_t i = 0; i < children.size(); ++i) {
            running =
                ft4::tag_op<Tag>(running, children[i]->d_measure);
            if (pred(running)) {
                std::optional<Digit> left_d;
                std::optional<Digit> right_d;
                if (i > 0) {
                    std::vector<EP> lv(
                        children.begin(),
                        children.begin() + static_cast<std::ptrdiff_t>(i));
                    left_d = ft4::vec_to_digit<T, Tag>(std::move(lv));
                }
                if (i + 1 < children.size()) {
                    std::vector<EP> rv(
                        children.begin() +
                            static_cast<std::ptrdiff_t>(i + 1),
                        children.end());
                    right_d = ft4::vec_to_digit<T, Tag>(std::move(rv));
                }
                return DigitSplit{std::move(left_d), std::move(children[i]),
                                  std::move(right_d)};
            }
        }
        assert(false && "split_within_elem: predicate never triggered");
        return DigitSplit{std::nullopt, node_ep, std::nullopt};
    }

    auto split_impl(const Pred &pred, Tag prefix) const
        -> std::optional<InternalSplit> {
        if (is_empty())
            return std::nullopt;

        if (auto *s = std::get_if<Single>(&d_repr)) {
            if (pred(ft4::tag_op<Tag>(prefix, s->d_elem->d_measure)))
                return InternalSplit{make_empty(), s->d_elem, make_empty()};
            return std::nullopt;
        }

        const auto &d = *std::get<DeepPtr>(d_repr);

        auto vl =
            ft4::tag_op<Tag>(prefix, ft4::digit_measure(d.d_left));
        if (pred(vl)) {
            auto ds = split_digit(pred, prefix, d.d_left);
            if (!ds.has_value())
                return std::nullopt;
            auto lt = ds->d_left.has_value() ? digit_to_tree(*ds->d_left)
                                              : make_empty();
            auto rt = assemble_l(std::move(ds->d_right), d.d_spine,
                                 d.d_right);
            return InternalSplit{std::move(lt), std::move(ds->d_pivot),
                                 std::move(rt)};
        }

        auto spine_m =
            d.d_spine ? d.d_spine->measure() : ft4::tag_id<Tag>();
        auto vm = ft4::tag_op<Tag>(vl, spine_m);
        if (pred(vm)) {
            if (!d.d_spine || d.d_spine->is_empty())
                return std::nullopt;

            Pred spine_pred = [&pred, &vl](const Tag &t) {
                return pred(ft4::tag_op<Tag>(vl, t));
            };
            auto ss = d.d_spine->split_impl(spine_pred, ft4::tag_id<Tag>());
            if (!ss.has_value())
                return std::nullopt;

            auto node_prefix =
                ft4::tag_op<Tag>(vl, ss->d_left.measure());
            auto ns = split_within_elem(pred, node_prefix, ss->d_pivot);

            SpinePtr sl;
            if (!ss->d_left.is_empty())
                sl = std::make_shared<const FingerTree4>(
                    std::move(ss->d_left));
            SpinePtr sr;
            if (!ss->d_right.is_empty())
                sr = std::make_shared<const FingerTree4>(
                    std::move(ss->d_right));

            auto lt =
                assemble_r(d.d_left, std::move(sl), std::move(ns.d_left));
            auto rt = assemble_l(std::move(ns.d_right), std::move(sr),
                                 d.d_right);
            return InternalSplit{std::move(lt), std::move(ns.d_pivot),
                                 std::move(rt)};
        }

        auto ds = split_digit(pred, vm, d.d_right);
        if (!ds.has_value())
            return std::nullopt;
        auto lt = assemble_r(d.d_left, d.d_spine, std::move(ds->d_left));
        auto rt = ds->d_right.has_value() ? digit_to_tree(*ds->d_right)
                                           : make_empty();
        return InternalSplit{std::move(lt), std::move(ds->d_pivot),
                             std::move(rt)};
    }

    static auto assemble_l(std::optional<Digit> left_d, SpinePtr spine,
                           Digit right) -> FingerTree4 {
        if (left_d.has_value())
            return make_deep(std::move(*left_d), std::move(spine),
                             std::move(right));
        return deep_l(std::move(spine), std::move(right));
    }

    static auto assemble_r(Digit left, SpinePtr spine,
                           std::optional<Digit> right_d) -> FingerTree4 {
        if (right_d.has_value())
            return make_deep(std::move(left), std::move(spine),
                             std::move(*right_d));
        return deep_r(std::move(left), std::move(spine));
    }

    // -- Internal flatten / for_each -----------------------------------------

    void flatten_elems(std::vector<T> &out) const {
        std::visit(
            ft4::overloaded{
                [](const Empty &) {},
                [&](const Single &s) { ft4::flatten_elem(s.d_elem, out); },
                [&](const DeepPtr &d) {
                    ft4::digit_for_each(
                        d->d_left,
                        [&](const EP &ep) { ft4::flatten_elem(ep, out); });
                    if (d->d_spine)
                        d->d_spine->flatten_elems(out);
                    ft4::digit_for_each(
                        d->d_right,
                        [&](const EP &ep) { ft4::flatten_elem(ep, out); });
                }},
            d_repr);
    }

    template <typename F>
    void for_each_internal(const F &fn) const {
        std::visit(
            ft4::overloaded{
                [](const Empty &) {},
                [&](const Single &s) { ft4::for_each_elem(s.d_elem, fn); },
                [&](const DeepPtr &d) {
                    ft4::digit_for_each(
                        d->d_left,
                        [&](const EP &ep) { ft4::for_each_elem(ep, fn); });
                    if (d->d_spine)
                        d->d_spine->for_each_internal(fn);
                    ft4::digit_for_each(
                        d->d_right,
                        [&](const EP &ep) { ft4::for_each_elem(ep, fn); });
                }},
            d_repr);
    }

    // ========================================================================
    //                          PUBLIC INTERFACE
    // ========================================================================
  public:
    using value_type = T;
    using tag_type = Tag;

    static auto empty() -> FingerTree4 { return make_empty(); }

    static auto leaf(T value) -> FingerTree4 {
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
            ft4::overloaded{
                [](const Empty &) { return ft4::tag_id<Tag>(); },
                [](const Single &s) -> Tag { return s.d_elem->d_measure; },
                [](const DeepPtr &d) -> Tag { return d->d_measure; }},
            d_repr);
    }

    auto value() const -> const T & {
        assert(is_leaf());
        auto &s = std::get<Single>(d_repr);
        assert(ft4::is_leaf(s.d_elem));
        return ft4::leaf_value(s.d_elem);
    }

    // -- cons: O(1) amortized ------------------------------------------------

    auto cons(T x) const -> FingerTree4 {
        return cons_internal(wrap_leaf(std::move(x)));
    }

    // -- snoc: O(1) amortized ------------------------------------------------

    auto snoc(T x) const -> FingerTree4 {
        return snoc_internal(wrap_leaf(std::move(x)));
    }

    // -- view_l: O(1) amortized ----------------------------------------------

    struct View {
        T d_value;
        FingerTree4 d_rest;
    };

    auto view_l() const -> std::optional<View> {
        auto iv = view_l_internal();
        if (!iv.has_value())
            return std::nullopt;
        assert(ft4::is_leaf(iv->d_elem));
        return View{ft4::leaf_value(iv->d_elem), std::move(iv->d_rest)};
    }

    // -- view_r: O(1) amortized ----------------------------------------------

    auto view_r() const -> std::optional<View> {
        auto iv = view_r_internal();
        if (!iv.has_value())
            return std::nullopt;
        assert(ft4::is_leaf(iv->d_elem));
        return View{ft4::leaf_value(iv->d_elem), std::move(iv->d_rest)};
    }

    auto head() const -> T {
        auto v = view_l();
        assert(v.has_value());
        return std::move(v->d_value);
    }

    auto tail() const -> FingerTree4 {
        auto v = view_l();
        return v.has_value() ? std::move(v->d_rest) : empty();
    }

    auto last() const -> T {
        auto v = view_r();
        assert(v.has_value());
        return std::move(v->d_value);
    }

    auto init() const -> FingerTree4 {
        auto v = view_r();
        return v.has_value() ? std::move(v->d_rest) : empty();
    }

    // -- flatten: O(n) -------------------------------------------------------

    auto flatten() const -> std::vector<T> {
        std::vector<T> out;
        flatten_elems(out);
        return out;
    }

    // -- for_each: O(n) no allocation ----------------------------------------

    template <typename F>
    void for_each(F &&fn) const {
        for_each_internal(fn);
    }

    // -- append / concat: O(log min(n,m)) ------------------------------------

    auto append(const FingerTree4 &right) const -> FingerTree4 {
        return app3(*this, {}, right);
    }

    static auto concat(const FingerTree4 &left, const FingerTree4 &right)
        -> FingerTree4 {
        return left.append(right);
    }

    // -- split: O(log n) -----------------------------------------------------

    struct Split {
        FingerTree4 d_left;
        T d_pivot;
        FingerTree4 d_right;
    };

    struct SplitAt {
        FingerTree4 d_left;
        FingerTree4 d_right;
    };

    template <typename PRED>
    auto search(PRED &&pred) const -> std::optional<T> {
        auto sp = split(std::forward<PRED>(pred));
        if (!sp.has_value())
            return std::nullopt;
        return std::move(sp->d_pivot);
    }

    template <typename PRED>
    auto split(PRED &&pred) const -> std::optional<Split> {
        Pred erased(std::forward<PRED>(pred));
        auto is = split_impl(erased, ft4::tag_id<Tag>());
        if (!is.has_value())
            return std::nullopt;
        assert(ft4::is_leaf(is->d_pivot));
        return Split{std::move(is->d_left),
                     ft4::leaf_value(is->d_pivot),
                     std::move(is->d_right)};
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

    static auto from_sequence(std::vector<T> values) -> FingerTree4 {
        auto result = empty();
        for (auto &v : values)
            result = result.snoc(std::move(v));
        return result;
    }
};

} // namespace smd::tree

#endif
