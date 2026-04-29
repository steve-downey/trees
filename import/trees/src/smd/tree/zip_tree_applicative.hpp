#ifndef INCLUDE_SMD_TREE_ZIP_TREE_APPLICATIVE_HPP
#define INCLUDE_SMD_TREE_ZIP_TREE_APPLICATIVE_HPP
#include <smd/tree/fix_tree.hpp>
namespace smd {

template<class T>
struct zip_tree_map {

  static auto pure(const T& x){
    return smd::tree::FixTree<T>::leaf(x);
  }

  template<class F,class A>
  static auto apply(const smd::tree::FixTree<F>& fs,
                    const smd::tree::FixTree<A>& xs){

    using R = decltype(fs.value()(xs.value()));

    if(fs.is_leaf() && xs.is_leaf())
      return smd::tree::FixTree<R>::leaf(fs.value()(xs.value()));

    if(!fs.is_leaf() && !xs.is_leaf())
      return smd::tree::FixTree<R>::node(
        apply(fs.left(),xs.left()),
        apply(fs.right(),xs.right()));

    return smd::tree::FixTree<R>::leaf(R{});
  }
};

}
#endif
