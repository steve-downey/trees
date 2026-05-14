// src/smd/tree/finger_tree_foldable.hpp                              -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_FINGER_TREE_FOLDABLE
#define INCLUDED_SMD_TREE_FINGER_TREE_FOLDABLE

#include <smd/tree/finger_tree2.hpp>
#include <smd/typeclass/foldable.hpp>

#include <functional>
#include <type_traits>
#include <utility>

namespace smd {

/** Foldable typeclass implementation for FingerTree; uses for_each to avoid
 * heap allocation during traversal.
 */
template <class T, class TAG_TYPE, class MEASURE_POLICY>
struct FingerTreeFoldableImpl {
    template <class F>
    auto
    fold_map(this auto &&, F &&function,
             const smd::tree::FingerTree2<T, TAG_TYPE, MEASURE_POLICY> &tree)
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

/** Foldable typeclass map entry for FingerTree. */
template <class T, class TAG_TYPE, class MEASURE_POLICY>
struct FingerTreeFoldableMap
    : Foldable<FingerTreeFoldableImpl<T, TAG_TYPE, MEASURE_POLICY>> {
    using FingerTreeFoldableImpl<T, TAG_TYPE, MEASURE_POLICY>::fold_map;
};

/** Registers FingerTree as a Foldable for all tag and measure combinations. */
template <class T, class TAG_TYPE, class MEASURE_POLICY>
inline constexpr auto
    foldable_typeclass<smd::tree::FingerTree2<T, TAG_TYPE, MEASURE_POLICY>> =
        FingerTreeFoldableMap<T, TAG_TYPE, MEASURE_POLICY>{};

} // namespace smd

#endif
