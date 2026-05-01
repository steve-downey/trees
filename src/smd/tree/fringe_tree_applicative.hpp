// src/smd/tree/fringe_tree_applicative.hpp                           -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_FRINGE_TREE_APPLICATIVE
#define INCLUDED_SMD_TREE_FRINGE_TREE_APPLICATIVE

#include <smd/tree/fringe_tree.hpp>
#include <smd/typeclass/applicative.hpp>

#include <type_traits>
#include <utility>

namespace smd {

template <class T>
struct FringeTreeApplicativeImpl {
  template <class VALUE>
  auto pure(this auto&&, VALUE&& value)
  {
    using U = remove_cvref_t<VALUE>;
    return smd::tree::FringeTree<U>::leaf(std::forward<VALUE>(value));
  }

  template <class F, class A>
  auto apply(this auto&& self,
             const smd::tree::FringeTree<F>& functions,
             const smd::tree::FringeTree<A>& arguments)
    -> smd::tree::FringeTree<std::invoke_result_t<const F&, const A&>>
  {
    using R = std::invoke_result_t<const F&, const A&>;

    if (functions.is_empty() || arguments.is_empty()) {
      return smd::tree::FringeTree<R>::empty();
    }

    if (functions.is_leaf()) {
      auto function = functions.value();
      if (arguments.is_leaf()) {
        return smd::tree::FringeTree<R>::leaf(function(arguments.value()));
      }
      return smd::tree::FringeTree<R>::branch(
        self.apply(functions, arguments.left()),
        self.apply(functions, arguments.right()));
    }

    if (arguments.is_leaf()) {
      return smd::tree::FringeTree<R>::branch(
        self.apply(functions.left(), arguments),
        self.apply(functions.right(), arguments));
    }

    return smd::tree::FringeTree<R>::branch(
      self.apply(functions.left(), arguments.left()),
      self.apply(functions.right(), arguments.right()));
  }
};

template <class T>
struct FringeTreeApplicativeMap : Applicative<FringeTreeApplicativeImpl<T> > {
  using FringeTreeApplicativeImpl<T>::apply;
  using FringeTreeApplicativeImpl<T>::pure;
};

template <class T>
inline constexpr auto applicative_typeclass<smd::tree::FringeTree<T> > =
  FringeTreeApplicativeMap<T>{};

}  // close namespace smd

#endif
