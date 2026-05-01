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

    template <class F>
    auto fold_map(this auto&& self, F&& f, const smd::tree::FixTree<T>& t)
    {
        if (t.is_leaf()) {
            return std::invoke(f, t.value());
        }

        auto lhs = self.fold_map(f, t.left());
        auto rhs = self.fold_map(f, t.right());

        using Result = std::remove_cvref_t<decltype(lhs)>;
        return smd::typeclass::monoid_v<Result>.combine(lhs, rhs);
    }
};

template <class T>
struct FixTreeFoldableMap : Foldable<FixTreeFoldableImpl<T> > {
    using FixTreeFoldableImpl<T>::fold_map;
};

template <class T>
inline constexpr auto foldable_typeclass<smd::tree::FixTree<T> > =
    FixTreeFoldableMap<T>{};

}  // close namespace smd

#endif
