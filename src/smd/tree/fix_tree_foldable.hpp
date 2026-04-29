#ifndef INCLUDE_SMD_TREE_FIX_TREE_FOLDABLE_HPP
#define INCLUDE_SMD_TREE_FIX_TREE_FOLDABLE_HPP

#include <smd/tree/fix_tree.hpp>
#include <smd/typeclass/foldable.hpp>

#include <type_traits>

namespace smd {

template<class T>
struct map<foldable_tag, smd::tree::FixTree<T>> {

  template<class F>
  static auto fold_map(F f, const smd::tree::FixTree<T>& t){
    if(t.is_leaf()) {
      return f(t.value());
    }

    auto lhs = fold_map(f, t.left());
    auto rhs = fold_map(f, t.right());

    using Result = std::remove_cvref_t<decltype(lhs)>;
    return smd::typeclass::monoid_v<Result>.combine(lhs, rhs);
  }
};

}

#endif
