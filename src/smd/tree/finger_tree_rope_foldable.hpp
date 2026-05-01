// src/smd/tree/finger_tree_rope_foldable.hpp                         -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_FINGER_TREE_ROPE_FOLDABLE
#define INCLUDED_SMD_TREE_FINGER_TREE_ROPE_FOLDABLE

#include <smd/tree/finger_tree_rope.hpp>
#include <smd/typeclass/foldable.hpp>

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

    auto acc = smd::typeclass::monoid_v<Result>.identity();
    for (const auto& chunk : rope.chunks()) {
      acc = smd::typeclass::monoid_v<Result>.combine(
        std::move(acc),
        std::invoke(function, chunk));
    }

    return acc;
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
