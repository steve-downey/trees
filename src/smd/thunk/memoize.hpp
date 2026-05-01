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

// erased_thunk<Result> — type-erased wrapper for any callable returning Result.
// Copies share the same underlying callable via shared_ptr.
template <typename Result>
class erased_thunk {
  struct ThunkBase {
    virtual ~ThunkBase() = default;
    virtual auto invoke() -> const Result& = 0;
  };

  template <typename Callable>
  struct ThunkModel final : ThunkBase {
    Callable d_callable;

    explicit ThunkModel(Callable callable)
      : d_callable(std::move(callable))
    {
    }

    auto invoke() -> const Result& override { return d_callable(); }
  };

  std::shared_ptr<ThunkBase> d_impl;

 public:
  erased_thunk() = default;

  template <typename Callable,
            typename CallableT = std::remove_cvref_t<Callable>,
            std::enable_if_t<!std::is_same_v<CallableT, erased_thunk>, int> = 0>
  erased_thunk(Callable&& callable)
    : d_impl(std::make_shared<ThunkModel<CallableT>>(std::forward<Callable>(callable)))
  {
  }

  [[nodiscard]] auto operator()() const -> const Result&
  {
    assert(d_impl != nullptr);
    return d_impl->invoke();
  }
};

// memoize(callable, args...) — returns a callable that evaluates
// delay(callable, args...) exactly once and caches the result.  All copies
// of the returned callable share the same cached value via shared_ptr.
//
// Not thread-safe: concurrent first calls race on the variant mutation.
// See thread-safety note at the top of this file.
template <typename Callable, typename... Args>
auto memoize(Callable&& c, Args&&... args)
{
  using Closure = decltype(delay(std::forward<Callable>(c), std::forward<Args>(args)...));
  using Result  = std::invoke_result_t<Closure&>;
  using State   = std::variant<std::monostate, Closure, Result>;

  auto state = std::make_shared<State>(
    std::in_place_index<1>,
    delay(std::forward<Callable>(c), std::forward<Args>(args)...));

  return [state = std::move(state)]() mutable -> const Result& {
    if (state->index() == 1U) {
      state->template emplace<2>(std::get<1>(*state)());
    }
    return std::get<2>(*state);
  };
}

// measured_memoize(measure, callable, args...) — like memoize(), but attaches
// a pre-computed Measure alongside the deferred result.  Returns a struct with
// cached_measure() and force() accessors.
template <typename Measure, typename Callable, typename... Args>
auto measured_memoize(Measure measure, Callable&& c, Args&&... args)
{
  auto deferred = memoize(std::forward<Callable>(c), std::forward<Args>(args)...);

  return [measure = std::move(measure), deferred = std::move(deferred)]() mutable {
    struct MeasuredAccess {
      Measure                   d_measure;
      mutable decltype(deferred) d_force;

      [[nodiscard]] auto cached_measure() const -> const Measure& { return d_measure; }
      [[nodiscard]] auto force() const -> decltype(auto) { return d_force(); }
    };
    return MeasuredAccess{std::move(measure), std::move(deferred)};
  }();
}

}  // namespace smd::thunk

#endif
