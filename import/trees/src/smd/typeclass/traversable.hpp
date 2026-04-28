#ifndef INCLUDE_SMD_TYPECLASS_TRAVERSABLE_HPP
#define INCLUDE_SMD_TYPECLASS_TRAVERSABLE_HPP

#include <smd/typeclass/typeclass_base.hpp>

namespace smd {

struct traversable_tag {};

template<class T, class F>
auto traverse(F f, const T& x) {
  return map<traversable_tag, T>::traverse(f, x);
}

}

#endif
