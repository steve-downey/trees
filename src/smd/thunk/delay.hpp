// src/smd/thunk/delay.hpp                                            -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_THUNK_DELAY
#define INCLUDED_SMD_THUNK_DELAY

#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>

namespace smd::thunk {

/**
 * @brief Capture a callable and its arguments into a re-evaluating closure.
 *
 * The returned nullary callable stores @p c and @p args... by value; each
 * invocation re-evaluates callable(args...) from scratch. For call-once
 * (memoized) semantics use smd::thunk::memoize() instead.
 *
 * @param c       callable to defer
 * @param args    arguments forwarded into the closure by value
 * @return nullary callable that invokes c(args...) on every call
 */
template <typename Callable, typename... Args>
auto delay(Callable &&c, Args &&...args) {
    using CallableT = std::remove_cvref_t<Callable>;
    using ArgsTuple = std::tuple<std::remove_cvref_t<Args>...>;

    return [callable = CallableT(std::forward<Callable>(c)),
            arguments = ArgsTuple(std::forward<Args>(args)...)]() mutable {
        return std::apply(
            [&](auto &...unpacked) {
                return std::invoke(callable, unpacked...);
            },
            arguments);
    };
}

} // namespace smd::thunk

#endif
