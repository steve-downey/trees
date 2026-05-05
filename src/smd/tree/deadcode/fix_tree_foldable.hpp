// src/smd/tree/fix_tree_foldable.hpp                                 -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_FIX_TREE_FOLDABLE
#define INCLUDED_SMD_TREE_FIX_TREE_FOLDABLE

#include <smd/tree/fix_tree.hpp>
#include <smd/typeclass/foldable.hpp>

#include <functional>
#include <type_traits>

namespace smd {

template <class T>
struct FixTreeFoldableImpl {

    // a3f7b2e1-9c4d-4f8a-b6e3-2d5c8a1f4b07
    template <class F>
    auto fold_map(this auto &&self, F &&f, const smd::tree::FixTree<T> &t) {
        if (t.is_leaf()) {
            return std::invoke(f, t.value());
        }

        auto lhs = self.fold_map(f, t.left());
        auto rhs = self.fold_map(f, t.right());

        using Result = std::remove_cvref_t<decltype(lhs)>;
        return smd::typeclass::monoid_v<Result>.combine(lhs, rhs);
    }
    // a3f7b2e1-9c4d-4f8a-b6e3-2d5c8a1f4b07 end
};

template <class T>
struct FixTreeFoldableMap : Foldable<FixTreeFoldableImpl<T>> {
    using FixTreeFoldableImpl<T>::fold_map;
};

// d6e2b9f4-1a7c-4b3e-8f5d-3c9a2e7b6f08
template <class T>
inline constexpr auto foldable_typeclass<smd::tree::FixTree<T>> =
    FixTreeFoldableMap<T>{};
// d6e2b9f4-1a7c-4b3e-8f5d-3c9a2e7b6f08 end

} // namespace smd

#endif
