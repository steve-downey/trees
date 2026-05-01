// src/smd/ranges/range_traversable.hpp                               -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_RANGES_RANGE_TRAVERSABLE
#define INCLUDED_SMD_RANGES_RANGE_TRAVERSABLE

#include <smd/ranges/range_applicative.hpp>
#include <smd/ranges/range_list.hpp>
#include <smd/typeclass/traversable.hpp>

#include <functional>
#include <type_traits>
#include <utility>
#include <vector>

namespace smd {

template <class VIEW>
    requires std::ranges::forward_range<VIEW>
struct ListRangeTraversableImpl {
    using element_type = typename smd::ranges::list_range<VIEW>::value_type;

    template <class APPLICATIVE, class FUNCTION>
    auto traverse(this auto&&,
                  const APPLICATIVE& applicative,
                  FUNCTION&& function,
                  const smd::ranges::list_range<VIEW>& values)
    {
        using Value = element_type;
        using Context = remove_cvref_t<std::invoke_result_t<FUNCTION, const Value&> >;
        using ResultValue = smd::applicative_value_t<Context>;

        auto current = std::ranges::begin(values);
        const auto last = std::ranges::end(values);

        if (current == last) {
            return applicative.map(
                [](auto&& materialized) {
                    return smd::ranges::from_vector(
                        std::forward<decltype(materialized)>(materialized));
                },
                applicative.pure(std::vector<ResultValue>{}));
        }

        auto collected = applicative.map(
            [](auto&& first_value) {
                using U = remove_cvref_t<decltype(first_value)>;
                return std::vector<U>{std::forward<decltype(first_value)>(first_value)};
            },
            std::invoke(function, *current));
        ++current;

        for (; current != last; ++current) {
            auto lifted_value = std::invoke(function, *current);
            collected = applicative.invoke(
                [](std::vector<ResultValue> acc, auto&& next_value) {
                    acc.push_back(std::forward<decltype(next_value)>(next_value));
                    return acc;
                },
                std::move(collected),
                std::move(lifted_value));
        }

        return applicative.map(
            [](auto&& materialized) {
                return smd::ranges::from_vector(
                    std::forward<decltype(materialized)>(materialized));
            },
            std::move(collected));
    }
};

template <class VIEW>
    requires std::ranges::forward_range<VIEW>
struct ListRangeTraversableMap : Traversable<ListRangeTraversableImpl<VIEW> > {
    using ListRangeTraversableImpl<VIEW>::traverse;
};

template <class VIEW>
    requires std::ranges::forward_range<VIEW>
inline constexpr auto traversable_typeclass<smd::ranges::list_range<VIEW> > =
    ListRangeTraversableMap<VIEW>{};

}  // close namespace smd

#endif
