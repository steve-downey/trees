# Test Coverage Audit: Findings & Implementation Plan
**Date:** 2026-05-01
**Scope:** trees/src/ (excluding deadcode/, conceptmap/)
**Status:** ✅ Audit complete, documentation updated, plan ready for implementation

---

## 🎯 Core Findings

### Rule Violation Summary

| Metric | Count | Status |
|--------|-------|--------|
| **Total `.hpp` files (active)** | 37 | ✅ Inventoried |
| **With test files (`.t.cpp`)** | 28 | ✅ Verified |
| **Missing test files** | 10 | ⚠️ **Violations found** |
| **Existing tests needing retrofit** | 28 | ⚠️ **Retrofit needed** |
| **Test files now passing** | 28 | ✅ Baseline established |

### Critical Rule: Every `.hpp` = Component = Must Have `.t.cpp`

**User's mandate:**
> "Any .hpp file should have a .t.cpp (or .test.cpp file) that includes it as the first substantive line of code, and then includes it again to assure no re-inclusion errors and idempotency."

**Status:** ❌ **Currently violated in 10 components** ⚠️

---

## 📋 Missing Test Files (10 Violations)

### HIGH PRIORITY (Core Components) — 5 files

| Component | Location | Notes |
|-----------|----------|-------|
| `range_list` | `src/smd/ranges/range_list.hpp` | Public adapter + typeclass instances |
| `binary_tree` | `src/smd/tree/binary_tree.hpp` | Core tree data structure |
| `fix_tree` | `src/smd/tree/fix_tree.hpp` | Minimal fixed-point tree |
| `memoized_thunk` | `src/smd/tree/memoized_thunk.hpp` | Lazy evaluation with caching |
| `zip_list` | `src/smd/ziplist/zip_list.hpp` | Finite/infinite list abstraction |

### MEDIUM PRIORITY (Typeclass Wrappers) — 4 files

These are thin wrapper headers for typeclass instances:

| Component | Location | Base Type |
|-----------|----------|-----------|
| `finger_tree_foldable` | `src/smd/tree/finger_tree_foldable.hpp` | `FingerTree` |
| `finger_tree_traversable` | `src/smd/tree/finger_tree_traversable.hpp` | `FingerTree` |
| `finger_tree_wrappers` | `src/smd/tree/finger_tree_wrappers.hpp` | `FingerTree` |
| `fringe_tree_foldable` | `src/smd/tree/fringe_tree_foldable.hpp` | `FringeTree` |

### Infrastructure Headers (Exempt)

These have headers but are **not independent components**:

- `src/smd/typeclass/test/test_support.hpp` — Law-checking utilities for tests
- `src/smd/typeclass/examples/examples.hpp` — Example code for slides

---

## 🔍 New Test File Requirement: Double-Include Pattern

### Why This Matters

```cpp
// PROBLEM: Missing re-inclusion safety check
#include <acme/net/socket.hpp>
#include <catch2/catch_test_macros.hpp>

TEST_CASE("SocketTest") { ... }

// SOLUTION: Add double-include verification
#include <acme/net/socket.hpp>
#include <acme/net/socket.hpp>  // ← Catches include guard bugs!

#include <catch2/catch_test_macros.hpp>

TEST_CASE("SocketTest - HeaderIsIdempotent")
{
    REQUIRE(true);  // ← TDD bootstrap: always passes, signals build success
}

TEST_CASE("SocketTest - SemanticBehavior")
{
    // Substantive tests follow...
}
```

### What This Catches

✅ **Missing include guards** — header fails to compile on second inclusion
✅ **Macro pollution** — first include pollutes namespace, second fails
✅ **Circular dependencies** — hidden by single-file compilation
✅ **Partial definitions** — missing forward declarations surface

---

## 📚 Rule Changes to `docs/codestyle.md`

**New section added:**
> "Test file double-include and TDD bootstrap pattern"

**Key mandates:**
1. Every test file includes the target header **twice** (consecutive lines)
2. Every test file begins with a **tautological test** (`REQUIRE(true)`)
3. Template provided with clear examples
4. Rationale documented (re-inclusion safety, build coherency, TDD discipline)

**Location in document:**
After "Test naming and placement" section (line ~286)

---

## 📊 Implementation Plan (Ready for Secondary Programmer)

### Phase 1: Documentation ✅ **COMPLETE**
- ✅ Added rule to `docs/codestyle.md`
- ✅ Created comprehensive plan: `docs/test-coverage-audit-and-plan.md`
- ✅ Documented exceptions and rationale
- ✅ Committed to main: `2c9329f`

### Phase 2: Retrofit Existing Tests (15 hours)
- 28 existing test files need second `#include` line added
- No logic changes, pure structure
- Validation: `make test` after each batch (every ~7 files)

### Phase 3: Create New Test Files (7–9 hours)
- 9 new `.t.cpp` files (one per missing component)
- Each includes: tautological test + 2–3 substantive tests
- Templates and guidelines provided in plan document

### Phase 4: Validation (2 hours)
- Full test suite: `make test` (170+ tests)
- Compile check: `make compile` (no warnings)
- Coverage check: `make coverage` (no regressions)

### Phase 5: Review & Merge (1 hour)
- Verify all double-includes present
- Check test quality and coverage
- Update `docs/live-src-main-built-targets.md` snapshot
- Merge to main with clear commit message

**Total Effort:** 26–28 hours (3–4 days)

---

## 📄 Deliverables Created

### 1. **Comprehensive Audit Plan**
📄 File: `trees/docs/test-coverage-audit-and-plan.md`
- 450+ lines
- Detailed violation table
- Phase-by-phase breakdown
- 28 file lists (retrofit + new files)
- Timeline, acceptance criteria, debugging tips
- Suitable for secondary programmer to implement

### 2. **Updated House Style**
📄 File: `trees/docs/codestyle.md` (modified)
- Added new subsection with templates
- Documented double-include pattern
- Explained rationale and TDD bootstrap
- Provided clear examples
- Historical note about retrofit need

### 3. **This Audit Summary**
📄 File: `AUDIT_FINDINGS.md` (this file)
- High-level overview of violations
- Missing files table
- Rationale for new rules
- Quick reference for stakeholders

---

## ✨ Next Steps for Implementation

### For Secondary Programmer:

1. **Read the full plan:**
   ```bash
   cat trees/docs/test-coverage-audit-and-plan.md
   ```

2. **Start Phase 2 (Retrofit):**
   - Pick a directory (e.g., `src/smd/typeclass/`)
   - Add second `#include <smd/typeclass/functor.hpp>` line to each `.t.cpp`
   - Run: `make test`
   - Commit when tests pass

3. **Progress to Phase 3 (New Files):**
   - Use template from plan document
   - Start with HIGH priority files (simpler components)
   - One file at a time; validate with `make test`

4. **Final validation (Phase 4):**
   - Run `make test` (full suite)
   - Run `make compile` (no warnings)
   - All 170+ tests pass, 0 failures

### For Reviewer:

- [ ] Verify all 28 retrofit files have double `#include` lines
- [ ] Confirm tautological test is first in each file
- [ ] Check new test files have 2–3 substantive tests
- [ ] Validate `make test` passes 100%
- [ ] Ensure no coverage regressions
- [ ] Approve commit with detailed message

---

## 📈 Impact & Benefits

**Immediately after Phase 5:**

✅ 37 active components each have test coverage
✅ 100% test file compliance with double-include rule
✅ Header hygiene verified at compile time
✅ TDD discipline enforced (bootstrap test first)
✅ Circular dependency issues caught early
✅ Refactoring confidence increased

**Long-term:**

✨ New components follow pattern by default
✨ Maintenance overhead minimal (one pattern)
✨ CI/CD can verify rule compliance automatically
✨ Documentation reflects actual practices

---

## 🔗 Related Files

- **Plan:** `/workarea/cppnow26/trees/docs/test-coverage-audit-and-plan.md`
- **Style:** `/workarea/cppnow26/trees/docs/codestyle.md` (modified)
- **Commit:** `2c9329f` (docs: add test coverage audit and double-include rule)

---

## 📝 Summary

**Rule:**
> Every `.hpp` file is a component. Every component must have a `.t.cpp` or `.test.cpp` test file that includes the header **twice** (for re-inclusion safety) and begins with a **tautological test** (for build coherency).

**Current state:**
- ✅ 28 tests pass (baseline)
- ❌ 10 components missing tests
- ⚠️ 28 tests lack double-include pattern

**Plan:**
- Documentation: ✅ **Complete** (rule + template + 450-line detailed plan)
- Implementation: 📋 **Ready for handoff** (26–28 hours, 5 phases, clear checklists)

**Status:**
🟢 **Audit Complete | Documentation Ready | Plan Finalized | Awaiting Implementation**
