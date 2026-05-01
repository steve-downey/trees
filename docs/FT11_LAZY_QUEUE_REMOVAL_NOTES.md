# FT11 Lazy Queue Removal Implementation Notes

## Overview

This document captures implementation challenges and solutions encountered during FT11 ("strict finger tree with lazy middle" queue removal phase), particularly around restoring lazy split/concat removal paths while maintaining memory safety and deterministic correctness.

## Problem Summary

The `FingerTreePriorityQueue` wrapper maintains dual balanced finger trees (MinTree and MaxTree) to support both `pop_min()` and `pop_max()` operations in $ O(\log n) $ time. When an element is popped (e.g., by value from MinTree), the same element must be removed from MaxTree to maintain consistency.

**Core Issue:** Lazy removal via split/concat is safe on the matching-measure tree but causes undefined behavior (SEGV) when applied cross-measure (e.g., using MinTree's split predicate on MaxTree).

## Why Cross-Measure Split Fails

### Root Cause

Measure-guided split in finger trees works by comparing a predicate result with measured intervals:

```cpp
auto split = tree.split([&needle](const MinTag<T>& prefix) {
  return prefix.d_value.has_value() && prefix.d_value.value() == needle;
});
```

This predicate produces `MinTag<T>` results that are combined according to the `MinTag` monoid (min-based order). When the same split predicate is applied to MaxTree:

1. **Predicate mismatch**: The predicate still produces `MinTag<T>`, but MaxTree combines them using the `MaxTag` monoid (max-based order).
2. **Measure incoherence**: The tree traversal follows measure-guided comparisons that no longer align with the structure of the tree.
3. **Out-of-bounds access**: Misaligned traversal can lead to accessing uninitialized memory or invalid tree structure pointers.

### Example Failure Scenario

```
MinTree structure:  [2, 5, 8, 2, 7]  (min-ordered by MinTag)
Removing element 8 via split on MinTree: OK (measure aligns)

MaxTree structure:  [2, 5, 8, 2, 7]  (max-ordered by MaxTag)
Removing element 8 via MinTree split:   SEGV (measures misaligned)
```

The maximal element (8) is positioned differently in MaxTree's internal structure than in MinTree's structure. Applying MinTree's split logic produces traversal decisions based on incorrect measure comparisons.

## Solution Implemented: Deterministic Rebuild Path

### Strategy

Instead of lazy split/concat removal across measures, use a simpler (but non-lazy) approach:

```cpp
template <typename TREE>
static auto remove_one_rebuild(const TREE& tree, const T& needle) -> TREE
{
  auto values = tree.flatten();
  auto it = std::find(values.begin(), values.end(), needle);
  if (it == values.end()) {
    return tree;
  }
  values.erase(it);
  return TREE::from_sequence(std::move(values));
}
```

### Advantages

1. **Correctness**: Works uniformly for both MinTree and MaxTree.
2. **Safety**: No measure-related traversal complexity; straightforward linear search.
3. **Stability**: Allows accumulation of other optimizations without cross-cutting concerns.
4. **Determinism**: Predictable behavior aids debugging and testing.

### Trade-offs

- **Not lazy**: Rebuilds both trees fully on each pop, defeating the laziness goal of FT11.
- **Performance**: $ O(n) $ per removal vs. intended $ O(\log n) $.
- **Memory**: Temporary allocation of flattened sequence during rebuild.

### Why This Is a Valid Interim Solution

The deterministic path is the correct fallback used in the main branch (commit 0e755f90). It has been validated over many test runs and serves as a stable checkpoint for future lazy optimization attempts.

## Testing & Validation

### Stress Regression Test

Added `RepeatedPushPopMatchesMultiset` (250 iterations):

```cpp
// Alternates push and pop_min/pop_max, validates results against std::multiset
for (int i = 0; i < 250; ++i) {
  auto value = (i * 7) % 11;
  q = q.push(value);
  expected.insert(value);

  if ((i % 2) == 0) {
    auto popped = q.pop_min();
    CHECK(popped->first == *expected.begin());
    expected.erase(expected.begin());
  } else {
    auto popped = q.pop_max();
    CHECK(popped->first == *std::prev(expected.end()));
    expected.erase(std::prev(expected.end()));
  }
}
```

This test catches logical divergence between queue and reference semantics.

### Validation Results

- **Debug**: 163/163 tests pass
- **Asan with leak detection**: 163/163 tests pass
- **Persistence shared-version test**: Passes (validates lazy thunk behavior in other contexts)

## Memory Safety Notes for Future Work

### OOM Prevention

1. **Build in parallel carefully**:
   - Use `-j1` for ASAN builds; parallel ASAN instrumentation multiplies memory overhead.
   - Parallel Debug builds can use `-j4` or higher safely on typical dev machines.

2. **Run tests with timeouts**:
   ```bash
   timeout 20s ./smd_tree_tests "FingerTreePriorityQueueTest - RepeatedPushPopMatchesMultiset" -r compact
   ```
   Prevents runaway processes from consuming all system memory.

3. **Monitor memory during long test runs**:
   - Watch `top` or `ps` output.
   - If resident memory (RSS) exceeds 2–3 GB during Asan runs, investigate for unbounded allocation.
   - If it reaches 10+ GB, kill the process immediately (can bring down WSL/system).

### ASAN Configuration

**DO NOT** combine ASAN with `ulimit -v`:

```bash
# WRONG: ulimit breaks ASAN shadow mapping
ulimit -v 2000000
./smd_tree_tests  # Will fail with Asan initialization errors
```

**DO** use Asan options for containment:

```bash
export ASAN_OPTIONS=detect_leaks=1:halt_on_error=1:abort_on_error=1
export LSAN_OPTIONS=exitcode=23:report_objects=1
./smd_tree_tests
```

### Debugging Crashes Without gdb Loops

If a test crashes deterministically:

1. **Run once with output capture**:
   ```bash
   ./smd_tree_tests "TestName" 2>&1 | tee crash.log
   ```

2. **Inspect error message carefully**: ASAN messages are usually verbose and self-contained.

3. **Avoid `gdb` backtrace loops**: Repeatedly running `gdb bt` can cause memory thrashing with ASAN.

4. **Use source inspection + test variation**:
   - Modify test parameters (e.g., iteration count, seed) to narrow failure conditions.
   - Add assertions in suspected code paths.
   - Recompile and rerun with timeout.

## Recommendations for Future Implementors

### Next Steps for Lazy Removal

To restore the original lazy split/concat removal intent:

1. **Investigate measure alignment** at the split site. Ensure predicates and monoid operations align before applying split to a tree.

2. **Consider measure-bridging helpers**:
   ```cpp
   // Hypothetical: convert MinTag predicate to MaxTag predicate
   auto max_predicate = [&](const MaxTag<T>& prefix) {
     // Reconstruct element from prefix measure?
     // Or use orthogonal search strategy?
   };
   ```

3. **Asymmetric removal strategy**:
   - Use split on matching-measure tree (MinTree for min removal).
   - Use linear rebuild on opposite-measure tree.
   - Hybrid approach avoids cross-measure split while recovering some laziness.

4. **Validate with stress tests**:
   - Keep `RepeatedPushPopMatchesMultiset` or expand it.
   - Add persistence tests (shared versions under pop operations).
   - Run under Asan + lsan to catch leaks early.

### Code Review Checklist

When reviewing queue removal changes:

- [ ] Pop operations call consistent removal helpers for both MinTree and MaxTree.
- [ ] Stress test (250+ iterations) passes without memory growth.
- [ ] Asan+lsan shows zero leak detections.
- [ ] Persistence test (RepeatedSplitPopAcrossSharedVersions) passes.
- [ ] No cross-measure split/concat without explicit measure-bridging logic.

### Documentation

Document any new removal strategy in the code:

```cpp
// SAFETY: This removal uses deterministic rebuild, not lazy split/concat.
// Lazy split/concat fails cross-measure due to monoid misalignment.
// Future: Investigate measure-bridging predicates or asymmetric removal.
static auto remove_one_rebuild(const TREE& tree, const T& needle) -> TREE { ... }
```

## References

- **Finger tree measure-guided split**: Pairing measure predicates with monoid combination is critical. Mixing measures breaks invariants.
- **Persistent data structure testing**: Shared-version tests (multiple active versions after pop) are essential for correctness validation.
- **Asan best practices**: Never use `ulimit -v` with Asan; use `*SAN_OPTIONS` environment variables for containment instead.
