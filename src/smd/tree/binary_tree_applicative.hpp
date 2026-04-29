#ifndef INCLUDE_SMD_TREE_BINARY_TREE_APPLICATIVE_HPP
#define INCLUDE_SMD_TREE_BINARY_TREE_APPLICATIVE_HPP

#include <smd/tree/binary_tree.hpp>
#include <smd/typeclass/applicative.hpp>

#include <memory>
#include <type_traits>
#include <utility>

namespace smd {

template <class T>
struct BinaryTreeApplicativeImpl {
  template <class VALUE>
  auto pure(this auto&&, VALUE&& value)
  {
    using U = remove_cvref_t<VALUE>;
    return smd::tree::BinaryTree<U>::leaf(std::forward<VALUE>(value));
  }

  template <class F, class A>
  auto apply(this auto&& self,
             const smd::tree::BinaryTree<F>& functions,
             const smd::tree::BinaryTree<A>& arguments)
    -> smd::tree::BinaryTree<std::invoke_result_t<const F&, const A&>>
  {
    using R = std::invoke_result_t<const F&, const A&>;

    std::shared_ptr<smd::tree::BinaryTree<R> > left{};
    std::shared_ptr<smd::tree::BinaryTree<R> > right{};

    const auto function_is_leaf = !functions.has_left() && !functions.has_right();
    const auto argument_is_leaf = !arguments.has_left() && !arguments.has_right();

    if (function_is_leaf) {
      // pure(f) should distribute f over the argument shape.
      if (arguments.has_left()) {
        left = smd::tree::BinaryTree<R>::make_ptr(
          self.apply(functions, arguments.left()));
      }
      if (arguments.has_right()) {
        right = smd::tree::BinaryTree<R>::make_ptr(
          self.apply(functions, arguments.right()));
      }
    } else if (argument_is_leaf) {
      // A non-leaf function tree can be applied pointwise to a single argument.
      if (functions.has_left()) {
        left = smd::tree::BinaryTree<R>::make_ptr(
          self.apply(functions.left(), arguments));
      }
      if (functions.has_right()) {
        right = smd::tree::BinaryTree<R>::make_ptr(
          self.apply(functions.right(), arguments));
      }
    } else {
      // If both have shape, recurse pairwise where both children exist.
      if (functions.has_left() && arguments.has_left()) {
        left = smd::tree::BinaryTree<R>::make_ptr(
          self.apply(functions.left(), arguments.left()));
      }

      if (functions.has_right() && arguments.has_right()) {
        right = smd::tree::BinaryTree<R>::make_ptr(
          self.apply(functions.right(), arguments.right()));
      }
    }

    return smd::tree::BinaryTree<R>::from_children_ptrs(
      functions.value()(arguments.value()),
      std::move(left),
      std::move(right));
  }
};

template <class T>
struct BinaryTreeApplicativeMap : Applicative<BinaryTreeApplicativeImpl<T> > {
  using BinaryTreeApplicativeImpl<T>::apply;
  using BinaryTreeApplicativeImpl<T>::pure;
};

template <class T>
inline constexpr auto applicative_typeclass<smd::tree::BinaryTree<T> > =
  BinaryTreeApplicativeMap<T>{};

}  // close namespace smd

#endif
