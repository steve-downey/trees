// src/smd/tree/finger_tree5_iterator.hpp                             -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_FINGER_TREE5_ITERATOR
#define INCLUDED_SMD_TREE_FINGER_TREE5_ITERATOR

// Bidirectional iterator for FingerTree5.
//
// Include this header to obtain iteration support:
//
//   #include <smd/tree/finger_tree5_iterator.hpp>
//   for (auto x : tree) { ... }           // range-based for via ADL
//   auto it = smd::tree::begin(tree);     // explicit free function
//
// Free functions begin(t)/end(t) work for any <T,Tag,Measure> combination.
// For the default measure <T, std::size_t, UnitMeasure5<T,std::size_t>>,
// tree size is computed in O(1) via measure().  For custom measures the size
// is computed in O(N) by counting elements through for_each.
//
// Iterator category: bidirectional (operator++ and operator-- are O(1)
// amortized).  Random-access is deliberately not claimed — tagging RA would
// lie to std::sort about the cost of distance and advance.
//
// Iterator validity: the iterator holds a shared_ptr to the root, so the
// tree can be destroyed while iterators remain valid.  Structural sharing
// means all nodes reachable from the iterator's snapshot stay alive.
//
// Comparing iterators from different trees is undefined behaviour.

#include <smd/tree/finger_tree5.hpp>

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <memory>
#include <utility>
#include <vector>

namespace smd::tree {

template <typename T, typename Tag, typename MP, typename Alloc>
class FingerTree5Iterator {
    using FT = FingerTree5<T, Tag, MP, Alloc>;
    using EP = ft5::ElemPtr<T, Tag>;
    using E = ft5::Elem<T, Tag>;

    // -----------------------------------------------------------------------
    // State
    // -----------------------------------------------------------------------

    struct SpineFrame {
        std::shared_ptr<const typename FT::Deep> deep;
        enum class Section : std::uint8_t {
            LEFT = 0,
            SPINE = 1,
            RIGHT = 2
        } section;
        std::size_t index; // digit index (unused when section == SPINE)
    };

    // Shorthand — dependent type so using enum is not allowed, use these.
    static constexpr auto k_left = SpineFrame::Section::LEFT;
    static constexpr auto k_spine = SpineFrame::Section::SPINE;
    static constexpr auto k_right = SpineFrame::Section::RIGHT;

    struct ElemFrame {
        EP elem;
        std::size_t child; // 0/1 for Node2, 0/1/2 for Node3
    };

    std::vector<SpineFrame> d_spine_path; // outermost → innermost Deep
    std::vector<ElemFrame> d_elem_path;   // outer → leaf-parent Node
    EP d_current_leaf;                    // null at end()
    std::size_t d_absolute_index{0};
    std::size_t d_tree_size{0};
    std::shared_ptr<const FT> d_root_keepalive;

    // -----------------------------------------------------------------------
    // Element-level helpers
    // -----------------------------------------------------------------------

    static auto num_children(const EP &ep) -> std::size_t {
        return std::visit(
            ft5::overloaded{
                [](const typename E::Leaf &) -> std::size_t { return 0; },
                [](const typename E::Node2 &) -> std::size_t { return 2; },
                [](const typename E::Node3 &) -> std::size_t { return 3; }},
            ep->d_data);
    }

    static auto nth_child(const EP &ep, std::size_t n) -> EP {
        return std::visit(
            ft5::overloaded{[](const typename E::Leaf &) -> EP {
                                assert(false && "nth_child called on Leaf");
                                std::unreachable();
                            },
                            [n](const typename E::Node2 &nd) -> EP {
                                return n == 0 ? nd.a : nd.b;
                            },
                            [n](const typename E::Node3 &nd) -> EP {
                                return n == 0 ? nd.a : n == 1 ? nd.b : nd.c;
                            }},
            ep->d_data);
    }

    // Walk down to the leftmost Leaf within ep, pushing ElemFrames.
    void descend_elem_left(EP ep) {
        while (!ft5::is_leaf(ep)) {
            d_elem_path.push_back({ep, 0});
            ep = nth_child(ep, 0);
        }
        d_current_leaf = std::move(ep);
    }

    // Walk down to the rightmost Leaf within ep, pushing ElemFrames.
    void descend_elem_right(EP ep) {
        while (!ft5::is_leaf(ep)) {
            std::size_t last = num_children(ep) - 1;
            d_elem_path.push_back({ep, last});
            ep = nth_child(ep, last);
        }
        d_current_leaf = std::move(ep);
    }

    // -----------------------------------------------------------------------
    // Spine-level descent helpers
    // -----------------------------------------------------------------------

    // Called when the top SpineFrame's LEFT digit is exhausted (going forward).
    // Sets the top frame's section to SPINE and descends into the spine.
    void enter_spine_left(const typename FT::SpinePtr &sp) {
        // Mark that we are now in the spine section of the current Deep.
        d_spine_path.back().section = k_spine;

        if (!sp || sp->is_empty()) {
            // No spine — go directly to RIGHT.
            d_spine_path.back().section = k_right;
            d_spine_path.back().index = 0;
            descend_elem_left(d_spine_path.back().deep->d_right[0]);
            return;
        }
        std::visit(ft5::overloaded{[](const typename FT::Empty &) {},
                                   [this](const typename FT::Single &s) {
                                       // No SpineFrame pushed; descend via
                                       // ElemFrames only.
                                       descend_elem_left(s.d_elem);
                                   },
                                   [this](const typename FT::DeepPtr &d) {
                                       // reserve(32) in constructor prevents
                                       // reallocation.
                                       d_spine_path.push_back({d, k_left, 0});
                                       descend_elem_left(
                                           d_spine_path.back().deep->d_left[0]);
                                   }},
                   sp->d_repr);
    }

    // Called when the top SpineFrame's RIGHT digit is exhausted (going
    // backward).
    void enter_spine_right(const typename FT::SpinePtr &sp) {
        d_spine_path.back().section = k_spine;

        if (!sp || sp->is_empty()) {
            // No spine — go directly to LEFT.
            auto sz = d_spine_path.back().deep->d_left.size();
            d_spine_path.back().section = k_left;
            d_spine_path.back().index = sz - 1;
            descend_elem_right(d_spine_path.back().deep->d_left.back());
            return;
        }
        std::visit(
            ft5::overloaded{[](const typename FT::Empty &) {},
                            [this](const typename FT::Single &s) {
                                descend_elem_right(s.d_elem);
                            },
                            [this](const typename FT::DeepPtr &d) {
                                d_spine_path.push_back(
                                    {d, k_right, d->d_right.size() - 1});
                                descend_elem_right(
                                    d_spine_path.back().deep->d_right.back());
                            }},
            sp->d_repr);
    }

    // -----------------------------------------------------------------------
    // Spine-level advance (operator++)
    // -----------------------------------------------------------------------

    void advance_spine() {
        while (!d_spine_path.empty()) {
            auto &top = d_spine_path.back();

            if (top.section == k_left) {
                if (top.index + 1 < top.deep->d_left.size()) {
                    ++top.index;
                    descend_elem_left(top.deep->d_left[top.index]);
                    return;
                }
                // Left digit exhausted — enter the spine.
                enter_spine_left(top.deep->d_spine);
                return;
            }

            if (top.section == k_spine) {
                // Returned from the spine subtree — move to RIGHT.
                top.section = k_right;
                top.index = 0;
                descend_elem_left(top.deep->d_right[0]);
                return;
            }

            // RIGHT section.
            if (top.index + 1 < top.deep->d_right.size()) {
                ++top.index;
                descend_elem_left(top.deep->d_right[top.index]);
                return;
            }
            // Right digit exhausted — pop back to parent's SPINE section.
            d_spine_path.pop_back();
        }
        // All frames exhausted: we are now at end().
        d_current_leaf = nullptr;
        d_absolute_index = d_tree_size;
    }

    // -----------------------------------------------------------------------
    // Spine-level retreat (operator--)
    // -----------------------------------------------------------------------

    void retreat_spine() {
        while (!d_spine_path.empty()) {
            auto &top = d_spine_path.back();

            if (top.section == k_right) {
                if (top.index > 0) {
                    --top.index;
                    descend_elem_right(top.deep->d_right[top.index]);
                    return;
                }
                // Right beginning — enter spine from right.
                enter_spine_right(top.deep->d_spine);
                return;
            }

            if (top.section == k_spine) {
                // Returned from the spine subtree — move to LEFT (last
                // element).
                auto sz = top.deep->d_left.size();
                top.section = k_left;
                top.index = sz - 1;
                descend_elem_right(top.deep->d_left.back());
                return;
            }

            // LEFT section.
            if (top.index > 0) {
                --top.index;
                descend_elem_right(top.deep->d_left[top.index]);
                return;
            }
            // Left beginning — pop back to parent's SPINE section.
            d_spine_path.pop_back();
        }
        // Decremented past begin — undefined behaviour; assert in debug.
        assert(false && "FingerTree5Iterator: decremented begin iterator");
        std::unreachable();
    }

    // -----------------------------------------------------------------------
    // Descent from the rightmost leaf of the full tree (end() -- )
    // -----------------------------------------------------------------------

    void descend_right_from_root() {
        std::visit(ft5::overloaded{
                       [](const typename FT::Empty &) {
                           assert(false && "decremented end() of empty tree");
                           std::unreachable();
                       },
                       [this](const typename FT::Single &s) {
                           descend_elem_right(s.d_elem);
                       },
                       [this](const typename FT::DeepPtr &d) {
                           d_spine_path.push_back(
                               {d, k_right, d->d_right.size() - 1});
                           descend_elem_right(
                               d_spine_path.back().deep->d_right.back());
                       }},
                   d_root_keepalive->d_repr);
    }

  public:
    // -----------------------------------------------------------------------
    // Iterator concept requirements
    // -----------------------------------------------------------------------

    // Default-constructed iterator is in a singular (unspecified) state.
    // Required by std::default_initializable (via std::weakly_incrementable).
    FingerTree5Iterator() = default;

    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = const T *;
    using reference = const T &;

    // -----------------------------------------------------------------------
    // Static factories
    // -----------------------------------------------------------------------

    static auto make_begin(const FT &tree, std::size_t n)
        -> FingerTree5Iterator {
        FingerTree5Iterator it;
        it.d_tree_size = n;
        it.d_root_keepalive = std::make_shared<const FT>(tree);
        it.d_absolute_index = 0;
        it.d_spine_path.reserve(
            32); // spine depth ≤ floor(log2(N)) ≤ 30 for N<2^31
        it.d_elem_path.reserve(32);

        std::visit(ft5::overloaded{
                       [&](const typename FT::Empty &) {
                           it.d_current_leaf = nullptr;
                           it.d_absolute_index = n; // consistent with end()
                       },
                       [&](const typename FT::Single &s) {
                           it.descend_elem_left(s.d_elem);
                       },
                       [&](const typename FT::DeepPtr &d) {
                           it.d_spine_path.push_back({d, k_left, 0});
                           it.descend_elem_left(d->d_left[0]);
                       }},
                   tree.d_repr);

        return it;
    }

    static auto make_end(const FT &tree, std::size_t n) -> FingerTree5Iterator {
        FingerTree5Iterator it;
        it.d_tree_size = n;
        it.d_root_keepalive = std::make_shared<const FT>(tree);
        it.d_current_leaf = nullptr;
        it.d_absolute_index = n;
        it.d_spine_path.reserve(32);
        it.d_elem_path.reserve(32);
        return it;
    }

    // -----------------------------------------------------------------------
    // Dereference
    // -----------------------------------------------------------------------

    auto operator*() const -> reference {
        assert(d_current_leaf && "dereferenced end() iterator");
        return ft5::leaf_value(d_current_leaf);
    }

    auto operator->() const -> pointer {
        assert(d_current_leaf && "dereferenced end() iterator");
        return &ft5::leaf_value(d_current_leaf);
    }

    // -----------------------------------------------------------------------
    // Advance
    // -----------------------------------------------------------------------

    auto operator++() -> FingerTree5Iterator & {
        assert(d_current_leaf && "incremented end() iterator");
        ++d_absolute_index;

        // Step 1: advance within the current Node tree.
        while (!d_elem_path.empty()) {
            auto &top = d_elem_path.back();
            if (top.child + 1 < num_children(top.elem)) {
                ++top.child;
                descend_elem_left(nth_child(top.elem, top.child));
                return *this;
            }
            d_elem_path.pop_back();
        }
        // Step 2: ElemFrames exhausted — advance across the spine structure.
        advance_spine();
        return *this;
    }

    auto operator++(int) -> FingerTree5Iterator {
        auto copy = *this;
        ++*this;
        return copy;
    }

    // -----------------------------------------------------------------------
    // Retreat
    // -----------------------------------------------------------------------

    auto operator--() -> FingerTree5Iterator & {
        if (!d_current_leaf) {
            // end() -- : descend to the rightmost leaf.
            descend_right_from_root();
            d_absolute_index = d_tree_size - 1;
            return *this;
        }
        --d_absolute_index;

        // Step 1: retreat within the current Node tree.
        while (!d_elem_path.empty()) {
            auto &top = d_elem_path.back();
            if (top.child > 0) {
                --top.child;
                descend_elem_right(nth_child(top.elem, top.child));
                return *this;
            }
            d_elem_path.pop_back();
        }
        // Step 2: ElemFrames exhausted — retreat across the spine structure.
        retreat_spine();
        return *this;
    }

    auto operator--(int) -> FingerTree5Iterator {
        auto copy = *this;
        --*this;
        return copy;
    }

    // -----------------------------------------------------------------------
    // Comparison
    // -----------------------------------------------------------------------

    // Precondition: both iterators are from the same tree snapshot.
    auto operator==(const FingerTree5Iterator &other) const -> bool {
        return d_absolute_index == other.d_absolute_index;
    }
};

// ============================================================================
//                       FREE begin / end (ADL-findable)
// ============================================================================

template <typename T, typename Tag, typename MP, typename Alloc>
auto begin(const FingerTree5<T, Tag, MP, Alloc> &t)
    -> FingerTree5Iterator<T, Tag, MP, Alloc> {
    // Compute tree size: O(1) for default measure, O(N) otherwise.
    std::size_t n = 0;
    if constexpr (std::same_as<Tag, std::size_t> &&
                  std::same_as<MP, UnitMeasure5<T, std::size_t>>) {
        n = t.measure();
    } else {
        t.for_each([&](const T &) { ++n; });
    }
    return FingerTree5Iterator<T, Tag, MP, Alloc>::make_begin(t, n);
}

template <typename T, typename Tag, typename MP, typename Alloc>
auto end(const FingerTree5<T, Tag, MP, Alloc> &t)
    -> FingerTree5Iterator<T, Tag, MP, Alloc> {
    std::size_t n = 0;
    if constexpr (std::same_as<Tag, std::size_t> &&
                  std::same_as<MP, UnitMeasure5<T, std::size_t>>) {
        n = t.measure();
    } else {
        t.for_each([&](const T &) { ++n; });
    }
    return FingerTree5Iterator<T, Tag, MP, Alloc>::make_end(t, n);
}

} // namespace smd::tree

// ============================================================================
// Out-of-line definitions of FingerTree5::begin() and FingerTree5::end()
// declared in finger_tree5.hpp.  Placed here so both FingerTree5Iterator and
// the free begin/end functions are complete when these bodies are compiled.
// ============================================================================

namespace smd::tree {

template <typename T, typename TAG_TYPE, typename MEASURE_POLICY,
          typename ALLOCATOR>
auto FingerTree5<T, TAG_TYPE, MEASURE_POLICY, ALLOCATOR>::begin() const
    -> FingerTree5Iterator<T, TAG_TYPE, MEASURE_POLICY, ALLOCATOR> {
    return smd::tree::begin(*this);
}

template <typename T, typename TAG_TYPE, typename MEASURE_POLICY,
          typename ALLOCATOR>
auto FingerTree5<T, TAG_TYPE, MEASURE_POLICY, ALLOCATOR>::end() const
    -> FingerTree5Iterator<T, TAG_TYPE, MEASURE_POLICY, ALLOCATOR> {
    return smd::tree::end(*this);
}

// Const and reverse iterator accessors — defined here so FingerTree5Iterator
// is a complete type when std::reverse_iterator is instantiated.

template <typename T, typename TAG_TYPE, typename MEASURE_POLICY,
          typename ALLOCATOR>
auto FingerTree5<T, TAG_TYPE, MEASURE_POLICY, ALLOCATOR>::cbegin() const
    -> const_iterator {
    return begin();
}

template <typename T, typename TAG_TYPE, typename MEASURE_POLICY,
          typename ALLOCATOR>
auto FingerTree5<T, TAG_TYPE, MEASURE_POLICY, ALLOCATOR>::cend() const
    -> const_iterator {
    return end();
}

template <typename T, typename TAG_TYPE, typename MEASURE_POLICY,
          typename ALLOCATOR>
auto FingerTree5<T, TAG_TYPE, MEASURE_POLICY, ALLOCATOR>::rbegin() const
    -> reverse_iterator {
    return reverse_iterator(end());
}

template <typename T, typename TAG_TYPE, typename MEASURE_POLICY,
          typename ALLOCATOR>
auto FingerTree5<T, TAG_TYPE, MEASURE_POLICY, ALLOCATOR>::rend() const
    -> reverse_iterator {
    return reverse_iterator(begin());
}

template <typename T, typename TAG_TYPE, typename MEASURE_POLICY,
          typename ALLOCATOR>
auto FingerTree5<T, TAG_TYPE, MEASURE_POLICY, ALLOCATOR>::crbegin() const
    -> const_reverse_iterator {
    return const_reverse_iterator(cend());
}

template <typename T, typename TAG_TYPE, typename MEASURE_POLICY,
          typename ALLOCATOR>
auto FingerTree5<T, TAG_TYPE, MEASURE_POLICY, ALLOCATOR>::crend() const
    -> const_reverse_iterator {
    return const_reverse_iterator(cbegin());
}

} // namespace smd::tree

#endif
