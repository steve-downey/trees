
#ifndef INCLUDE_SMD_TREE_FIX_TREE_TRAVERSABLE_HPP
#define INCLUDE_SMD_TREE_FIX_TREE_TRAVERSABLE_HPP

#include <smd/tree/fix_tree.hpp>
#include <smd/tree/fix_tree_applicative.hpp>
#include <smd/typeclass/traversable.hpp>

#include <functional>
#include <type_traits>
#include <utility>

namespace smd {

template <class T>
struct map<traversable_tag, smd::tree::FixTree<T> > {
  template <class F>
  static auto traverse(F&& f, const smd::tree::FixTree<T>& t)
  {
    if (t.is_leaf()) {
      return smd::invoke(
        [](auto&& value) {
          using U = std::remove_cvref_t<decltype(value)>;
          return smd::tree::FixTree<U>::leaf(
            std::forward<decltype(value)>(value));
        },
        std::invoke(std::forward<F>(f), t.value()));
    }

    auto left = traverse(f, t.left());
    auto right = traverse(f, t.right());

    return smd::invoke(
      [](auto&& l, auto&& r) {
        using U = std::remove_cvref_t<decltype(l.value())>;
        return smd::tree::FixTree<U>::node(std::forward<decltype(l)>(l),
                           std::forward<decltype(r)>(r));
      },
      left,
      right);
  }
};

}  // close namespace smd

#endif
