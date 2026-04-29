#ifndef INCLUDE_SMD_TREE_FIX_TREE_FOLDABLE_HPP
#define INCLUDE_SMD_TREE_FIX_TREE_FOLDABLE_HPP

#include <smd/tree/fix_tree.hpp>
#include <smd/typeclass/foldable.hpp>

namespace smd {

template<class T>
struct map<foldable_tag, smd::tree::FixTree<T>> {

  template<class F>
  static auto fold_map(F f, const smd::tree::FixTree<T>& t){
    if(t.is_leaf()) return f(t.value());
    return fold_map(f, t.left()) + fold_map(f, t.right());
  }
};

}

#endif
