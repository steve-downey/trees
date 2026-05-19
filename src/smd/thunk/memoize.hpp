// src/smd/thunk/memoize.hpp                                          -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_THUNK_MEMOIZE
#define INCLUDED_SMD_THUNK_MEMOIZE

// Thread-safety note (reviewer-three.org, 2026-05-01):
// memoize() uses a mutable std::variant<> behind a std::shared_ptr.  Copies
// of the returned callable share the same State object, so concurrent first
// calls from multiple threads constitute a data race.  While std::shared_ptr's
// reference counting is thread-safe, mutating the variant is not.  If
// thread-safe memoization is required, replace the mutable-variant approach
// with std::call_once + std::once_flag inside SharedState.

#include <smd/thunk/delay.hpp>

#include <cassert>
#include <functional>
#include <memory>
#include <type_traits>
#include <utility>
#include <variant>

namespace smd::thunk {

/** Type-erased, copyable wrapper for any callable returning @p Result.
 * All copies of an erased_thunk share the same underlying callable via
 * shared_ptr, so invoking any copy calls the same object.
 * Virtual dispatch is used so that the concrete callable type need not be
 * known at the point of storage (e.g. in a heterogeneous container).
 */
template <typename Result>
class erased_thunk {
    struct ThunkBase {
        virtual ~ThunkBase() = default;
        virtual auto invoke() -> const Result & = 0;
    };

    template <typename Callable>
    struct ThunkModel final : ThunkBase {
        Callable d_callable;

        explicit ThunkModel(Callable callable)
            : d_callable(std::move(callable)) {}

        auto invoke() -> const Result & override { return d_callable(); }
    };

    std::shared_ptr<ThunkBase> d_impl;

  public:
    erased_thunk() = default;

    /** Construct from any compatible callable (excluded: self-type). */
    template <
        typename Callable, typename CallableT = std::remove_cvref_t<Callable>,
        std::enable_if_t<!std::is_same_v<CallableT, erased_thunk>, int> = 0>
    erased_thunk(Callable &&callable)
        : d_impl(std::make_shared<ThunkModel<CallableT>>(
              std::forward<Callable>(callable))) {}

    /** Invoke the wrapped callable and return a reference to its result. */
    [[nodiscard]] auto operator()() const -> const Result & {
        assert(d_impl != nullptr);
        return d_impl->invoke();
    }
};

/**
 * @brief Return a callable that evaluates delay(callable, args...) exactly
 *        once and caches the result; all copies share the cached value.
 *
 * Not thread-safe: concurrent first calls race on the internal variant
 * mutation. See the thread-safety note at the top of this file.
 *
 * @param c     callable to defer and memoize
 * @param args  arguments forwarded into the underlying delay closure
 * @return nullary callable returning const Result& on every call after the
 *         first evaluation
 */
template <typename Callable, typename... Args>
auto memoize(Callable &&c, Args &&...args) {
    using Closure =
        decltype(delay(std::forward<Callable>(c), std::forward<Args>(args)...));
    using Result = std::invoke_result_t<Closure &>;
    using State = std::variant<std::monostate, Closure, Result>;

    auto state = std::make_shared<State>(
        std::in_place_index<1>,
        delay(std::forward<Callable>(c), std::forward<Args>(args)...));

    return [state = std::move(state)]() mutable -> const Result & {
        if (state->index() == 1U) {
            state->template emplace<2>(std::get<1>(*state)());
        }
        return std::get<2>(*state);
    };
}

/**
 * @brief Like memoize(), but attaches a pre-computed @p measure alongside the
 *        deferred result.
 *
 * Returns a struct with cached_measure() (returns the measure without forcing
 * the thunk) and force() (evaluates or returns the cached result).
 *
 * @param measure  pre-computed annotation value stored eagerly
 * @param c        callable to memoize
 * @param args     arguments forwarded to the underlying memoize closure
 */
template <typename Measure, typename Callable, typename... Args>
auto measured_memoize(Measure measure, Callable &&c, Args &&...args) {
    auto deferred =
        memoize(std::forward<Callable>(c), std::forward<Args>(args)...);

    return [measure = std::move(measure),
            deferred = std::move(deferred)]() mutable {
        struct MeasuredAccess {
            Measure d_measure;
            mutable decltype(deferred) d_force;

            /** Return the pre-computed measure without evaluating the thunk. */
            [[nodiscard]] auto cached_measure() const -> const Measure & {
                return d_measure;
            }
            /** Force evaluation (or return cached value) of the deferred
             * callable. */
            [[nodiscard]] auto force() const -> decltype(auto) {
                return d_force();
            }
        };
        return MeasuredAccess{std::move(measure), std::move(deferred)};
    }();
}

} // namespace smd::thunk

#endif
