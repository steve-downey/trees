#ifndef INCLUDE_SMD_TREE_FIX_TREE_APPLICATIVE_HPP
#define INCLUDE_SMD_TREE_FIX_TREE_APPLICATIVE_HPP

#include <smd/tree/fix_tree.hpp>
#include <smd/typeclass/applicative.hpp>

#include <beman/optional/optional.hpp>

#include <functional>
#include <type_traits>
#include <utility>

namespace smd::typeclass {

template <class VALUE_TYPE>
struct Applicative<smd::tree::FixTree<VALUE_TYPE> > {
    template <class VALUE>
    constexpr auto pure(VALUE&& value) const
        -> smd::tree::FixTree<std::remove_cvref_t<VALUE> >;

    template <class FUNCTION, class... TREES>
    constexpr auto invoke(FUNCTION&& function, const TREES&... trees) const;
};

template <class VALUE_TYPE>
template <class VALUE>
constexpr auto Applicative<smd::tree::FixTree<VALUE_TYPE> >::pure(VALUE&& value) const
    -> smd::tree::FixTree<std::remove_cvref_t<VALUE> >
{
    return smd::tree::FixTree<std::remove_cvref_t<VALUE> >::leaf(
        std::forward<VALUE>(value));
}

template <class VALUE_TYPE>
template <class FUNCTION, class... TREES>
constexpr auto Applicative<smd::tree::FixTree<VALUE_TYPE> >::invoke(
    FUNCTION&& function,
    const TREES&... trees) const
{
    using Result = std::invoke_result_t<FUNCTION, typename TREES::value_type...>;
    using ResultTree = smd::tree::FixTree<std::remove_cvref_t<Result> >;
    using OptionalResultTree = beman::optional::optional<ResultTree>;

    const bool allLeaves = (trees.is_leaf() && ...);
    const bool allBranches = (trees.is_branch() && ...);

    if (allLeaves) {
        return OptionalResultTree{ResultTree::leaf(
            std::invoke(std::forward<FUNCTION>(function), trees.value()...))};
    }

    if (!allBranches) {
        return OptionalResultTree{};
    }

    auto leftResult = this->invoke(function, trees.left()...);
    auto rightResult = this->invoke(std::forward<FUNCTION>(function), trees.right()...);

    if (!leftResult || !rightResult) {
        return OptionalResultTree{};
    }

    return OptionalResultTree{ResultTree::branch(std::move(*leftResult),
                                                std::move(*rightResult))};
}

}  // close namespace smd::typeclass

#endif  // INCLUDE_SMD_TREE_FIX_TREE_APPLICATIVE_HPP
