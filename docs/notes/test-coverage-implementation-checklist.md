# Test Coverage Implementation Checklist

**Date Started:** [YYYY-MM-DD]  
**Secondary Programmer:** [Name]  
**Reviewer:** [Name]  

Use this checklist to track progress through the 5-phase implementation plan.

---

## Phase 1: Documentation ✅ COMPLETE

- [x] Add double-include rule section to `docs/codestyle.md`
- [x] Provide clear template with example
- [x] Document TDD bootstrap pattern and tautological test
- [x] Create comprehensive plan: `docs/test-coverage-audit-and-plan.md`
- [x] Commit to main

**Commit:** `2c9329f` | **Date:** 2026-05-01

---

## Phase 2: Retrofit Existing Test Files (28 total)

### 2.1 Typeclass Tests (5 files)

- [ ] `src/smd/typeclass/applicative.t.cpp` — Add 2nd include
- [ ] `src/smd/typeclass/foldable.t.cpp` — Add 2nd include
- [ ] `src/smd/typeclass/functor.t.cpp` — Add 2nd include
- [ ] `src/smd/typeclass/monoid.t.cpp` — Add 2nd include
- [ ] `src/smd/typeclass/traversable.t.cpp` — Add 2nd include
- [ ] `src/smd/typeclass/typeclass_base.t.cpp` — Add 2nd include

**Validation:** `make test` (should pass 170+ tests)  
**Commit when done:** "test: retrofit typeclass tests with double-include"

### 2.2 Ranges Tests (4 files)

- [ ] `src/smd/ranges/range_applicative.t.cpp` — Add 2nd include
- [ ] `src/smd/ranges/range_foldable.t.cpp` — Add 2nd include
- [ ] `src/smd/ranges/range_functor.t.cpp` — Add 2nd include
- [ ] `src/smd/ranges/range_traversable.t.cpp` — Add 2nd include

**Validation:** `make test`  
**Commit when done:** "test: retrofit ranges tests with double-include"

### 2.3 Binary Tree Tests (3 files)

- [ ] `src/smd/tree/binary_tree_applicative.t.cpp` — Add 2nd include
- [ ] `src/smd/tree/binary_tree_foldable.t.cpp` — Add 2nd include
- [ ] `src/smd/tree/binary_tree_traversable.t.cpp` — Add 2nd include

**Validation:** `make test`  
**Commit when done:** "test: retrofit binary_tree tests with double-include"

### 2.4 Finger Tree Tests (5 files)

- [ ] `src/smd/tree/finger_tree.t.cpp` — Add 2nd include
- [ ] `src/smd/tree/finger_tree_interval_index.t.cpp` — Add 2nd include
- [ ] `src/smd/tree/finger_tree_priority_queue.t.cpp` — Add 2nd include
- [ ] `src/smd/tree/finger_tree_random_access.t.cpp` — Add 2nd include
- [ ] `src/smd/tree/finger_tree_rope.t.cpp` — Add 2nd include

**Validation:** `make test`  
**Commit when done:** "test: retrofit finger_tree tests with double-include"

### 2.5 Fix Tree Tests (3 files)

- [ ] `src/smd/tree/fix_tree_applicative.t.cpp` — Add 2nd include
- [ ] `src/smd/tree/fix_tree_foldable.t.cpp` — Add 2nd include
- [ ] `src/smd/tree/fix_tree_traversable.t.cpp` — Add 2nd include

**Validation:** `make test`  
**Commit when done:** "test: retrofit fix_tree tests with double-include"

### 2.6 Fringe Tree Tests (3 files)

- [ ] `src/smd/tree/fringe_tree.t.cpp` — Add 2nd include
- [ ] `src/smd/tree/fringe_tree_applicative.t.cpp` — Add 2nd include
- [ ] `src/smd/tree/fringe_tree_traversable.t.cpp` — Add 2nd include

**Validation:** `make test`  
**Commit when done:** "test: retrofit fringe_tree tests with double-include"

### 2.7 Typeclass Examples & ZipList (2 files)

- [ ] `src/smd/typeclass/examples/examples.t.cpp` — Add 2nd include
- [ ] `src/smd/ziplist/zip_list_applicative.t.cpp` — Add 2nd include

**Validation:** `make test`  
**Commit when done:** "test: retrofit examples and ziplist tests with double-include"

**Phase 2 Subtotal:** 28 files retrofitted  
**Estimated Time:** 15 hours  
**Overall Progress:** 50%

---

## Phase 3: Create New Test Files (9 total)

### 3.1 HIGH PRIORITY Core Components (5 files)

- [ ] **`src/smd/ranges/range_list.t.cpp`**
  - Header: `#include <smd/ranges/range_list.hpp>` (×2)
  - Tests: Construction, foldable/applicative operations
  - Lines: ~80–100
  - Validation: `ctest -R range_list`
  - Commit: "test: add range_list test file"

- [ ] **`src/smd/tree/binary_tree.t.cpp`**
  - Header: `#include <smd/tree/binary_tree.hpp>` (×2)
  - Tests: Construction, traversal, structure invariant
  - Lines: ~100–120
  - Validation: `ctest -R binary_tree`
  - Commit: "test: add binary_tree test file"

- [ ] **`src/smd/tree/fix_tree.t.cpp`**
  - Header: `#include <smd/tree/fix_tree.hpp>` (×2)
  - Tests: Fixed-point recursion, pattern matching
  - Lines: ~80–100
  - Validation: `ctest -R fix_tree`
  - Commit: "test: add fix_tree test file"

- [ ] **`src/smd/tree/memoized_thunk.t.cpp`**
  - Header: `#include <smd/tree/memoized_thunk.hpp>` (×2)
  - Tests: Lazy evaluation, caching behavior, computed-once invariant
  - Lines: ~100–120
  - Validation: `ctest -R memoized_thunk`
  - Commit: "test: add memoized_thunk test file"

- [ ] **`src/smd/ziplist/zip_list.t.cpp`**
  - Header: `#include <smd/ziplist/zip_list.hpp>` (×2)
  - Tests: Construction, iteration, applicative semantics (non-Cartesian)
  - Lines: ~100–120
  - Validation: `ctest -R zip_list`
  - Commit: "test: add zip_list test file"

**Subtotal:** 5 files, ~450–560 lines  
**Estimated Time:** 4–5 hours

### 3.2 MEDIUM PRIORITY Typeclass Wrappers (4 files)

- [ ] **`src/smd/tree/finger_tree_foldable.t.cpp`**
  - Header: `#include <smd/tree/finger_tree_foldable.hpp>` (×2)
  - Tests: Fold operations on varied finger tree shapes
  - Lines: ~80–100
  - Validation: `ctest -R finger_tree_foldable`
  - Commit: "test: add finger_tree_foldable test file"

- [ ] **`src/smd/tree/finger_tree_traversable.t.cpp`**
  - Header: `#include <smd/tree/finger_tree_traversable.hpp>` (×2)
  - Tests: Shape preservation under traverse
  - Lines: ~80–100
  - Validation: `ctest -R finger_tree_traversable`
  - Commit: "test: add finger_tree_traversable test file"

- [ ] **`src/smd/tree/finger_tree_wrappers.t.cpp`**
  - Header: `#include <smd/tree/finger_tree_wrappers.hpp>` (×2)
  - Tests: Wrapper type conversions and adapters
  - Lines: ~60–80
  - Validation: `ctest -R finger_tree_wrappers`
  - Commit: "test: add finger_tree_wrappers test file"

- [ ] **`src/smd/tree/fringe_tree_foldable.t.cpp`**
  - Header: `#include <smd/tree/fringe_tree_foldable.hpp>` (×2)
  - Tests: Foldable operations on variant tree
  - Lines: ~80–100
  - Validation: `ctest -R fringe_tree_foldable`
  - Commit: "test: add fringe_tree_foldable test file"

**Subtotal:** 4 files, ~300–380 lines  
**Estimated Time:** 3–4 hours

**Phase 3 Total:** 9 files, ~750–940 lines  
**Estimated Time:** 7–9 hours  
**Overall Progress:** 85%

---

## Phase 4: Integrated Validation

### 4.1 Full Test Suite

- [ ] Run: `cd /workarea/cppnow26/trees && make test`
- [ ] Expected: All 170+ tests pass
- [ ] Check: No new warnings or regressions
- [ ] Validation criteria:
  - [ ] All tests pass (100%)
  - [ ] No new compiler warnings
  - [ ] No segfaults or hangs
  - [ ] Test output clean (no macro re-definition errors)

### 4.2 Compile Check

- [ ] Run: `cd /workarea/cppnow26/trees && make compile`
- [ ] Expected: All sources compile cleanly
- [ ] Check: No include-related warnings
- [ ] Validation criteria:
  - [ ] All files compile
  - [ ] No `-Wredefined-macros` warnings
  - [ ] No `-Winclude-guards` warnings
  - [ ] No circular dependency errors

### 4.3 Coverage Check (Optional)

- [ ] Run: `cd /workarea/cppnow26/trees && make coverage`
- [ ] Expected: New test files improve coverage
- [ ] Check: Overall coverage doesn't regress
- [ ] Validation criteria:
  - [ ] New files have >80% line coverage
  - [ ] No regression in overall coverage
  - [ ] HTML report generated

**Phase 4 Estimated Time:** 2 hours  
**Overall Progress:** 90%

---

## Phase 5: Review & Merge

### 5.1 Code Review Checklist

- [ ] All 28 retrofit files have second `#include` line
- [ ] All 28 retrofit files compile without errors
- [ ] All 9 new test files follow template
- [ ] All 9 new test files include tautological test
- [ ] All 9 new test files have 2–3 substantive tests
- [ ] No substantive code changes in retrofit (pure structure)
- [ ] All commits have clear, detailed messages
- [ ] No whitespace-only changes in unrelated files

### 5.2 Documentation Updates

- [ ] Verify `docs/codestyle.md` has new rule section
- [ ] Confirm examples are clear and correct
- [ ] Check plan document exists: `docs/test-coverage-audit-and-plan.md`
- [ ] Regenerate source snapshot if needed: `make -C trees docs-index`

### 5.3 Merge to Main

- [ ] All tests pass
- [ ] All code reviewed
- [ ] Commit message comprehensive
- [ ] Merge with `--no-ff`: `git merge --no-ff <branch-name>`
- [ ] Verify merge commit is on main

**Phase 5 Estimated Time:** 1 hour  
**Overall Progress:** 100%

---

## Summary & Sign-Off

| Phase | Task | Estimated | Actual | Status |
|-------|------|-----------|--------|--------|
| 1 | Documentation | 1 h | ✅ | ✅ COMPLETE |
| 2 | Retrofit 28 files | 15 h | | ⏳ Pending |
| 3 | Create 9 new files | 7–9 h | | ⏳ Pending |
| 4 | Validation | 2 h | | ⏳ Pending |
| 5 | Review & Merge | 1 h | | ⏳ Pending |
| **TOTAL** | | **26–28 h** | | **Awaiting implementation** |

---

## Notes

- Run `make test` after every 5–7 file modifications
- Use the template from plan document for consistency
- Keep commit messages clear and detailed
- Do not skip the double-include for any file
- Do not add tests to infrastructure headers (test_support.hpp, examples.hpp)

---

**Secondary Programmer:** _____________________ | **Date:** _________

**Reviewer:** _____________________ | **Date:** _________

**Approved for Merge:** _____________________ | **Date:** _________
