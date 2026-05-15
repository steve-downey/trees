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
#include <utility>
#include <variant>

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
};

} // namespace smd::tree

#endif
