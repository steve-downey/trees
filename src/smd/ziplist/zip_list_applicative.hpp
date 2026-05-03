// src/smd/ziplist/zip_list_applicative.hpp                           -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_ZIPLIST_ZIP_LIST_APPLICATIVE
#define INCLUDED_SMD_ZIPLIST_ZIP_LIST_APPLICATIVE

#include <smd/typeclass/applicative.hpp>
#include <smd/ziplist/zip_list.hpp>

#include <algorithm>
#include <cstddef>
#include <functional>
#include <optional>
#include <type_traits>
#include <utility>

namespace smd {

namespace detail {

template <class T>
auto zip_list_finite_length(const zip_list<T> &list)
    -> std::optional<std::size_t> {
    if (list.is_repeating()) {
        return std::nullopt;
    }
    return list.finite_size();
}

template <class T>
auto zip_list_value_at(const zip_list<T> &list, std::size_t index)
    -> const T & {
    if (list.is_repeating()) {
        return *list.repeated;
    }
    return list.data[index];
}

template <class FIRST, class... REST>
auto zip_list_result_size(const FIRST &first, const REST &...rest)
    -> std::optional<std::size_t> {
    auto count = zip_list_finite_length(first);
    ((count = count
                  ? std::optional<std::size_t>{std::min(
                        *count, zip_list_finite_length(rest).value_or(*count))}
                  : zip_list_finite_length(rest)),
     ...);
    return count;
}

} // namespace detail

template <class T>
struct ZipListApplicativeImpl {
    template <class VALUE>
    auto pure(this auto &&, VALUE &&value) {
        using U = remove_cvref_t<VALUE>;
        return zip_list<U>::repeat(U(std::forward<VALUE>(value)));
    }

    template <class F, class A>
    auto apply(this auto &&, const zip_list<F> &functions,
               const zip_list<A> &arguments) {
        using Result = std::invoke_result_t<const F &, const A &>;
        using U = remove_cvref_t<Result>;

        const auto count = detail::zip_list_result_size(functions, arguments);
        if (!count.has_value()) {
            return zip_list<U>::repeat(
                std::invoke(detail::zip_list_value_at(functions, 0),
                            detail::zip_list_value_at(arguments, 0)));
        }

        zip_list<U> result;
        result.data.reserve(*count);

        for (std::size_t index = 0; index < *count; ++index) {
            result.data.push_back(
                std::invoke(detail::zip_list_value_at(functions, index),
                            detail::zip_list_value_at(arguments, index)));
        }

        return result;
    }

    template <class FUNCTION, class FIRST, class... REST>
    auto invoke(this auto &&, FUNCTION &&function, const FIRST &first,
                const REST &...rest) {
        using Result =
            std::invoke_result_t<FUNCTION, const typename FIRST::value_type &,
                                 const typename REST::value_type &...>;

        using U = remove_cvref_t<Result>;
        auto callable = std::forward<FUNCTION>(function);
        const auto count = detail::zip_list_result_size(first, rest...);

        if (!count.has_value()) {
            return zip_list<U>::repeat(
                std::invoke(callable, detail::zip_list_value_at(first, 0),
                            detail::zip_list_value_at(rest, 0)...));
        }

        zip_list<U> result;
        result.data.reserve(*count);

        for (std::size_t index = 0; index < *count; ++index) {
            result.data.push_back(
                std::invoke(callable, detail::zip_list_value_at(first, index),
                            detail::zip_list_value_at(rest, index)...));
        }

        return result;
    }
};

template <class T>
struct ZipListApplicativeMap : Applicative<ZipListApplicativeImpl<T>> {
    using ZipListApplicativeImpl<T>::apply;
    using ZipListApplicativeImpl<T>::pure;
};

template <class T>
inline constexpr auto applicative_typeclass<zip_list<T>> =
    ZipListApplicativeMap<T>{};

} // namespace smd

#endif
