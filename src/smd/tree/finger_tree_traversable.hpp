#ifndef INCLUDE_SMD_TREE_FINGER_TREE_TRAVERSABLE_HPP
#define INCLUDE_SMD_TREE_FINGER_TREE_TRAVERSABLE_HPP

#include <smd/tree/finger_tree.hpp>
#include <smd/typeclass/applicative.hpp>
#include <smd/typeclass/traversable.hpp>

#include <functional>
#include <type_traits>
#include <utility>
#include <vector>

namespace smd {

template <class T, class TAG_TYPE, class MEASURE_POLICY>
struct FingerTreeTraversableImpl {
  template <class F>
  auto traverse(this auto&&,
                F&& function,
                const smd::tree::FingerTree<T, TAG_TYPE, MEASURE_POLICY>& tree)
  {
    using Context = remove_cvref_t<std::invoke_result_t<F, const T&>>;
    const auto& applicative = smd::applicative_typeclass<Context>;
    using U = smd::applicative_value_t<Context>;

    auto accumulated = applicative.pure(std::vector<U>{});

    for (const auto& value : tree.flatten()) {
      auto lifted = std::invoke(function, value);
      accumulated = applicative.invoke(
        [](std::vector<U> values, U element) {
          values.push_back(std::move(element));
          return values;
        },
        std::move(accumulated),
        std::move(lifted));
    }

    return applicative.invoke(
      [](std::vector<U> values) {
        return smd::tree::FingerTree<U>::from_sequence(std::move(values));
      },
      std::move(accumulated));
  }
};

template <class T, class TAG_TYPE, class MEASURE_POLICY>
struct FingerTreeTraversableMap
  : Traversable<FingerTreeTraversableImpl<T, TAG_TYPE, MEASURE_POLICY>> {
  using FingerTreeTraversableImpl<T, TAG_TYPE, MEASURE_POLICY>::traverse;
};

template <class T, class TAG_TYPE, class MEASURE_POLICY>
inline constexpr auto traversable_typeclass<
  smd::tree::FingerTree<T, TAG_TYPE, MEASURE_POLICY>> =
  FingerTreeTraversableMap<T, TAG_TYPE, MEASURE_POLICY>{};

}  // close namespace smd

#endif
