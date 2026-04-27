
#ifndef INCLUDE_SMD_TREE_FIX_TREE_FOLDABLE_HPP
#define INCLUDE_SMD_TREE_FIX_TREE_FOLDABLE_HPP

#include <smd/tree/fix_tree.hpp>

namespace smd::tree {

template<class F, class T>
auto fold_map(F f, const FixTree<T>& t){
  if(t.is_leaf()) return f(t.value());
  return fold_map(f,t.left()) + fold_map(f,t.right());
}

}

#endif
