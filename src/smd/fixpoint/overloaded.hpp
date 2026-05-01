// src/smd/fixpoint/overloaded.hpp                                    -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_FIXPOINT_OVERLOADED
#define INCLUDED_SMD_FIXPOINT_OVERLOADED

namespace smd::fixpoint {

template <typename... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};

template <typename... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

}  // close namespace smd::fixpoint

#endif
