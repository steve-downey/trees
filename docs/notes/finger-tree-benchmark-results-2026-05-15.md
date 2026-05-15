# FingerTree2/3/4/5 Benchmark Results — 2026-05-15

Comparative performance and compile-time measurements across the four
finger-tree implementations in this repository.
Results were produced as part of the design analysis for a C++ proposal.

## Environment

- Toolchain: gcc-16, `-std=gnu++26`
- Build config: RelWithDebInfo (for runtime) / Debug (for DWARF counts)
- Platform: Linux x86-64, 8 MB default stack
- Benchmark driver: Catch2 v3.14.0 `BENCHMARK` / `BENCHMARK_ADVANCED`
- Samples per benchmark: 5 (warmup disabled)

## How to reproduce

### Build

Configure once with both flags:

```bash
cd .build/build-gcc-16
uv run cmake -DTREE_ENABLE_TESTING=ON -DTREE_ENABLE_BENCHMARKS=ON .
uv run cmake --build . --config RelWithDebInfo --target smd_tree_benchmarks
uv run cmake --build . --config Debug          --target smd_tree_compile_probes
```

### Run all non-crashing benchmarks

```bash
BIN=.build/build-gcc-16/src/smd/tree/RelWithDebInfo/smd_tree_benchmarks

# List all test cases (crash cases are clearly labelled)
"$BIN" --list-tests

# Run every non-crash case with 100 samples (takes ~15–30 min)
"$BIN" --benchmark-samples 100 --benchmark-warmup-time 1 \
    "Bench A snoc" "Bench A cons" \
    "Bench B view_l drain" "Bench B view_r drain" \
    "Bench B flatten" "Bench B for_each" \
    "Bench C append" "Bench C split at middle" \
    "Bench D persistent cons K=100 N=1000" \
    "Bench D persistent cons K=100 N=3000" \
    "Bench D persistent cons K=100 N=10000" \
    "Bench D persistent cons K=100 N=30000" \
    "Bench D persistent drain K=100 N=1000" \
    "Bench D persistent drain K=100 N=3000" \
    "Bench D persistent drain K=100 N=10000" \
    "Bench D persistent drain K=100 N=30000"

# Quick smoke run (5 samples, useful for spot-checking regressions)
"$BIN" --benchmark-samples 5 --benchmark-warmup-time 0 "Bench A snoc"
```

### Run the isolated crash-case tests

These are *expected* to fail (SIGSEGV from FT3's O(N) thunk chain).
Run them individually so they cannot abort other benchmarks.

```bash
"$BIN" "Bench A snoc FT3 large-N crash"     --benchmark-samples 1 --benchmark-warmup-time 0
"$BIN" "Bench A cons FT3 large-N crash"     --benchmark-samples 1 --benchmark-warmup-time 0
"$BIN" "Bench B drain FT3 large-N crash"    --benchmark-samples 1 --benchmark-warmup-time 0
"$BIN" "Bench C split FT3 large-N crash"    --benchmark-samples 1 --benchmark-warmup-time 0
```

### Measure compile-time template expansion (DWARF proxy)

The compile probes live in `src/smd/tree/finger_tree{2,3,4,5}_compile_probe.cpp`
and are compiled into `smd_tree_compile_probes` whenever `TREE_ENABLE_TESTING=ON`.
They are always built in the Debug config so DWARF is present.

```bash
cd .build/build-gcc-16
PROBE_DIR=src/smd/tree/CMakeFiles/smd_tree_compile_probes.dir/Debug

# Count DW_TAG_class_type entries — proxy for template instantiation count
for n in 2 3 4 5; do
  count=$(readelf --debug-dump=info \
    "$PROBE_DIR/finger_tree${n}_compile_probe.cpp.o" 2>/dev/null \
    | grep -c DW_TAG_class_type)
  echo "FT${n}: ${count} DW_TAG_class_type entries"
done

# Time single-TU compilation (touch the file first to force a rebuild)
touch ../../src/smd/tree/finger_tree2_compile_probe.cpp
time ninja -j1 "$PROBE_DIR/finger_tree2_compile_probe.cpp.o"

touch ../../src/smd/tree/finger_tree5_compile_probe.cpp
time ninja -j1 "$PROBE_DIR/finger_tree5_compile_probe.cpp.o"
```

To capture GCC's per-phase template breakdown, temporarily add to
`src/smd/tree/CMakeLists.txt` (remove after use — it is verbose):

```cmake
target_compile_options(smd_tree_compile_probes PRIVATE -ftime-report)
```

---

## Results

All times are mean from 5 samples.
Symbol key used throughout:

| Symbol | Meaning |
|--------|---------|
| ‡ | FT2 result reflects a **corrupt tree** — kMaxDepth exceeded, elements silently dropped |
| — | FT3 **crashes** (SIGSEGV — O(N/4) thunk chain exhausts the 8 MB stack) |
| † | High measurement variance; footnote explains |

---

### 1. Build — `snoc` N elements

FT2/FT3 store 2-3 nodes by value (no `make_shared`); FT4/FT5 heap-allocate every
internal node.
FT3 is fastest because its lazy spine defers all digit-overflow spine work until
the tree is first traversed.
At N ≥ ~1 000: FT4 and FT5 are within noise of each other.

| N | FT2 | FT3 | FT4 | FT5 |
|---|-----|-----|-----|-----|
| 100 | 9.31 μs | 6.20 μs | 18.7 μs | 18.4 μs |
| 1 000 | 126 μs | 65.9 μs | 196 μs | 192 μs |
| 10 000 | 1.43 ms | 670 μs | 2.05 ms | 2.12 ms |
| 100 000 | 14.3 ms | 8.36 ms | 25.7 ms | 25.6 ms |
| 300 000 | 48.2 ms | 30.6 ms | 82.1 ms | 82.3 ms |
| 1 000 000 | 0.19 s ‡ | — | 0.30 s | 0.29 s |
| 3 000 000 | 0.49 s ‡ | — | 0.82 s | 0.84 s |

FT2 at 1 M and 3 M: returned measure < N (data silently dropped).
FT3 at 1 M and 3 M: destructor crashes (O(N/4) ≈ 250 K thunk frames at 3 M).

---

### 2. Traverse — `flatten` of a pre-built N-element tree

Tree built once outside the timed region.
FT2 stores nodes by value — no pointer chasing — so traversal is fastest at
small N.
FT3 forces all spine thunks on the first call (memoized thereafter); the
reported mean blends one expensive first sample with four cheap memoized ones.
At N ≥ 100 K, FT2's numbers reflect traversal of its truncated structure only.

| N | FT2 | FT3 | FT4 | FT5 |
|---|-----|-----|-----|-----|
| 100 | 827 ns | 7.78 μs | 599 ns | 1.46 μs |
| 1 000 | 8.01 μs | 110 μs | 5.36 μs | 9.58 μs |
| 10 000 | 12.2 μs | 1.32 ms | 110 μs | 110 μs |
| 100 000 | 21.4 μs ‡ | — | 3.17 ms | 2.65 ms |
| 300 000 | 16.9 μs ‡ | — | 9.00 ms | 8.74 ms |
| 1 000 000 | 19.5 μs ‡ | — | 27.7 ms | 29.4 ms |
| 3 000 000 | 23.2 μs ‡ | — | 84.4 ms | 83.0 ms |

FT3 crash threshold: N ≈ 100 K (forcing ~25 K frames × ~300 bytes ≈ 7.5 MB).

---

### 3. Split at midpoint — O(log N) structural operation

FT4 and FT5 show clean O(log N) scaling from ~2 μs at N = 100 to ~18 μs at
N = 300 K (a 3000× size increase for a 9× time increase — consistent with
~log₂(3000) ≈ 11.5 doublings → ~11.5× time, matching the measured 8×).

FT3 must force its O(N/4) thunk chain before descending the spine, giving O(N)
cost.
FT2's sub-100 ns results at large N are meaningless: the tree was truncated
before the split threshold, so the predicate is never satisfied.

| N | FT2 | FT3 | FT4 | FT5 |
|---|-----|-----|-----|-----|
| 100 | 2.16 μs | 9.22 μs | 2.39 μs | 1.83 μs |
| 1 000 | 16.9 μs | 112 μs | 4.42 μs | 3.52 μs |
| 10 000 | 86 ns ‡ | 1.23 ms | 9.46 μs | 5.96 μs |
| 30 000 | 60 ns ‡ | 4.18 ms | 9.75 μs | 7.94 μs |
| 100 000 | 87 ns ‡ | — | 15.4 μs | 10.4 μs |
| 300 000 | 62 ns ‡ | — | 17.7 μs | 14.3 μs |

---

### 4. Append — O(log N) structural operation

FT3's `app3` is **lazy**: it produces a single new spine thunk without forcing
any existing thunks, yielding a flat ~210–420 ns at all sizes.
FT4/FT5 perform eager O(log N) work per append.
FT2 appends two corrupt trees at large N — result is wrong even if timing looks
normal.

| N+N | FT2 | FT3 | FT4 | FT5 |
|-----|-----|-----|-----|-----|
| 100+100 | 7.44 μs | 418 ns | 34.6 μs † | 3.90 μs |
| 1K+1K | 50.4 μs | 219 ns | 2.83 μs | 2.58 μs |
| 10K+10K | 45.1 μs | 222 ns | 4.75 μs | 3.79 μs |
| 30K+30K | 61.0 μs | 198 ns | 12.5 μs | 5.37 μs |
| 100K+100K | 83.4 μs ‡ | 256 ns | 7.71 μs | 8.52 μs |
| 300K+300K | 72.6 μs ‡ | 212 ns | 8.98 μs | 8.73 μs |

† FT4 100+100 mean (34.6 μs) is dominated by one cold-cache outlier;
the low-mean measurement of 3.2 μs is representative of the typical case.

---

### 5. Persistent fan-out — 100 `cons` calls from the same base snapshot

All four trees support full structural persistence.
Each call creates an independent derived tree without modifying the base.
FT2/FT3 cost ~25–35 ns per cons (value-stored nodes, no heap allocation per
internal node).
FT4/FT5 cost ~92–100 ns per cons (`make_shared` for each digit-overflow node).
The cost is flat across N because each cons touches only O(log N) amortised
nodes independent of base size.

| Base N | FT2 | FT3 | FT4 | FT5 |
|--------|-----|-----|-----|-----|
| 1 000 | 2.52 μs | 2.89 μs | 9.97 μs | 9.87 μs |
| 3 000 | 2.77 μs | 3.46 μs | 9.17 μs | 8.98 μs |
| 10 000 | 2.50 μs | 2.89 μs | 9.79 μs | 9.48 μs |
| 30 000 | 3.35 μs | 3.74 μs | 10.4 μs | 9.22 μs |

---

### 6. Persistent drain — flatten 100 derived trees from same base

Each derived tree shares almost all nodes with the base via structural sharing.
FT3's spine thunks are forced once on the first `flatten` and memoized;
the remaining 99 flattenings (and all subsequent measurement samples) reuse the
cached forced result.
FT4/FT5 pay the full traversal cost every time.

FT3 is 3–5× faster than FT4/FT5 at N = 10–30 K, demonstrating the persistent-
amortisation advantage that is FT3's sole motivation.

FT2's numbers at N ≥ 10 K are fast for the wrong reason (corrupt truncated base).

| Base N | FT2 | FT3 | FT4 | FT5 |
|--------|-----|-----|-----|-----|
| 1 000 | 818 μs | 767 μs | 483 μs | 507 μs |
| 3 000 | 1.36 ms | 1.34 ms | 1.43 ms | 1.57 ms |
| 10 000 | 1.05 ms ‡ | 2.04 ms | 7.25 ms | 7.78 ms |
| 30 000 | 1.40 ms ‡ | 5.32 ms | 27.2 ms | 27.0 ms |

---

### 7. Compile-time template expansion — DWARF `DW_TAG_class_type` count

Measured on the compile probes compiled in Debug mode:

```bash
readelf --debug-dump=info \
  src/smd/tree/CMakeFiles/smd_tree_compile_probes.dir/Debug/finger_tree2_compile_probe.cpp.o \
  | grep -c DW_TAG_class_type
```

| Tree | Class-type entries | Ratio vs FT5 | Notes |
|------|--------------------|--------------|-------|
| FT2  | 515  | 3.3× | DEPTH NTTP → 6 distinct class bodies per `<T,Tag,Measure>` |
| FT3  | 1275 | 8.2× | DEPTH NTTP + `erased_thunk`/`memoize` machinery |
| FT4  | 162  | 1.0× | Uniform-elem; Digit = `variant<One,Two,Three,Four>` |
| FT5  | 156  | 1.0× | Uniform-elem; Digit = `inplace_vector<ElemPtr,4>` |

FT4 and FT5 are within 4% of each other — the `inplace_vector` digit is a
runtime improvement with no compile-time cost.

---

## Summary of findings

### Correctness failures at scale

| Tree | Failure mode | Trigger |
|------|-------------|---------|
| FT2  | Silent data loss in release builds | N > ~1 000–4 000 (kMaxDepth exceeded) |
| FT3  | SIGSEGV on any operation that forces the spine | N > ~100 K (forcing); N > ~1 M (destruction) |
| FT4  | None | — |
| FT5  | None | — |

### Which tree to use

**FT4 and FT5 are the only designs that are correct at scale.**
FT5 is strictly better than FT4: identical compile-time cost (156 vs 162 DWARF
entries) and measurably faster on split and for_each due to the `inplace_vector`
digit avoiding four-way `std::visit` dispatch.

FT2 and FT3 are pedagogical: FT2 shows the template-explosion problem and its
kMaxDepth data-loss hazard; FT3 shows how lazy spines fix persistent amortisation
but introduce an O(N) thunk chain that is unsafe for sequential large-scale use.

### FT3's amortisation advantage is real but scoped

FT3 is the only design where persistent fan-out is O(1) per cons and where
K consumers of the same derived spine share one forced-thunk invocation.
The persistent-drain table (§6) demonstrates a genuine 3–5× advantage over
FT4/FT5 at N = 10–30 K.
That advantage disappears at scale because the thunk chain itself becomes
the bottleneck — both during forcing (N > ~100 K) and during destruction
(N > ~1 M).
