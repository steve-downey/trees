// src/smd/tree/finger_tree_wrappers.hpp                              -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_FINGER_TREE_WRAPPERS
#define INCLUDED_SMD_TREE_FINGER_TREE_WRAPPERS

/** Convenience header that includes all four finger tree wrapper types:
 * - FingerTreeIntervalIndex  — interval stabbing/overlap queries
 * - FingerTreePriorityQueue  — persistent double-ended priority queue
 * - FingerTreeRandomAccess   — persistent random-access sequence
 * - FingerTreeRope            — persistent text rope
 */

#include <smd/tree/finger_tree_interval_index.hpp>
#include <smd/tree/finger_tree_priority_queue.hpp>
#include <smd/tree/finger_tree_random_access.hpp>
#include <smd/tree/finger_tree_rope.hpp>

#endif
