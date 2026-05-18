# FingerTree5 Allocator Design

## Context

FingerTree5 is an immutable, structurally-shared persistent container.
Its internal representation is a recursive type of `shared_ptr`-wrapped
nodes (Leaf, Node2, Node3, Deep) where each node carries a cached measure.
Structural sharing means that a cons or split operation produces a new
tree that shares O(log N) spine nodes with the original.

Adding allocator support required resolving a fundamental tension: the
`shared_ptr` model that enables structural sharing and the Lakos rule that
all allocations within a single object graph must come from the same
memory resource.

---

## The Lakos Coherency Rule

An object that participates in a memory resource (allocator) must satisfy:

> All heap pointers reachable from the object point into memory owned by
> the same resource, or into static program storage, or into memory owned
> by the object itself.

Violations are subtle and catastrophic:
- A monotonic buffer backed by a memory-mapped file must contain only
  pointers that are valid after the file is remapped at the same address
  in a restarted process.
- Two program instances sharing a memory segment must agree that every
  pointer in the segment stays within the segment.
- Releasing one resource while a second resource holds pointers into it
  creates use-after-free with no warning at the site of creation.

The failure mode is silent at allocation time and catastrophic at access
time, and the defect may only manifest under specific memory-layout
conditions.

---

## Design Decisions

### 1. Allocator as a 4th template parameter

```cpp
template <typename T,
          typename TAG_TYPE       = std::size_t,
          typename MEASURE_POLICY = UnitMeasure5<T, TAG_TYPE>,
          typename ALLOCATOR      = std::allocator<std::byte>>
class FingerTree5;
```

`std::byte` as the `value_type` matches `std::pmr::polymorphic_allocator`
and signals that the tree internally rebinds to the specific node types
it needs to allocate (`Elem<T,Tag>`, `Deep`, internal shells).

All existing code using `FingerTree5<int>` continues to work unchanged;
the default `std::allocator<std::byte>` is stateless and adds no storage
overhead (`[[no_unique_address]]`).

### 2. Which allocations use the custom allocator

**Yes** — `Elem<T,Tag>` (leaf and node objects, 64 bytes each) and `Deep`
(digit + spine + measure, ~168 bytes each).  These are the user-data
nodes and the vast majority of allocation cost.

**No** — the `FingerTree5` spine shells (a 24-byte variant holding a
`DeepPtr`).  These are internal implementation artifacts that hold no
user data; they do not need to come from the same resource for the Lakos
property to hold on the user-visible data graph.

The exception is deliberate: using `allocate_shared<const FingerTree5>`
for the shell would trigger the `std::uses_allocator` construction
protocol, which calls the allocator-extended constructor.  That
constructor enforces coherency by rebuilding trees from leaf values — but
a spine shell may contain Node3 objects (not leaf values), so flattening
and rebuilding would produce a structurally incorrect tree (leaves where
nodes should be).  `make_shared` avoids the protocol entirely.

### 3. Coherency enforcement at the public API surface

Operations that **combine two trees** must not silently create a mixed
object graph.  The rule: if the two operands share the same allocator
(same memory resource), structural sharing is safe and fast.  If they
differ, the foreign subtree is **rebuilt** using the left operand's
allocator before the combination proceeds.

This applies to:

| Operation | Same allocator | Different allocators |
|-----------|----------------|----------------------|
| `append(right)` | O(log N), structural sharing | O(\|right\|) rebuild of `right` |
| `copy operator=` | O(1), share d_repr | O(N) rebuild |
| `move operator=` | O(1), move d_repr | O(N) rebuild |
| Extended ctor `FT(FT&&, alloc)` | O(1), share | O(N) rebuild |

The cost increase is accepted explicitly per the Lakos rule: silent mixing
is an unacceptable correctness risk; the rebuild cost is bounded and
predictable.

### 4. Detecting allocator equality

```cpp
static auto alloc_equal(const ALLOCATOR& a, const ALLOCATOR& b) noexcept
    -> bool {
    if constexpr (std::allocator_traits<ALLOCATOR>::is_always_equal::value)
        return true;  // compile-time elimination for stateless allocators
    else
        return a == b;  // runtime check (resource pointer comparison for PMR)
}
```

For `std::allocator<std::byte>`, `is_always_equal::value = true` and the
runtime check is compiled away entirely — zero overhead on the default
path.  For `pmr::polymorphic_allocator`, the check compares
`resource()` pointers.

### 5. The `uses_allocator` extended constructor

```cpp
FingerTree5(FingerTree5 other, ALLOCATOR alloc)
    : d_alloc(std::move(alloc)) {
    if (alloc_equal(d_alloc, other.d_alloc))
        d_repr = std::move(other.d_repr);
    else
        *this = from_sequence(other.flatten(), d_alloc);
}
```

This is the "trailing allocator" form required by the `std::uses_allocator`
construction protocol.  `alloc` is taken **by value** (not `const&`) so
that `pmr::polymorphic_allocator<FingerTree5>` (produced by
`allocator_traits::rebind_alloc`) can be implicitly converted to
`pmr::polymorphic_allocator<std::byte>` (= `ALLOCATOR`).

The coherency rule is baked into the constructor: different resource →
full rebuild.  This makes it impossible to accidentally produce a mixed
tree via the standard construction protocol.

### 6. `propagate_on_container_*` traits

Copy and move assignment respect the standard traits:

- **Propagating allocators** (`std::allocator`): allocator propagates,
  data is shared/moved.  No extra cost, no rebuild.
- **Non-propagating allocators** (`pmr::polymorphic_allocator`):
  allocator stays; data is shared if same resource, rebuilt if different.

Move assignment is **not** `noexcept` for non-propagating allocators with
different resources (rebuild may allocate).  This is the correct behavior
per the standard.

### 7. The `append` fast path

```cpp
auto append(const FingerTree5& right) const -> FingerTree5 {
    if (!alloc_equal(d_alloc, right.d_alloc)) {
        auto rebuilt = from_sequence(right.flatten(), d_alloc);
        return app3(d_alloc, *this, {}, rebuilt);
    }
    return app3(d_alloc, *this, {}, right);
}
```

The rebuild path is O(|right|).  For the common case where both trees
share the same memory resource (the expected use of PMR), the fast
structural-sharing path is taken with no overhead.

### 8. Spine shell allocation — `allocate_spine`

The spine shells (`FingerTree5` value objects stored inside `SpinePtr`)
are allocated from the custom allocator via manual placement new.

**Why not `allocate_shared`?**  `std::allocate_shared<const FT5>(fta, t)`
would fire `std::uses_allocator_construction_args`, which selects the
public extended constructor `FT5(FT5&&, ALLOCATOR)`.  That constructor's
coherency check calls `flatten()` when the allocator differs — but spine
shells contain `Node3`/`Node2` elements (not leaf `T` values), so
`flatten()` would produce a structurally invalid tree (leaves where nodes
should be), corrupting the spine.

**The implemented approach** in `allocate_spine(alloc, t)`:
1. `allocator_traits::allocate(fta, 1)` — shell object from custom alloc
2. `::new(raw) FingerTree5(std::move(t))` — placement new, bypasses
   `uses_allocator_construction_args` entirely; no protocol, no rebuild
3. `shared_ptr(raw, deleter, fta)` — control block from custom alloc via
   the 3-arg `shared_ptr` constructor (two allocations vs one for
   `make_shared`, but both from the correct resource)

**Self-enforcing test** (`finger_tree5_pmr.t.cpp`): the arena is backed by
`null_memory_resource()` as the upstream.  Any allocation that tries to
escape to the global heap immediately throws `std::bad_alloc`, making the
test an automatic check that no allocation bypasses the arena.

---

## Known Limitations

### Empty trees from `split` carry the default allocator

`split_impl` returns `InternalSplit` with `make_empty()` (`FingerTree5{}`)
as the left or right half.  These empty trees have `d_alloc = ALLOCATOR{}`
(default) rather than `d_alloc = this->d_alloc`.  Any subsequent mutation
on an empty split result (e.g., `split_result.d_left.snoc(x)`) will
allocate from the default resource.

This is a correctness gap for strict arena usage; it is documented but not
yet fixed.  The fix is to thread `d_alloc` through all `make_empty()`
calls in `split_impl`.

### Static `leaf()` and `from_sequence()` factories use `ALLOCATOR{}`

```cpp
static auto leaf(T value) -> FingerTree5;
static auto from_sequence(std::vector<T> values) -> FingerTree5;
```

These are static, so they have no allocator context.  They use
`ALLOCATOR{}` (default) for all allocations.  For PMR trees, the
allocator-aware overload must be used:

```cpp
auto t = pmr::FingerTree5<int>::from_sequence({1, 2, 3}, &mr);
```

---

## PMR `smd::tree::pmr::FingerTree5`

```cpp
namespace smd::tree::pmr {
template <typename T,
          typename TAG_TYPE       = std::size_t,
          typename MEASURE_POLICY = UnitMeasure5<T, TAG_TYPE>>
using FingerTree5 =
    smd::tree::FingerTree5<T, TAG_TYPE, MEASURE_POLICY,
                           std::pmr::polymorphic_allocator<std::byte>>;
}
```

A typedef that wires `pmr::polymorphic_allocator<std::byte>` as the
allocator.  It inherits all the coherency guarantees above.  The
`get_allocator().resource()` accessor returns the underlying
`memory_resource*`.

Structural sharing across two `pmr::FingerTree5` instances backed by
**different** resources is always safe: `append` detects the mismatch and
rebuilds; assignment and the extended constructor do the same.  The result
is always coherent with the left-hand operand's resource.
