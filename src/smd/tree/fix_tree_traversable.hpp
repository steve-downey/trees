#ifndef INCLUDE_SMD_TREE_FIX_TREE_TRAVERSABLE_HPP
#define INCLUDE_SMD_TREE_FIX_TREE_TRAVERSABLE_HPP

#include <smd/tree/fix_tree.hpp>
#include <smd/typeclass/traversable.hpp>

#include <functional>
#include <type_traits>
#include <utility>

namespace smd::typeclass {

template <class VALUE_TYPE>
struct Traversable<smd::tree::FixTree<VALUE_TYPE> > {
    template <class FUNCTION>
    constexpr auto traverse(FUNCTION&& function,
                            const smd::tree::FixTree<VALUE_TYPE>& tree) const;
};

template <class VALUE_TYPE>
template <class FUNCTION>
constexpr auto Traversable<smd::tree::FixTree<VALUE_TYPE> >::traverse(
    FUNCTION&& function,
    const smd::tree::FixTree<VALUE_TYPE>& tree) const
{
    if (tree.is_leaf()) {
        return smd::invoke(
            [](auto value) {
                using ResultValue = std::remove_cvref_t<decltype(value)>;
                return smd::tree::FixTree<ResultValue>::leaf(std::move(value));
            },
            std::invoke(std::forward<FUNCTION>(function), tree.value()));
    }

    auto leftResult = this->traverse(function, tree.left());
    auto rightResult = this->traverse(std::forward<FUNCTION>(function), tree.right());

    return smd::invoke(
        [](auto left, auto right) {
            using ResultTree = std::remove_cvref_t<decltype(left)>;
            return ResultTree::branch(std::move(left), std::move(right));
        },
        std::move(leftResult),
        std::move(rightResult));
}

}  // close namespace smd::typeclass

#endif  // INCLUDE_SMD_TREE_FIX_TREE_TRAVERSABLE_HPP
