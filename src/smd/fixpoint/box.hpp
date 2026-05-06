// src/smd/fixpoint/box.hpp                                          -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_FIXPOINT_BOX
#define INCLUDED_SMD_FIXPOINT_BOX

#include <memory>

namespace smd::fixpoint {

/** Indirection type used inside F<Fix<F>> to break infinite template
 * instantiation. Box<A> = shared_ptr<A>; structural sharing is a side-effect.
 * @tparam A the pointed-to type (typically a recursive Fix instantiation)
 */
template <typename A>
using Box = std::shared_ptr<A>;

/** Construct a Box<A> in-place, forwarding @p args to A's constructor. */
template <typename A, typename... Args>
auto make_box(Args &&...args) -> Box<A> {
    return std::make_shared<A>(std::forward<Args>(args)...);
}

} // namespace smd::fixpoint

#endif
