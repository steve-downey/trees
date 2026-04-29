
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
struct FixTreeTraversableImpl {
  template <class F>
  auto traverse(this auto&& self, F&& f, const smd::tree::FixTree<T>& t)
  {
    if (t.is_leaf()) {
      auto lifted = std::invoke(std::forward<F>(f), t.value());
      using Context = std::remove_cvref_t<decltype(lifted)>;
      const auto& applicative = smd::applicative_typeclass<Context>;

      return applicative.invoke(
        [](auto&& value) {
          using U = std::remove_cvref_t<decltype(value)>;
          return smd::tree::FixTree<U>::leaf(
            std::forward<decltype(value)>(value));
        },
        lifted);
    }

      auto left = self.traverse(f, t.left());
      auto right = self.traverse(f, t.right());
    using Context = std::remove_cvref_t<decltype(left)>;
    const auto& applicative = smd::applicative_typeclass<Context>;

    return applicative.invoke(
      [](auto&& l, auto&& r) {
        using U = std::remove_cvref_t<decltype(l.value())>;
        return smd::tree::FixTree<U>::node(std::forward<decltype(l)>(l),
                           std::forward<decltype(r)>(r));
      },
      left,
      right);
  }
};

template <class T>
struct FixTreeTraversableMap : Traversable<FixTreeTraversableImpl<T> > {
  using FixTreeTraversableImpl<T>::traverse;
};

template <class T>
inline constexpr auto traversable_typeclass<smd::tree::FixTree<T> > =
    FixTreeTraversableMap<T>{};

}  // close namespace smd

#endif
