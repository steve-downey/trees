# Plan: Benchmark Comparing FT5 Wrappers Against std Library Types

## Context

The comparison document (`docs/notes/std-comparison-finger-tree-wrappers.md`)
makes complexity claims.  This plan specifies benchmarks that substantiate
those claims with measured wall-clock times, revealing the actual crossover
points where FT5 beats std (and vice versa).

## Output

A single new benchmark file:
`src/smd/tree/finger_tree_std_compare.bench.cpp`

Wired into the existing `smd_tree_benchmarks` target (already under
`TREE_ENABLE_BENCHMARKS`).

---

## Design principles

1. **Apples-to-apples**: each benchmark performs the exact same logical
   operation on both the FT5 wrapper and the std equivalent, measuring only
   the operation itself (setup outside `meter.measure`).

2. **Size axis**: every benchmark runs at N = 100, 1 000, 10 000, 100 000.
   For operations that are O(log N) vs O(N), the crossover is expected
   between 100 and 10 000.

3. **No persistence penalty on std**: when benchmarking a "pure mutation"
   operation (e.g. insert-at-middle), the std benchmark mutates in-place.
   The FT5 benchmark returns a new persistent value (the natural API).
   This is an honest comparison: std pays the in-place cost; FT5 pays the
   persistence cost.  A separate "persistence" category measures the cost
   of keeping K snapshots, which std cannot do cheaply.

4. **BENCHMARK_ADVANCED throughout**: pre-build the data structure in
   setup, measure only the timed operation.  Return a value to prevent
   dead-code elimination.

5. **Naming convention**: `"<Wrapper> vs <std> - <operation> N=<size>"`
   e.g. `"RandomAccess vs vector - insert_mid N=10000"`.

---

## Category 1: RandomAccess vs std::vector / std::deque

### Benchmarks

For each N in {100, 1000, 10000, 100000}:

| Benchmark name | FT5 operation | std operation |
|----------------|---------------|---------------|
| `push_back N` | `seq.push_back(x)` repeated N times from empty | `vec.push_back(x)` repeated N times |
| `push_front N` | `seq.push_front(x)` repeated N times from empty | `deque.push_front(x)` repeated N |
| `at_random N` | `seq.at(random_index)` measured K=1000 times on pre-built N | `vec[random_index]` same |
| `insert_mid N` | `seq.insert(N/2, x)` on pre-built N-element seq | `vec.insert(vec.begin()+N/2, x)` on pre-built N vec |
| `erase_mid N` | `seq.erase(N/2)` on pre-built N | `vec.erase(vec.begin()+N/2)` on pre-built N |
| `concat N` | `seq_a.append(seq_b)` (N/2 each) | new vector + copy both halves |
| `iterate_sum N` | `for (auto x : seq) sum += x` via FT5 iterator | `for (auto x : vec) sum += x` |
| `snapshot N` | `auto copy = seq` (O(1) — shared_ptr copy) | `auto copy = vec` (O(N) deep copy) |

### What to measure

- `push_back`: FT5 should be similar to vector at all sizes (both O(1) amort).
  Expected: FT5 2–4× slower due to `make_shared` per overflow.
- `push_front`: FT5 should match deque; vector is O(N) and will be much worse.
- `at_random`: vector wins by 10–100× (O(1) vs O(log N) + pointer chasing).
- `insert_mid`: FT5 wins at N ≥ ~1000 (O(log N) vs O(N) memmove).
- `erase_mid`: same as insert.
- `concat`: FT5 wins at all N (O(log N) vs O(N) copy).
- `iterate_sum`: vector wins by 10–50× (cache effects).
- `snapshot`: FT5 is O(1) (trivial), vector is O(N) — FT5 wins enormously.

### Helper code

```cpp
// Pre-build a std::vector<int> of size n
auto make_vector(int n) -> std::vector<int> {
    std::vector<int> v(n);
    std::iota(v.begin(), v.end(), 0);
    return v;
}

// Pre-build a FingerTreeRandomAccess<int> of size n
auto make_ft5_seq(int n) -> smd::tree::FingerTreeRandomAccess<int> {
    std::vector<int> v(n);
    std::iota(v.begin(), v.end(), 0);
    return smd::tree::FingerTreeRandomAccess<int>::from_sequence(std::move(v));
}
```

---

## Category 2: Rope vs std::string

### Benchmarks

For each N in {1000, 10000, 100000, 1000000} (bytes):

| Benchmark name | FT5 operation | std operation |
|----------------|---------------|---------------|
| `insert_mid N` | `rope.insert(N/2, "hello")` on pre-built N-byte rope | `str.insert(N/2, "hello")` on pre-built N-byte string |
| `erase_mid N` | `rope.erase(N/2, 5)` on pre-built N-byte rope | `str.erase(N/2, 5)` on pre-built N string |
| `concat N` | `rope_a.insert(rope_a.size_bytes(), rope_b.to_string())` (both N/2) — OR if we add a rope.concat method: direct concat | `str_a + str_b` |
| `build N` | `Rope::from_text(text, 64)` from N-byte string | already have the string (O(1) — just assign) |
| `iterate_full N` | `rope.to_string()` (materialise) | iterate `str` directly (O(N), no materialise needed) |
| `snapshot N` | `auto copy = rope` (O(1)) | `auto copy = str` (O(N)) |
| `undo_stack K=100 N` | Create rope, perform K insert-at-middle ops keeping all K versions alive.  Measure total memory / time. | Same with string: K copies. |

### What to measure

- `insert_mid`: FT5 dominates for N ≥ 10 000.  Crossover expected ~1 000–5 000.
- `erase_mid`: same as insert.
- `concat`: FT5 wins at all N > ~100.
- `build`: string "wins" trivially (it's already built); the benchmark shows
  the cost of chunking for the rope.
- `iterate_full`: string wins (already contiguous vs materialisation cost).
- `snapshot`: the killer feature — O(1) vs O(N).
- `undo_stack`: this is the money benchmark.  K=100 undoable edits on an
  N-byte document.  FT5 total memory ≈ K × O(log N).  String total memory ≈
  K × O(N).  At N=100 000 and K=100: FT5 ≈ 100 × 20 nodes ≈ negligible;
  string ≈ 100 × 100 KB = 10 MB.

---

## Category 3: PriorityQueue vs std::priority_queue

### Benchmarks

For each N in {100, 1000, 10000, 100000}:

| Benchmark name | FT5 operation | std operation |
|----------------|---------------|---------------|
| `push N` | Push N random ints from empty | `std::priority_queue<int>` push N random ints |
| `pop_min_all N` | Pop all N elements via pop_min from pre-built N | `std::priority_queue<int, vec, std::greater>` pop all N |
| `pop_max_all N` | Pop all N elements via pop_max from pre-built N | `std::priority_queue<int>` (default max-heap) pop all N |
| `push_then_pop_min N` | Interleaved: push 2, pop_min 1, repeated N times | Same interleaved pattern with std::priority_queue |
| `merge N` | Merge two N/2-element FT5 PQs via push onto one from the other — NO.  Actually: there's no merge wrapper method currently.  Use the raw tree: `pq1_tree.append(pq2_tree)`.  But the wrapper doesn't expose the raw tree... Skip this or add a merge method. | Build N/2 + N/2 queues, push all of one into the other = O(N log N) |

Note: the PQ wrapper doesn't currently expose `merge`.  Either skip the merge
benchmark or add a `merge` method to the wrapper first.  The plan should note
this as a known gap — the benchmark file should include a TODO comment for
merge when the method exists.

### What to measure

- `push`: FT5 O(1) amortized vs std O(log N).  FT5 should be faster at large
  N, but may lose at small N due to allocation overhead.
- `pop_min_all` / `pop_max_all`: both O(N log N) total.  The constant factor
  difference (cache vs pointer) determines the winner.
- `push_then_pop`: realistic workload.  Shows amortised behaviour under mixed
  operations.

---

## Category 4: IntervalIndex vs std::multimap (brute-force baseline)

### Benchmarks

For each N in {100, 1000, 10000, 100000}:

| Benchmark name | FT5 operation | Baseline operation |
|----------------|---------------|--------------------|
| `build N` | Insert N random intervals via `.insert()` from empty | Insert N intervals into a `std::vector<Interval>` (unsorted) |
| `query_point N` | `idx.query_point(random_point)` on pre-built N | Linear scan of vector checking overlap |
| `query_point_10 N` | 10 consecutive point queries | 10 linear scans |
| `query_overlap N` | `idx.query_overlap(lo, hi)` on pre-built N | Linear scan checking overlap |
| `insert_then_query N` | Build N/2 index, then interleave N/2 inserts with queries | Same with vector: push_back + linear scan |

### What to measure

- `build`: both O(N) for the FT5 index (N × O(1) amort insert) vs O(N) for
  the vector.  FT5 is slower per-element (allocation overhead).
- `query_point`: FT5 O(log N + k) vs brute-force O(N).  FT5 should win
  dramatically at N ≥ 1000.
- `query_overlap`: same.
- `insert_then_query`: realistic workload showing incremental index advantage.

---

## Category 5: Persistence (the cross-cutting advantage)

This category doesn't compare a specific wrapper — it compares the COST OF
PERSISTENCE across all container types.

### Benchmarks

For N = 10 000 and K in {1, 10, 100}:

| Benchmark name | What it measures |
|----------------|------------------|
| `snapshot_cost_ft5_seq K N` | Create base RandomAccess, produce K snapshots by inserting one element each.  Measure time for the K insertions (each yields a new persistent version). |
| `snapshot_cost_vector K N` | Create base vector, produce K snapshots by copying and inserting one element.  Measure time for the K copy+insert operations. |
| `snapshot_cost_ft5_pq K N` | Same pattern with PQ: K pushes from same base. |
| `snapshot_cost_std_pq K N` | K copies of std::priority_queue + one push each. |
| `all_versions_alive K N` | After creating K FT5 versions, iterate all K (sum their sizes).  Measures memory sharing efficiency: all K trees should be alive simultaneously. |
| `all_versions_alive_vec K N` | Same with K vector copies.  Measures O(K×N) memory. |

### What to measure

- At K=1: roughly equivalent (one copy vs one persistent step).
- At K=10: FT5 should be ~10× cheaper than vector (10 path copies vs 10 full copies).
- At K=100: FT5 should be ~100× cheaper.

This category is the strongest argument for the proposal: it shows that the
"cost of persistence" with finger trees is O(K × log N), while the std
alternative is O(K × N).

---

## Category 6: Sequential iteration (where std wins)

This category exists to honestly show the scenario where std::vector is
unbeatable.  Including it in the benchmark suite demonstrates intellectual
honesty and helps users know when NOT to use FT5.

For N in {1000, 10000, 100000, 1000000}:

| Benchmark | FT5 | std |
|-----------|-----|-----|
| `sum_iterate N` | `int sum = 0; for (auto x : ft5_seq) sum += x;` | `int sum = 0; for (auto x : vec) sum += x;` |
| `transform_iterate N` | iterate FT5, push results to a vector | iterate vec, push results to a vector |
| `find_element N` | `std::ranges::find(ft5_seq, target)` | `std::find(vec.begin(), vec.end(), target)` (target at midpoint) |

### Expected results

- `sum_iterate`: vector 10–50× faster (cache contiguous).
- `transform_iterate`: vector faster (same reason).
- `find_element`: vector faster for sequential scan; FT5 bidirectional
  iterator has higher per-step cost.

---

## Implementation structure

```
src/smd/tree/finger_tree_std_compare.bench.cpp
```

The file should:
1. Include: `<smd/tree/finger_tree_random_access.hpp>`,
   `<smd/tree/finger_tree_rope.hpp>`,
   `<smd/tree/finger_tree_priority_queue.hpp>`,
   `<smd/tree/finger_tree_interval_index.hpp>`,
   `<vector>`, `<deque>`, `<queue>`, `<string>`, `<numeric>`, `<algorithm>`,
   `<random>`,
   `<catch2/benchmark/catch_benchmark_all.hpp>`,
   `<catch2/catch_test_macros.hpp>`

2. Use an anonymous namespace with helpers: `make_vector(n)`, `make_ft5_seq(n)`,
   `make_string(n)`, `make_rope(n)`, `make_random_intervals(n)`, etc.

3. One `TEST_CASE` per category:
   - `"Bench Std: RandomAccess vs vector"`
   - `"Bench Std: Rope vs string"`
   - `"Bench Std: PQ vs std::priority_queue"`
   - `"Bench Std: IntervalIndex vs brute-force"`
   - `"Bench Std: Persistence cost"`
   - `"Bench Std: Sequential iteration (std wins)"`

4. Within each TEST_CASE, BENCHMARK_ADVANCED entries named per the tables
   above, grouped by size.

5. Use a fixed random seed (`std::mt19937 rng{42}`) so results are
   reproducible across runs.

## CMakeLists change

Add `finger_tree_std_compare.bench.cpp` to the existing
`smd_tree_benchmarks` target (already under `TREE_ENABLE_BENCHMARKS`):

```cmake
add_executable(
    smd_tree_benchmarks
    finger_tree_compare.bench.cpp
    finger_tree_std_compare.bench.cpp   # NEW
)
```

## Running

```bash
# Build
cd .build/build-gcc-16
uv run cmake --build . --config RelWithDebInfo --target smd_tree_benchmarks

# Run one category at a time
BIN=src/smd/tree/RelWithDebInfo/smd_tree_benchmarks
"$BIN" "Bench Std: RandomAccess vs vector" --benchmark-samples 20 --benchmark-warmup-time 1
"$BIN" "Bench Std: Rope vs string" --benchmark-samples 20 --benchmark-warmup-time 1
"$BIN" "Bench Std: PQ vs std::priority_queue" --benchmark-samples 20
"$BIN" "Bench Std: IntervalIndex vs brute-force" --benchmark-samples 20
"$BIN" "Bench Std: Persistence cost" --benchmark-samples 20
"$BIN" "Bench Std: Sequential iteration (std wins)" --benchmark-samples 50
```

## Known gaps / TODOs for implementer

1. **PQ merge**: the wrapper doesn't expose a `merge` method.  Include a
   TODO comment in the benchmark; implement the benchmark once merge exists.

2. **Rope concat**: the wrapper doesn't expose a direct `rope.concat(other_rope)`
   — insert at end is equivalent.  Use `rope.insert(rope.size_bytes(), other.to_string())`
   for now, noting this materialises the rhs (unfair to rope).  A future
   `concat(rope, rope)` method would be the fair comparison.

3. **Deque**: include deque only for push_front and iterate benchmarks.
   For insert/erase/concat, deque and vector are equivalent (both O(N)).

4. **Memory measurement**: Catch2 benchmarks measure time, not memory.
   The persistence category measures time-to-create-K-snapshots as a proxy.
   Actual memory measurement would require a custom allocator or /proc/self/status
   snapshots — note this as future work, don't implement in v1.

## Verification

After implementation:
- Build succeeds with -DTREE_ENABLE_BENCHMARKS=ON
- Each category runs without crash at all sizes
- Results are consistent with the complexity claims in the comparison doc
- "Bench Std: Sequential iteration (std wins)" shows vector winning (honesty check)
- "Bench Std: Persistence cost" shows FT5 winning by K× at large K
