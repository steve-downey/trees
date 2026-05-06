// src/smd/fixpoint/overloaded.hpp                                    -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_FIXPOINT_OVERLOADED
#define INCLUDED_SMD_FIXPOINT_OVERLOADED

namespace smd::fixpoint {

/** Aggregate that inherits operator() from each of @p Ts.
 * Used with std::visit to combine multiple lambdas into a single visitor
 * without writing a hand-rolled visitor struct.
 * Example: std::visit(overloaded{case1, case2, ...}, variant)
 */
template <typename... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};

/** Deduction guide so overloaded{...} works without explicit template args. */
template <typename... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

} // namespace smd::fixpoint

#endif
