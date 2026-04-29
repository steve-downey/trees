#ifndef INCLUDE_SMD_TYPECLASS_FOLDABLE_HPP
#define INCLUDE_SMD_TYPECLASS_FOLDABLE_HPP

#include <smd/typeclass/monoid.hpp>
#include <smd/typeclass/typeclass_base.hpp>

#include <cstddef>
#include <functional>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

namespace smd::detail {

template <class STATE>
struct LeftFoldProgram {
  std::function<STATE(STATE)> d_run;

  auto operator()(STATE state) const -> STATE
  {
    return d_run(std::move(state));
  }
};

template <class STATE>
struct RightFoldProgram {
  std::function<STATE(STATE)> d_run;

  auto operator()(STATE state) const -> STATE
  {
    return d_run(std::move(state));
  }
};

struct Any {
  bool d_value;
};

struct All {
  bool d_value;
};

template <class VALUE_TYPE>
struct First {
  std::optional<VALUE_TYPE> d_value;
};

}  // close namespace smd::detail

namespace smd::typeclass {

template <class STATE>
struct Monoid<smd::detail::LeftFoldProgram<STATE> > {
  auto identity() const -> smd::detail::LeftFoldProgram<STATE>
  {
    return smd::detail::LeftFoldProgram<STATE>{
      [](STATE s) { return s; }};
  }

  auto combine(const smd::detail::LeftFoldProgram<STATE>& lhs,
         const smd::detail::LeftFoldProgram<STATE>& rhs) const
    -> smd::detail::LeftFoldProgram<STATE>
  {
    return smd::detail::LeftFoldProgram<STATE>{
      [lhs, rhs](STATE s) { return rhs(lhs(std::move(s))); }};
  }
};

template <class STATE>
struct Monoid<smd::detail::RightFoldProgram<STATE> > {
  auto identity() const -> smd::detail::RightFoldProgram<STATE>
  {
    return smd::detail::RightFoldProgram<STATE>{
      [](STATE s) { return s; }};
  }

  auto combine(const smd::detail::RightFoldProgram<STATE>& lhs,
         const smd::detail::RightFoldProgram<STATE>& rhs) const
    -> smd::detail::RightFoldProgram<STATE>
  {
    return smd::detail::RightFoldProgram<STATE>{
      [lhs, rhs](STATE s) { return lhs(rhs(std::move(s))); }};
  }
};

template <>
struct Monoid<smd::detail::Any> {
  constexpr auto identity() const -> smd::detail::Any { return {false}; }

  constexpr auto combine(smd::detail::Any lhs, smd::detail::Any rhs) const
    -> smd::detail::Any
  {
    return {lhs.d_value || rhs.d_value};
  }
};

template <>
struct Monoid<smd::detail::All> {
  constexpr auto identity() const -> smd::detail::All { return {true}; }

  constexpr auto combine(smd::detail::All lhs, smd::detail::All rhs) const
    -> smd::detail::All
  {
    return {lhs.d_value && rhs.d_value};
  }
};

template <class VALUE_TYPE>
struct Monoid<smd::detail::First<VALUE_TYPE> > {
  auto identity() const -> smd::detail::First<VALUE_TYPE> { return {{}}; }

  auto combine(const smd::detail::First<VALUE_TYPE>& lhs,
         const smd::detail::First<VALUE_TYPE>& rhs) const
    -> smd::detail::First<VALUE_TYPE>
  {
    if (lhs.d_value) {
      return lhs;
    }
    return rhs;
  }
};

}  // close namespace smd::typeclass

namespace smd {

// Foldable pattern invariants:
// - Generic entry point is fold_map(F, T) via foldable_typeclass<T>.
// - Instances provide an object with fold_map(F, T) and specialize
//   foldable_typeclass<Concrete>.
// - Derived APIs (length, fold_left, fold_right, combine_all, any_of, all_of,
//   empty, to_vector, find_first) live on the same looked-up object.
// - Traversal order is instance-defined but must be coherent per instance.

template <class Impl>
struct Foldable : protected Impl {
  using Impl::fold_map;

  template <class T>
  auto length(this auto&& self, T&& value) -> std::size_t
  {
    const auto count = self.fold_map(
      [](const auto&) { return typeclass::Count{1}; },
      std::forward<T>(value));
    return count.d_value;
  }

  template <class T, class STATE, class F>
  auto fold_left(this auto&& self, T&& value, STATE initial_state, F&& function)
  {
    using StateType = remove_cvref_t<STATE>;
    auto step = std::forward<F>(function);

    const auto program = self.fold_map(
      [&step](const auto& x) {
        using ValueType = remove_cvref_t<decltype(x)>;
        return detail::LeftFoldProgram<StateType>{
          [x_copy = ValueType(x), &step](StateType s) {
            return std::invoke(step, std::move(s), x_copy);
          }};
      },
      std::forward<T>(value));

    return program(StateType(std::move(initial_state)));
  }

  template <class T, class STATE, class F>
  auto fold_right(this auto&& self, T&& value, STATE initial_state, F&& function)
  {
    using StateType = remove_cvref_t<STATE>;
    auto step = std::forward<F>(function);

    const auto program = self.fold_map(
      [&step](const auto& x) {
        using ValueType = remove_cvref_t<decltype(x)>;
        return detail::RightFoldProgram<StateType>{
          [x_copy = ValueType(x), &step](StateType s) {
            return std::invoke(step, x_copy, std::move(s));
          }};
      },
      std::forward<T>(value));

    return program(StateType(std::move(initial_state)));
  }

  template <class T>
  auto combine_all(this auto&& self, T&& value)
  {
    return self.fold_map([](const auto& x) { return x; },
               std::forward<T>(value));
  }

  template <class T, class PREDICATE>
  auto any_of(this auto&& self, T&& value, PREDICATE&& predicate) -> bool
  {
    const auto result = self.fold_map(
      [&predicate](const auto& x) {
        return detail::Any{std::invoke(predicate, x)};
      },
      std::forward<T>(value));

    return result.d_value;
  }

  template <class T, class PREDICATE>
  auto all_of(this auto&& self, T&& value, PREDICATE&& predicate) -> bool
  {
    const auto result = self.fold_map(
      [&predicate](const auto& x) {
        return detail::All{std::invoke(predicate, x)};
      },
      std::forward<T>(value));

    return result.d_value;
  }

  template <class T>
  auto empty(this auto&& self, T&& value) -> bool
  {
    return !self.any_of(std::forward<T>(value), [](const auto&) {
      return true;
    });
  }

  template <class T>
  auto to_vector(this auto&& self, T&& value)
  {
    return self.fold_map(
      [](const auto& x) {
        using ValueType = remove_cvref_t<decltype(x)>;
        return std::vector<ValueType>{x};
      },
      std::forward<T>(value));
  }

  template <class T, class PREDICATE>
  auto find_first(this auto&& self, T&& value, PREDICATE&& predicate)
  {
    const auto result = self.fold_map(
      [&predicate](const auto& x) {
        using X = remove_cvref_t<decltype(x)>;
        if (std::invoke(predicate, x)) {
          return detail::First<X>{{x}};
        }
        return detail::First<X>{{}};
      },
      std::forward<T>(value));

    return result.d_value;
  }
};

template <class T>
inline constexpr auto foldable_typeclass = std::false_type{};

}  // close namespace smd

#endif
