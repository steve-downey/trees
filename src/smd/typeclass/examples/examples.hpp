#ifndef INCLUDE_SMD_TYPECLASS_EXAMPLES_EXAMPLES_HPP
#define INCLUDE_SMD_TYPECLASS_EXAMPLES_EXAMPLES_HPP

#include <beman/optional/optional.hpp>

#include <cstddef>
#include <optional>

namespace smd::typeclass::examples {

auto generic_length_example() -> std::size_t;
auto generic_length_binary_tree_example() -> std::size_t;
auto generic_length_fringe_tree_example() -> std::size_t;
auto applicative_invoke_example() -> beman::optional::optional<int>;
auto traversable_relabel_example() -> beman::optional::optional<std::size_t>;
auto bad_applicative_example() -> std::size_t;
auto explicit_object_lookup_example() -> std::optional<int>;
auto nttp_object_lookup_example() -> std::optional<int>;

}  // close namespace smd::typeclass::examples

#endif  // INCLUDE_SMD_TYPECLASS_EXAMPLES_EXAMPLES_HPP
