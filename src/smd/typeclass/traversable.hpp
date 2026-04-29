#ifndef INCLUDE_SMD_TYPECLASS_TRAVERSABLE_HPP
#define INCLUDE_SMD_TYPECLASS_TRAVERSABLE_HPP

#include <smd/typeclass/functor.hpp>
#include <smd/typeclass/typeclass_base.hpp>

#include <type_traits>
#include <utility>

namespace smd {

// Traversable pattern invariants:
// - Instances are single lookup objects that provide traverse(F, T).
// - sequence is a derived object operation implemented from traverse(identity).
// - Dispatch happens through a provided object or traversable_typeclass<Concrete>.
// - Traversal must preserve container shape while sequencing effects.

template <class Impl>
struct Traversable : protected Impl {
    using Impl::traverse;

    template <class T>
    auto sequence(this auto&& self, T&& value)
    {
        return self.traverse(
            [](auto&& x) { return std::forward<decltype(x)>(x); },
            std::forward<T>(value));
    }
};

template <class T>
inline constexpr auto traversable_typeclass = std::false_type{};

}  // close namespace smd

#endif
