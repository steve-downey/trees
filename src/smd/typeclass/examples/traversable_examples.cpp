#include <smd/typeclass/examples/examples.hpp>

#include <smd/tree/fixpoint_tree.hpp>
#include <smd/tree/fixpoint_tree_foldable.hpp>
#include <smd/tree/fixpoint_tree_traversable.hpp>
#include <smd/tree/fringe_tree.hpp>
#include <smd/tree/fringe_tree_foldable.hpp>
#include <smd/tree/fringe_tree_traversable.hpp>
#include <smd/typeclass/applicative.hpp>
#include <smd/typeclass/traversable.hpp>

#include <beman/optional/optional.hpp>

namespace smd::typeclass::examples {

using smd::tree::add_expr;
using smd::tree::const_expr;
using smd::tree::eval;
using smd::tree::Expr;

auto traversable_relabel_example() -> beman::optional::optional<std::size_t> {
    using Fringe = smd::tree::FringeTree<int>;
    auto tree = Fringe::branch(Fringe::leaf(1), Fringe::leaf(2));

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

    const auto &foldable = smd::foldable_typeclass<Fringe>;
    return foldable.length(*relabelled);
}

auto traversable_preserves_shape_example() -> bool {
    // (1 + (2 + 3)) — traverse maps each constant, rebuilding the same Expr
    // shape.
    auto tree =
        add_expr(const_expr(1.0), add_expr(const_expr(2.0), const_expr(3.0)));

    using beman::optional::optional;

    // d804ec63-77d1-4fa0-99a6-9effce6f741b
    auto mapped = smd::traverse(
        [](double x) -> optional<double> { return optional<double>{x + 10.0}; },
        tree);
    // d804ec63-77d1-4fa0-99a6-9effce6f741b end

    if (!mapped) {
        return false;
    }

    // Shape is preserved: (11 + (12 + 13)) = 36
    return eval(*mapped) == 36.0;
}

} // namespace smd::typeclass::examples
