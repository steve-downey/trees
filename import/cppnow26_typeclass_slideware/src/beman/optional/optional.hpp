#ifndef INCLUDE_BEMAN_OPTIONAL_OPTIONAL_HPP
#define INCLUDE_BEMAN_OPTIONAL_OPTIONAL_HPP

#include <optional>

namespace beman::optional {

template <class VALUE_TYPE>
using optional = std::optional<VALUE_TYPE>;

using std::nullopt;
using std::nullopt_t;

}  // close namespace beman::optional

#endif  // INCLUDE_BEMAN_OPTIONAL_OPTIONAL_HPP
