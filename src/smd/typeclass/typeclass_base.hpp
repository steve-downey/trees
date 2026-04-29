#ifndef INCLUDE_SMD_TYPECLASS_TYPECLASS_BASE_HPP
#define INCLUDE_SMD_TYPECLASS_TYPECLASS_BASE_HPP

#include <type_traits>

namespace smd {

template <class Tag, class T>
struct map;

template <class T>
using remove_cvref_t = std::remove_cvref_t<T>;

}  // close namespace smd

#endif  // INCLUDE_SMD_TYPECLASS_TYPECLASS_BASE_HPP
