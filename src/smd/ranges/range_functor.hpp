// src/smd/ranges/range_functor.hpp                                   -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_RANGES_RANGE_FUNCTOR
#define INCLUDED_SMD_RANGES_RANGE_FUNCTOR

#include <smd/ranges/range_list.hpp>
#include <smd/typeclass/functor.hpp>

#include <ranges>
#include <utility>

namespace smd {

template <class VIEW>
struct ListRangeFunctorImpl {
    template <class FUNCTION>
    auto fmap(this auto &&, FUNCTION &&function,
              const smd::ranges::list_range<VIEW> &values) {
        return smd::ranges::all(
            values | std::views::transform(std::forward<FUNCTION>(function)));
    }
};

template <class VIEW>
struct ListRangeFunctorMap : Functor<ListRangeFunctorImpl<VIEW>> {
    using ListRangeFunctorImpl<VIEW>::fmap;
};

template <class VIEW>
inline constexpr auto functor_typeclass<smd::ranges::list_range<VIEW>> =
    ListRangeFunctorMap<VIEW>{};

} // namespace smd

#endif
