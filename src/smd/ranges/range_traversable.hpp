#ifndef INCLUDE_SMD_RANGES_RANGE_TRAVERSABLE_HPP
#define INCLUDE_SMD_RANGES_RANGE_TRAVERSABLE_HPP

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
    template <class FUNCTION>
    auto traverse(this auto&&,
                  FUNCTION&& function,
                  const smd::ranges::list_range<VIEW>& values)
    {
        using Value = typename smd::ranges::list_range<VIEW>::value_type;
        using Context = remove_cvref_t<std::invoke_result_t<FUNCTION, const Value&> >;
        using ResultValue = smd::applicative_value_t<Context>;
        const auto& applicative = smd::applicative_typeclass<Context>;

        auto collected = applicative.pure(std::vector<ResultValue>{});

        for (const auto& value : values) {
            auto lifted_value = std::invoke(function, value);
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
