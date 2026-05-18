# Updated Beman Migration Plan (v2)

Date: 2026-05-18

This document supersedes the relevant sections of `beman-integration-plan.md`
and incorporates the gaps identified in `beman-extraction-gaps.md`.
It does not change the strategic decisions in the original plan — naming
philosophy, public/detail split, two-repo framing, Copier bootstrap — all
of those remain correct.
What this document adds is:

- a history-preservation strategy (the most critical missing piece)
- a complete file mapping including the omitted components
- a mechanical rewrite specification
- explicit pre-extraction work items that are design tasks, not moves
- a two-repo synchronization policy
- an updated extraction sequence

Read in conjunction with the original `beman-integration-plan.md` and the
gaps note (`beman-extraction-gaps.md`).

---

## Strategic decisions unchanged from v1

- One Beman repo for the coordinated paper set.
- Working short name: **`structure`**.
  (`beman::structure`, `include/beman/structure/`, `beman::structure`)
- `trees` remains the pedagogy, exploration, and design workspace.
- Beman is the production-tracking, standardization-facing library.
- Copier-capable `bemanproject/exemplar` as the bootstrap baseline.
- Public headers flat under `include/beman/structure/`.
- Implementation-support headers under `include/beman/structure/detail/`.
- Tests under `tests/beman/structure/` named `*.test.cpp`.
- Examples under `examples/`.
- Papers under `papers/`.

---

## Phase 0: Pre-extraction work (must complete before any file moves)

These are decisions and design tasks that shape everything downstream.
Do not begin Phase 1 until all of them are resolved.

### 0.1 Commit to a history-preservation mechanism

**Recommended: `git filter-repo` with path renames**

```bash
# Conceptual sketch — exact invocation depends on the full path list
git filter-repo \
  --path src/smd/tree/finger_tree5.hpp \
  --path src/smd/tree/finger_tree5_iterator.hpp \
  --path src/smd/tree/finger_tree5_pmr.hpp \
  --path src/smd/tree/finger_tree5_pmr_probe.cpp \
  --path src/smd/tree/finger_tree5.t.cpp \
  --path src/smd/tree/finger_tree5_iterator.t.cpp \
  --path src/smd/typeclass/traversable.hpp \
  ... (full list from §Complete file mapping) \
  --path-rename src/smd/tree/finger_tree5.hpp:include/beman/structure/finger_tree.hpp \
  --path-rename src/smd/tree/finger_tree5_iterator.hpp:include/beman/structure/detail/finger_tree_iterator.hpp \
  ... (full rename list from §Complete file mapping)
```

The resulting branch is fetched into the Beman repo as the initial import.
`git log`, `git blame`, and `git bisect` work for every extracted file.

**Alternative: cherry-pick series**
For commits that touch many files simultaneously (the Phase 4 allocator
work touches 20+ files), `filter-repo` may produce commits containing
files not yet extracted.
A curated cherry-pick list avoids this at the cost of more manual work.

**Fallback: graft notes**
If `filter-repo` proves too entangled, create the Beman repo from a clean
stamp and add a `git notes` reference on the initial import commit pointing
to the `trees` commit range it corresponds to.
This loses binary blame but keeps the archaeology accessible from `trees`.
Acceptable only if the preferred options fail.

**Why history matters here specifically:**
The commits in `trees` explain:
- Why the spine shell uses placement new (not `allocate_shared`):
  `uses_allocator` would call `flatten()` on a Node3-containing tree,
  producing structurally invalid results.
- Why `append` rebuilds when allocators differ: Lakos rule — mmap'd files
  and shared-memory segments cannot tolerate cross-resource pointers.
- Why `from_sequence`'s scaffolding uses `EpVec` with a rebound allocator:
  the PMR probe found global-heap escapes; the fix is in the commit that
  introduced `EpAlloc`.
- Why `inplace_vector<EP,12>` in `app3`: a proof in the commit message
  that combined size ≤ 4+4+4=12.
Without this history, reviewers see a complete implementation with no
audit trail for any design choice.

### 0.2 Write and test the mechanical rewrite script

Path renames (from `filter-repo` or manual moves) change physical paths
but not file contents.
A separate content-rewrite pass must be written and tested first.

Minimum substitutions for `finger_tree.hpp` alone:

| From | To |
|------|----|
| `INCLUDED_SMD_TREE_FINGER_TREE5` | `INCLUDED_BEMAN_STRUCTURE_FINGER_TREE` |
| `// src/smd/tree/finger_tree5.hpp` | `// include/beman/structure/finger_tree.hpp` |
| `namespace smd::tree {` | `namespace beman::structure {` |
| `namespace smd::tree::pmr {` | `namespace beman::structure::pmr {` |
| `<smd/tree/finger_tree5_iterator.hpp>` | `<beman/structure/detail/finger_tree_iterator.hpp>` |
| `<smd/typeclass/monoid.hpp>` | `<beman/structure/monoid.hpp>` |
| `<smd/fixpoint/overloaded.hpp>` | `<beman/structure/detail/overloaded.hpp>` |
| `<smd/tree/finger_tree5_pmr.hpp>` | `<beman/structure/finger_tree_pmr.hpp>` |
| `UnitMeasure5` | `unit_measure` (or resolved name from §0.4) |
| `smd::tree::` (qualified uses) | `beman::structure::` |
| `smd::typeclass::` | `beman::structure::` (or keep separate if typeclass is its own paper) |

The same table must be produced for every extracted header.
This is a prerequisite for the filter-repo run so that the rewrite can be
applied as a single commit on top of the renamed branch.

### 0.3 Sketch the `recursive_fold.hpp` and `recursive_build.hpp` interfaces

These do not exist yet.
`fixpoint_tree.hpp` and its supporting headers (`box.hpp`, `fix.hpp`,
`cata.hpp`) are source material, not the output.
The public interface must be designed before the Beman repo is created.

Open questions that must be answered:

- Does `recursive_fold.hpp` expose `cata` under a user-facing name, or a
  higher-level wrapper that hides the fixpoint machinery?
- Does `recursive_build.hpp` expose `ana` (anamorphism) or a coarser
  build-from-seed operation?
- Does `Fix<F>` appear in the public API or only in `detail/`?
- Does `fixpoint_tree_algorithm.hpp` (the algorithm-composition pattern
  demo) serve as the primary usage example?

Deliverable: a header sketch showing the intended public surface of both
`recursive_fold.hpp` and `recursive_build.hpp`, reviewed against the
Paper D scope before implementation begins.

### 0.4 Decide stable non-versioned names for measure policy types

The following names in `finger_tree5.hpp` carry implementation-generation
markers and must be replaced in the public API:

| Current name | Proposed Beman name | Scope |
|-------------|---------------------|-------|
| `UnitMeasure5<T, Tag>` | `unit_measure<T, Tag>` | public default in `finger_tree.hpp` |
| `RopeChunkMeasure` | `rope_chunk_measure` | `detail/` or internal to `rope.hpp` |
| `PriorityMeasure<T>` | `priority_measure<T>` | `detail/` or internal to `priority_queue.hpp` |
| `IntervalMeasure<T>` | `interval_measure<T>` | `detail/` or internal to `interval_index.hpp` |
| `IntervalMaxEndTag<T>` | `interval_max_end_tag<T>` | `detail/` |

The three wrapper-specific measures (`Rope`, `Priority`, `Interval`)
are likely best moved into their respective wrapper headers and made
`detail`-scoped rather than exported at top level.
`unit_measure<T, Tag>` is the only one that belongs in the top-level
`finger_tree.hpp` signature as the default template argument.

### 0.5 Decide the PMR header surface

`finger_tree5_pmr.hpp` provides `smd::tree::pmr::FingerTree5`.
Two options for the Beman surface:

**Option A: top-level header**
```
include/beman/structure/finger_tree_pmr.hpp
namespace beman::structure::pmr { using finger_tree = ...; }
```
Included separately; users who don't use PMR pay no inclusion cost.

**Option B: folded into `finger_tree.hpp`**
The PMR alias lives in `beman::structure::pmr` inside `finger_tree.hpp`.
Simpler surface; users always get the alias whether or not they use PMR.

Given that `<memory_resource>` is a heavy include on some implementations,
Option A is recommended.
Record the decision and update the mapping accordingly.

### 0.6 Decide the `*_ft5.t.cpp` merge strategy

The generic-wrapper tests (`finger_tree_rope.t.cpp`) and the FT5-specific
cross-checks (`finger_tree_rope_ft5.t.cpp`) duplicate coverage in the
`trees` repo because `trees` has multiple implementations.
In the Beman repo there is no FT2/FT3/FT4, so the FT5 path is the only
path.

**Recommended:** Merge the `_ft5.t.cpp` coverage into the canonical wrapper
test files before extraction.
The merged tests in the Beman repo are simply the wrapper tests with
complete coverage, not split across implementation variants.

Alternatively, keep the `_ft5` tests as regression tests inside `tests/`
to preserve the "semantic equivalence against a simpler baseline" framing
for reviewers.

### 0.7 Write the two-repo synchronization policy

Stated here for the record; it should appear in the Beman repo's
`CONTRIBUTING.md` or `docs/development.md`:

**Policy:**
- `trees` is the design, pedagogy, and archaeology repository.
  It is allowed to diverge from `beman::structure` once the Beman repo is
  active.
- Critical correctness fixes found in the Beman repo should be manually
  back-ported to `trees` if the bug would mislead future design work or
  invalidate live talk examples.
  This is a human judgment call, not an automated process.
- API evolution in the Beman repo (driven by paper revisions) does not
  flow back to `trees` unless the change is so fundamental that the
  `trees` version of the code would become misleading.
- The `finger_tree_compare.bench.cpp` compile-time probes in `trees`
  (comparing FT2-5 instantiation depth) are archaeology, not Beman
  production.
  Their data may be cited in papers with a reference to the `trees` repo.

---

## Complete file mapping

This extends the mapping in `beman-integration-plan.md` to include the
files that were omitted.
Rows from the original plan are retained and corrected where needed.

### Algorithm / typeclass surface

| Source (`trees`) | Beman target | Category | Notes |
|-----------------|--------------|----------|-------|
| `src/smd/typeclass/traversable.hpp` | `include/beman/structure/traverse.hpp` | public-api | Verb-first user-facing name |
| `src/smd/typeclass/traversable.hpp` | `include/beman/structure/transpose.hpp` | public-api | Structure/context flipping verb |
| `src/smd/typeclass/foldable.hpp` | `include/beman/structure/fold.hpp` | public-api | |
| `src/smd/typeclass/applicative.hpp` | `include/beman/structure/apply.hpp` | public-support | |
| `src/smd/typeclass/functor.hpp` | `include/beman/structure/functor.hpp` | public-support | |
| `src/smd/typeclass/monad.hpp` | `include/beman/structure/monad.hpp` | public-support | |
| `src/smd/typeclass/monoid.hpp` | `include/beman/structure/monoid.hpp` | public-support | |
| `src/smd/typeclass/dual_monoid.hpp` | `include/beman/structure/dual_monoid.hpp` | public-support | Advanced; needed by `reversed()` |
| `src/smd/typeclass/typeclass_base.hpp` | `include/beman/structure/detail/typeclass_base.hpp` | detail | |

### Fixpoint module (previously unaddressed)

| Source (`trees`) | Beman target | Category | Notes |
|-----------------|--------------|----------|-------|
| `src/smd/fixpoint/overloaded.hpp` | `include/beman/structure/detail/overloaded.hpp` | detail | Direct dependency of `finger_tree.hpp`; `consteval` exhaustiveness check must be preserved |
| `src/smd/fixpoint/box.hpp` | `include/beman/structure/detail/box.hpp` | detail | Source material for recursive surfaces |
| `src/smd/fixpoint/fix.hpp` | `include/beman/structure/detail/fix.hpp` | detail | Core fixpoint primitive |
| `src/smd/fixpoint/cata.hpp` | `include/beman/structure/detail/cata.hpp` | detail | Core recursion scheme |

### Finger-tree core

| Source (`trees`) | Beman target | Category | Notes |
|-----------------|--------------|----------|-------|
| `src/smd/tree/finger_tree5.hpp` | `include/beman/structure/finger_tree.hpp` | public-api | Drop version suffix; full rewrite pass required |
| `src/smd/tree/finger_tree5_iterator.hpp` | `include/beman/structure/detail/finger_tree_iterator.hpp` | detail | May need public exposure for iterator type name |
| `src/smd/tree/finger_tree5_pmr.hpp` | `include/beman/structure/finger_tree_pmr.hpp` | public-api | Decision 0.5; may fold into `finger_tree.hpp` instead |

### Wrapper surface

| Source (`trees`) | Beman target | Category | Notes |
|-----------------|--------------|----------|-------|
| `src/smd/tree/finger_tree_random_access.hpp` | `include/beman/structure/random_access.hpp` | public-api | Capability-facing name |
| `src/smd/tree/finger_tree_rope.hpp` | `include/beman/structure/rope.hpp` | public-api | |
| `src/smd/tree/finger_tree_priority_queue.hpp` | `include/beman/structure/priority_queue.hpp` | public-api | |
| `src/smd/tree/finger_tree_interval_index.hpp` | `include/beman/structure/interval_index.hpp` | public-api | |
| `src/smd/tree/finger_tree_wrappers.hpp` | `include/beman/structure/finger_tree_wrappers.hpp` | public-convenience | Optional umbrella |

### Recursive-algorithm surface (design task, not file copy)

| Source (`trees`) | Beman target | Category | Notes |
|-----------------|--------------|----------|-------|
| `src/smd/tree/fixpoint_tree.hpp` | `include/beman/structure/recursive_fold.hpp` | source-material | Interface must be designed first (§0.3) |
| `src/smd/tree/fixpoint_tree.hpp` | `include/beman/structure/recursive_build.hpp` | source-material | Same |
| `src/smd/tree/fixpoint_tree_algorithm.hpp` | `examples/recursive_fold_example.cpp` | source-material | Algorithm-composition pattern demo |

### Supporting tree types

| Source (`trees`) | Beman target | Category | Notes |
|-----------------|--------------|----------|-------|
| `src/smd/tree/binary_tree.hpp` | `include/beman/structure/binary_tree.hpp` | public-api | |
| `src/smd/tree/fringe_tree.hpp` | `include/beman/structure/fringe_tree.hpp` | public-api | |

### Tests

| Source (`trees`) | Beman target | Category | Notes |
|-----------------|--------------|----------|-------|
| `src/smd/typeclass/traversable.t.cpp` | `tests/beman/structure/traverse.test.cpp` | test | Split traverse/transpose coverage |
| `src/smd/typeclass/traversable.t.cpp` | `tests/beman/structure/transpose.test.cpp` | test | |
| `src/smd/typeclass/foldable.t.cpp` | `tests/beman/structure/fold.test.cpp` | test | |
| `src/smd/typeclass/applicative.t.cpp` | `tests/beman/structure/apply.test.cpp` | test | |
| `src/smd/typeclass/functor.t.cpp` | `tests/beman/structure/functor.test.cpp` | test | |
| `src/smd/typeclass/monad.t.cpp` | `tests/beman/structure/monad.test.cpp` | test | |
| `src/smd/typeclass/monoid.t.cpp` | `tests/beman/structure/monoid.test.cpp` | test | |
| `src/smd/typeclass/dual_monoid.t.cpp` | `tests/beman/structure/dual_monoid.test.cpp` | test | |
| `src/smd/typeclass/typeclass_base.t.cpp` | `tests/beman/structure/detail/typeclass_base.test.cpp` | test | |
| `src/smd/tree/finger_tree5.t.cpp` | `tests/beman/structure/finger_tree.test.cpp` | test | 488 tests; full history matters |
| `src/smd/tree/finger_tree5_iterator.t.cpp` | `tests/beman/structure/detail/finger_tree_iterator.test.cpp` | test | |
| `src/smd/tree/finger_tree5_pmr.t.cpp` | `tests/beman/structure/finger_tree_pmr.test.cpp` | test | Includes allocator coherency tests |
| `src/smd/tree/finger_tree5_pmr_probe.cpp` | `tests/beman/structure/finger_tree_pmr_probe.cpp` | proof-executable | Zero-global-alloc proof; self-enforcing with null_memory_resource |
| `src/smd/tree/finger_tree_random_access.t.cpp` | `tests/beman/structure/random_access.test.cpp` | test | Merge with _ft5 variant (decision 0.6) |
| `src/smd/tree/finger_tree_rope.t.cpp` | `tests/beman/structure/rope.test.cpp` | test | Merge with _ft5 variant |
| `src/smd/tree/finger_tree_priority_queue.t.cpp` | `tests/beman/structure/priority_queue.test.cpp` | test | Merge with _ft5 variant |
| `src/smd/tree/finger_tree_interval_index.t.cpp` | `tests/beman/structure/interval_index.test.cpp` | test | Merge with _ft5 variant |
| `src/smd/tree/binary_tree.t.cpp` | `tests/beman/structure/binary_tree.test.cpp` | test | |
| `src/smd/tree/fringe_tree.t.cpp` | `tests/beman/structure/fringe_tree.test.cpp` | test | |
| `src/smd/tree/finger_tree_wrappers.t.cpp` | `tests/beman/structure/finger_tree_wrappers.test.cpp` | test | |
| `src/smd/fixpoint/overloaded.t.cpp` | `tests/beman/structure/detail/overloaded.test.cpp` | test | |
| `src/smd/fixpoint/box.t.cpp` | `tests/beman/structure/detail/box.test.cpp` | test | |
| `src/smd/fixpoint/fix.t.cpp` | `tests/beman/structure/detail/fix.test.cpp` | test | |
| `src/smd/fixpoint/cata.t.cpp` | `tests/beman/structure/detail/cata.test.cpp` | test | |

### Examples

| Source (`trees`) | Beman target | Category | Notes |
|-----------------|--------------|----------|-------|
| `src/smd/typeclass/examples/traversable_examples.cpp` | `examples/traverse_example.cpp` | example | |
| `src/smd/typeclass/examples/traversable_examples.cpp` | `examples/transpose_example.cpp` | example | Split |
| `src/smd/typeclass/examples/foldable_examples.cpp` | `examples/fold_example.cpp` | example | |
| `src/smd/typeclass/examples/applicative_examples.cpp` | `examples/apply_example.cpp` | example | |
| `src/examples/fixpoint_tree_example.cpp` | `examples/recursive_fold_example.cpp` | source-material | After interface design |

### Documentation

| Source (`trees`) | Beman target | Category | Notes |
|-----------------|--------------|----------|-------|
| `docs/finger-tree5-allocator-design.md` | `docs/finger_tree_allocator_design.md` | doc | Lakos rule analysis; design decisions for AllocatorAware |

### Benchmarks (evidence, not production tests)

| Source (`trees`) | Beman target | Category | Notes |
|-----------------|--------------|----------|-------|
| `src/smd/tree/finger_tree_std_compare.bench.cpp` | `benchmarks/finger_tree_std_compare.cpp` | benchmark | Six-category comparison; standardization evidence |
| `src/smd/tree/finger_tree_compare.bench.cpp` | stay-in-trees | benchmark | FT2-5 internal comparison; archaeology |

### Stay in `trees`

The following should not move to the Beman repo:

- `src/smd/tree/finger_tree2.hpp`, `finger_tree3.hpp`, `finger_tree4.hpp`
  and their tests and compile probes — implementation history, comparison,
  pedagogy.
- `src/smd/tree/deadcode/` — archaeology.
- `src/smd/typeclass/examples/blog_*.cpp`, `lookup_modes_examples.cpp` —
  pedagogy and blog assets.
- `src/examples/cpo_example.cpp`, `map_example.cpp`, `main.cpp` — local
  demonstration harnesses.
- `docs/notes/` (the planning notes) — stay in `trees` as design history.

---

## Adapter consolidation rule (unchanged from v1, restated for completeness)

These files should **not** survive as separate top-level public headers
in the Beman repo:

- `finger_tree5_foldable.hpp`, `finger_tree5_traversable.hpp`
- `finger_tree_foldable.hpp`, `finger_tree_traversable.hpp`
- `binary_tree_applicative.hpp`, `binary_tree_foldable.hpp`,
  `binary_tree_traversable.hpp`
- `fringe_tree_applicative.hpp`, `fringe_tree_foldable.hpp`,
  `fringe_tree_traversable.hpp`
- `fixpoint_tree_foldable.hpp`, `fixpoint_tree_traversable.hpp`

Treatment: merge into the corresponding primary public facade header, or
move under `detail/` if the split remains useful for testing.

---

## Extraction sequence

Execute in this order.
Each phase is a separate PR (or equivalent) to the Beman repo so the
import history is readable.

### Phase 0: Pre-extraction decisions (no file moves)

Complete all items in §"Phase 0: Pre-extraction work".
Write the rewrite script and test it on a copy of `trees`.
Resolve the history-preservation mechanism.

### Phase 1: Bootstrap the Beman repo

- Stamp from a Copier-capable `bemanproject/exemplar` baseline.
- Set the project name, namespace (`beman::structure`), CMake target name,
  and Beman README metadata.
- Establish the empty public include tree skeleton.
- No production code yet — only the project shell.

### Phase 2: Import the typeclass / algorithm surface

This is the simpler end because the typeclass module has fewer dependencies.

Order within this phase:

1. `monoid.hpp` + its tests — no dependencies.
2. `typeclass_base.hpp` (detail) + its tests.
3. `functor.hpp` + tests.
4. `foldable.hpp` → `fold.hpp` + tests.
5. `applicative.hpp` → `apply.hpp` + tests.
6. `monad.hpp` + tests.
7. `dual_monoid.hpp` + tests.
8. `traversable.hpp` → `traverse.hpp` + `transpose.hpp` + split tests.

Apply the content-rewrite script to each file as it is imported.
At the end of this phase, the Beman repo has a working typeclass surface
with passing tests.

### Phase 3: Import the fixpoint module (`detail/`)

These are dependencies of the finger-tree core and must be present before
Phase 4 imports `finger_tree.hpp`.

1. `overloaded.hpp` → `detail/overloaded.hpp` + tests.
2. `box.hpp` → `detail/box.hpp` + tests.
3. `fix.hpp` → `detail/fix.hpp` + tests.
4. `cata.hpp` → `detail/cata.hpp` + tests.

### Phase 4: Import the finger-tree core

1. `finger_tree5.hpp` → `finger_tree.hpp` + full test suite (488 tests).
   This is the largest single import.
   Apply the full content-rewrite script.
   The test file (`finger_tree5.t.cpp` → `finger_tree.test.cpp`) carries
   the Phase 1–4 test history and must import with history intact.

2. `finger_tree5_iterator.hpp` → `detail/finger_tree_iterator.hpp` +
   iterator tests.

3. `finger_tree5_pmr.hpp` → `finger_tree_pmr.hpp` (per decision 0.5) +
   PMR tests + PMR probe.
   The `finger_tree5_pmr_probe.cpp` is the zero-global-alloc proof — it
   must be wired into the Beman build as a CTest-registered executable
   (non-Asan build only, as stated in its CMakeLists entry).

4. Allocator design document → `docs/finger_tree_allocator_design.md`.

### Phase 5: Import the wrapper surface

In any order, since wrappers depend only on `finger_tree.hpp`:

- `random_access.hpp` + merged tests
- `rope.hpp` + merged tests
- `priority_queue.hpp` + merged tests
- `interval_index.hpp` + merged tests
- `finger_tree_wrappers.hpp` + tests

Apply `*_ft5.t.cpp` merge decision (§0.6) before import.

### Phase 6: Import supporting tree types

- `binary_tree.hpp` + tests
- `fringe_tree.hpp` + tests

These bring in the binary- and fringe-tree typeclass adapters.
The adapters (`binary_tree_foldable.hpp`, etc.) are folded into the
primary headers per the adapter-consolidation rule.

### Phase 7: Import the recursive-algorithm surface

This phase depends on Phase 0.3 (interface design for `recursive_fold.hpp`
and `recursive_build.hpp`) being complete.

1. Import `detail/fix.hpp` etc. (already done in Phase 3).
2. Create `recursive_fold.hpp` using `fixpoint_tree.hpp` as source
   material — this is authoring work, not a file copy.
3. Create `recursive_build.hpp` similarly.
4. Add `examples/recursive_fold_example.cpp`.

### Phase 8: Add benchmarks

- Add `benchmarks/finger_tree_std_compare.cpp` with a clear README note
  that these are standardization-evidence benchmarks, not production CI
  artifacts.
- Wire into the build as an optional `BEMAN_STRUCTURE_BUILD_BENCHMARKS`
  target.

### Phase 9: Final compliance checks

- Verify the full public include tree is present and buildable.
- Confirm no versioned names (`finger_tree5`, `UnitMeasure5`, etc.) appear
  in any public header.
- Confirm adapter-split headers are not in the top-level public tree.
- Run all tests.
- Confirm the PMR probe exits 0 in RelWithDebInfo.
- Write the Beman README "Implements" section with the active paper list.
- Verify Copier update path is intact.

---

## `detail/` namespace and header rule (unchanged from v1)

Use `detail/` when a component is all of:

- real code that benefits from separate testing
- too large or structural to belong inline in a front-door header
- too implementation-oriented to deserve a top-level public name

Headers under `detail/` ship with the project but are not front-door API.
Their entities live under `beman::structure::detail`.

---

## Comparison to `beman-integration-plan.md`

| Area | v1 | v2 |
|------|----|----|
| History preservation | Not addressed | §Phase 0.1: filter-repo strategy |
| Mechanical rewrite spec | Not addressed | §Phase 0.2: full substitution table |
| `recursive_fold/build` | Listed as source-material, no interface | §Phase 0.3: design task before extraction |
| Measure type names | Not addressed | §Phase 0.4: stable names required |
| PMR header | Not in mapping | §Phase 0.5: decision + mapping entry |
| PMR probe | Not in mapping | Added as proof-executable |
| Allocator design doc | Not in mapping | Added to docs/ |
| `fixpoint/` module | Not in mapping | Full entries added |
| Benchmark suite | Not addressed | Added as optional evidence artifact |
| `*_ft5.t.cpp` | Open question | §Phase 0.6: merge recommended |
| Two-repo sync policy | Not addressed | §Phase 0.7: written policy |
| Extraction sequence | Implicit | Explicit 9-phase sequence |

---

## Summary

The v1 plan has the right strategic shape.
The gaps are almost entirely in *how* to execute rather than *what* to
extract.

The single most important addition is the history-preservation mechanism.
Without it, the Beman repo lands without an audit trail for the design
decisions that LEWG will most certainly ask about, and without the ability
to `git blame` a line and understand why it exists.

The second most important addition is the pre-extraction design work for
`recursive_fold.hpp` and `recursive_build.hpp`.
Those targets do not exist yet; the Beman repo should not be created until
their interfaces are at least sketched.

Everything else is mechanical once those two decisions are made.
