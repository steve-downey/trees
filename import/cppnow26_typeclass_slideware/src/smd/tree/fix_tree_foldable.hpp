#ifndef INCLUDE_SMD_TREE_FIX_TREE_FOLDABLE_HPP
#define INCLUDE_SMD_TREE_FIX_TREE_FOLDABLE_HPP

#include <smd/tree/fix_tree.hpp>
#include <smd/typeclass/foldable.hpp>

#include <functional>
#include <type_traits>
#include <utility>

namespace smd::typeclass {

template <class VALUE_TYPE>
struct Foldable<smd::tree::FixTree<VALUE_TYPE> > {
    template <class FUNCTION>
    constexpr auto fold_map(FUNCTION&& function,
                            const smd::tree::FixTree<VALUE_TYPE>& tree) const;
};

template <class VALUE_TYPE>
template <class FUNCTION>
constexpr auto Foldable<smd::tree::FixTree<VALUE_TYPE> >::fold_map(
    FUNCTION&& function,
    const smd::tree::FixTree<VALUE_TYPE>& tree) const
{
    using Result = std::remove_cvref_t<decltype(std::invoke(function, tree.value()))>;

    if (tree.is_leaf()) {
        return std::invoke(std::forward<FUNCTION>(function), tree.value());
    }

    auto leftResult = this->fold_map(function, tree.left());
    auto rightResult = this->fold_map(std::forward<FUNCTION>(function), tree.right());
    return monoid_v<Result>.combine(std::move(leftResult), std::move(rightResult));
}

}  // close namespace smd::typeclass

#endif  // INCLUDE_SMD_TREE_FIX_TREE_FOLDABLE_HPP
