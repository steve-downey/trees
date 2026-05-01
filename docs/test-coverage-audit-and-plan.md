# Test Coverage Audit and Implementation Plan

**Date:** 2026-05-01  
**Scope:** Trees project (`trees/src/`)  
**Excludes:** deadcode/, conceptmap/ directories  
**Status:** Audit complete, plan ready for implementation

---

## Executive Summary

This audit enforces a critical house style rule: **every `.hpp` file is a component that must have a corresponding `.t.cpp` test file.**

Additionally, every test file must follow a strict structure:
1. Include the target header twice (once after canonical includes, once again before framework imports) to verify re-inclusion safety
2. Begin with a tautologically failing test to establish build coherency (TDD bootstrap)
3. Follow with substantive tests

**Current State:**
- ✅ 28 test files exist and build successfully
- ⚠️ 28 test files **lack double-include verification** (retrofit needed)
- ❌ 10 components lack test files entirely (new files required)
- 🔧 Infrastructure headers (test_support.hpp, examples.hpp) are exempt (non-components)

**Impact:**
- ~40–50 hours of work to complete (including retrofit + new tests + validation)
- Improves header hygiene and catches circular dependency issues early
- Enables confident refactoring of internal headers

---

## Audit Findings

### A. Components Missing Test Files (10 violations)

| Component | Path | Reason | Priority |
|-----------|------|--------|----------|
| `range_list` | `src/smd/ranges/range_list.hpp` | Public adapter + typeclass instances | High |
| `binary_tree` | `src/smd/tree/binary_tree.hpp` | Core tree data structure | High |
| `finger_tree_foldable` | `src/smd/tree/finger_tree_foldable.hpp` | Typeclass instance wrapper | Medium |
| `finger_tree_traversable` | `src/smd/tree/finger_tree_traversable.hpp` | Typeclass instance wrapper | Medium |
| `finger_tree_wrappers` | `src/smd/tree/finger_tree_wrappers.hpp` | Specialized finger tree adapters | Medium |
| `fix_tree` | `src/smd/tree/fix_tree.hpp` | Core minimal tree structure | High |
| `fringe_tree_foldable` | `src/smd/tree/fringe_tree_foldable.hpp` | Typeclass instance wrapper | Medium |
| `memoized_thunk` | `src/smd/tree/memoized_thunk.hpp` | Lazy evaluation caching | High |
| `zip_list` | `src/smd/ziplist/zip_list.hpp` | Finite/infinite list abstraction | High |

**Typeclass Wrappers (4 files):**
These are thin wrapper headers that instantiate typeclass instances for a core data structure. Each should verify the base component's instances work correctly.

**Priority tiers:**
- **High:** Core data structures or general-purpose utilities
- **Medium:** Specialized adapters or variant instances

### B. Infrastructure Headers (Exempt from this rule)

| Header | Purpose | Reason Exempt |
|--------|---------|---------------|
| `src/smd/typeclass/test/test_support.hpp` | Typeclass law checking utilities | Infrastructure, not a component |
| `src/smd/typeclass/examples/examples.hpp` | Example code for slides | Example/documentation, not production |

These files have headers because they are textual bundles of helpers, not independent components to be tested. They may be used by tests but do not themselves need test files.

---

## Required Rule Change: House Style Update

### Change to `docs/codestyle.md`

Insert a new subsection in the **C++ House Rules** section after "Test naming and placement":

```markdown
## Test file double-include and TDD bootstrap pattern

Every test file (`.test.cpp` or `.t.cpp`) must enforce header re-inclusion safety and establish a baseline test immediately:

### Double-include pattern

The target header is included twice—once at the module scope and once before framework imports—to verify that:
1. The header is idempotent (no partial re-parse errors)
2. Include guards or `#pragma once` are correctly placed
3. Macro-based customization does not leak from first to second inclusion

```cpp
// src/acme/net/socket.test.cpp                                 -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <acme/net/socket.hpp>      // First include: canonical form
#include <acme/net/socket.hpp>      // Second include: re-inclusion check

#include <catch2/catch_test_macros.hpp>
#include <string>

TEST_CASE("SocketTest - HeaderIsIdempotent")
{
    // This test always passes if the file compiles.
    // It serves as a placeholder until substantive tests are added.
    REQUIRE(true);
}

TEST_CASE("SocketTest - Construction")
{
    acme::net::socket s("localhost:8080");
    CHECK(s.is_connected() == false);
}
```

### Rationale

1. **Re-inclusion safety:** Catches accidental `#ifndef` omissions or incorrect guard boundaries
2. **Macro isolation:** Verifies that first-pass macro definitions do not corrupt second-pass parsing
3. **Build coherency:** A passing re-inclusion test proves the header can be safely included by multiple translation units
4. **TDD discipline:** Tautological test enforces immediate build-time correctness; substantive tests follow in order

### Imperative for adding new components

When adding a new component with its own header:
1. Create the `.hpp` with full content and include guards
2. Create the `.t.cpp` immediately with the double-include pattern and tautological test
3. Do not merge to main until the `.t.cpp` builds and runs to completion
4. Add substantive tests incrementally; the framework structure is stable from first commit

```
# Timeline: add the skeleton together, tests and business logic follow
component.hpp (new)
├── Include guards: ✓
├── Full interface: ✓
└── Builds standalone: ✓

component.t.cpp (new)
├── Double includes: ✓
├── Tautological test: ✓
├── Compiles and runs: ✓
└── Substantive tests: follow in next PR
```

### Historical note

Existing test files in the trees project currently do not include the target header twice. All such files should be retrofitted to include this safety check as a low-risk, high-value improvement.
```

---

## Implementation Plan

This plan is structured for a secondary programmer to implement while the primary reviewer validates each step.

### Phase 1: Documentation (Immediate — 1 hour)

**Task 1.1:** Update `docs/codestyle.md` with the rule change above  
**Deliverable:** Amended codestyle document with new subsection and examples  
**Validation:** Read back to confirm rule is clear and unambiguous  

---

### Phase 2: Retrofit Existing Test Files (Medium effort — ~15 hours)

**Approach:**
All 28 existing test files must be retrofitted to include the target header twice at the top.

**Task 2.1–2.28:** For each existing test file, prepend a second include of the target header

**Example:** `src/smd/typeclass/functor.t.cpp`

**Before:**
```cpp
#include <smd/typeclass/functor.hpp>

#include <catch2/catch_test_macros.hpp>
...
```

**After:**
```cpp
#include <smd/typeclass/functor.hpp>
#include <smd/typeclass/functor.hpp>  // Re-inclusion check

#include <catch2/catch_test_macros.hpp>
...
```

**List of files to retrofit:**

**Ranges (4 files):**
- `src/smd/ranges/range_applicative.t.cpp`
- `src/smd/ranges/range_foldable.t.cpp`
- `src/smd/ranges/range_functor.t.cpp`
- `src/smd/ranges/range_traversable.t.cpp`

**Trees - Binary (3 files):**
- `src/smd/tree/binary_tree_applicative.t.cpp`
- `src/smd/tree/binary_tree_foldable.t.cpp`
- `src/smd/tree/binary_tree_traversable.t.cpp`

**Trees - Finger (5 files):**
- `src/smd/tree/finger_tree.t.cpp`
- `src/smd/tree/finger_tree_interval_index.t.cpp`
- `src/smd/tree/finger_tree_priority_queue.t.cpp`
- `src/smd/tree/finger_tree_random_access.t.cpp`
- `src/smd/tree/finger_tree_rope.t.cpp`

**Trees - Fix (3 files):**
- `src/smd/tree/fix_tree_applicative.t.cpp`
- `src/smd/tree/fix_tree_foldable.t.cpp`
- `src/smd/tree/fix_tree_traversable.t.cpp`

**Trees - Fringe (3 files):**
- `src/smd/tree/fringe_tree.t.cpp`
- `src/smd/tree/fringe_tree_applicative.t.cpp`
- `src/smd/tree/fringe_tree_traversable.t.cpp`

**Typeclass (5 files):**
- `src/smd/typeclass/applicative.t.cpp`
- `src/smd/typeclass/foldable.t.cpp`
- `src/smd/typeclass/functor.t.cpp`
- `src/smd/typeclass/monoid.t.cpp`
- `src/smd/typeclass/traversable.t.cpp`
- `src/smd/typeclass/typeclass_base.t.cpp`

**Typeclass Examples (1 file):**
- `src/smd/typeclass/examples/examples.t.cpp`

**ZipList (1 file):**
- `src/smd/ziplist/zip_list_applicative.t.cpp`

**Validation:**
- Run `make test` after each batch (every ~7 files)
- Confirm all tests pass with green output
- No changes to test behavior expected; only structure

**Commit Strategy:**
- Single commit: "test: add double-include verification to all existing test files"
- Message includes rationale and counts (28 files retrofitted)

---

### Phase 3: Create New Test Files (High effort — ~20 hours)

**Approach:**
Create 9 new test files for missing components (one per missing .hpp). Each starts with tautological test, then adds 2–3 substantive tests for build coherency.

**Task 3.1–3.9:** Create new test files

**Template for new test file:**

```cpp
// src/smd/<component>/<component>.t.cpp                            -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <smd/<component>/<component>.hpp>
#include <smd/<component>/<component>.hpp>  // Re-inclusion check

#include <catch2/catch_test_macros.hpp>

TEST_CASE("<ComponentName>Test - HeaderIsIdempotent")
{
    // Placeholder: verifies header re-inclusion safety and build coherency.
    // Substantive tests follow.
    REQUIRE(true);
}

TEST_CASE("<ComponentName>Test - <FirstBreathingTest>")
{
    // Minimal instantiation test: verify type is constructible and basic operation works.
    // [implementation]
}

TEST_CASE("<ComponentName>Test - <SecondSemanticTest>")
{
    // Behavior test: verify the component's main algorithm or invariant.
    // [implementation]
}
```

**New test files to create:**

| File | Component | Target Tests | Notes |
|------|-----------|--------------|-------|
| `src/smd/ranges/range_list.t.cpp` | `range_list.hpp` | Construction, foldable, applicative | Public adapter; existing foldable/applicative tests for ranges; verify wrapper composition |
| `src/smd/tree/binary_tree.t.cpp` | `binary_tree.hpp` | Construction, traversal, structure | Core tree; add breathing tests + shape invariant |
| `src/smd/tree/finger_tree_foldable.t.cpp` | `finger_tree_foldable.hpp` | Foldable operations on finger tree | Wrapper instance; verify fold_map, length, to_vector on varied finger tree shapes |
| `src/smd/tree/finger_tree_traversable.t.cpp` | `finger_tree_traversable.hpp` | Traversable on finger tree | Wrapper instance; verify shape preservation under traverse |
| `src/smd/tree/finger_tree_wrappers.t.cpp` | `finger_tree_wrappers.hpp` | Wrapper types and conversions | Verify wrappers correctly adapt finger tree interface |
| `src/smd/tree/fix_tree.t.cpp` | `fix_tree.hpp` | Construction, fixed-point recursion | Core minimal tree; verify isomorphism with pattern match |
| `src/smd/tree/fringe_tree_foldable.t.cpp` | `fringe_tree_foldable.hpp` | Foldable operations | Wrapper instance for variant tree |
| `src/smd/tree/memoized_thunk.t.cpp` | `memoized_thunk.hpp` | Lazy evaluation, caching | Core utility; verify memo'd result is computed once, returned thereafter |
| `src/smd/ziplist/zip_list.t.cpp` | `zip_list.hpp` | Construction, iteration, applicative semantics | Public list type; verify zippy applicative behavior (element-wise, not Cartesian) |

**Effort breakdown per file:**
- Tautological test: ~5 min
- Breathing test (minimal instantiation): ~15 min
- Semantic test (algorithm/invariant): ~20–40 min (depends on component complexity)
- Integration with build system: ~10 min
- **Total per file: 50–60 min**
- **Total for 9 files: ~7–9 hours**

**Validation:**
- Each file compiles and runs independently: `ctest -R <component>`
- All tests pass
- No warnings or errors

**Commit Strategy:**
- Either:
  - Single commit: "test: add test files for 9 missing components"
  - Or: One commit per component (9 commits, fine-grained tracking)
- Choose strategy based on reviewer preference

---

### Phase 4: Integrated Validation (Short — ~2 hours)

**Task 4.1:** Run full test suite

```bash
cd /workarea/cppnow26/trees
make test
```

**Validation criteria:**
- All 170 tests pass (or 170+ if new tests added)
- No warnings related to includes or idempotency
- All typeclass instances still function correctly
- No regressions in existing behavior

**Task 4.2:** Validate re-inclusion safety at compile time

```bash
make compile
```

**Validation criteria:**
- All source files compile without macro re-definition warnings
- No `#define` conflicts or symbol redeclaration errors

**Task 4.3:** Check code coverage (optional)

```bash
make coverage
```

**Expected:** New test files improve coverage in their respective namespaces; no regression in overall coverage.

---

### Phase 5: Review and Documentation (Short — ~1 hour)

**Task 5.1:** Review all changed files

- Verify double-include is present in all test files
- Confirm tautological tests are meaningful (not trivial)
- Check new test files have 2–3 substantive tests

**Task 5.2:** Update `docs/live-src-main-built-targets.md` snapshot

After new test files are added, regenerate the source snapshot to include them:

```bash
cd /workarea/cppnow26/trees
make docs-index
```

This updates the canonical source listing and ensures documentation stays current.

**Task 5.3:** Commit summary and merge

Final commit message template:

```
test: enforce double-include pattern and add missing test files

Phase 1: Documentation
- Updated docs/codestyle.md with test file double-include rule and TDD bootstrap pattern

Phase 2: Retrofit (28 files)
- Added re-inclusion verification to all existing test files
- No behavioral changes; structural improvement only

Phase 3: New Test Files (9 files)
- Added test files for range_list, binary_tree, finger_tree_foldable, 
  finger_tree_traversable, finger_tree_wrappers, fix_tree, 
  fringe_tree_foldable, memoized_thunk, zip_list
- Each includes tautological test + 2–3 substantive tests

Phase 4: Validation
- All 170+ tests pass
- No regressions in existing behavior
- Full test suite runs cleanly

This ensures every component has explicit test coverage and headers are 
re-inclusion safe (no circular or partial macro issues).
```

---

## Timeline Estimate

| Phase | Task | Effort | Elapsed |
|-------|------|--------|---------|
| 1 | Documentation | 1 h | 1 h |
| 2 | Retrofit 28 files | 15 h | 16 h |
| 3 | Create 9 new test files | 7–9 h | 23–25 h |
| 4 | Integrated validation | 2 h | 25–27 h |
| 5 | Review + merge | 1 h | 26–28 h |

**Total estimated effort: 26–28 hours** (or ~3–4 days at standard pace)

---

## Implementation Notes for Secondary Programmer

### Do's

✅ Run `make test` after every 5–7 file modifications to catch issues early  
✅ Use the template above for consistency  
✅ Add a comment in each new test file explaining the component's purpose  
✅ Verify that double-includes work by checking compiler output (should see no warnings)  
✅ Keep each test case focused on one behavior  
✅ Use descriptive names like `ComponentTest - <Behavior>`  

### Don'ts

❌ Do not remove or rename the tautological test; it serves as a build-time signal  
❌ Do not skip re-inclusion for any file (even if it seems "trivial")  
❌ Do not add tests to infrastructure headers (test_support.hpp, examples.hpp)  
❌ Do not change the order of includes; canonical header must always come first  
❌ Do not commit before running `make test` end-to-end  

### Debugging Tips

**If compilation fails after retrofit:**
1. Check that the second `#include` line is syntactically identical to the first
2. Verify no typos in the header path
3. Run `make clean` and rebuild from scratch

**If tests fail with re-inclusion errors:**
1. Confirm the header has proper include guards: `#ifndef INCLUDED_...`
2. Check for `#undef` of macros in the header (bad practice; contact owner)
3. Verify the header does not use `#pragma once` mixed with guards (must be one or the other)

**If new test file doesn't link:**
1. Verify the component's implementation file exists and is listed in CMakeLists.txt
2. Confirm the test target is linked correctly (check CMakeLists.txt)
3. Review `make compile` output for any unresolved symbols

---

## Acceptance Criteria

✅ All 28 existing test files include the target header twice  
✅ All 28 existing tests still pass (no regressions)  
✅ All 9 new test files exist and compile  
✅ Each new test file includes tautological + 2–3 substantive tests  
✅ `make test` runs 170+ tests and passes 100%  
✅ `make compile` produces no include-related warnings  
✅ Code style documentation updated with new rule  
✅ Secondary programmer can follow the plan with minimal guidance  

---

## Review Checklist (for Primary Reviewer)

- [ ] Codestyle document updated and clear
- [ ] Double-include pattern applied uniformly across all 28 retrofit files
- [ ] New test files follow template and include all required parts
- [ ] No substantive code changes in retrofit (pure structure)
- [ ] All tests pass both individually and as a suite
- [ ] No coverage regressions
- [ ] Commit messages are clear and detailed
- [ ] Documentation (live-src, index.org) regenerated if needed
