#ifndef INCLUDE_SMD_TREE_FIX_TREE_APPLICATIVE_HPP
#define INCLUDE_SMD_TREE_FIX_TREE_APPLICATIVE_HPP

#include <smd/tree/fix_tree.hpp>
#include <smd/typeclass/applicative.hpp>

namespace smd {

template<class T>
struct map<applicative_tag, smd::tree::FixTree<T>> {

  static auto pure(const T& x){
    return smd::tree::FixTree<T>::leaf(x);
  }

  template<class F, class A>
  static auto apply(const smd::tree::FixTree<F>& fs,
                    const smd::tree::FixTree<A>& xs){

    using R = decltype(std::declval<F>()(std::declval<A>()));

    if(fs.is_leaf()){
      auto f = fs.value();

      if(xs.is_leaf()){
        return smd::tree::FixTree<R>::leaf(f(xs.value()));
      }

      return smd::tree::FixTree<R>::node(
        apply(fs, xs.left()),
        apply(fs, xs.right())
      );
    }

    return smd::tree::FixTree<R>::node(
      apply(fs.left(), xs),
      apply(fs.right(), xs)
    );
  }
};

}

#endif
