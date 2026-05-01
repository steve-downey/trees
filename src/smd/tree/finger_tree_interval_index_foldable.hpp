// src/smd/tree/finger_tree_interval_index_foldable.hpp               -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_FINGER_TREE_INTERVAL_INDEX_FOLDABLE
#define INCLUDED_SMD_TREE_FINGER_TREE_INTERVAL_INDEX_FOLDABLE

#include <smd/tree/finger_tree_interval_index.hpp>
#include <smd/typeclass/foldable.hpp>

namespace smd {

template <class PAYLOAD_TYPE>
struct FingerTreeIntervalIndexFoldableImpl {
  template <class F>
  auto fold_map(this auto&&,
                F&& function,
                const smd::tree::FingerTreeIntervalIndex<PAYLOAD_TYPE>& index)
    -> remove_cvref_t<std::invoke_result_t<F, const PAYLOAD_TYPE&>>
  {
    using Result = remove_cvref_t<std::invoke_result_t<F, const PAYLOAD_TYPE&>>;

    auto acc = smd::typeclass::monoid_v<Result>.identity();
    for (const auto& entry : index.entries()) {
      acc = smd::typeclass::monoid_v<Result>.combine(
        std::move(acc),
        std::invoke(function, entry.d_payload));
    }

    return acc;
  }
};

template <class PAYLOAD_TYPE>
struct FingerTreeIntervalIndexFoldableMap
  : Foldable<FingerTreeIntervalIndexFoldableImpl<PAYLOAD_TYPE>> {
  using FingerTreeIntervalIndexFoldableImpl<PAYLOAD_TYPE>::fold_map;
};

template <class PAYLOAD_TYPE>
inline constexpr auto foldable_typeclass<
  smd::tree::FingerTreeIntervalIndex<PAYLOAD_TYPE>> =
  FingerTreeIntervalIndexFoldableMap<PAYLOAD_TYPE>{};

}  // namespace smd

#endif
