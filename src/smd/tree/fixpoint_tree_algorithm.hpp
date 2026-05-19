// src/smd/tree/fixpoint_tree_algorithm.hpp                            -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_FIXPOINT_TREE_ALGORITHM
#define INCLUDED_SMD_TREE_FIXPOINT_TREE_ALGORITHM

#include <smd/typeclass/foldable.hpp>
#include <smd/typeclass/traversable.hpp>

#include <cstddef>
#include <optional>
#include <utility>

namespace smd::tree::algorithm {

namespace detail {

// validate_impl: inherits the Traversable typeclass for T.
// The typeclass operations (traverse, for_each, sequence) are available
// as unqualified member calls through `this->`.
template <class T,
          const auto &TC = smd::traversable_typeclass<smd::remove_cvref_t<T>>>
struct validate_impl : smd::remove_cvref_t<decltype(TC)> {
    template <class Pred>
    auto call(Pred &&pred, const T &value) const {
        using element_type =
            typename smd::remove_cvref_t<decltype(TC)>::element_type;
        return this->for_each(
            value,
            [&](const element_type &elem) -> std::optional<element_type> {
                if (pred(elem))
                    return {elem};
                return std::nullopt;
            });
    }
};

// transform_if_large_impl: inherits both Foldable and Traversable for T.
// Demonstrates multi-typeclass composition — Foldable gives length/any_of,
// Traversable gives for_each. Both are empty bases.
template <class T,
          const auto &FC = smd::foldable_typeclass<smd::remove_cvref_t<T>>,
          const auto &TC = smd::traversable_typeclass<smd::remove_cvref_t<T>>>
struct transform_if_large_impl : smd::remove_cvref_t<decltype(FC)>,
                                 smd::remove_cvref_t<decltype(TC)> {
    using foldable_base = smd::remove_cvref_t<decltype(FC)>;
    using traversable_base = smd::remove_cvref_t<decltype(TC)>;
    using element_type = typename traversable_base::element_type;

    template <class F>
    auto call(std::size_t min_size, F &&f, const T &value) const {
        auto n = this->foldable_base::length(value);
        if (n < min_size)
            return std::optional<T>{};

        return this->traversable_base::for_each(
            value,
            [&](const element_type &elem) -> std::optional<element_type> {
                return {f(elem)};
            });
    }
};

} // namespace detail

// validate: returns optional<Tree>; nullopt if any element fails the predicate.
// Caller writes: algorithm::validate(pred, tree) — T is fully deduced.
struct validate_fn {
    template <class Pred, class T>
    auto operator()(Pred &&pred, T &&value) const {
        return detail::validate_impl<smd::remove_cvref_t<T>>{}.call(
            std::forward<Pred>(pred), std::forward<T>(value));
    }
};
inline constexpr validate_fn validate{};

// transform_if_large: applies f to every element only if the tree has at least
// min_size elements. Returns optional<Tree>: the transformed tree, or nullopt
// if the tree is too small. Shows Foldable + Traversable composition.
struct transform_if_large_fn {
    template <class F, class T>
    auto operator()(std::size_t min_size, F &&f, T &&value) const {
        return detail::transform_if_large_impl<smd::remove_cvref_t<T>>{}.call(
            min_size, std::forward<F>(f), std::forward<T>(value));
    }
};
inline constexpr transform_if_large_fn transform_if_large{};

} // namespace smd::tree::algorithm

#endif
