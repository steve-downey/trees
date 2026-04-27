
#ifndef INCLUDE_SMD_TREE_FIX_TREE_APPLICATIVE_HPP
#define INCLUDE_SMD_TREE_FIX_TREE_APPLICATIVE_HPP

#include <smd/tree/fix_tree.hpp>

namespace smd::tree {

template<class T>
FixTree<T> pure(T x){
  return FixTree<T>::leaf(x);
}

template<class F,class A>
auto apply(const FixTree<F>& fs, const FixTree<A>& xs){
  using R = decltype(fs.value()(xs.value()));

  if(fs.is_leaf()){
    auto f = fs.value();
    if(xs.is_leaf()){
      return FixTree<R>::leaf(f(xs.value()));
    }
    return FixTree<R>::node(
      apply(fs, xs.left()),
      apply(fs, xs.right()));
  }

  return FixTree<R>::node(
    apply(fs.left(), xs),
    apply(fs.right(), xs));
}

}

#endif
