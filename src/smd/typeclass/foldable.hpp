#ifndef INCLUDE_SMD_TYPECLASS_FOLDABLE_HPP
#define INCLUDE_SMD_TYPECLASS_FOLDABLE_HPP

#include <smd/typeclass/monoid.hpp>
#include <smd/typeclass/typeclass_base.hpp>

#include <cstddef>
#include <utility>

namespace smd {

struct foldable_tag {};

template <class T, class F>
auto fold_map(F&& function, T&& value)
{
  using ValueType = remove_cvref_t<T>;
  return map<foldable_tag, ValueType>::fold_map(std::forward<F>(function),
                          std::forward<T>(value));
}

template <class T>
auto length(T&& value) -> std::size_t
{
  const auto count = fold_map(
    [](const auto&) { return typeclass::Count{1}; }, std::forward<T>(value));
  return count.d_value;
}

}  // close namespace smd

#endif
