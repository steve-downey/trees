#ifndef INCLUDE_SMD_TYPECLASS_FOLDABLE_HPP
#define INCLUDE_SMD_TYPECLASS_FOLDABLE_HPP

#include <smd/typeclass/monoid.hpp>
#include <smd/typeclass/typeclass_base.hpp>

#include <cstddef>
#include <functional>
#include <type_traits>
#include <utility>
#include <vector>

namespace smd::typeclass {

template <class STRUCTURE>
struct Foldable;

template <class STRUCTURE>
inline constexpr Foldable<RemoveCvRef<STRUCTURE> > foldable_v =
    Foldable<RemoveCvRef<STRUCTURE> >{};

}  // close namespace smd::typeclass

namespace smd {

template <class FUNCTION, class STRUCTURE>
constexpr decltype(auto) fold_map(FUNCTION&& function, STRUCTURE&& structure);

template <class STRUCTURE>
constexpr auto length(STRUCTURE&& structure) -> std::size_t;

template <class STRUCTURE>
constexpr auto to_vector(STRUCTURE&& structure)
    -> std::vector<typename std::remove_cvref_t<STRUCTURE>::value_type>;

}  // close namespace smd

namespace smd {

template <class FUNCTION, class STRUCTURE>
constexpr decltype(auto) fold_map(FUNCTION&& function, STRUCTURE&& structure)
{
    return typeclass::foldable_v<STRUCTURE>.fold_map(
        std::forward<FUNCTION>(function), std::forward<STRUCTURE>(structure));
}

template <class STRUCTURE>
constexpr auto length(STRUCTURE&& structure) -> std::size_t
{
    const auto count = smd::fold_map(
        [](const auto&) { return typeclass::Count{1}; },
        std::forward<STRUCTURE>(structure));
    return count.d_value;
}

template <class STRUCTURE>
constexpr auto to_vector(STRUCTURE&& structure)
    -> std::vector<typename std::remove_cvref_t<STRUCTURE>::value_type>
{
    using Value = typename std::remove_cvref_t<STRUCTURE>::value_type;
    return smd::fold_map(
        [](const Value& value) { return std::vector<Value>{value}; },
        std::forward<STRUCTURE>(structure));
}

}  // close namespace smd

#endif  // INCLUDE_SMD_TYPECLASS_FOLDABLE_HPP
