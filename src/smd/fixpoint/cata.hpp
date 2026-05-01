// src/smd/fixpoint/cata.hpp                                          -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_FIXPOINT_CATA
#define INCLUDED_SMD_FIXPOINT_CATA

#include <smd/fixpoint/fix.hpp>

namespace smd::fixpoint {

template <typename Result,
          template <typename> class F,
          typename Algebra,
          typename FMap>
auto cata(const Algebra& algebra, const FMap& fmap_fn, const Fix<F>& tree)
    -> Result
{
    const auto& layer = unwrap(tree);
    auto evaluated = fmap_fn(
        [&](const Fix<F>& child) -> Result {
            return cata<Result>(algebra, fmap_fn, child);
        },
        layer);
    return algebra(evaluated);
}

}  // close namespace smd::fixpoint

#endif
