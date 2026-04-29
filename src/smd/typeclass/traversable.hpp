#ifndef INCLUDE_SMD_TYPECLASS_TRAVERSABLE_HPP
#define INCLUDE_SMD_TYPECLASS_TRAVERSABLE_HPP

#include <smd/typeclass/functor.hpp>
#include <smd/typeclass/typeclass_base.hpp>

#include <type_traits>
#include <utility>

namespace smd {

// Traversable pattern invariants:
// - Generic entry point is traverse(F, T) via traversable_typeclass<T>.
// - Instances provide an object with traverse(F, T) and specialize
//   traversable_typeclass<Concrete>.
// - sequence is derived from traverse(identity) and should stay generic.
// - Traversal must preserve container shape while sequencing effects.

template <class T>
inline constexpr auto traversable_typeclass = std::false_type{};

template <class TRAVERSABLE_MAP, class T, class F>
auto traverse(const TRAVERSABLE_MAP& traversable_map, F&& function, T&& value)
{
    return traversable_map.traverse(std::forward<F>(function),
                                    std::forward<T>(value));
}

template <class T, class F>
auto traverse(F&& function, T&& value)
{
    using ValueType = remove_cvref_t<T>;
    return traverse(traversable_typeclass<ValueType>,
                    std::forward<F>(function),
                    std::forward<T>(value));
}

template <class T>
auto sequence(T&& value)
{
    return traverse([](auto&& x) { return std::forward<decltype(x)>(x); },
                    std::forward<T>(value));
}

}  // close namespace smd

#endif
