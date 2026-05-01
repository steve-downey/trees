// src/smd/typeclass/traversable.hpp                                  -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TYPECLASS_TRAVERSABLE
#define INCLUDED_SMD_TYPECLASS_TRAVERSABLE

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

    // 8f1d5c4a-1a7e-4b9e-8cb4-908f4ab0ca11

    // d5a2c1f8-7e3b-4d1a-c6b2-2f9e5d7a1c46
    template <class T, class F>
    auto for_each(this auto&& self, T&& value, F&& function)
    {
        return self.traverse(std::forward<F>(function),
                             std::forward<T>(value));
    }
    // d5a2c1f8-7e3b-4d1a-c6b2-2f9e5d7a1c46 end

    // c1f8e7a2-9b6d-4c4f-a5e3-1b2d9c8f6a79
    template <class T>
    auto sequence(this auto&& self, T&& value)
    {
        return self.traverse(
            [](auto&& x) { return std::forward<decltype(x)>(x); },
            std::forward<T>(value));
    }
    // c1f8e7a2-9b6d-4c4f-a5e3-1b2d9c8f6a79 end

    template <class TRAVERSABLE_MAP, class T, class F>
    auto traverse_with(this auto&&,
                       const TRAVERSABLE_MAP& traversable_map,
                       F&& function,
                       T&& value)
    {
        return traversable_map.traverse(std::forward<F>(function),
                                        std::forward<T>(value));
    }

    template <class TRAVERSABLE_MAP, class T>
    auto sequence_with(this auto&& self,
                       const TRAVERSABLE_MAP& traversable_map,
                       T&& value)
    {
        return self.traverse_with(
            traversable_map,
            [](auto&& x) { return std::forward<decltype(x)>(x); },
            std::forward<T>(value));
    }
    // 8f1d5c4a-1a7e-4b9e-8cb4-908f4ab0ca11 end
};

template <class T>
inline constexpr auto traversable_typeclass = std::false_type{};

}  // close namespace smd

#endif
