#ifndef INCLUDE_SMD_ZIPLIST_ZIP_LIST_HPP
#define INCLUDE_SMD_ZIPLIST_ZIP_LIST_HPP

#include <vector>

namespace smd {

template <class T>
struct zip_list {
  using value_type = T;

  std::vector<T> data;
};

}  // close namespace smd

#endif
