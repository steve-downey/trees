// src/smd/tree/finger_tree_random_access_foldable.hpp                -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_FINGER_TREE_RANDOM_ACCESS_FOLDABLE
#define INCLUDED_SMD_TREE_FINGER_TREE_RANDOM_ACCESS_FOLDABLE

#include <smd/tree/finger_tree_random_access.hpp>
#include <smd/typeclass/foldable.hpp>

#include <algorithm>
#include <functional>
#include <utility>

namespace smd {

template <class T>
struct FingerTreeRandomAccessFoldableImpl {
  template <class F>
  auto fold_map(this auto&&,
                F&& function,
                const smd::tree::FingerTreeRandomAccess<T>& sequence)
    -> remove_cvref_t<std::invoke_result_t<F, const T&>>
  {
    using Result = remove_cvref_t<std::invoke_result_t<F, const T&>>;

    return std::ranges::fold_left(
        sequence.to_vector(),
        smd::typeclass::monoid_v<Result>.identity(),
        [&](Result acc, const auto& value) {
          return smd::typeclass::monoid_v<Result>.combine(
              std::move(acc), std::invoke(function, value));
        });
  }
};

template <class T>
struct FingerTreeRandomAccessFoldableMap
  : Foldable<FingerTreeRandomAccessFoldableImpl<T>> {
  using FingerTreeRandomAccessFoldableImpl<T>::fold_map;
};

template <class T>
inline constexpr auto foldable_typeclass<smd::tree::FingerTreeRandomAccess<T>> =
  FingerTreeRandomAccessFoldableMap<T>{};

}  // namespace smd

#endif
