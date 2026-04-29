
#ifndef INCLUDE_SMD_TYPECLASS_ZIP_LIST_APPLICATIVE_HPP
#define INCLUDE_SMD_TYPECLASS_ZIP_LIST_APPLICATIVE_HPP

#include <smd/typeclass/typeclass_base.hpp>
#include <smd/typeclass/applicative.hpp>
#include <smd/typeclass/zip_list.hpp>

#include <algorithm>
#include <functional>

namespace smd {

template<class T>
struct ZipListApplicativeImpl {

  auto pure(this auto&&, const T& x) {
    return zip_list<T>{{x}};
  }

  template<class F,class A>
  auto apply(this auto&&, const zip_list<F>& fs, const zip_list<A>& xs) {
    using R = decltype(fs.data[0](xs.data[0]));
    zip_list<R> r;

    auto n = std::min(fs.data.size(), xs.data.size());
    for(size_t i=0;i<n;++i){
      r.data.push_back(fs.data[i](xs.data[i]));
    }
    return r;
  }

  template <class FUNCTION, class FIRST, class... REST>
  auto invoke(this auto&&,
              FUNCTION&& function,
              const FIRST& first,
              const REST&... rest)
  {
    using R = std::invoke_result_t<FUNCTION,
                                   typename FIRST::value_type,
                                   typename REST::value_type...>;

    zip_list<R> result;
    size_t n = first.data.size();
    ((n = std::min(n, rest.data.size())), ...);

    for (size_t i = 0; i < n; ++i) {
      result.data.push_back(std::invoke(function, first.data[i], rest.data[i]...));
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

}

#endif
