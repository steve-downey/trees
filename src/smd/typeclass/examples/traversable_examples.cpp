#include <smd/typeclass/examples/examples.hpp>

#include <smd/tree/fix_tree.hpp>
#include <smd/tree/fix_tree_foldable.hpp>
#include <smd/tree/fix_tree_traversable.hpp>
#include <smd/typeclass/applicative.hpp>
#include <smd/typeclass/traversable.hpp>

#include <beman/optional/optional.hpp>

namespace smd::typeclass::examples {

auto traversable_relabel_example() -> beman::optional::optional<std::size_t>
{
    using IntTree = smd::tree::FixTree<int>;
    auto tree = IntTree::branch(IntTree::leaf(1), IntTree::leaf(2));
    const auto& traversable = smd::traversable_typeclass<IntTree>;

    // 5c6b2d3e-7a44-4c8a-9c31-3d1e2a9b77c2
    using beman::optional::optional;

    auto relabelled = traversable.traverse(
        [](int x) -> optional<int> {
            return x >= 0 ? optional<int>{x + 1} : optional<int>{};
        },
        tree);
    // 5c6b2d3e-7a44-4c8a-9c31-3d1e2a9b77c2 end

    if (!relabelled) {
        return {};
    }

    const auto& foldable = smd::foldable_typeclass<IntTree>;
    return foldable.length(*relabelled);
}

auto traversable_preserves_shape_example() -> bool
{
        using IntTree = smd::tree::FixTree<int>;
        using beman::optional::optional;

        auto tree = IntTree::branch(
            IntTree::leaf(1),
            IntTree::branch(IntTree::leaf(2), IntTree::leaf(3)));
        const auto& traversable = smd::traversable_typeclass<IntTree>;

        // d804ec63-77d1-4fa0-99a6-9effce6f741b
        auto mapped = traversable.traverse(
            [](int x) -> optional<int> { return optional<int>{x + 10}; },
            tree);
        // d804ec63-77d1-4fa0-99a6-9effce6f741b end

        if (!mapped || mapped->is_leaf()) {
                return false;
        }

        return mapped->left().is_leaf() &&
                     mapped->left().value() == 11 &&
                     !mapped->right().is_leaf() &&
                     mapped->right().left().is_leaf() &&
                     mapped->right().left().value() == 12 &&
                     mapped->right().right().is_leaf() &&
                     mapped->right().right().value() == 13;
}

}  // close namespace smd::typeclass::examples
