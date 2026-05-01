// src/smd/thunk/delay.hpp                                            -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_THUNK_DELAY
#define INCLUDED_SMD_THUNK_DELAY

#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>

namespace smd::thunk {

// delay(callable, args...) — capture a callable and its arguments into a
// closure that invokes them on demand.  The closure is not memoized; each
// call re-invokes callable.  Use smd::thunk::memoize() for call-once
// semantics.
template <typename Callable, typename... Args>
auto delay(Callable&& c, Args&&... args)
{
  using CallableT = std::remove_cvref_t<Callable>;
  using ArgsTuple = std::tuple<std::remove_cvref_t<Args>...>;

  return [callable = CallableT(std::forward<Callable>(c)),
          arguments = ArgsTuple(std::forward<Args>(args)...)]() mutable {
    return std::apply(
      [&](auto&... unpacked) { return std::invoke(callable, unpacked...); },
      arguments);
  };
}

}  // namespace smd::thunk

#endif
