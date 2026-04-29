// tests/beman/optional/test_utilities.hpp                            -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef TESTS_BEMAN_OPTIONAL_TEST_UTILITIES_HPP
#define TESTS_BEMAN_OPTIONAL_TEST_UTILITIES_HPP

namespace beman::optional::tests {
/***
 * Evaluate and return an expression in a consteval context for testing
 * constexpr correctness.
 */
auto consteval constify(auto expr) { return (expr); }
} // namespace beman::optional::tests

#endif // TESTS_BEMAN_OPTIONAL_TEST_UTILITIES_HPP
