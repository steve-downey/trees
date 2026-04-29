#ifndef INCLUDE_SMD_TYPECLASS_APPLICATIVE_HPP
#define INCLUDE_SMD_TYPECLASS_APPLICATIVE_HPP
#include <smd/typeclass/typeclass_base.hpp>
namespace smd {

struct applicative_tag{};

template<class T>
auto pure(const auto& x){
  return map<applicative_tag,T>::pure(x);
}

template<class T>
auto apply(const T& f,const T& x){
  return map<applicative_tag,T>::apply(f,x);
}

template<class T>
auto invoke(const auto& f,const T& x){
  return apply<T>(pure<T>(f),x);
}

template<class Map,class T>
auto invoke_with(const auto& f,const T& x){
  return Map::apply(Map::pure(f),x);
}

}
#endif
