// src/smd/fixpoint/fix.hpp                                           -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_FIXPOINT_FIX
#define INCLUDED_SMD_FIXPOINT_FIX

#include <utility>

namespace smd::fixpoint {

template <template <typename> class F>
struct Fix {
    F<Fix<F>> inner;
};

template <template <typename> class F>
constexpr auto wrap(F<Fix<F>> layer) -> Fix<F>
{
    return Fix<F>{std::move(layer)};
}

template <template <typename> class F>
constexpr auto unwrap(const Fix<F>& fixed) -> const F<Fix<F>>&
{
    return fixed.inner;
}

}  // close namespace smd::fixpoint

#endif
