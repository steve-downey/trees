#ifndef INCLUDE_SMD_TREE_FINGER_TREE_RANDOM_ACCESS_HPP
#define INCLUDE_SMD_TREE_FINGER_TREE_RANDOM_ACCESS_HPP

#include <smd/tree/finger_tree.hpp>
#include <smd/typeclass/applicative.hpp>
#include <smd/typeclass/foldable.hpp>
#include <smd/typeclass/traversable.hpp>

#include <cstddef>
#include <functional>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

namespace smd::tree {

template <typename T>
class FingerTreeRandomAccess {
  FingerTree<T> d_tree;

 public:
  FingerTreeRandomAccess()
    : d_tree(FingerTree<T>::empty())
  {
  }

  explicit FingerTreeRandomAccess(FingerTree<T> tree)
    : d_tree(std::move(tree))
  {
  }

  static auto from_sequence(std::vector<T> values) -> FingerTreeRandomAccess
  {
    return FingerTreeRandomAccess(FingerTree<T>::from_sequence(std::move(values)));
  }

  auto size() const -> std::size_t { return d_tree.breadth(); }

  auto empty() const -> bool { return d_tree.is_empty(); }

  auto at(std::size_t index) const -> std::optional<T>
  {
    if (index >= size()) {
      return std::nullopt;
    }
    // Use indexed split to minimize materialization
    // Split at index+1 to get everything up to and including the element
    auto parts = d_tree.split_at_index(index + 1);
    // The element at index is the last element of the left part
    auto left_vec = parts.d_left.flatten();
    return left_vec.back();
  }

  auto push_back(T value) const -> FingerTreeRandomAccess
  {
    return FingerTreeRandomAccess(d_tree.snoc(std::move(value)));
  }

  auto push_front(T value) const -> FingerTreeRandomAccess
  {
    return FingerTreeRandomAccess(d_tree.cons(std::move(value)));
  }

  auto insert(std::size_t index, T value) const -> FingerTreeRandomAccess
  {
    auto parts = d_tree.split_at_index(index);
    auto middle = FingerTree<T>::leaf(std::move(value));
    return FingerTreeRandomAccess(FingerTree<T>::concat(FingerTree<T>::concat(parts.d_left, middle), parts.d_right));
  }

  auto erase(std::size_t index) const -> FingerTreeRandomAccess
  {
    if (index >= size()) {
      return *this;
    }

    auto left_right = d_tree.split_at_index(index);
    auto drop_rest = left_right.d_right.tail();
    return FingerTreeRandomAccess(FingerTree<T>::concat(left_right.d_left, drop_rest));
  }

  auto update(std::size_t index, T value) const -> FingerTreeRandomAccess
  {
    return erase(index).insert(index, std::move(value));
  }

  auto to_vector() const -> std::vector<T> { return d_tree.flatten(); }
};

}  // namespace smd::tree

#endif

#include <smd/tree/finger_tree_random_access_foldable.hpp>
#include <smd/tree/finger_tree_random_access_traversable.hpp>
