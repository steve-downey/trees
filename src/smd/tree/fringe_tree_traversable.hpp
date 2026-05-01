// src/smd/tree/fringe_tree_traversable.hpp                           -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_FRINGE_TREE_TRAVERSABLE
#define INCLUDED_SMD_TREE_FRINGE_TREE_TRAVERSABLE

#include <smd/tree/fringe_tree.hpp>
#include <smd/tree/fringe_tree_applicative.hpp>
#include <smd/typeclass/traversable.hpp>

#include <functional>
#include <type_traits>
#include <utility>

namespace smd {

template <class T>
struct FringeTreeTraversableImpl {
  template <class F>
  auto traverse(this auto&& self, F&& function, const smd::tree::FringeTree<T>& tree)
    -> decltype(smd::applicative_typeclass<remove_cvref_t<std::invoke_result_t<F, const T&> >>.invoke(
      [](auto&& value) {
        using U = remove_cvref_t<decltype(value)>;
        return smd::tree::FringeTree<U>::leaf(std::forward<decltype(value)>(value));
      },
      std::invoke(std::forward<F>(function), tree.value())))
  {
    using Context = remove_cvref_t<std::invoke_result_t<F, const T&>>;
    const auto& applicative = smd::applicative_typeclass<Context>;
    using U = smd::applicative_value_t<Context>;

    if (tree.is_empty()) {
      return applicative.pure(smd::tree::FringeTree<U>::empty());
    }

    if (tree.is_leaf()) {
      auto lifted = std::invoke(std::forward<F>(function), tree.value());

      return applicative.invoke(
        [](auto&& value) {
          using U = remove_cvref_t<decltype(value)>;
          return smd::tree::FringeTree<U>::leaf(std::forward<decltype(value)>(value));
        },
        lifted);
    }

    auto left = self.traverse(function, tree.left());
    auto right = self.traverse(function, tree.right());

    return applicative.invoke(
      [](auto&& l, auto&& r) {
        return smd::tree::FringeTree<remove_cvref_t<decltype(l.value())> >::branch(
          std::forward<decltype(l)>(l),
          std::forward<decltype(r)>(r));
      },
      left,
      right);
  }
};

template <class T>
struct FringeTreeTraversableMap : Traversable<FringeTreeTraversableImpl<T> > {
  using FringeTreeTraversableImpl<T>::traverse;
};

template <class T>
inline constexpr auto traversable_typeclass<smd::tree::FringeTree<T> > =
  FringeTreeTraversableMap<T>{};

}  // close namespace smd

#endif
