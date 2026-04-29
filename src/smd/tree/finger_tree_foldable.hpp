#ifndef INCLUDE_SMD_TREE_FINGER_TREE_FOLDABLE_HPP
#define INCLUDE_SMD_TREE_FINGER_TREE_FOLDABLE_HPP

#include <smd/tree/finger_tree.hpp>
#include <smd/typeclass/foldable.hpp>

#include <functional>
#include <type_traits>
#include <utility>

namespace smd {

template <class T, class TAG_TYPE, class MEASURE_POLICY>
struct FingerTreeFoldableImpl {
  template <class F>
  auto fold_map(this auto&&,
                F&& function,
                const smd::tree::FingerTree<T, TAG_TYPE, MEASURE_POLICY>& tree)
    -> remove_cvref_t<std::invoke_result_t<F, const T&>>
  {
    using Result = remove_cvref_t<std::invoke_result_t<F, const T&>>;

    auto acc = smd::typeclass::monoid_v<Result>.identity();
    for (const auto& value : tree.flatten()) {
      acc = smd::typeclass::monoid_v<Result>.combine(
        std::move(acc),
        std::invoke(function, value));
    }

    return acc;
  }
};

template <class T, class TAG_TYPE, class MEASURE_POLICY>
struct FingerTreeFoldableMap
  : Foldable<FingerTreeFoldableImpl<T, TAG_TYPE, MEASURE_POLICY>> {
  using FingerTreeFoldableImpl<T, TAG_TYPE, MEASURE_POLICY>::fold_map;
};

template <class T, class TAG_TYPE, class MEASURE_POLICY>
inline constexpr auto foldable_typeclass<
  smd::tree::FingerTree<T, TAG_TYPE, MEASURE_POLICY>> =
  FingerTreeFoldableMap<T, TAG_TYPE, MEASURE_POLICY>{};

}  // close namespace smd

#endif
