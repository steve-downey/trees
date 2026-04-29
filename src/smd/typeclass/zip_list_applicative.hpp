
#ifndef INCLUDE_SMD_TYPECLASS_ZIP_LIST_APPLICATIVE_HPP
#define INCLUDE_SMD_TYPECLASS_ZIP_LIST_APPLICATIVE_HPP

#include <smd/typeclass/typeclass_base.hpp>
#include <smd/typeclass/applicative.hpp>
#include <smd/typeclass/zip_list.hpp>

namespace smd {

template<class T>
struct ZipListApplicativeMap {

  auto pure(const T& x) const {
    return zip_list<T>{{x}};
  }

  template<class F,class A>
  auto apply(const zip_list<F>& fs,const zip_list<A>& xs) const {
    using R = decltype(fs.data[0](xs.data[0]));
    zip_list<R> r;

    auto n = std::min(fs.data.size(), xs.data.size());
    for(size_t i=0;i<n;++i){
      r.data.push_back(fs.data[i](xs.data[i]));
    }
    return r;
  }
};

template <class T>
inline constexpr auto applicative_typeclass<zip_list<T> > =
    ZipListApplicativeMap<T>{};

}

#endif
