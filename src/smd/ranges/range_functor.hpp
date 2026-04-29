#ifndef INCLUDE_SMD_RANGES_RANGE_FUNCTOR_HPP
#define INCLUDE_SMD_RANGES_RANGE_FUNCTOR_HPP

#include <smd/ranges/range_list.hpp>
#include <smd/typeclass/functor.hpp>

#include <ranges>
#include <utility>

namespace smd {

template <class VIEW>
struct ListRangeFunctorImpl {
    template <class FUNCTION>
    auto fmap(this auto&&,
              FUNCTION&& function,
              const smd::ranges::list_range<VIEW>& values)
    {
        return smd::ranges::all(
            values | std::views::transform(std::forward<FUNCTION>(function)));
    }
};

template <class VIEW>
struct ListRangeFunctorMap : Functor<ListRangeFunctorImpl<VIEW> > {
    using ListRangeFunctorImpl<VIEW>::fmap;
};

template <class VIEW>
inline constexpr auto functor_typeclass<smd::ranges::list_range<VIEW> > =
    ListRangeFunctorMap<VIEW>{};

}  // close namespace smd

#endif
