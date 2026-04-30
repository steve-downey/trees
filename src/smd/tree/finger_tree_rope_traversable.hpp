#ifndef INCLUDE_SMD_TREE_FINGER_TREE_ROPE_TRAVERSABLE_HPP
#define INCLUDE_SMD_TREE_FINGER_TREE_ROPE_TRAVERSABLE_HPP

#include <smd/tree/finger_tree_rope.hpp>
#include <smd/typeclass/applicative.hpp>
#include <smd/typeclass/traversable.hpp>

#include <type_traits>
#include <utility>
#include <vector>

namespace smd {

struct FingerTreeRopeTraversableImpl {
  template <class F>
  auto traverse(this auto&&,
                F&& function,
                const smd::tree::FingerTreeRope& rope)
  {
    using Context = remove_cvref_t<std::invoke_result_t<F, const std::string&>>;
    const auto& applicative = smd::applicative_typeclass<Context>;
    using U = smd::applicative_value_t<Context>;

    auto accumulated = applicative.pure(std::vector<U>{});

    for (const auto& chunk : rope.chunks()) {
      auto lifted = std::invoke(function, chunk);
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
        return smd::tree::FingerTreeRope::from_chunks(std::move(values));
      },
      std::move(accumulated));
  }
};

struct FingerTreeRopeTraversableMap : Traversable<FingerTreeRopeTraversableImpl> {
  using FingerTreeRopeTraversableImpl::traverse;
};

template <>
inline constexpr auto traversable_typeclass<smd::tree::FingerTreeRope> =
  FingerTreeRopeTraversableMap{};

}  // namespace smd

#endif
