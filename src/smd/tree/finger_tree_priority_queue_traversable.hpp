// src/smd/tree/finger_tree_priority_queue_traversable.hpp            -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_FINGER_TREE_PRIORITY_QUEUE_TRAVERSABLE
#define INCLUDED_SMD_TREE_FINGER_TREE_PRIORITY_QUEUE_TRAVERSABLE

#include <smd/tree/finger_tree_priority_queue.hpp>
#include <smd/typeclass/applicative.hpp>
#include <smd/typeclass/traversable.hpp>

#include <type_traits>
#include <utility>
#include <vector>

namespace smd {

template <class T>
struct FingerTreePriorityQueueTraversableImpl {
  using element_type = T;

  template <class APPLICATIVE, class F>
  auto traverse(this auto&&,
                const APPLICATIVE& applicative,
                F&& function,
                const smd::tree::FingerTreePriorityQueue<T>& queue)
  {
    using Context = remove_cvref_t<std::invoke_result_t<F, const T&>>;
    using U = smd::applicative_value_t<Context>;

    auto accumulated = applicative.pure(std::vector<U>{});

    for (const auto& value : queue.to_vector()) {
      auto lifted = std::invoke(function, value);
      accumulated = applicative.invoke(
        [](std::vector<U> values, U element) {
          values.push_back(std::move(element));
          return values;
        },
        std::move(accumulated),
        std::move(lifted));
    }

    return applicative.invoke(
      [](std::vector<U> values) {
        return smd::tree::FingerTreePriorityQueue<U>::from_values(
          std::move(values));
      },
      std::move(accumulated));
  }
};

template <class T>
struct FingerTreePriorityQueueTraversableMap
  : Traversable<FingerTreePriorityQueueTraversableImpl<T>> {
  using FingerTreePriorityQueueTraversableImpl<T>::traverse;
};

template <class T>
inline constexpr auto traversable_typeclass<
  smd::tree::FingerTreePriorityQueue<T>> =
  FingerTreePriorityQueueTraversableMap<T>{};

}  // namespace smd

#endif
