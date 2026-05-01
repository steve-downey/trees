// src/smd/tree/finger_tree_rope_foldable.hpp                         -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_FINGER_TREE_ROPE_FOLDABLE
#define INCLUDED_SMD_TREE_FINGER_TREE_ROPE_FOLDABLE

#include <smd/tree/finger_tree_rope.hpp>
#include <smd/typeclass/foldable.hpp>

#include <algorithm>
#include <functional>
#include <utility>

namespace smd {

struct FingerTreeRopeFoldableImpl {
  template <class F>
  auto fold_map(this auto&&,
                F&& function,
                const smd::tree::FingerTreeRope& rope)
    -> remove_cvref_t<std::invoke_result_t<F, const std::string&>>
  {
    using Result =
      remove_cvref_t<std::invoke_result_t<F, const std::string&>>;

    return std::ranges::fold_left(
        rope.chunks(),
        smd::typeclass::monoid_v<Result>.identity(),
        [&](Result acc, const auto& chunk) {
          return smd::typeclass::monoid_v<Result>.combine(
              std::move(acc), std::invoke(function, chunk));
        });
  }
};

struct FingerTreeRopeFoldableMap : Foldable<FingerTreeRopeFoldableImpl> {
  using FingerTreeRopeFoldableImpl::fold_map;
};

template <>
inline constexpr auto foldable_typeclass<smd::tree::FingerTreeRope> =
  FingerTreeRopeFoldableMap{};

}  // namespace smd

#endif
