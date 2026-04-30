#ifndef INCLUDE_SMD_TREE_MEMOIZED_THUNK_HPP
#define INCLUDE_SMD_TREE_MEMOIZED_THUNK_HPP

#include <functional>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

namespace smd::tree::detail {

template <typename Callable, typename... Args>
auto delay(Callable&& c, Args&&... args)
{
  using CallableT = std::remove_cvref_t<Callable>;
  using ArgsTuple = std::tuple<std::remove_cvref_t<Args>...>;

  return [callable = CallableT(std::forward<Callable>(c)),
          arguments = ArgsTuple(std::forward<Args>(args)...)]() mutable {
    return std::apply(
      [&](auto&... unpacked) {
        return std::invoke(callable, unpacked...);
      },
      arguments);
  };
}

template <typename Callable, typename... Args>
auto thunk(Callable&& c, Args&&... args)
{
  using Closure = decltype(delay(std::forward<Callable>(c), std::forward<Args>(args)...));
  using Result = std::invoke_result_t<Closure&>;
  using State = std::variant<std::monostate, Closure, Result>;

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

template <typename Measure, typename Callable, typename... Args>
auto measured_thunk(Measure measure, Callable&& c, Args&&... args)
{
  auto delayed = thunk(std::forward<Callable>(c), std::forward<Args>(args)...);

  return [measure = std::move(measure), delayed = std::move(delayed)]() mutable {
    struct MeasuredThunkAccess {
      Measure d_measure;
      mutable decltype(delayed) d_force;

      [[nodiscard]] auto cached_measure() const -> const Measure&
      {
        return d_measure;
      }

      [[nodiscard]] auto force() const -> decltype(auto)
      {
        return d_force();
      }
    };

    return MeasuredThunkAccess{std::move(measure), std::move(delayed)};
  }();
}

}  // namespace smd::tree::detail

#endif
