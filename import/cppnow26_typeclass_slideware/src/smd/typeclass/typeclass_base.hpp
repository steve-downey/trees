#ifndef INCLUDE_SMD_TYPECLASS_TYPECLASS_BASE_HPP
#define INCLUDE_SMD_TYPECLASS_TYPECLASS_BASE_HPP

#include <type_traits>
#include <utility>

namespace smd::typeclass {

template <class TYPE>
using RemoveCvRef = std::remove_cvref_t<TYPE>;

template <class FIRST, class... REST>
struct FirstType {
    using Type = FIRST;
};

template <class FIRST, class... REST>
using FirstTypeT = typename FirstType<FIRST, REST...>::Type;

}  // close namespace smd::typeclass

#endif  // INCLUDE_SMD_TYPECLASS_TYPECLASS_BASE_HPP
