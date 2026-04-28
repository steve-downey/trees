#ifndef INCLUDE_SMD_TYPECLASS_TRAVERSABLE_HPP
#define INCLUDE_SMD_TYPECLASS_TRAVERSABLE_HPP

#include <smd/typeclass/applicative.hpp>
#include <smd/typeclass/foldable.hpp>
#include <smd/typeclass/typeclass_base.hpp>

#include <type_traits>
#include <utility>

namespace smd::typeclass {

template <class STRUCTURE>
struct Traversable;

template <class STRUCTURE>
inline constexpr Traversable<RemoveCvRef<STRUCTURE> > traversable_v =
    Traversable<RemoveCvRef<STRUCTURE> >{};

}  // close namespace smd::typeclass

namespace smd {

template <class FUNCTION, class STRUCTURE>
constexpr decltype(auto) traverse(FUNCTION&& function, STRUCTURE&& structure);

}  // close namespace smd

namespace smd {

template <class FUNCTION, class STRUCTURE>
constexpr decltype(auto) traverse(FUNCTION&& function, STRUCTURE&& structure)
{
    return typeclass::traversable_v<STRUCTURE>.traverse(
        std::forward<FUNCTION>(function),
        std::forward<STRUCTURE>(structure));
}

}  // close namespace smd

#endif  // INCLUDE_SMD_TYPECLASS_TRAVERSABLE_HPP
