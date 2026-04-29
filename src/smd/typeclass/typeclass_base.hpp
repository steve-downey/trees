#ifndef INCLUDE_SMD_TYPECLASS_TYPECLASS_BASE_HPP
#define INCLUDE_SMD_TYPECLASS_TYPECLASS_BASE_HPP

#include <type_traits>

namespace smd {

// Design invariants for the typeclass object pattern:
// - map<Tag, T> is the only customization lookup point for typeclass dispatch.
// - Generic algorithms call through map<Tag, remove_cvref_t<T>>.
// - New concepts should introduce a tag and keep lookup static and explicit.
// - Avoid adding parallel ADL-only customization paths for the same concept.

template <class Tag, class T>
struct map;

template <class T>
using remove_cvref_t = std::remove_cvref_t<T>;

}  // close namespace smd

#endif  // INCLUDE_SMD_TYPECLASS_TYPECLASS_BASE_HPP
