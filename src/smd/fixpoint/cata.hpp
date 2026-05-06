// src/smd/fixpoint/cata.hpp                                          -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_FIXPOINT_CATA
#define INCLUDED_SMD_FIXPOINT_CATA

#include <smd/fixpoint/fix.hpp>

namespace smd::fixpoint {

/**
 * @brief Catamorphism: bottom-up fold over a Fix<F> tree.
 *
 * Recursively descends into the tree, applying @p algebra at each level.
 * At each node the @p fmap_fn lifts the recursive fold into the functor layer
 * so that @p algebra receives an F<Result> rather than an F<Fix<F>>.
 *
 * @tparam Result   the type produced at each level by @p algebra
 * @tparam F        the non-recursive functor whose fixed-point is being folded
 * @param algebra   function F<Result> -> Result (the catamorphism algebra)
 * @param fmap_fn   function (Fix<F>->Result, F<Fix<F>>) -> F<Result>
 *                  — lifts the fold function over one layer of F
 * @param tree      the fixed-point tree to fold
 * @return          the folded result at the root
 */
template <typename Result, template <typename> class F, typename Algebra,
          typename FMap>
auto cata(const Algebra &algebra, const FMap &fmap_fn, const Fix<F> &tree)
    -> Result {
    const auto &layer = unwrap(tree);
    auto evaluated = fmap_fn(
        [&](const Fix<F> &child) -> Result {
            return cata<Result>(algebra, fmap_fn, child);
        },
        layer);
    return algebra(evaluated);
}

} // namespace smd::fixpoint

#endif
