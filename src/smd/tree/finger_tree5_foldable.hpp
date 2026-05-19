// src/smd/tree/finger_tree5_foldable.hpp                             -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_FINGER_TREE5_FOLDABLE
#define INCLUDED_SMD_TREE_FINGER_TREE5_FOLDABLE

#include <smd/tree/finger_tree5.hpp>
#include <smd/typeclass/foldable.hpp>

#include <functional>
#include <type_traits>
#include <utility>

namespace smd {

/** Foldable typeclass implementation for FingerTree5; uses for_each to
 * traverse leaves in left-to-right order without heap allocation.
 */
template <class T, class TAG_TYPE, class MEASURE_POLICY, class ALLOCATOR>
struct FingerTree5FoldableImpl {
    template <class F>
    auto fold_map(this auto &&, F &&function,
                  const smd::tree::FingerTree5<T, TAG_TYPE, MEASURE_POLICY,
                                               ALLOCATOR> &tree)
        -> remove_cvref_t<std::invoke_result_t<F, const T &>> {
        using Result = remove_cvref_t<std::invoke_result_t<F, const T &>>;

        Result acc = smd::typeclass::monoid_v<Result>.identity();
        tree.for_each([&](const T &value) {
            acc = smd::typeclass::monoid_v<Result>.combine(
                std::move(acc), std::invoke(function, value));
        });
        return acc;
    }
};

/** Foldable typeclass map entry for FingerTree5. */
template <class T, class TAG_TYPE, class MEASURE_POLICY, class ALLOCATOR>
struct FingerTree5FoldableMap
    : Foldable<
          FingerTree5FoldableImpl<T, TAG_TYPE, MEASURE_POLICY, ALLOCATOR>> {
    using FingerTree5FoldableImpl<T, TAG_TYPE, MEASURE_POLICY,
                                  ALLOCATOR>::fold_map;
};

/** Registers FingerTree5 as a Foldable for all tag and measure combinations. */
template <class T, class TAG_TYPE, class MEASURE_POLICY, class ALLOCATOR>
inline constexpr auto foldable_typeclass<
    smd::tree::FingerTree5<T, TAG_TYPE, MEASURE_POLICY, ALLOCATOR>> =
    FingerTree5FoldableMap<T, TAG_TYPE, MEASURE_POLICY, ALLOCATOR>{};

} // namespace smd

#endif
