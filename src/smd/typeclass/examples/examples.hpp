// src/smd/typeclass/examples/examples.hpp                            -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TYPECLASS_EXAMPLES_EXAMPLES
#define INCLUDED_SMD_TYPECLASS_EXAMPLES_EXAMPLES

#include <beman/optional/optional.hpp>

#include <cstddef>
#include <optional>

namespace smd::typeclass::examples {

auto generic_length_example() -> std::size_t;
auto generic_length_binary_tree_example() -> std::size_t;
auto generic_length_fringe_tree_example() -> std::size_t;
auto foldable_flattens_shape_example() -> bool;
auto applicative_invoke_example() -> beman::optional::optional<int>;
auto traversable_relabel_example() -> beman::optional::optional<std::size_t>;
auto traversable_preserves_shape_example() -> bool;
auto bad_applicative_example() -> std::size_t;
auto explicit_object_lookup_example() -> std::optional<int>;
auto nttp_object_lookup_example() -> std::optional<int>;

}  // close namespace smd::typeclass::examples

#endif  // INCLUDED_SMD_TYPECLASS_EXAMPLES_EXAMPLES
