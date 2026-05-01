// src/smd/tree/memoized_thunk.hpp                                    -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_MEMOIZED_THUNK
#define INCLUDED_SMD_TREE_MEMOIZED_THUNK

// Forwarding header — canonical implementations now live in smd/thunk/.
// Use <smd/thunk/delay.hpp> and <smd/thunk/memoize.hpp> directly in new code.

#include <smd/thunk/delay.hpp>
#include <smd/thunk/memoize.hpp>

namespace smd::tree::detail {

// Aliases into the canonical smd::thunk namespace, preserving the old names
// used by finger_tree.t.cpp and any other existing callers.
using smd::thunk::erased_thunk;

template <typename Callable, typename... Args>
auto delay(Callable&& c, Args&&... args)
{
  return smd::thunk::delay(std::forward<Callable>(c), std::forward<Args>(args)...);
}

template <typename Callable, typename... Args>
auto thunk(Callable&& c, Args&&... args)
{
  return smd::thunk::memoize(std::forward<Callable>(c), std::forward<Args>(args)...);
}

template <typename Measure, typename Callable, typename... Args>
auto measured_thunk(Measure measure, Callable&& c, Args&&... args)
{
  return smd::thunk::measured_memoize(
    std::move(measure),
    std::forward<Callable>(c),
    std::forward<Args>(args)...);
}

}  // namespace smd::tree::detail

#endif
