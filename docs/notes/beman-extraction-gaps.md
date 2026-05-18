# Pre-Migration Gaps: Work Required Before Beman Extraction

Date: 2026-05-18

This note captures the concrete gaps in the current Beman extraction plans
that must be resolved before any code migration begins.
It is a follow-up to the expert review of the three planning documents
(`standardization-inverted-triangle-plan.md`, `beman-integration-plan.md`,
`beman-extraction-checklist.md`) and the mapping script
(`scripts/emit_beman_extraction_plan.py`).

Each item below is either a decision that must be made, a design task that
must be completed, or a specification that must be written before the
extraction can proceed correctly.

---

## Gap 1: No history preservation strategy

**What is missing**

The current plan describes *what* to move but not *how* to move it while
keeping the git history that backs up every design decision.
The commits in `trees` now explain in detail:

- Why the `empty()` static factory was renamed to a default constructor
  (Container concept naming conflict — removing it required 20+ call-site
  updates across wrappers, tests, and benchmarks)
- Why the spine shell uses placement new rather than `allocate_shared`
  (the `uses_allocator` protocol would call `flatten()` on a tree whose
  elements are Node3 objects, producing structurally invalid results)
- Why `append` triggers an O(|right|) rebuild when allocators differ
  (Lakos rule: mmap'd file or shared-memory regions cannot tolerate
  cross-resource pointers; silent mixing is catastrophic)
- Why `from_sequence`'s internal scaffolding vectors use a rebound
  allocator (`EpVec`) rather than `std::vector<EP>` (the PMR probe
  found that bare `std::vector<EP>` hit the global heap; the fix is
  in the commit that introduced `EpAlloc`/`EpVec`)
- Why `nodes_from` uses `inplace_vector<EP,12>` (proof that combined
  size ≤ 4+4+4=12 is in the commit message)

If the Beman repo is bootstrapped by copying files, this history becomes
folklore and cannot be recovered by future maintainers or reviewers.

**Work required**

Decide on an extraction mechanism before the first file moves:

- **Preferred: `git filter-repo` with path renames.**
  This rewrites the `trees` history to produce a branch containing only
  the commits that touch the extracted paths, renamed to their Beman
  target paths.
  The branch is fetched into the new Beman repo as the initial import.
  `git log`, `git blame`, and `git bisect` then work correctly for every
  extracted file from day one.
  Path renames must be accompanied by a string-substitution pass (see
  Gap 3) applied during or after the filter-repo run.

- **Alternative: cherry-pick series.**
  For commits that touch many files simultaneously (the Phase 4 allocator
  work, for example), `filter-repo` may produce commits that touch files
  not yet extracted.
  A curated cherry-pick list from `trees` into the Beman repo avoids
  this but requires more manual work.

- **Fallback: graft notes.**
  Create the Beman repo fresh and add a `git notes` reference on the
  initial import commit pointing at the range of `trees` commits it
  corresponds to.
  This loses binary blame history but keeps the archaeology accessible.
  Acceptable only if `filter-repo` proves too entangled.

**Decision needed from:** repository owner.

---

## Gap 2: Incomplete file mapping — missing components

The current checklist and script omit the following components that belong
in the Beman extraction:

### 2a. PMR typedef header

`src/smd/tree/finger_tree5_pmr.hpp` is not listed in the mapping.
This header provides the `smd::tree::pmr::FingerTree5` alias backed by
`std::pmr::polymorphic_allocator<std::byte>`.
In the Beman layout it becomes either a top-level public header
(`include/beman/structure/finger_tree_pmr.hpp`) or is folded into
`finger_tree.hpp` as a nested `namespace beman::structure::pmr` alias.
The decision affects the public include spelling and should be made
explicitly before extraction.

### 2b. Standalone PMR allocation probe

`src/smd/tree/finger_tree5_pmr_probe.cpp` is not listed in the mapping.
This is not a test in the Catch2 sense — it is a standalone executable
that replaces `operator new`/`delete` globally and verifies that a
PMR-backed `FingerTree5` produces **zero global heap allocations** across
18 operations.
It is a key piece of implementation evidence for the standardization
proposal.
In the Beman layout it belongs under `tests/beman/structure/` or
`examples/` with a clear description of what it proves.
Its companion `null_memory_resource` arena design means that any regression
in allocator threading causes an immediate `std::bad_alloc` — the proof
is self-enforcing.

### 2c. Allocator design document

`docs/finger-tree5-allocator-design.md` is not listed in the mapping.
This document records:

- The Lakos rule and why it applies to this implementation
- Every major allocator design decision and the alternative considered
- The `uses_allocator` extended-constructor mechanism
- Why the spine shell uses placement new
- The two remaining known limitations (static factories, two-repo sync)

It should move to `docs/` or `papers/` in the Beman repo.
Without it, the allocator implementation looks arbitrary.

### 2d. The `src/smd/fixpoint/` directory

The mapping mentions `fixpoint_tree.hpp` as source material for the
recursive surfaces but does not address the `fixpoint/` module:

- `src/smd/fixpoint/overloaded.hpp` — `finger_tree5.hpp` currently
  includes this via `using smd::fixpoint::overloaded`.
  In the Beman layout it becomes either
  `include/beman/structure/detail/overloaded.hpp` or is merged into
  another detail header.
  It must be listed because it is a direct dependency of `finger_tree.hpp`.
  The `consteval void operator()(auto)` exhaustiveness-check design
  (promoted during Phase 1) should be preserved in the Beman version.

- `src/smd/fixpoint/box.hpp`, `fix.hpp`, `cata.hpp` — These are the
  fixpoint primitives that underpin `recursive_fold.hpp` and
  `recursive_build.hpp` alongside `fixpoint_tree.hpp`.
  The mapping lists only `fixpoint_tree.hpp` as source material, missing
  the core machinery.

### 2e. Benchmark suite

`src/smd/tree/finger_tree_std_compare.bench.cpp` (six-category comparison
against std library types) and `src/smd/tree/finger_tree_compare.bench.cpp`
(FT2/FT3/FT4/FT5 internal comparison) are not listed.
Performance evidence matters for standardization review.
Benchmarks do not need to live in the Beman repo as production tests, but
the std-comparison benchmark should accompany the proposal as evidence.
A top-level `benchmarks/` or `perf/` directory in the Beman repo, or a
note in `docs/` pointing to the `trees` benchmark suite, is needed.

---

## Gap 3: No mechanical rewrite specification

The checklist says "move" files.
The actual extraction requires a systematic string-substitution pass.
For `finger_tree5.hpp` → `finger_tree.hpp` alone, the changes include:

| Artifact | Current form | Target form |
|----------|-------------|-------------|
| Include guard | `INCLUDED_SMD_TREE_FINGER_TREE5` | `INCLUDED_BEMAN_STRUCTURE_FINGER_TREE` |
| File header comment | `// src/smd/tree/finger_tree5.hpp` | `// include/beman/structure/finger_tree.hpp` |
| Namespace | `namespace smd::tree {` | `namespace beman::structure {` |
| PMR namespace | `namespace smd::tree::pmr {` | `namespace beman::structure::pmr {` |
| Self-include | `<smd/tree/finger_tree5_iterator.hpp>` | `<beman/structure/detail/finger_tree_iterator.hpp>` |
| Typeclass includes | `<smd/typeclass/monoid.hpp>` | `<beman/structure/monoid.hpp>` |
| Fixpoint include | `<smd/fixpoint/overloaded.hpp>` | `<beman/structure/detail/overloaded.hpp>` |
| PMR include | `<smd/tree/finger_tree5_pmr.hpp>` | `<beman/structure/finger_tree_pmr.hpp>` |
| Measure type names | `UnitMeasure5<T, Tag>` | `unit_measure<T, Tag>` or other non-versioned name |
| Iterator type | `FingerTree5Iterator` | `finger_tree_iterator` or keep verbatim? |
| Friend declaration | `friend class FingerTree5Iterator` | updated to match |

This must be specced as a repeatable script, not left to ad hoc editing.
The `filter-repo` path-rename mechanism handles the physical path but does
NOT rewrite file contents — a separate content-rewrite pass is needed.

**Work required:** Write and test the rewrite script before any file moves.
A sed/awk script or Python pass over the extracted branch is appropriate.

---

## Gap 4: `recursive_fold.hpp` and `recursive_build.hpp` need interface design

These are listed as `source-material` targets in the mapping script,
correctly signalling that they do not yet exist.
But the plan does not sketch what their interfaces should look like.
This is API design work, not a file move.

**Specific questions that must be answered:**

- Does `recursive_fold.hpp` expose `cata` (catamorphism) under a new
  name, or a higher-level wrapper that hides the fixpoint machinery?
- Does `recursive_build.hpp` expose `ana` (anamorphism) or something
  coarser?
- Do `Fix<F>` and the functor-map machinery live in
  `beman::structure::detail` or become part of the public API?
- Is `fixpoint_tree_algorithm.hpp` (the "algorithm composition pattern"
  demo using multiple typeclass ops) extracted as a motivating example?

**Work required:** Sketch the public interface of both headers before the
Beman repo is created.
The interface must be stable enough that the paper (D/P_XXX) can reference
it without requiring a breaking change on next revision.

---

## Gap 5: `UnitMeasure5` and other versioned measure type names

The public template defaults in `finger_tree5.hpp` use `UnitMeasure5<T, Tag>`,
`RopeChunkMeasure`, `PriorityMeasure<T>`, `IntervalMeasure<T>`.
In the Beman public API, these appear as default template arguments and
are therefore visible to users:

```cpp
// Current
template <typename T, typename Tag = std::size_t,
          typename MP = UnitMeasure5<T, Tag>, typename Alloc = ...>
class FingerTree5;

// Beman target — what does this look like?
template <typename T, typename Tag = std::size_t,
          typename MP = ???<T, Tag>, typename Alloc = ...>
class finger_tree;
```

The `5` suffix is an implementation-generation marker that must not survive
into the public API.
The replacement names must be chosen and specified before extraction.

**Work required:** Decide on names for:
- `UnitMeasure5` → `unit_measure<T, Tag>` or similar
- `RopeChunkMeasure` → `rope_chunk_measure` or merge into `rope.hpp` detail
- `PriorityMeasure<T>` → `priority_measure<T>` or merge into `priority_queue.hpp` detail
- `IntervalMeasure<T>` → `interval_measure<T>` or merge into `interval_index.hpp` detail
- `IntervalMaxEndTag<T>` → similar

---

## Gap 6: Two-repo synchronization policy

The plan correctly says `trees` remains the pedagogy repo and Beman is the
production repo.
It does not address what happens after the initial extraction:

- **Bug fixes in Beman:** If a correctness bug is found in
  `beman::structure::finger_tree` and fixed there, does it get
  back-ported to `trees/src/smd/tree/finger_tree5.hpp`?
  If not, `trees` becomes misleading for future talks and design work.
- **API evolution in Beman:** The Beman repo will evolve its API with
  each paper revision.
  `trees` will diverge.
  Is this acceptable?
  The answer is probably yes for the production API, but it needs to be
  a stated policy rather than an accident.
- **Compile-time probes:** The `finger_tree5_compile_probe.cpp` in `trees`
  compares instantiation depth across FT2-5.
  That evidence is a claim the Beman implementation should be able to
  reproduce, but the probes are `trees`-only artifacts.
  A note should indicate where the archaeology lives.

**Work required:** Write a short policy section in the updated migration
plan stating the intended post-extraction relationship between the two
repos (one-way copy, occasional back-port, or deliberate divergence).

---

## Gap 7: The `*_ft5.t.cpp` merge decision

The checklist identifies these test files as an open question:

- `src/smd/tree/finger_tree_interval_index_ft5.t.cpp`
- `src/smd/tree/finger_tree_priority_queue_ft5.t.cpp`
- `src/smd/tree/finger_tree_random_access_ft5.t.cpp`
- `src/smd/tree/finger_tree_rope_ft5.t.cpp`

These test the FT5-backed wrappers specifically, cross-checking semantic
equivalence with older implementations.
In the Beman repo there are no FT2/FT3/FT4 alternatives, so the FT5 path
is the only path.

**Work required:** Decide and document before extraction whether to:
- Merge the `_ft5.t.cpp` coverage into the canonical wrapper test files
  (recommended — removes the implementation-variant framing from the
  Beman test suite)
- Keep them as separate regression tests inside the Beman `tests/` tree
  (retains the "is FT5 semantically equivalent to the simpler version?"
  framing, which may be useful evidence for reviewers)

---

## Summary: Pre-extraction checklist

Before any file is moved to the Beman repo:

- [ ] **Gap 1**: Commit to a history-preservation mechanism (filter-repo,
  cherry-pick, or graft notes).
- [ ] **Gap 2a**: Decide how `finger_tree5_pmr.hpp` is exposed in the Beman
  surface (top-level header vs nested alias in `finger_tree.hpp`).
- [ ] **Gap 2b**: Add `finger_tree5_pmr_probe.cpp` to the file mapping with
  a target location decision.
- [ ] **Gap 2c**: Add `docs/finger-tree5-allocator-design.md` to the mapping
  with a target in Beman `docs/` or `papers/`.
- [ ] **Gap 2d**: Add the full `src/smd/fixpoint/` contents to the mapping
  with target locations (public, detail, or stay-in-trees per file).
- [ ] **Gap 2e**: Add the benchmark suite to the mapping with a target or a
  policy statement about where performance evidence lives.
- [ ] **Gap 3**: Write and test the mechanical rewrite script before moving
  any files.
- [ ] **Gap 4**: Sketch the public interface of `recursive_fold.hpp` and
  `recursive_build.hpp`.
- [ ] **Gap 5**: Decide on stable non-versioned public names for the measure
  policy types.
- [ ] **Gap 6**: Write the two-repo synchronization policy.
- [ ] **Gap 7**: Decide the `*_ft5.t.cpp` merge strategy.
