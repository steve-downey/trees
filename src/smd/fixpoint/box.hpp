// src/smd/fixpoint/box.hpp                                          -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_FIXPOINT_BOX
#define INCLUDED_SMD_FIXPOINT_BOX

#include <memory>

namespace smd::fixpoint {

template <typename A>
using Box = std::shared_ptr<A>;

template <typename A, typename... Args>
auto make_box(Args &&...args) -> Box<A> {
    return std::make_shared<A>(std::forward<Args>(args)...);
}

} // namespace smd::fixpoint

#endif
