// src/smd/ranges/range_applicative.hpp                               -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_RANGES_RANGE_APPLICATIVE
#define INCLUDED_SMD_RANGES_RANGE_APPLICATIVE

#include <smd/ranges/range_list.hpp>
#include <smd/typeclass/applicative.hpp>

#include <functional>
#include <ranges>
#include <type_traits>
#include <utility>
#include <vector>

namespace smd {

/** Applicative typeclass instance for list_range<VIEW> with Cartesian-product
 * (nondeterminism / list-monad) semantics.
 *
 * pure(x) = {x} (singleton range).
 * apply(fs, xs) = {f(x) | f <- fs, x <- xs} — every function applied to
 * every argument, producing all combinations in fs-major order.
 * @tparam VIEW underlying view type of the list_range
 */
template <class VIEW>
struct ListRangeApplicativeImpl {
    /** Lift a value into a single-element range. */
    template <class VALUE>
    auto pure(this auto &&, VALUE &&value) {
        using Stored = remove_cvref_t<VALUE>;
        return smd::ranges::from_vector(
            std::vector<Stored>{std::forward<VALUE>(value)});
    }

    /**
     * @brief Apply every function in @p functions to every argument in
     *        @p arguments (Cartesian product).
     * @return range of size |functions| * |arguments|, in fs-major order
     */
    template <class FUNCTION_VIEW, class ARGUMENT_VIEW>
    auto apply(this auto &&,
               const smd::ranges::list_range<FUNCTION_VIEW> &functions,
               const smd::ranges::list_range<ARGUMENT_VIEW> &arguments) {
        using Function =
            std::ranges::range_value_t<smd::ranges::list_range<FUNCTION_VIEW>>;
        using Argument =
            std::ranges::range_value_t<smd::ranges::list_range<ARGUMENT_VIEW>>;
        using Result = std::invoke_result_t<const Function &, const Argument &>;

        auto function_values = smd::ranges::detail::materialize(functions);
        auto argument_values = smd::ranges::detail::materialize(arguments);
        std::vector<remove_cvref_t<Result>> output;
        output.reserve(function_values.size() * argument_values.size());

        for (const auto &function : function_values) {
            for (const auto &argument : argument_values) {
                output.push_back(std::invoke(function, argument));
            }
        }

        return smd::ranges::from_vector(std::move(output));
    }
};

/** Applicative map exposing pure and apply for list_range<VIEW>. */
template <class VIEW>
struct ListRangeApplicativeMap : Applicative<ListRangeApplicativeImpl<VIEW>> {
    using ListRangeApplicativeImpl<VIEW>::apply;
    using ListRangeApplicativeImpl<VIEW>::pure;
};

/** Registers ListRangeApplicativeMap as the Applicative instance for
 * list_range<VIEW>. */
template <class VIEW>
inline constexpr auto applicative_typeclass<smd::ranges::list_range<VIEW>> =
    ListRangeApplicativeMap<VIEW>{};

} // namespace smd

#endif
