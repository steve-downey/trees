// src/smd/tree/fix_tree_traversable.hpp                              -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDED_SMD_TREE_FIX_TREE_TRAVERSABLE
#define INCLUDED_SMD_TREE_FIX_TREE_TRAVERSABLE

#include <smd/tree/fix_tree.hpp>
#include <smd/tree/fix_tree_applicative.hpp>
#include <smd/typeclass/traversable.hpp>

#include <functional>
#include <type_traits>
#include <utility>

namespace smd {

template <class T>
struct FixTreeTraversableImpl {
    using element_type = T;

    template <class APPLICATIVE, class F>
    auto traverse(this auto &&self, const APPLICATIVE &applicative, F &&f,
                  const smd::tree::FixTree<T> &t) {
        if (t.is_leaf()) {
            return applicative.invoke(
                [](auto &&value) {
                    using U = std::remove_cvref_t<decltype(value)>;
                    return smd::tree::FixTree<U>::leaf(
                        std::forward<decltype(value)>(value));
                },
                std::invoke(std::forward<F>(f), t.value()));
        }

        auto left = self.traverse(applicative, f, t.left());
        auto right = self.traverse(applicative, f, t.right());

        return applicative.invoke(
            [](auto &&l, auto &&r) {
                using U = std::remove_cvref_t<decltype(l.value())>;
                return smd::tree::FixTree<U>::node(
                    std::forward<decltype(l)>(l), std::forward<decltype(r)>(r));
            },
            left, right);
    }
};

template <class T>
struct FixTreeTraversableMap : Traversable<FixTreeTraversableImpl<T>> {
    using FixTreeTraversableImpl<T>::traverse;
};

template <class T>
inline constexpr auto traversable_typeclass<smd::tree::FixTree<T>> =
    FixTreeTraversableMap<T>{};

} // namespace smd

#endif
