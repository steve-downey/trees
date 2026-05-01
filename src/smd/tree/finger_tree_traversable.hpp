// src/smd/tree/finger_tree_traversable.hpp                           -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_FINGER_TREE_TRAVERSABLE
#define INCLUDED_SMD_TREE_FINGER_TREE_TRAVERSABLE

// Traversable constraint for FingerTree core:
// - Materialization: traverse materializes the tree via flatten() into a vector.
// - Preservation: monoid measure semantics are preserved through the traversal.
// - Reconstruction: results are rebuilt via FingerTree<U>::from_sequence() with same measure policy.
// - Applicative semantics: all traversals follow left-to-right order independent of tree structure.
//
// Rationale: FingerTree provides efficient structural operations (cons, snoc, split);
// traversal is not a primary performance path, so O(n) materialization is acceptable.
// Wrapper types override with specialized traversal that preserves wrapper invariants.

#include <smd/tree/finger_tree.hpp>
#include <smd/typeclass/applicative.hpp>
#include <smd/typeclass/traversable.hpp>

#include <functional>
#include <type_traits>
#include <utility>
#include <vector>

namespace smd {

template <class T, class TAG_TYPE, class MEASURE_POLICY>
struct FingerTreeTraversableImpl {
  using element_type = T;

  template <class APPLICATIVE, class F>
  auto traverse(this auto&&,
                const APPLICATIVE& applicative,
                F&& function,
                const smd::tree::FingerTree<T, TAG_TYPE, MEASURE_POLICY>& tree)
  {
    using Context = remove_cvref_t<std::invoke_result_t<F, const T&>>;
    using U = smd::applicative_value_t<Context>;

    auto accumulated = applicative.pure(std::vector<U>{});

    for (const auto& value : tree.flatten()) {
      auto lifted = std::invoke(function, value);
      accumulated = applicative.invoke(
        [](std::vector<U> values, U element) {
          values.push_back(std::move(element));
          return values;
        },
        std::move(accumulated),
        std::move(lifted));
    }

    return applicative.invoke(
      [](std::vector<U> values) {
        return smd::tree::FingerTree<U>::from_sequence(std::move(values));
      },
      std::move(accumulated));
  }
};

template <class T, class TAG_TYPE, class MEASURE_POLICY>
struct FingerTreeTraversableMap
  : Traversable<FingerTreeTraversableImpl<T, TAG_TYPE, MEASURE_POLICY>> {
  using FingerTreeTraversableImpl<T, TAG_TYPE, MEASURE_POLICY>::traverse;
};

template <class T, class TAG_TYPE, class MEASURE_POLICY>
inline constexpr auto traversable_typeclass<
  smd::tree::FingerTree<T, TAG_TYPE, MEASURE_POLICY>> =
  FingerTreeTraversableMap<T, TAG_TYPE, MEASURE_POLICY>{};

}  // close namespace smd

#endif
