// src/smd/tree/fix_tree_applicative.hpp                              -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_FIX_TREE_APPLICATIVE
#define INCLUDED_SMD_TREE_FIX_TREE_APPLICATIVE

#include <smd/tree/fix_tree.hpp>
#include <smd/typeclass/applicative.hpp>

#include <functional>
#include <type_traits>
#include <utility>

namespace smd {

template <class T>
struct FixTreeApplicativeImpl {
  template <class VALUE>
  auto pure(this auto&&, VALUE&& x)
  {
    using U = std::remove_cvref_t<VALUE>;
    return smd::tree::FixTree<U>::leaf(std::forward<VALUE>(x));
  }

  template <class F, class A>
  auto apply(this auto&& self,
             const smd::tree::FixTree<F>& fs,
             const smd::tree::FixTree<A>& xs)
  {
    using R = decltype(fs.value()(xs.value()));
    if (fs.is_leaf()) {
      auto f = fs.value();
      if (xs.is_leaf()) {
        return smd::tree::FixTree<R>::leaf(f(xs.value()));
      }
      return smd::tree::FixTree<R>::node(
        self.apply(fs, xs.left()), self.apply(fs, xs.right()));
    }
    return smd::tree::FixTree<R>::node(self.apply(fs.left(), xs),
                                       self.apply(fs.right(), xs));
  }
};

template <class T>
struct FixTreeApplicativeMap : Applicative<FixTreeApplicativeImpl<T> > {
  using FixTreeApplicativeImpl<T>::apply;
  using FixTreeApplicativeImpl<T>::pure;
};

template <class T>
inline constexpr auto applicative_typeclass<smd::tree::FixTree<T> > =
  FixTreeApplicativeMap<T>{};

}

#endif
