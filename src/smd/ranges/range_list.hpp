// src/smd/ranges/range_list.hpp                                      -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_RANGES_RANGE_LIST
#define INCLUDED_SMD_RANGES_RANGE_LIST

#include <algorithm>
#include <iterator>
#include <ranges>
#include <type_traits>
#include <utility>
#include <vector>

namespace smd::ranges {

namespace detail {

template <std::ranges::input_range RANGE>
auto materialize(RANGE &&range) {
    using Value = std::ranges::range_value_t<RANGE>;
    std::vector<Value> values;

    if constexpr (std::ranges::sized_range<RANGE>) {
        values.reserve(std::ranges::size(range));
    }

    std::ranges::copy(range, std::back_inserter(values));

    return values;
}

} // namespace detail

template <class VIEW>
    requires(std::ranges::view<VIEW> && std::ranges::input_range<VIEW>)
class list_range : public std::ranges::view_interface<list_range<VIEW>> {
    VIEW d_view;

  public:
    using value_type = std::ranges::range_value_t<VIEW>;
    using view_type = VIEW;

    list_range()
        requires std::default_initializable<VIEW>
    = default;

    constexpr explicit list_range(VIEW view) : d_view(std::move(view)) {}

    constexpr auto begin() { return std::ranges::begin(d_view); }

    constexpr auto begin() const
        requires std::ranges::range<const VIEW>
    {
        return std::ranges::begin(d_view);
    }

    constexpr auto end() { return std::ranges::end(d_view); }

    constexpr auto end() const
        requires std::ranges::range<const VIEW>
    {
        return std::ranges::end(d_view);
    }

    constexpr auto base() const &
        requires std::copy_constructible<VIEW>
    {
        return d_view;
    }

    constexpr auto base() && { return std::move(d_view); }
};

template <std::ranges::viewable_range RANGE>
auto all(RANGE &&range) {
    using View = std::views::all_t<RANGE>;
    return list_range<View>{std::views::all(std::forward<RANGE>(range))};
}

template <class VALUE>
auto single(VALUE &&value) {
    using Stored = std::remove_cvref_t<VALUE>;
    return list_range<std::ranges::single_view<Stored>>{
        std::views::single(std::forward<VALUE>(value))};
}

template <class VALUE>
auto from_vector(std::vector<VALUE> values) {
    return all(std::move(values));
}

} // namespace smd::ranges

#endif
