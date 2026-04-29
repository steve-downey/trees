#include <smd/typeclass/examples/examples.hpp>

#include <smd/tree/fix_tree.hpp>
#include <smd/tree/fix_tree_foldable.hpp>
#include <smd/typeclass/foldable.hpp>

namespace smd::typeclass::examples {

using IntTree = smd::tree::FixTree<int>;

auto make_sample_tree() -> IntTree
{
    return IntTree::branch(IntTree::leaf(1),
                           IntTree::branch(IntTree::leaf(2), IntTree::leaf(3)));
}

auto generic_length_example() -> std::size_t
{
    auto tree = make_sample_tree();
    const auto& foldable = smd::foldable_typeclass<IntTree>;

    // 9a1c4e2b-2c7e-4b1a-9f55-8b6a4d2e91aa
    auto n = foldable.length(tree);
    // 9a1c4e2b-2c7e-4b1a-9f55-8b6a4d2e91aa end

    return n;
}

}  // close namespace smd::typeclass::examples
