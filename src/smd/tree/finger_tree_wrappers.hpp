// src/smd/tree/finger_tree_wrappers.hpp                              -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_FINGER_TREE_WRAPPERS
#define INCLUDED_SMD_TREE_FINGER_TREE_WRAPPERS

// Core wrapper classes
#include <smd/tree/finger_tree_interval_index.hpp>
#include <smd/tree/finger_tree_priority_queue.hpp>
#include <smd/tree/finger_tree_random_access.hpp>
#include <smd/tree/finger_tree_rope.hpp>

// Wrapper-specific typeclass implementations
#include <smd/tree/finger_tree_interval_index_foldable.hpp>
#include <smd/tree/finger_tree_interval_index_traversable.hpp>
#include <smd/tree/finger_tree_priority_queue_foldable.hpp>
#include <smd/tree/finger_tree_priority_queue_traversable.hpp>
#include <smd/tree/finger_tree_random_access_foldable.hpp>
#include <smd/tree/finger_tree_random_access_traversable.hpp>
#include <smd/tree/finger_tree_rope_foldable.hpp>
#include <smd/tree/finger_tree_rope_traversable.hpp>

#endif
