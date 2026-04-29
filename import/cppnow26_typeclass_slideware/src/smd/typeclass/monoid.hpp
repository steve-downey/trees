#ifndef INCLUDE_SMD_TYPECLASS_MONOID_HPP
#define INCLUDE_SMD_TYPECLASS_MONOID_HPP

#include <smd/typeclass/typeclass_base.hpp>

#include <cstddef>
#include <utility>
#include <vector>

namespace smd::typeclass {

template <class VALUE_TYPE>
struct Monoid;

template <class VALUE_TYPE>
inline constexpr Monoid<VALUE_TYPE> monoid_v = Monoid<VALUE_TYPE>{};

struct Count {
    std::size_t d_value;

    friend constexpr bool operator==(const Count& lhs, const Count& rhs) = default;
};

template <>
struct Monoid<Count> {
    constexpr auto identity() const -> Count;
    constexpr auto combine(const Count& lhs, const Count& rhs) const -> Count;
};

template <class VALUE_TYPE>
struct Monoid<std::vector<VALUE_TYPE> > {
    auto identity() const -> std::vector<VALUE_TYPE>;
    auto combine(std::vector<VALUE_TYPE> lhs,
                 const std::vector<VALUE_TYPE>& rhs) const
        -> std::vector<VALUE_TYPE>;
};

}  // close namespace smd::typeclass

namespace smd::typeclass {

constexpr auto Monoid<Count>::identity() const -> Count
{
    return Count{0};
}

constexpr auto Monoid<Count>::combine(const Count& lhs, const Count& rhs) const
    -> Count
{
    return Count{lhs.d_value + rhs.d_value};
}

template <class VALUE_TYPE>
auto Monoid<std::vector<VALUE_TYPE> >::identity() const -> std::vector<VALUE_TYPE>
{
    return {};
}

template <class VALUE_TYPE>
auto Monoid<std::vector<VALUE_TYPE> >::combine(
    std::vector<VALUE_TYPE> lhs,
    const std::vector<VALUE_TYPE>& rhs) const -> std::vector<VALUE_TYPE>
{
    lhs.insert(lhs.end(), rhs.begin(), rhs.end());
    return lhs;
}

}  // close namespace smd::typeclass

#endif  // INCLUDE_SMD_TYPECLASS_MONOID_HPP
