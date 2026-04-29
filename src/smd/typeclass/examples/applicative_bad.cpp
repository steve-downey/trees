#include <smd/typeclass/examples/examples.hpp>

#include <smd/tree/fix_tree.hpp>

#include <cstddef>
#include <utility>

namespace smd::typeclass::examples {

template <class LEFT, class RIGHT>
auto cartesian_product(const smd::tree::FixTree<LEFT>&,
                       const smd::tree::FixTree<RIGHT>&)
    -> smd::tree::FixTree<std::pair<LEFT, RIGHT> >
{
    using PairTree = smd::tree::FixTree<std::pair<LEFT, RIGHT> >;
    return PairTree::leaf(std::pair<LEFT, RIGHT>{});
}

auto bad_applicative_example() -> std::size_t
{
    using IntTree = smd::tree::FixTree<int>;
    auto tx = IntTree::branch(IntTree::leaf(1), IntTree::leaf(2));
    auto ty = IntTree::leaf(3);

    // d2e7a1c9-0f3b-4b2e-9d55-1a8e7c4b2f90
    // Hypothetical: expands structure instead of preserving shape.
    auto bad = cartesian_product(tx, ty);
    // d2e7a1c9-0f3b-4b2e-9d55-1a8e7c4b2f90 end

    return bad.is_leaf() ? 1U : 0U;
}

}  // close namespace smd::typeclass::examples
