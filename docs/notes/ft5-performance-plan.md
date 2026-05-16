# Performance Analysis & Optimization Plan for FingerTree5

## Measured type sizes (gcc-16, x86-64)

```
sizeof(Elem<int, size_t>)    =  64 bytes  (8 measure + 56 variant<Leaf,Node2,Node3>)
sizeof(Elem<string, size_t>) =  64 bytes  (same — string in Leaf still fits in 56)
sizeof(ElemPtr)              =  16 bytes  (shared_ptr = 2 pointers)
sizeof(Digit)                =  72 bytes  (inplace_vector<EP,4> = 4×16 + 8 size)
sizeof(FingerTree5<int>)     =  24 bytes  (variant<Empty,Single,DeepPtr> = 16+8)
sizeof(Elem::Leaf)           =   4 bytes  (just T=int)
sizeof(Elem::Node2)          =  32 bytes  (2×ElemPtr = 2×16)
sizeof(Elem::Node3)          =  48 bytes  (3×ElemPtr = 3×16)
sizeof(Deep) (approx)        = 168 bytes  (8 measure + 72 d_left + 16 d_spine + 72 d_right)
sizeof(shared_ptr ctrl block)=  16 bytes  (refcount + weak count, typical)
```

---

## Identified hot spots and pessimizations

### H1. `make_shared` on every internal node — the dominant cost

**13 call sites** allocate through `make_shared`.  Every cons/snoc that
overflows a digit allocates:
- 1× `make_shared<Elem>` for the Node3 (64 bytes + 16 ctrl = 80 bytes)
- 1× `make_shared<Deep>` for the new Deep (~168 + 16 = 184 bytes)
- 1× `make_shared<FingerTree5>` for the new SpinePtr (24 + 16 = 40 bytes)

Amortized per element: ~1 `make_shared` for the Leaf (every cons/snoc),
plus ~0.33 for the overflow Node3, plus ~0.33 for the new Deep.
Total: ~1.66 heap allocations per element inserted.

**Impact**: The benchmark shows FT5 snoc at 2× the cost of FT2 (which stores
nodes by value).  For `int` elements, the allocation overhead dominates the
actual data manipulation.

**Measurement**: Already measured by "Bench A snoc" in
`finger_tree_compare.bench.cpp` and the compile probes' DWARF counts.
Additional instrumentation: counting allocator that records allocation
count and total bytes per operation.

**Optimization opportunities**:
1. **Arena/pool allocator** — pre-allocate a slab for Elem and Deep objects.
   With PMR (Phase 3 of Container plan), users can pass a
   `monotonic_buffer_resource` to eliminate per-node malloc overhead entirely.
   Expected speedup: 2–4× for build operations.
2. **Intrusive refcounting** — replace `shared_ptr` with a custom
   `intrusive_ptr<const Elem>` where the refcount lives inside the Elem struct
   itself.  Eliminates the separate control block allocation (saves 16 bytes
   per node) and halves pointer indirection.  Expected speedup: 20–40% on
   traversal (one fewer cache miss per node).
3. **Node coalescing in `from_sequence`** — build bottom-up instead of
   repeated snoc.  Allocate all Leaf Elems in one batch, group into Node3s,
   group those into Node3s at the next level, etc.  Eliminates intermediate
   Deep/SpinePtr allocations entirely during bulk construction.

---

### H2. `std::visit` dispatch — variant overhead on the hot path

12 `std::visit` calls exist.  The variant `d_repr` holds `{Empty, Single,
DeepPtr}` (3 alternatives); the variant `d_data` in Elem holds
`{Leaf, Node2, Node3}` (3 alternatives).

Each `std::visit` call:
- Checks the variant index (one branch)
- Dispatches through a function pointer table or switch

For small operations (cons with a non-full digit, measure()), the visit
dispatch overhead may be a significant fraction of the total work.

**Measurement**:
- `perf stat` on a tight cons loop to count branch mispredictions.
- Micro-benchmark comparing `std::visit` dispatch cost vs `if constexpr` /
  `switch(d_repr.index())` with manual `std::get_if`.

**Optimization opportunity**:
- Replace `std::visit(overloaded{...}, d_repr)` with explicit
  `if (auto* p = std::get_if<DeepPtr>(&d_repr))` for the hot path.
  In `cons_internal`, the Deep case is by far the most common after the
  tree grows past 2 elements.  Checking `get_if<DeepPtr>` first (predicted
  branch) followed by the rare Empty/Single cases would help the branch
  predictor.

---

### H3. Digit copy in `digit_with_pushed_front` / `digit_with_pushed_back`

Every non-overflow cons/snoc creates a NEW Digit by copying the existing
digit elements (shared_ptr copies = refcount increments) into a fresh
`inplace_vector`:

```cpp
auto digit_with_pushed_front(const Digit &d, ElemPtr x) -> Digit {
    Digit out;
    out.push_back(std::move(x));
    for (const auto &e : d) out.push_back(e);  // copies 1–3 shared_ptrs
    return out;
}
```

For a 3-element digit, this is 3 atomic refcount increments + 1 move.
The digit is 72 bytes; copying it by value is fast (stack memory), but
the atomic increments are not free on x86 (LOCK XADD).

**Measurement**: Track `std::atomic::fetch_add` count per cons/snoc using
a custom shared_ptr replacement or `perf stat` on atomic operations.

**Optimization opportunity**:
- **Move-aware digits**: since the tree is immutable, each Deep is created
  once and never modified.  The digit members of Deep are const after
  construction.  But when we're building a new tree from `cons_internal`,
  we READ the old Deep's digits (const reference).  No way around the copy
  except...
- **COW on Digit** (too complex, not recommended).
- **Batch construction** avoids repeated digit copies by building the Digit
  directly rather than extending one element at a time.

---

### H4. `make_deep` recomputes `digit_measure` every time

**Line 369–376**:
```cpp
static auto make_deep(Digit left, SpinePtr spine, Digit right) -> FingerTree5 {
    auto m = ft5::tag_op<Tag>(
        ft5::tag_op<Tag>(ft5::digit_measure(left),
                         spine ? spine->measure() : ft5::tag_id<Tag>()),
        ft5::digit_measure(right));
    ...
}
```

`digit_measure` iterates up to 4 ElemPtrs and combines their measures.
For `Tag = size_t` with `UnitMeasure5`, this is just adding 1+1+1+1.  For
more complex measures (string concat, interval max), this could be expensive.

**Measurement**: Profile `digit_measure` cost in "Bench C split" where
`make_deep` is called on every split result.

**Optimization opportunity**:
- **Cache digit measure in the Digit itself** — change `Digit` from
  `inplace_vector<EP, 4>` to a wrapper that caches the combined measure:
  ```cpp
  template <typename T, typename Tag>
  struct Digit {
      std::inplace_vector<ElemPtr<T,Tag>, 4> d_elems;
      Tag d_measure;  // sum of d_elems[i]->d_measure
  };
  ```
  Requires updating all digit construction sites.  Eliminates the
  redundant re-traversal in `make_deep`.

---

### H5. `from_sequence` — O(N) repeated snoc with high constant factor

**Line 946**:
```cpp
static auto from_sequence(std::vector<T> values) -> FingerTree5 {
    auto result = empty();
    for (auto &v : values) result = result.snoc(std::move(v));
    return result;
}
```

For N elements, this performs:
- N `make_shared<Elem>` (Leaf construction)
- ~N/3 `make_shared<Elem>` (Node3 construction on overflow)
- ~N/3 `make_shared<Deep>` (new Deep per overflow)
- ~N/3 `make_shared<FingerTree5>` (new SpinePtr per overflow)

Total: ~2N heap allocations for N elements.

**Measurement**: Already measured by "Bench A snoc" at various N.

**Optimization opportunity**:
- **Bottom-up construction** (Hinze & Paterson §4.3): build all Leaf Elems
  first, group into Node3s bottom-up, construct a balanced tree directly.
  Expected: ~N/3 allocations total (one per Node), eliminating all
  intermediate Deep and SpinePtr allocations.
- **Batch leaf allocation**: allocate all N Leaf Elems in a single arena
  and construct the tree from the pre-allocated ElemPtrs.

---

### H6. `app3` creates intermediate `std::vector` allocations

**Lines 741–749**:
```cpp
auto combined = ft5::digit_to_vec<T, Tag>(ld.d_right);       // alloc 1
combined.insert(combined.end(), ...middle...);                // possible realloc
auto rl = ft5::digit_to_vec<T, Tag>(rd.d_left);              // alloc 2
combined.insert(combined.end(), rl.begin(), rl.end());        // possible realloc
auto ns = ft5::nodes_from<T, Tag>(std::move(combined));      // alloc 3
```

Three `std::vector` allocations per spine level of the concat path.
For a concat of two N-element trees, the path is O(log N) deep, so
this is O(log N) vector allocations.

**Measurement**: Profile "Bench C append" allocation counts using a
counting allocator or `valgrind --tool=massif`.

**Optimization opportunity**:
- **Use `inplace_vector<EP, 12>`** for the combined buffer.  The maximum
  size of `combined` is 4 (right digit) + middle.size() + 4 (left digit).
  The `middle` vector comes from `nodes_from` at the previous level and has
  at most 4 elements (from a 12-element input grouped into 4 nodes).
  So `combined` has at most 4 + 4 + 4 = 12 elements.
  An `inplace_vector<EP, 12>` eliminates ALL heap allocations in this path.
- **Similarly for `nodes_from` output**: the output has at most
  ceil(12/2) = 6 elements.  `inplace_vector<EP, 6>` suffices.

This would make `app3` allocation-free at each spine level (only the
`make_node2`/`make_node3` calls for the new nodes allocate, which is
unavoidable).

---

### H7. `flatten` / `for_each` traverse via recursive `std::visit`

`flatten_elem` and `for_each_elem` (lines 128–162) recurse via `std::visit`
on each ElemPtr.  For a tree with N elements, this is:
- N leaf visits (one per element)
- ~N/3 Node2/Node3 visits
- Each visit: one variant dispatch + pointer dereference to the shared_ptr

The pointer-chasing pattern is inherently cache-unfriendly.  Each ElemPtr
dereference is a random pointer chase (non-sequential memory access).

**Measurement**: `perf stat -e cache-misses,L1-dcache-loads` on
"Bench B flatten" at large N (3M).  Compare with a contiguous vector
traversal of the same size.

**Optimization opportunity**:
- **Iterative stack-based traversal** instead of recursive `std::visit`.
  Push ElemPtrs onto an explicit stack, pop and process.  This avoids
  function call overhead per recursion level but doesn't fix cache misses.
- **Compaction**: for `flatten()`, pre-allocate the output vector to
  `size()` elements (available in O(1) for the default measure) and use
  a pointer-write loop.  Already done implicitly via `push_back` but
  without the pre-reserve:
  ```cpp
  auto flatten() const -> std::vector<T> {
      std::vector<T> out;
      out.reserve(measure());  // <-- add this for default measure
      flatten_elems(out);
      return out;
  }
  ```
  The `reserve` eliminates vector reallocations during traversal.

---

### H8. Iterator `make_begin` copies the tree into a `make_shared`

**Iterator line 293**:
```cpp
it.d_root_keepalive = std::make_shared<const FT>(tree);
```

Every call to `begin()` allocates a `shared_ptr<const FingerTree5>` (40 bytes)
just to keep the root alive.  For short-lived iterators (e.g., range-based
for that immediately runs to completion), this is pure overhead.

**Measurement**: "Bench B view_l drain" already shows the iteration cost.
A targeted benchmark: time `begin(t)` alone at various N.

**Optimization opportunity**:
- If the user guarantees the tree outlives the iterator (common case for
  range-based for), provide an "unsafe" begin that skips the keepalive:
  ```cpp
  auto begin_unsafe() const -> FingerTree5Iterator<...>;
  ```
  Or use a `weak_ptr` + check pattern.  However, this adds complexity and
  the allocation is one-time per iteration session, so it's low priority.

---

### H9. `view_l` / `view_r` copy the element value

**Line 900**:
```cpp
return View{ft5::leaf_value(iv->d_elem), std::move(iv->d_rest)};
```

`View::d_value` is `T` by value.  For `T = std::string`, this copies the
string out of the shared_ptr-held Leaf.  The copy is unnecessary for
read-only access patterns (e.g., `head()` just returns the value).

**Measurement**: Benchmark `head()` for `FingerTree5<std::string>` with large
strings.  Compare against a hypothetical `head_ref()` returning `const T&`.

**Optimization opportunity**:
- Add `head_ref()` / `last_ref()` returning `const T&` (safe because the
  tree is immutable and the shared_ptr keeps the Leaf alive).
- Keep `head()` / `last()` returning by value for API consistency.
- The View struct itself could hold a `const T&` if the user promises not
  to outlive the tree.

---

## How to measure before making changes

### Tool 1: Counting allocator

```cpp
struct CountingAllocator {
    static inline std::size_t alloc_count = 0;
    static inline std::size_t total_bytes = 0;
    // ... standard allocator interface ...
};
```

Use with `std::allocate_shared` (after Phase 3 of Container plan) to count
allocations per operation.  Before Phase 3, instrument via a global
`operator new` override in a test binary.

### Tool 2: `perf stat` on benchmark binary

```bash
perf stat -e cache-misses,cache-references,branch-misses,instructions \
    ./smd_tree_benchmarks "Bench B flatten" --benchmark-samples 50
```

Compare FT5 vs std::vector flatten for the same N to quantify the cache-miss
penalty.

### Tool 3: `perf record` + flame graph

```bash
perf record -g ./smd_tree_benchmarks "Bench A snoc" --benchmark-samples 20
perf script | stackcollapse-perf.pl | flamegraph.pl > snoc.svg
```

Identifies where time is spent: `make_shared`, atomic refcount ops, measure
computation, visit dispatch, or the actual algorithm.

### Tool 4: Valgrind DHAT (heap profiler)

```bash
valgrind --tool=dhat ./smd_tree_benchmarks "Bench A snoc" --benchmark-samples 1
```

Reports: allocation count, total bytes, allocation site, access pattern
(short-lived vs long-lived), and wasted bytes (internal fragmentation).

### Tool 5: Targeted micro-benchmarks

Add to `finger_tree_compare.bench.cpp` or a new file:

| Benchmark | What it isolates |
|-----------|-----------------|
| `make_leaf_only N` | Just Leaf creation, no tree structure |
| `make_deep_only N` | Measure `make_deep` construction cost |
| `digit_copy N` | Time `digit_with_pushed_back` vs in-place mutation |
| `visit_dispatch N` | Variant dispatch cost alone |
| `reserve_flatten N` | With vs without vector reserve before flatten |

---

## Prioritized optimization roadmap

| Priority | Optimization | Expected gain | Effort | Prerequisite |
|----------|-------------|---------------|--------|--------------|
| 1 | `flatten()` reserve (H7) | 10–30% on flatten | Trivial | None |
| 2 | `inplace_vector<EP,12>` in app3 (H6) | Eliminates ~3 allocs/level in concat | Low | None |
| 3 | Bottom-up `from_sequence` (H5) | 2–3× faster bulk build | Medium | None |
| 4 | Digit measure cache (H4) | Faster make_deep for complex measures | Medium | None |
| 5 | Arena allocator via PMR (H1) | 2–4× faster build ops | Medium | Phase 3 Container plan |
| 6 | Intrusive refcounting (H1) | 20–40% faster traversal | High | Major refactor |
| 7 | `head_ref()` / `last_ref()` (H9) | Avoids copy for string/large T | Low | API decision |
| 8 | Hot-path `get_if` instead of `visit` (H2) | 5–15% on cons/snoc | Low | Profiling data |

Items 1–4 are independent, low-risk, and can be validated by re-running the
existing benchmark suite.  Item 5 depends on the allocator work.  Item 6 is a
fundamental architecture change that should only be done with strong profiling
justification.

---

## Key principle: measure first, then optimize

Every optimization above should be:
1. **Benchmarked before** — establish the current number
2. **Implemented in isolation** — one change per commit
3. **Benchmarked after** — confirm improvement without regression
4. **Documented** — the commit message states the measured improvement

The existing `finger_tree_compare.bench.cpp` and
`finger_tree_std_compare.bench.cpp` (planned) provide the measurement
framework.  Each optimization should produce a before/after table for the
relevant benchmark category.
