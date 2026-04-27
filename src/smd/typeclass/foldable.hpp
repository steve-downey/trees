#ifndef INCLUDE_SMD_TYPECLASS_FOLDABLE_HPP
#define INCLUDE_SMD_TYPECLASS_FOLDABLE_HPP

#include <smd/typeclass/typeclass_base.hpp>

namespace smd {

struct foldable_tag {};

template<class T, class F>
auto fold_map(F f, const T& x) {
  return map<foldable_tag, T>::fold_map(f, x);
}

}

#endif
