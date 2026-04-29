#include <smd/typeclass/examples/examples.hpp>

#include <smd/tree/fix_tree.hpp>
#include <smd/tree/fix_tree_foldable.hpp>
#include <smd/tree/binary_tree.hpp>
#include <smd/tree/binary_tree_foldable.hpp>
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

auto generic_length_binary_tree_example() -> std::size_t
{
    using IntBinaryTree = smd::tree::BinaryTree<int>;
    auto tree = IntBinaryTree::from_children_ptrs(
      2,
      IntBinaryTree::make_ptr(IntBinaryTree::leaf(1)),
      IntBinaryTree::make_ptr(IntBinaryTree::from_children_ptrs(
        3,
        {},
        IntBinaryTree::make_ptr(IntBinaryTree::leaf(4)))));

    const auto& foldable = smd::foldable_typeclass<IntBinaryTree>;

    // 53b9f5b4-3b3a-4e18-9b3c-07b7e2c980f4
    auto n = foldable.length(tree);
    // 53b9f5b4-3b3a-4e18-9b3c-07b7e2c980f4 end

    return n;
}

}  // close namespace smd::typeclass::examples
