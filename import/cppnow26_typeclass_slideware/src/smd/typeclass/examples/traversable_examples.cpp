#include <smd/typeclass/examples/examples.hpp>

#include <smd/tree/fix_tree.hpp>
#include <smd/tree/fix_tree_foldable.hpp>
#include <smd/tree/fix_tree_traversable.hpp>
#include <smd/typeclass/test/test_optional_applicative.hpp>
#include <smd/typeclass/traversable.hpp>

#include <beman/optional/optional.hpp>

namespace smd::typeclass::examples {

auto traversable_relabel_example() -> beman::optional::optional<std::size_t>
{
    using IntTree = smd::tree::FixTree<int>;
    auto tree = IntTree::branch(IntTree::leaf(1), IntTree::leaf(2));

    // 5c6b2d3e-7a44-4c8a-9c31-3d1e2a9b77c2
    using beman::optional::optional;

    auto relabelled = smd::traverse(
        [](int x) -> optional<int> {
            return x >= 0 ? optional<int>{x + 1} : optional<int>{};
        },
        tree);
    // 5c6b2d3e-7a44-4c8a-9c31-3d1e2a9b77c2 end

    if (!relabelled) {
        return {};
    }

    return smd::length(*relabelled);
}

}  // close namespace smd::typeclass::examples
