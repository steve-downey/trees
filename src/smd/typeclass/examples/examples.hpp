// src/smd/typeclass/examples/examples.hpp                            -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TYPECLASS_EXAMPLES_EXAMPLES
#define INCLUDED_SMD_TYPECLASS_EXAMPLES_EXAMPLES

#include <beman/optional/optional.hpp>

#include <cstddef>
#include <optional>

namespace smd::typeclass::examples {

/** Count elements in a range using the generic Foldable `length`. */
auto generic_length_example() -> std::size_t;
/** Count elements in a BinaryTree using the generic Foldable `length`. */
auto generic_length_binary_tree_example() -> std::size_t;
/** Count elements in a FringeTree using the generic Foldable `length`. */
auto generic_length_fringe_tree_example() -> std::size_t;
/** Demonstrate that `fold_map` linearises tree shape (order is depth-first). */
auto foldable_flattens_shape_example() -> bool;
/** Lift a binary function into optional using `invoke`; returns empty if any
 * arg is empty. */
auto applicative_invoke_example() -> beman::optional::optional<int>;
/** Relabel tree nodes with sequential indices by traversing through optional.
 */
auto traversable_relabel_example() -> beman::optional::optional<std::size_t>;
/** Verify that `traverse` rebuilds a tree of the same shape as the input. */
auto traversable_preserves_shape_example() -> bool;
/** Illustrate a semantically wrong tree Applicative that compiles but violates
 * laws. */
auto bad_applicative_example() -> std::size_t;
/** Call a generic algorithm passing the typeclass object explicitly at the call
 * site. */
auto explicit_object_lookup_example() -> std::optional<int>;
/** Call a generic algorithm with the typeclass object bound as an NTTP default.
 */
auto nttp_object_lookup_example() -> std::optional<int>;

/** Blog post: consolidated Foldable derived-ops showcase on a fixpoint tree. */
auto blog_foldable_showcase() -> bool;
/** Blog post: traverse with validation (all-or-nothing). */
auto blog_traverse_validate() -> bool;
/** Blog post: traverse relabeling (double every constant). */
auto blog_traverse_relabel() -> bool;

/** Blog post: numeric_limits parallel — typeclass lookup works like
 * numeric_limits. */
auto blog_numeric_limits_parallel() -> bool;
/** Blog post: three lookup modes on one type. */
auto blog_three_lookup_modes() -> bool;
/** Blog post: derived operations — fmap gives replace for free. */
auto blog_derived_operations() -> bool;

} // namespace smd::typeclass::examples

#endif // INCLUDED_SMD_TYPECLASS_EXAMPLES_EXAMPLES
