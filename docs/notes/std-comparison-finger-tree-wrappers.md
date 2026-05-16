# Structured Comparison: FingerTree5 Wrappers vs Standard Library Types

This document compares the four FingerTree5-backed wrapper types in this
repository against the overlapping standard library containers, both positive
and negative.
It is intended for proposal reviewers and potential users evaluating
whether persistent finger trees are appropriate for their use-case.

---

## What FingerTree5 provides

FingerTree5 is a persistent (immutable, structurally-shared) 2-3 finger tree
implemented with a uniform `Elem<T,Tag>` variant and `shared_ptr<const ...>`
spine.
Every mutation returns a new tree; the old version is unchanged and valid.
Structural sharing via reference-counted pointers means that the "cost of
persistence" is O(log N) path copies, not O(N) full copies.

Four domain wrappers specialise the tree with different monoid measures:

| Wrapper | Measure | Primary use |
|---------|---------|-------------|
| `FingerTreeRandomAccess<T>` | Element count | Persistent indexed sequence |
| `FingerTreeRope` | Byte count | Persistent text buffer |
| `FingerTreePriorityQueue<T>` | Min/max tracking | Persistent double-ended PQ |
| `FingerTreeIntervalIndex<P>` | Max-endpoint | Persistent interval stabbing |

---

## 1. FingerTreeRandomAccess vs std::vector / std::deque

### Operation complexity

| Operation | FT5 RandomAccess | std::vector | std::deque |
|-----------|-----------------|-------------|------------|
| `push_back` | O(1) amort | O(1) amort | O(1) amort |
| `push_front` | O(1) amort | **O(N)** | O(1) amort |
| `at(i)` | O(log N) | **O(1)** | **O(1)** |
| `insert(i, v)` | **O(log N)** | O(N) | O(N) |
| `erase(i)` | **O(log N)** | O(N) | O(N) |
| `concat(a, b)` | **O(log N)** | O(N) | O(N) |
| `split_at(i)` | **O(log N)** | O(N) | O(N) |
| Persistent snapshot | **O(1)** | O(N) copy | O(N) copy |
| Full iteration | O(N) bidir | O(N) random | O(N) random |

### Where FingerTree5 wins

- **O(log N) structural edits**: insert, erase, concat, split are all
  sub-linear.
  For a 1M-element sequence, insert-at-middle is ~14 μs in FT5 vs ~500 μs
  in a vector (memcpy of half the array).
- **O(1) persistent snapshots**: creating a "checkpoint" of the full sequence
  is free.
  This enables undo/redo, speculative execution, and multi-version
  concurrency control without copying.
- **O(1) push_front**: deque also offers this, but vector does not.
- **No iterator invalidation**: iterators into an FT5 snapshot remain valid
  forever — the shared_ptr keepalive guarantees node lifetime.

### Where std types win

- **O(1) random access**: FT5's `at(i)` is O(log N) — for an inner loop
  that indexes every element, this is a strict constant-factor loss.
- **Cache performance**: std::vector stores elements contiguously; sequential
  iteration triggers hardware prefetching.
  FT5 follows `shared_ptr` chains with poor spatial locality.
  For pure-iteration workloads, vector is 10–100× faster.
- **No per-element heap allocation**: vector allocates once for the entire
  backing array; FT5 allocates a `shared_ptr<const Elem>` for every internal
  node.
- **Trivially serializable**: POD vectors can be `memcpy`'d; FT5 requires
  tree traversal to serialise.
- **Smaller memory footprint**: each FT5 element carries a shared_ptr control
  block (16–24 bytes overhead per node vs 0 for contiguous storage).

---

## 2. FingerTreeRope vs std::string

### Operation complexity

| Operation | FT5 Rope | std::string |
|-----------|----------|-------------|
| `insert(pos, text)` | **O(log N)** | O(N) |
| `erase(pos, count)` | **O(log N)** | O(N) |
| `concat` | **O(log N)** | O(N) |
| `size_bytes` | O(1) | O(1) |
| `operator[](i)` | O(log N) | **O(1)** |
| Persistent snapshot | **O(1)** | O(N) copy |
| Contiguous `c_str()` | O(N) materialise | **O(1)** |

### Where FingerTree5 wins

- **O(log N) insert/erase/concat**: for text buffers in an editor, these are
  the dominant operations.
  Editing a 10 MB document with std::string moves ~5 MB of memory per
  keystroke; a rope moves O(log N) ≈ 20–30 pointers.
- **Persistence = free undo stack**: each edit produces a new rope; the old
  rope IS the undo state.
  An undo stack of K edits costs O(K × log N) total memory, not O(K × N).
- **O(log N) concat**: joining two ropes (e.g., paste at cursor) is sub-linear.
  std::string must allocate a new buffer and copy both halves.

### Where std::string wins

- **O(1) character indexing**: ropes cannot offer `operator[]` in constant time.
- **SSO**: strings under ~22 bytes live entirely on the stack — zero heap
  traffic.
  For short strings, the overhead of creating tree nodes is absurd.
- **Cache-contiguous reads**: scanning a string for a pattern benefits from
  hardware prefetching on contiguous memory.
- **C API interop**: `c_str()` returns a contiguous pointer immediately.
  The rope must materialise into a temporary buffer (O(N)).
- **Simpler mental model**: std::string is a familiar vocabulary type.

### Verdict

Use the rope when the document is large (> ~1 KB) AND structural edits are
frequent AND persistence/undo is needed.
Use std::string for short strings, read-heavy workloads, and C API boundaries.

---

## 3. FingerTreePriorityQueue vs std::priority_queue

### Operation complexity

| Operation | FT5 PQ | std::priority_queue |
|-----------|--------|---------------------|
| `push` | **O(1) amort** | O(log N) |
| `pop_min` | O(log N) | O(log N) |
| `pop_max` | O(log N) | **not available** |
| `min()` | O(1) | O(1) (top only) |
| `max()` | **O(1)** | not available |
| `merge(pq1, pq2)` | **O(log N)** | O(N) |
| Persistent snapshot | **O(1)** | O(N) copy |
| `size()` | O(N) | O(1) |

### Where FingerTree5 wins

- **Double-ended**: access BOTH min and max in O(1), pop from either end in
  O(log N).
  std::priority_queue only exposes one end.
  A "min-max heap" exists but is not in the standard.
- **O(1) amortized push**: the finger tree's snoc is amortized O(1), beating
  the binary heap's O(log N) push.
- **O(log N) merge**: combining two priority queues is sub-linear.
  std::priority_queue has no merge operation — you'd push all elements of one
  into the other at O(N log N).
- **Persistence**: can "undo" a pop, try alternative scheduling, or keep
  multiple queue versions alive at no extra cost.

### Where std::priority_queue wins

- **O(1) size**: FT5's PQ uses a non-counting measure (PriorityTag), so
  `size()` is O(N).
  A production version could add a product measure to include a count.
- **Better constant factors**: the binary heap lives in a contiguous array;
  sift-up/sift-down operations touch adjacent memory.
  FT5's pointer-chasing per node is slower in practice for small N.
- **Vocabulary type**: `std::priority_queue` is universally understood.
- **Simpler API**: push/top/pop — no need to reason about persistence.

### Verdict

Use FT5 PQ when you need double-ended access, queue merging, or persistent
scheduling (job queues with rollback, A* with branch-and-bound, simulation
with time-travel).
Use std::priority_queue for simple single-ended scheduling where cache
performance matters.

---

## 4. FingerTreeIntervalIndex — no std equivalent

There is no standard-library container for interval indexing.
The closest alternatives and how they compare:

| Alternative | point query | overlap query | insert | persistent |
|-------------|-------------|---------------|--------|------------|
| **FT5 IntervalIndex** | O(log N + k) | O(log N + k) | O(1) amort | O(1) |
| std::multimap | O(log N) for one endpoint | O(N) scan | O(log N) | O(N) copy |
| Sorted vector + binary search | O(log N) | O(N) | O(N) | O(N) copy |
| Boost.ICL interval_map | O(log N) | O(log N + k) | O(log N) | O(N) copy |

The FT5 interval index is unique in offering all of:
- O(1) amortized insert
- O(log N + k) query with measure-guided subtree pruning
- Free persistence (old index versions survive new insertions)

Use cases: genomics (overlapping annotations), scheduling (resource conflicts),
computational geometry (window queries), text editors (overlapping syntax
highlights).

---

## 5. Cross-cutting properties

### 5a. Persistence and structural sharing

This is FingerTree5's core architectural advantage over all mutable standard
containers.

Every operation on an FT5 value returns a **new** value.
The old value remains unchanged and valid.
Because the new value shares most of its internal nodes with the old (via
`shared_ptr<const Elem>`), the cost of creating a new version is O(log N)
path copies — not O(N) full copies.

**Concrete consequence**: maintaining K historical versions of an N-element
structure costs O(K × log N) total memory, not O(K × N).

Applications where persistence dominates:
- **Undo/redo**: each edit IS the undo stack entry — no separate "undo buffer"
- **Speculative execution**: try a mutation, check if it's valid, roll back if not
- **Concurrent readers**: each thread holds its own snapshot — no mutex, no data race
- **Event sourcing**: reconstruct state at any point in history by replaying

### 5b. Iterator invalidation

| Container | When iterators are invalidated |
|-----------|--------------------------------|
| std::vector | Any reallocation (push_back past capacity), any insert/erase |
| std::deque | Any insert/erase |
| std::list | Only the erased element |
| std::priority_queue | (no iterators exposed) |
| **FingerTree5** | **Never** |

FT5 iterators hold a `shared_ptr` to the root of their snapshot.
This keeps every node reachable from that iterator alive for as long as the
iterator exists, regardless of what happens to the "live" version of the tree.

This eliminates an entire category of bugs: dangling iterators, invalidation
after insert, use-after-move.

### 5c. Concurrency safety

Immutable values are inherently thread-safe.
Multiple threads holding different FT5 snapshots can read, iterate, and query
them concurrently with **zero synchronisation** — no mutex, no reader-writer
lock, no atomic fence beyond the lock-free `shared_ptr` refcount.

In contrast, any std container shared across threads requires external
synchronisation.
Even read-only concurrent access to a `std::vector` requires a guarantee that
no writer exists — typically enforced by a reader-writer lock.

FT5's model: producers create new versions (single-threaded write path);
consumers read their own snapshot (concurrent, lock-free).
This is the "single-writer, multiple-reader" pattern with zero coordination
cost for readers.

### 5d. Memory layout and cache performance

This is FingerTree5's primary weakness.

| Aspect | std::vector | FingerTree5 |
|--------|-------------|-------------|
| Element storage | Contiguous array | `shared_ptr<const Elem>` per node |
| Cache lines per element | 1 (amortised over 64 bytes) | 1+ (pointer chase per node) |
| Hardware prefetch | Effective (sequential pattern) | Ineffective (random pointers) |
| Allocation count for N elements | 1 | O(N) (one `make_shared` per internal node) |
| Overhead per element | 0 bytes | ~32–48 bytes (control block + variant tag) |

For workloads dominated by sequential reads (sum, transform, copy-to-output),
std::vector will be 10–100× faster than FT5 purely due to cache effects.

The trade-off is justified when O(log N) structural operations or free
persistence offset the per-element overhead.
The crossover point depends on the workload: if insert/erase/concat/split
operations are frequent relative to sequential reads, FT5 wins on total
throughput despite worse per-element access.

---

## 6. Summary decision matrix

| Criterion | Best choice |
|-----------|-------------|
| Need multiple live versions (persistence) | **FingerTree5** |
| Frequent insert/erase at arbitrary positions | **FingerTree5** |
| Frequent concat or split | **FingerTree5** |
| Concurrent readers without locks | **FingerTree5** |
| Undo/redo with bounded memory | **FingerTree5** |
| Double-ended priority queue | **FingerTree5 PQ** |
| Interval stabbing queries | **FingerTree5 IntervalIndex** |
| Sequential iteration in a tight loop | std::vector / std::deque |
| Random access by index as the hot path | std::vector / std::deque |
| Small data (< 100 elements) | std::vector (constant factors dominate) |
| C API interop (contiguous pointer) | std::vector / std::string |
| Simple single-ended scheduling | std::priority_queue |

---

## 7. What FingerTree5 enables that has no std equivalent

These patterns are either impossible or prohibitively expensive with standard
containers:

1. **O(1) persistent snapshots** — no std container offers this.
2. **O(log N) concatenation** — std::list has O(1) splice but O(N) for two
   independent lists; no std sequence supports sub-linear concat of two
   arbitrary halves.
3. **O(log N) split at a measure threshold** — the monoid-parameterised split
   is unique to finger trees and enables all four wrappers from a single
   algorithm.
4. **Simultaneous min/max queries** — std::priority_queue exposes only one
   end; FT5's PriorityTag caches both.
5. **Measure-guided subtree pruning** — the interval index skips entire
   subtrees whose cached max-endpoint proves they cannot contain a match.
   This is not achievable with sorted arrays or std::multimap without
   auxiliary data structures.
6. **Lock-free concurrent reads from different history points** — readers
   can hold snapshots from different points in time and never block each
   other or a single writer.
