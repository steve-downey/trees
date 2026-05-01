// src/smd/tree/finger_tree_random_access.hpp                         -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_FINGER_TREE_RANDOM_ACCESS
#define INCLUDED_SMD_TREE_FINGER_TREE_RANDOM_ACCESS

#include <smd/tree/finger_tree.hpp>
#include <smd/typeclass/applicative.hpp>
#include <smd/typeclass/foldable.hpp>
#include <smd/typeclass/traversable.hpp>

#include <algorithm>
#include <cstddef>
#include <functional>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

namespace smd::tree {

template <typename T>
class FingerTreeRandomAccess {
  FingerTree<T> d_tree;

 public:
  FingerTreeRandomAccess()
    : d_tree(FingerTree<T>::empty())
  {
  }

  explicit FingerTreeRandomAccess(FingerTree<T> tree)
    : d_tree(std::move(tree))
  {
  }

  static auto from_sequence(std::vector<T> values) -> FingerTreeRandomAccess
  {
    return FingerTreeRandomAccess(FingerTree<T>::from_sequence(std::move(values)));
  }

  auto size() const -> std::size_t { return d_tree.breadth(); }

  auto empty() const -> bool { return d_tree.is_empty(); }

  auto at(std::size_t index) const -> std::optional<T>
  {
    if (index >= size()) {
      return std::nullopt;
    }
    // split() with count predicate: pivot is the element at position index. O(log n).
    auto sp = d_tree.split([index](std::size_t prefix) { return prefix > index; });
    if (!sp.has_value()) {
      return std::nullopt;
    }
    return sp->d_pivot;
  }

  auto push_back(T value) const -> FingerTreeRandomAccess
  {
    return FingerTreeRandomAccess(d_tree.snoc(std::move(value)));
  }

  auto push_front(T value) const -> FingerTreeRandomAccess
  {
    return FingerTreeRandomAccess(d_tree.cons(std::move(value)));
  }

  auto insert(std::size_t index, T value) const -> FingerTreeRandomAccess
  {
    // split_at with count predicate puts [0,index) left, [index,n) right. O(log n).
    auto parts = d_tree.split_at([index](std::size_t prefix) { return prefix > index; });
    return FingerTreeRandomAccess(FingerTree<T>::concat(
      FingerTree<T>::concat(parts.d_left, FingerTree<T>::leaf(std::move(value))),
      parts.d_right));
  }

  auto erase(std::size_t index) const -> FingerTreeRandomAccess
  {
    if (index >= size()) {
      return *this;
    }
    // split() finds the element at index as pivot; drop it by concat(left, right). O(log n).
    auto sp = d_tree.split([index](std::size_t prefix) { return prefix > index; });
    if (!sp.has_value()) {
      return *this;
    }
    return FingerTreeRandomAccess(FingerTree<T>::concat(sp->d_left, sp->d_right));
  }

  auto update(std::size_t index, T value) const -> FingerTreeRandomAccess
  {
    if (index >= size()) {
      return *this;
    }
    // Single split+replace: find element at index as pivot, swap in the new value. O(log n).
    auto sp = d_tree.split([index](std::size_t prefix) { return prefix > index; });
    if (!sp.has_value()) {
      return *this;
    }
    return FingerTreeRandomAccess(
      FingerTree<T>::concat(sp->d_left.snoc(std::move(value)), sp->d_right));
  }

  auto to_vector() const -> std::vector<T> { return d_tree.flatten(); }
};

}  // namespace smd::tree

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

template <class T>
struct FingerTreeRandomAccessTraversableImpl {
  using element_type = T;

  template <class APPLICATIVE, class F>
  auto traverse(this auto&&,
                const APPLICATIVE& applicative,
                F&& function,
                const smd::tree::FingerTreeRandomAccess<T>& sequence)
  {
    using Context = remove_cvref_t<std::invoke_result_t<F, const T&>>;
    using U = smd::applicative_value_t<Context>;

    auto accumulated = applicative.pure(std::vector<U>{});

    for (const auto& value : sequence.to_vector()) {
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
        return smd::tree::FingerTreeRandomAccess<U>::from_sequence(
          std::move(values));
      },
      std::move(accumulated));
  }
};

template <class T>
struct FingerTreeRandomAccessTraversableMap
  : Traversable<FingerTreeRandomAccessTraversableImpl<T>> {
  using FingerTreeRandomAccessTraversableImpl<T>::traverse;
};

template <class T>
inline constexpr auto traversable_typeclass<smd::tree::FingerTreeRandomAccess<T>> =
  FingerTreeRandomAccessTraversableMap<T>{};

}  // namespace smd

#endif
