
#ifndef INCLUDE_SMD_TYPECLASS_ZIP_LIST_HPP
#define INCLUDE_SMD_TYPECLASS_ZIP_LIST_HPP

#include <vector>

namespace smd {

template<class T>
struct zip_list {
  std::vector<T> data;
};

}

#endif
