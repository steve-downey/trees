
#ifndef INCLUDE_SMD_TREE_FIX_TREE_TRAVERSABLE_HPP
#define INCLUDE_SMD_TREE_FIX_TREE_TRAVERSABLE_HPP

#include <smd/tree/fix_tree.hpp>
#include <smd/tree/fix_tree_applicative.hpp>

namespace smd::tree {

template<class F,class T>
auto traverse(F f, const FixTree<T>& t){
  if(t.is_leaf()) return f(t.value());

  auto l = traverse(f, t.left());
  auto r = traverse(f, t.right());

  return apply(
    apply(pure([](auto a, auto b){
      return FixTree<decltype(a)>::node(a,b);
    }), l),
    r);
}

}

#endif
