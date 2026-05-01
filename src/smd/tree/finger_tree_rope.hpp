// src/smd/tree/finger_tree_rope.hpp                                  -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_FINGER_TREE_ROPE
#define INCLUDED_SMD_TREE_FINGER_TREE_ROPE

#include <smd/tree/finger_tree.hpp>
#include <smd/typeclass/applicative.hpp>
#include <smd/typeclass/foldable.hpp>
#include <smd/typeclass/traversable.hpp>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <functional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace smd::tree {

struct RopeChunkMeasure {
  auto operator()(const std::string& value) const -> std::size_t
  {
    return value.size();
  }
};

class FingerTreeRope {
  using Tree = FingerTree<std::string, std::size_t, RopeChunkMeasure>;

  Tree d_tree;

  auto split_chars(std::size_t pos) const -> std::pair<FingerTreeRope, FingerTreeRope>
  {
    if (pos == 0) {
      return {FingerTreeRope{}, *this};
    }

    if (pos >= size_bytes()) {
      return {*this, FingerTreeRope{}};
    }

    auto split = d_tree.split([pos](std::size_t prefix) {
      return prefix > pos;
    });

    if (!split.has_value()) {
      return {*this, FingerTreeRope{}};
    }

    auto left_prefix_bytes = split->d_left.measure();
    auto local = pos - left_prefix_bytes;

    const auto& pivot = split->d_pivot;
    assert(local < pivot.size());

    auto left = split->d_left;
    if (local > 0) {
      left = left.snoc(pivot.substr(0, local));
    }

    auto right = split->d_right;
    if (local < pivot.size()) {
      right = right.cons(pivot.substr(local));
    }

    return {FingerTreeRope{std::move(left)}, FingerTreeRope{std::move(right)}};
  }

 public:
  FingerTreeRope()
    : d_tree(Tree::empty())
  {
  }

  static auto from_chunks(std::vector<std::string> chunks) -> FingerTreeRope
  {
    return FingerTreeRope{Tree::from_sequence(std::move(chunks))};
  }

  static auto from_text(std::string_view text, std::size_t chunk_size = 16)
    -> FingerTreeRope
  {
    std::vector<std::string> chunks;
    chunks.reserve((text.size() / chunk_size) + 1);

    for (std::size_t i = 0; i < text.size(); i += chunk_size) {
      const auto n = std::min(chunk_size, text.size() - i);
      chunks.emplace_back(text.substr(i, n));
    }

    return from_chunks(std::move(chunks));
  }

  auto size_bytes() const -> std::size_t { return d_tree.measure(); }

  auto to_string() const -> std::string
  {
    std::string out;
    out.reserve(size_bytes());
    for (const auto& chunk : d_tree.flatten()) {
      out += chunk;
    }
    return out;
  }

  auto insert(std::size_t pos, std::string_view text) const -> FingerTreeRope
  {
    auto [left, right] = split_chars(pos);
    auto middle = from_text(text);
    return FingerTreeRope{Tree::concat(Tree::concat(left.d_tree, middle.d_tree),
                                        right.d_tree)};
  }

  auto erase(std::size_t pos, std::size_t count) const -> FingerTreeRope
  {
    auto [left, rest] = split_chars(pos);
    auto [drop, right] = rest.split_chars(count);
    static_cast<void>(drop);
    return FingerTreeRope{Tree::concat(left.d_tree, right.d_tree)};
  }

  auto replace(std::size_t pos, std::size_t count, std::string_view text) const
    -> FingerTreeRope
  {
    return erase(pos, count).insert(pos, text);
  }

  auto chunks() const -> std::vector<std::string> { return d_tree.flatten(); }

 private:
  explicit FingerTreeRope(Tree tree)
    : d_tree(std::move(tree))
  {
  }
};

}  // namespace smd::tree

#endif

#include <smd/tree/finger_tree_rope_foldable.hpp>
#include <smd/tree/finger_tree_rope_traversable.hpp>
