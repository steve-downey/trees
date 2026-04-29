#ifndef INCLUDE_SMD_ZIPLIST_ZIP_LIST_APPLICATIVE_HPP
#define INCLUDE_SMD_ZIPLIST_ZIP_LIST_APPLICATIVE_HPP

#include <smd/typeclass/applicative.hpp>
#include <smd/ziplist/zip_list.hpp>

#include <algorithm>
#include <cstddef>
#include <functional>
#include <type_traits>
#include <utility>

namespace smd {

template <class T>
struct ZipListApplicativeImpl {
  template <class VALUE>
  auto pure(this auto&&, VALUE&& value)
  {
    using U = remove_cvref_t<VALUE>;
    return zip_list<U>{{std::forward<VALUE>(value)}};
  }

  template <class F, class A>
  auto apply(this auto&&, const zip_list<F>& functions, const zip_list<A>& arguments)
  {
    using Result = std::invoke_result_t<const F&, const A&>;
    zip_list<remove_cvref_t<Result> > result;

    const auto count = std::min(functions.data.size(), arguments.data.size());
    result.data.reserve(count);

    for (std::size_t index = 0; index < count; ++index) {
      result.data.push_back(
        std::invoke(functions.data[index], arguments.data[index]));
    }

    return result;
  }

  template <class FUNCTION, class FIRST, class... REST>
  auto invoke(this auto&&,
              FUNCTION&& function,
              const FIRST& first,
              const REST&... rest)
  {
    using Result = std::invoke_result_t<
      FUNCTION,
      const typename FIRST::value_type&,
      const typename REST::value_type&...>;

    zip_list<remove_cvref_t<Result> > result;
    auto count = first.data.size();
    ((count = std::min(count, rest.data.size())), ...);
    result.data.reserve(count);

    for (std::size_t index = 0; index < count; ++index) {
      result.data.push_back(
        std::invoke(std::forward<FUNCTION>(function),
                    first.data[index],
                    rest.data[index]...));
    }

    return result;
  }
};

template <class T>
struct ZipListApplicativeMap : Applicative<ZipListApplicativeImpl<T> > {
  using ZipListApplicativeImpl<T>::apply;
  using ZipListApplicativeImpl<T>::pure;
};

template <class T>
inline constexpr auto applicative_typeclass<zip_list<T> > =
  ZipListApplicativeMap<T>{};

}  // close namespace smd

#endif
