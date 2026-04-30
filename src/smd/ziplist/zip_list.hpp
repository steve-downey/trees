#ifndef INCLUDE_SMD_ZIPLIST_ZIP_LIST_HPP
#define INCLUDE_SMD_ZIPLIST_ZIP_LIST_HPP

#include <cstddef>
#include <optional>
#include <utility>
#include <vector>

namespace smd {

template <class T>
struct zip_list {
  using value_type = T;

  // Invariant: when repeated has a value, this zip_list models an infinite
  // repetition of that value and data is ignored.
  std::vector<T> data;
  std::optional<T> repeated{};

  static auto repeat(T value) -> zip_list
  {
    return zip_list{{}, std::move(value)};
  }

  auto is_repeating() const -> bool { return repeated.has_value(); }

  auto finite_size() const -> std::size_t { return data.size(); }
};

}  // close namespace smd

#endif
