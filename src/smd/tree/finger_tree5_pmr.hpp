// src/smd/tree/finger_tree5_pmr.hpp                                    -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_FINGER_TREE5_PMR
#define INCLUDED_SMD_TREE_FINGER_TREE5_PMR

// Convenience aliases for FingerTree5 backed by std::pmr::polymorphic_allocator.
//
// Usage:
//   #include <smd/tree/finger_tree5_pmr.hpp>
//
//   std::pmr::monotonic_buffer_resource buf;
//   smd::tree::pmr::FingerTree5<int> t(&buf);
//   t = t.snoc(42);
//
// All node allocations (Leaf, Node2/Node3 Elem, Deep, SpinePtr) go through
// the polymorphic allocator.  Structural sharing works across trees with
// different allocators because each node stores its own allocator in the
// shared_ptr control block.

#include <smd/tree/finger_tree5.hpp>

#include <memory_resource>

namespace smd::tree::pmr {

template <typename T,
          typename TAG_TYPE       = std::size_t,
          typename MEASURE_POLICY = UnitMeasure5<T, TAG_TYPE>>
using FingerTree5 =
    smd::tree::FingerTree5<T, TAG_TYPE, MEASURE_POLICY,
                           std::pmr::polymorphic_allocator<std::byte>>;

} // namespace smd::tree::pmr

#endif
