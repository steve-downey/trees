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

/** Range adaptor that wraps any view and satisfies the Foldable, Applicative,
 * and Traversable interfaces. list_range is the canonical container type for
 * the range-based typeclass instances in smd::ranges.
 * @tparam VIEW underlying view type; must satisfy std::ranges::view and
 *              std::ranges::input_range
 */
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

    /** Construct a list_range wrapping @p view. */
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

    /** Return a copy of the underlying view (lvalue overload). */
    constexpr auto base() const &
        requires std::copy_constructible<VIEW>
    {
        return d_view;
    }

    /** Return the underlying view by move (rvalue overload). */
    constexpr auto base() && { return std::move(d_view); }
};

/** Wrap any viewable range in a list_range using std::views::all. */
template <std::ranges::viewable_range RANGE>
auto all(RANGE &&range) {
    using View = std::views::all_t<RANGE>;
    return list_range<View>{std::views::all(std::forward<RANGE>(range))};
}

/** Create a single-element list_range holding @p value. */
template <class VALUE>
auto single(VALUE &&value) {
    using Stored = std::remove_cvref_t<VALUE>;
    return list_range<std::ranges::single_view<Stored>>{
        std::views::single(std::forward<VALUE>(value))};
}

/** Create a list_range from an owned vector. */
template <class VALUE>
auto from_vector(std::vector<VALUE> values) {
    return all(std::move(values));
}

} // namespace smd::ranges

#endif
