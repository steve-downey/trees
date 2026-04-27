#ifndef INCLUDE_SMD_TYPECLASS_APPLICATIVE_HPP
#define INCLUDE_SMD_TYPECLASS_APPLICATIVE_HPP
#include <smd/typeclass/typeclass_base.hpp>
namespace smd {
struct applicative_tag{};
template<class T,class F,class X>
auto invoke(F f,const X& x){
  return map<applicative_tag,T>::apply(
    map<applicative_tag,T>::pure(f),x);
}
}
#endif
