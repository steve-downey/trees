---
name: FingerTree5 technical state — post Phase 4
description: Key technical facts about the FingerTree5 implementation needed to resume work correctly
type: project
originSessionId: febb364f-d8bd-42ed-a1af-9c37e8675e7e
---
## FingerTree5 current technical state (post Phase 4, 2026-05-18)

**File**: `src/smd/tree/finger_tree5.hpp` + `finger_tree5_iterator.hpp` + `finger_tree5_pmr.hpp`

**Template signature**:
```cpp
template <typename T,
          typename TAG_TYPE       = std::size_t,
          typename MEASURE_POLICY = UnitMeasure5<T, TAG_TYPE>,
          typename ALLOCATOR      = std::allocator<std::byte>>
class FingerTree5;
```

**Key design decisions recorded in commit history** (do not change without understanding):
- `empty()` static factory was removed; `FingerTree5{}` default-constructs to empty. `bool empty() const` is the Container query. This resolved a C++ naming conflict (can't have static + non-static with same name).
- Spine shells use `allocate_spine()` with placement new (not `allocate_shared`) because `uses_allocator` protocol would call `flatten()` on a Node3-containing tree, producing structurally invalid results.
- `append()` and copy/move assignment enforce Lakos-rule allocator coherency: if allocators differ and non-propagating, the foreign tree is rebuilt using the target allocator rather than silently mixing.
- The extended constructor `FingerTree5(FingerTree5, ALLOCATOR)` (trailing form for `uses_allocator`) takes ALLOCATOR by value so `pmr::poly<FT5>` implicitly converts to `pmr::poly<byte>`.
- `from_sequence` takes `const std::vector<T>&` (not by value) to avoid copying the input container.
- Internal scaffolding vectors use `EpVec = std::vector<EP, EpAlloc>` where `EpAlloc = rebind_alloc<EP>` so construction stays in the PMR arena.

**Test count**: 488 (includes PMR allocation coherency tests, null_memory_resource self-enforcing tests, Container compliance static_asserts)

**PMR probe**: `src/smd/tree/finger_tree5_pmr_probe.cpp` — standalone executable proving zero global heap allocations for 18 operations. Run in RelWithDebInfo only (Asan wraps new/delete).

**Known limitations** (documented in `docs/finger-tree5-allocator-design.md`):
1. Empty trees returned by `split` static factories (`leaf()`, `from_sequence()` no-alloc overload) use `ALLOCATOR{}` — by design (no allocator context)
2. Two-repo sync: `trees` and eventual `beman::structure` will diverge post-extraction

**Wrapper types** (all parameterized on Tree, defaulting to FT5):
- `FingerTreeRandomAccess<T>` — indexed sequence
- `FingerTreeRope<>` — text editing
- `FingerTreePriorityQueue<T>` — double-ended priority queue
- `FingerTreeIntervalIndex<T>` — overlap query

**Build system**: `make test` builds and runs with Asan config. Use `make ctest CONFIG=RelWithDebInfo` for benchmarks/probe.

**Worktrees**: all campaign worktrees at `/workarea/ft5-*` are merged and safe to remove. Main worktree is `/workarea/trees`.
