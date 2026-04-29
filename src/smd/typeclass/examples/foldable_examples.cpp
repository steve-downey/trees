#include <smd/typeclass/examples/examples.hpp>

#include <smd/tree/fix_tree.hpp>
#include <smd/tree/fix_tree_foldable.hpp>
#include <smd/tree/fringe_tree.hpp>
#include <smd/tree/fringe_tree_foldable.hpp>
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

auto generic_length_fringe_tree_example() -> std::size_t
{
        using Fringe = smd::tree::FringeTree<int>;
        auto tree = Fringe::branch(
            Fringe::branch(Fringe::leaf(1), Fringe::leaf(2)),
            Fringe::leaf(3));

        const auto& foldable = smd::foldable_typeclass<Fringe>;

        // 7c2f11d9-ef09-45e2-80da-9229f3c8d82c
        auto n = foldable.length(tree);
        // 7c2f11d9-ef09-45e2-80da-9229f3c8d82c end

        return n;
}

auto foldable_flattens_shape_example() -> bool
{
        using Tree = smd::tree::FixTree<int>;
        auto left_heavy = Tree::branch(
            Tree::leaf(1),
            Tree::branch(Tree::leaf(2), Tree::leaf(3)));
        auto right_heavy = Tree::branch(
            Tree::branch(Tree::leaf(1), Tree::leaf(2)),
            Tree::leaf(3));

        const auto& foldable = smd::foldable_typeclass<Tree>;

        // b1fd4b92-b060-4c47-8c08-97328ec02329
        auto left_flat = foldable.to_vector(left_heavy);
        auto right_flat = foldable.to_vector(right_heavy);
        // b1fd4b92-b060-4c47-8c08-97328ec02329 end

        return left_flat == right_flat;
}

}  // close namespace smd::typeclass::examples
