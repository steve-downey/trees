# Plan: Make FingerTree5 a C++ Container + AllocatorAware + PMR

## Context

With the bidirectional iterator now in place, FingerTree5 is close to
satisfying the C++ Container named requirements.  This plan covers what's
missing, how to add it, and how to make the tree allocator-aware (including
a `std::pmr` typedef).

FingerTree5 is an immutable/persistent container.  It models **Container**
and **ReversibleContainer** but NOT **SequenceContainer** — the latter
requires in-place mutation (`push_back` modifying `*this`), which contradicts
the persistent semantics.  This is analogous to how `std::set` is a Container
but not a SequenceContainer.

---

## Current state (gap audit)

### Type aliases present
- `value_type = T`
- `tag_type = Tag`

### Type aliases MISSING (Container requires)
- `reference` → `const T&` (immutable container — all access is const)
- `const_reference` → `const T&`
- `iterator` → `FingerTree5Iterator<T, Tag, MP>`
- `const_iterator` → same as `iterator` (tree is immutable, no mutable iteration)
- `difference_type` → `std::ptrdiff_t`
- `size_type` → `std::size_t`

### Members present
- `begin()` / `end()` ✓
- `is_empty()` ✓ (wrong name — Container requires `empty()`)
- Copy/move ctor and assignment ✓ (implicit)

### Members MISSING
- `empty()` — aliased name (currently `is_empty()`)
- `size()` — currently no direct member (must use `measure()` for default)
- `max_size()` — not defined
- `cbegin()` / `cend()` — not defined
- `swap()` — not defined
- `operator==` / `operator!=` — not defined
- `rbegin()` / `rend()` / `crbegin()` / `crend()` — not defined

### Allocation pattern
- 13 call sites use `std::make_shared` (Elem nodes, Deep nodes, SpinePtrs)
- `ElemPtr<T,Tag> = shared_ptr<const Elem<T,Tag>>`
- `DeepPtr = shared_ptr<const Deep>`
- `SpinePtr = shared_ptr<const FingerTree5>`
- No allocator parameter anywhere; no `allocate_shared`; no PMR

---

## Phase 1 — Container named requirements

### File modified
`src/smd/tree/finger_tree5.hpp`

### Changes

**Add type aliases** to the public section:

```cpp
using reference       = const T&;
using const_reference = const T&;
using iterator        = FingerTree5Iterator<T, TAG_TYPE, MEASURE_POLICY>;
using const_iterator  = iterator; // immutable container
using difference_type = std::ptrdiff_t;
using size_type       = std::size_t;
```

**Add `empty()`** — aliased to `is_empty()`.  Keep `is_empty()` for backward
compatibility but add `empty()` as the standard name:

```cpp
auto empty() const -> bool { return is_empty(); }
```

**Add `size()`** — for the default measure (Tag=size_t, MP=UnitMeasure5) this
is O(1) via `measure()`.  For custom measures, O(N) via `for_each` counting:

```cpp
auto size() const -> size_type {
    if constexpr (std::same_as<Tag, std::size_t> &&
                  std::same_as<Meas, UnitMeasure5<T, std::size_t>>) {
        return measure();
    } else {
        size_type n = 0;
        for_each([&](const T&) { ++n; });
        return n;
    }
}
```

**Add `max_size()`**:

```cpp
auto max_size() const -> size_type {
    return std::numeric_limits<size_type>::max();
}
```

**Add `swap()`**:

```cpp
void swap(FingerTree5& other) noexcept { std::swap(d_repr, other.d_repr); }
friend void swap(FingerTree5& a, FingerTree5& b) noexcept { a.swap(b); }
```

**Add `cbegin()` / `cend()`** — identical to begin/end (immutable container):

```cpp
auto cbegin() const -> const_iterator { return begin(); }
auto cend()   const -> const_iterator { return end(); }
```

**Add `operator==`** — element-wise comparison via iterators.  Short-circuit
on size for the default measure:

```cpp
friend auto operator==(const FingerTree5& lhs, const FingerTree5& rhs) -> bool {
    if constexpr (std::same_as<Tag, std::size_t> &&
                  std::same_as<Meas, UnitMeasure5<T, std::size_t>>) {
        if (lhs.measure() != rhs.measure()) return false;
    }
    return std::ranges::equal(lhs, rhs);
}
```

### Tests to add (in finger_tree5.t.cpp)

- `ContainerTypeAliases` — `static_assert` on all type aliases
- `SizeMember` — `t.size() == t.measure()` for default, O(N) for custom
- `EmptyMember` — `empty() == is_empty()`
- `SwapMember` — swap two trees, verify contents swapped
- `EqualityOperator` — same content → equal; different → not equal; different size → not equal

---

## Phase 2 — ReversibleContainer

### File modified
`src/smd/tree/finger_tree5.hpp` (or `finger_tree5_iterator.hpp`)

### Changes

**Add reverse iterator type aliases**:

```cpp
using reverse_iterator       = std::reverse_iterator<iterator>;
using const_reverse_iterator = std::reverse_iterator<const_iterator>;
```

**Add `rbegin()` / `rend()` / `crbegin()` / `crend()`**:

```cpp
auto rbegin()  const -> reverse_iterator       { return reverse_iterator(end()); }
auto rend()    const -> reverse_iterator       { return reverse_iterator(begin()); }
auto crbegin() const -> const_reverse_iterator { return const_reverse_iterator(cend()); }
auto crend()   const -> const_reverse_iterator { return const_reverse_iterator(cbegin()); }
```

`std::reverse_iterator` wraps our bidirectional iterator and provides `--`
semantics automatically.  Since our iterator's `operator--` is O(1) amortized,
the reverse iterator is also O(1) amortized per step.

### Tests to add

- `ReverseIteratorMatchesReversedFlatten`
- `RBeginREndDistance`
- `StaticAssertReversibleRange` — `static_assert(std::ranges::bidirectional_range<FT>)`

---

## Phase 3 — AllocatorAwareContainer

### Design decisions

**Allocator parameter position**: 4th template parameter, after MEASURE_POLICY.
Default: `std::allocator<std::byte>`.

```cpp
template <typename T,
          typename TAG_TYPE = std::size_t,
          typename MEASURE_POLICY = UnitMeasure5<T, TAG_TYPE>,
          typename ALLOCATOR = std::allocator<std::byte>>
class FingerTree5 { ... };
```

Using `std::byte` as the allocator's value_type (like `std::pmr::polymorphic_allocator<std::byte>`)
because the tree allocates multiple different types (Elem, Deep, FingerTree5)
through the same allocator.  Internal allocation sites rebind as needed.

**Allocator storage**: stored as a compressed member (no space overhead for
stateless allocators like `std::allocator<>`) using `[[no_unique_address]]`:

```cpp
[[no_unique_address]] ALLOCATOR d_alloc;
```

**Allocator threading**: all `std::make_shared<X>(...)` calls become
`std::allocate_shared<X>(rebind_alloc, ...)` where:

```cpp
using ElemAlloc = typename std::allocator_traits<ALLOCATOR>::template rebind_alloc<Elem<T,Tag>>;
using DeepAlloc = typename std::allocator_traits<ALLOCATOR>::template rebind_alloc<Deep>;
using TreeAlloc = typename std::allocator_traits<ALLOCATOR>::template rebind_alloc<FingerTree5>;
```

**Propagation**: follow `std::allocator_traits<ALLOCATOR>::propagate_on_*`
for copy/move assignment and swap.  For immutable trees, copy assignment is
the main concern (move doesn't allocate, swap just exchanges pointers).

**Helper refactoring**: the `ft5::make_leaf`, `ft5::make_node2`, `ft5::make_node3`
functions currently take no allocator.  They must be changed to accept an
allocator parameter:

```cpp
template <typename T, typename Tag, typename Alloc, typename MeasFn>
auto make_leaf(Alloc& alloc, MeasFn &&mf, T value) -> ElemPtr<T, Tag> {
    using ElemAlloc = typename std::allocator_traits<Alloc>::template rebind_alloc<Elem<T,Tag>>;
    ElemAlloc ea(alloc);
    auto m = mf(value);
    return std::allocate_shared<const Elem<T, Tag>>(
        ea, Elem<T, Tag>{std::move(m), typename Elem<T, Tag>::Leaf{std::move(value)}});
}
```

Similarly for `make_node2`, `make_node3`.

The `make_deep` and spine-construction helpers inside `FingerTree5` already
have access to `d_alloc` (they are member functions), so they use
`d_alloc` directly after rebinding.

**Key insight**: since `shared_ptr` stores its deleter/allocator in the
control block, nodes allocated with different allocators can coexist in the
same tree (the deleter is per-node, not per-tree).  This means structural
sharing across trees with different allocators is safe — each node is
deallocated by the allocator that created it.

### Files modified

- `src/smd/tree/finger_tree5.hpp` — add ALLOCATOR parameter, refactor all
  `make_shared` → `allocate_shared`, add `get_allocator()`, allocator-extended
  constructors
- `src/smd/tree/finger_tree5_iterator.hpp` — add ALLOCATOR to template params
  in the friend declaration and begin/end signatures

### Public API additions

```cpp
using allocator_type = ALLOCATOR;

auto get_allocator() const -> allocator_type { return d_alloc; }

// Allocator-extended constructors
explicit FingerTree5(const ALLOCATOR& alloc);  // empty tree with alloc
FingerTree5(const FingerTree5& other, const ALLOCATOR& alloc);  // copy with alloc
FingerTree5(FingerTree5&& other, const ALLOCATOR& alloc);  // move with alloc
```

### Tests to add

- `AllocatorTypeAlias` — static_assert `allocator_type` exists
- `GetAllocator` — verify returned allocator matches construction
- `CustomAllocator` — use a counting allocator, verify allocations go through it
- `AllocatorPropagation` — copy/move/swap propagation semantics
- `PmrBasic` — use `std::pmr::monotonic_buffer_resource`, construct a tree,
  verify all allocations hit the buffer

---

## Phase 4 — PMR typedef

### New file
`src/smd/tree/finger_tree5_pmr.hpp` (or add to `finger_tree5.hpp`)

### Content

```cpp
#include <memory_resource>

namespace smd::tree::pmr {

template <typename T,
          typename TAG_TYPE = std::size_t,
          typename MEASURE_POLICY = UnitMeasure5<T, TAG_TYPE>>
using FingerTree5 = smd::tree::FingerTree5<T, TAG_TYPE, MEASURE_POLICY,
                                            std::pmr::polymorphic_allocator<std::byte>>;

} // namespace smd::tree::pmr
```

### Tests

- `PmrDefaultResource` — construct via `pmr::FingerTree5<int>`, operates normally
- `PmrMonotonicBuffer` — arena allocation, verify no default-allocator calls
- `PmrPoolResource` — pool allocation for small nodes
- `PmrCrossResourceSharing` — trees from different resources can share nodes
  (shared_ptr control blocks remember their own allocator)

---

## Phase 5 — Convenience aliases for familiarity (optional)

Add names that match standard container vocabulary:

```cpp
auto front() const -> const_reference { return head(); }
auto back()  const -> const_reference { return last(); }
```

These don't make FingerTree5 a SequenceContainer (no mutation) but make the
API feel familiar to std users.

---

## Verification

After each phase:

```bash
make compile
uv run ctest --test-dir .build/build-gcc-16 -C Asan -R "FingerTree5" --output-on-failure
make compile-headers
```

After Phase 3, add a `static_assert`:

```cpp
static_assert(std::ranges::bidirectional_range<FingerTree5<int>>);
static_assert(std::ranges::common_range<FingerTree5<int>>);
```

After Phase 4:

```cpp
static_assert(std::ranges::bidirectional_range<smd::tree::pmr::FingerTree5<int>>);
```

---

## Commit sequence

1. "add Container type aliases and missing members to FingerTree5" (Phase 1)
2. "add ReversibleContainer support via std::reverse_iterator" (Phase 2)
3. "make FingerTree5 allocator-aware (AllocatorAwareContainer)" (Phase 3)
4. "add smd::tree::pmr::FingerTree5 typedef" (Phase 4)
5. "add front()/back() convenience aliases" (Phase 5, optional)

---

## Risk: Allocator parameter changes the type signature

Adding `ALLOCATOR` as a 4th template parameter changes `FingerTree5<int>` to
`FingerTree5<int, std::size_t, UnitMeasure5<int,std::size_t>, std::allocator<std::byte>>`.
With a default argument, `FingerTree5<int>` still works — the allocator
defaults to `std::allocator<std::byte>`.

However, ALL existing code that forward-declares or partially specializes
`FingerTree5` must be updated:
- `template <typename, typename, typename> friend class FingerTree5Iterator` → add 4th param
- Typeclass specializations (`foldable_typeclass<FingerTree5<T,Tag,MP>>`) → add allocator param
- Wrapper default parameters → add allocator to the full tree specialization
- Compile probes → minor updates

This is a breaking change to the template signature (though not to typical
usage like `FingerTree5<int>`).  Phase 3 is the largest and riskiest phase.
