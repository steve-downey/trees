#include <smd/typeclass/examples/examples.hpp>

#include <smd/tree/binary_tree.hpp>
#include <smd/tree/binary_tree_foldable.hpp>
#include <smd/tree/fixpoint_tree.hpp>
#include <smd/tree/fixpoint_tree_foldable.hpp>
#include <smd/tree/fringe_tree.hpp>
#include <smd/tree/fringe_tree_foldable.hpp>
#include <smd/typeclass/foldable.hpp>

namespace smd::typeclass::examples {

using smd::tree::add_expr;
using smd::tree::const_expr;
using smd::tree::Expr;
using smd::tree::mul_expr;

auto generic_length_example() -> std::size_t {
    auto tree =
        add_expr(const_expr(1.0), add_expr(const_expr(2.0), const_expr(3.0)));
    const auto &foldable = smd::foldable_typeclass<Expr>;

    // 9a1c4e2b-2c7e-4b1a-9f55-8b6a4d2e91aa
    auto n = foldable.length(tree);
    // 9a1c4e2b-2c7e-4b1a-9f55-8b6a4d2e91aa end

    return n;
}

auto generic_length_binary_tree_example() -> std::size_t {
    using IntBinaryTree = smd::tree::BinaryTree<int>;
    auto tree = IntBinaryTree::from_children_ptrs(
        2, IntBinaryTree::make_ptr(IntBinaryTree::leaf(1)),
        IntBinaryTree::make_ptr(IntBinaryTree::from_children_ptrs(
            3, {}, IntBinaryTree::make_ptr(IntBinaryTree::leaf(4)))));

    const auto &foldable = smd::foldable_typeclass<IntBinaryTree>;

    // 53b9f5b4-3b3a-4e18-9b3c-07b7e2c980f4
    auto n = foldable.length(tree);
    // 53b9f5b4-3b3a-4e18-9b3c-07b7e2c980f4 end

    return n;
}

auto generic_length_fringe_tree_example() -> std::size_t {
    using Fringe = smd::tree::FringeTree<int>;
    auto tree = Fringe::branch(Fringe::branch(Fringe::leaf(1), Fringe::leaf(2)),
                               Fringe::leaf(3));

    const auto &foldable = smd::foldable_typeclass<Fringe>;

    // 7c2f11d9-ef09-45e2-80da-9229f3c8d82c
    auto n = foldable.length(tree);
    // 7c2f11d9-ef09-45e2-80da-9229f3c8d82c end

    return n;
}

auto foldable_flattens_shape_example() -> bool {
    // Two differently-structured expressions with the same leaf constants.
    auto right_deep =
        add_expr(const_expr(1.0), add_expr(const_expr(2.0), const_expr(3.0)));
    auto left_deep =
        add_expr(add_expr(const_expr(1.0), const_expr(2.0)), const_expr(3.0));

    const auto &foldable = smd::foldable_typeclass<Expr>;

    // b1fd4b92-b060-4c47-8c08-97328ec02329
    auto right_flat = foldable.to_vector(right_deep);
    auto left_flat = foldable.to_vector(left_deep);
    // b1fd4b92-b060-4c47-8c08-97328ec02329 end

    return right_flat == left_flat;
}

} // namespace smd::typeclass::examples
