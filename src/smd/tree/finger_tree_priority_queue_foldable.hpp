// src/smd/tree/finger_tree_priority_queue_foldable.hpp               -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_FINGER_TREE_PRIORITY_QUEUE_FOLDABLE
#define INCLUDED_SMD_TREE_FINGER_TREE_PRIORITY_QUEUE_FOLDABLE

#include <smd/tree/finger_tree_priority_queue.hpp>
#include <smd/typeclass/foldable.hpp>

#include <algorithm>
#include <functional>
#include <utility>

namespace smd {

template <class T>
struct FingerTreePriorityQueueFoldableImpl {
  template <class F>
  auto fold_map(this auto&&,
                F&& function,
                const smd::tree::FingerTreePriorityQueue<T>& queue)
    -> remove_cvref_t<std::invoke_result_t<F, const T&>>
  {
    using Result = remove_cvref_t<std::invoke_result_t<F, const T&>>;

    return std::ranges::fold_left(
        queue.to_vector(),
        smd::typeclass::monoid_v<Result>.identity(),
        [&](Result acc, const auto& value) {
          return smd::typeclass::monoid_v<Result>.combine(
              std::move(acc), std::invoke(function, value));
        });
  }
};

template <class T>
struct FingerTreePriorityQueueFoldableMap
  : Foldable<FingerTreePriorityQueueFoldableImpl<T>> {
  using FingerTreePriorityQueueFoldableImpl<T>::fold_map;
};

template <class T>
inline constexpr auto foldable_typeclass<smd::tree::FingerTreePriorityQueue<T>> =
  FingerTreePriorityQueueFoldableMap<T>{};

}  // namespace smd

#endif
