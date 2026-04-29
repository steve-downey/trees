#ifndef INCLUDE_SMD_TYPECLASS_FOLDABLE_HPP
#define INCLUDE_SMD_TYPECLASS_FOLDABLE_HPP

#include <smd/typeclass/monoid.hpp>
#include <smd/typeclass/typeclass_base.hpp>

#include <cstddef>
#include <utility>

namespace smd {

// Foldable pattern invariants:
// - Generic entry point is fold_map(F, T) via foldable_typeclass<T>.
// - Instances provide an object with fold_map(F, T) and specialize
//   foldable_typeclass<Concrete>.
// - length is derived from fold_map using Count monoid and should stay generic.
// - Traversal order is instance-defined but must be coherent per instance.

template <class T>
inline constexpr auto foldable_typeclass = std::false_type{};

template <class FOLDABLE_MAP, class T, class F>
auto fold_map(const FOLDABLE_MAP& foldable_map, F&& function, T&& value)
{
    return foldable_map.fold_map(std::forward<F>(function),
                                 std::forward<T>(value));
}

template <class T, class F>
auto fold_map(F&& function, T&& value)
{
    using ValueType = remove_cvref_t<T>;
    return fold_map(foldable_typeclass<ValueType>,
                    std::forward<F>(function),
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
