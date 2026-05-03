# Current-State Audit: 2026-05-02

**Scope**: Full worktree review after recent merge activity; focus on semantic correctness, test coverage, and repo health.  
**Status**: TREES BUILD PASSES `240/240` under `make TOOLCHAIN=clang test`; no blockers detected.  
**Intent**: Document concrete findings and repair items.

---

## Findings Summary

### 1. [CORRECTNESS] `split_at_index` fast-path contract violation (MEDIUM)

**Location**: [trees/src/smd/tree/finger_tree.hpp](trees/src/smd/tree/finger_tree.hpp#L1145-L1153)

**Issue**:
```cpp
if constexpr (std::is_same_v<Tag, std::size_t>) {
    // Prefix measure = running element count; navigate structurally, no flatten.
    return split_at([index](std::size_t prefix) { return prefix > index; });
} else {
    // Non-size_t tags don't accumulate position; fall back to flatten-and-rebuild.
    ...
}
```

The condition checks only `Tag == std::size_t` but **does not verify that `std::size_t` tags accumulate element counts**. A `FingerTree<T, std::size_t, WeightedMeasure>` with a non-count measure (e.g., cumulative weight) will incorrectly use the fast path, splitting by accumulated measure instead of by element index.

**Test Coverage**:
- ✅ `FingerTreeTest - SplitAtIndexConvenience` covers count-tag `split_at_index` (line 296).
- ✅ `FingerTreeTest - SplitAtMeasureConvenience` covers weighted `split_at_measure` with a non-`std::size_t` tag (line 314).
- ❌ **Missing**: `FingerTree<T, std::size_t, WeightedMeasure>::split_at_index()` with weighted-but-size_t-tagged tree. This case would expose the bug if executed.

**Severity**: Medium – the bug only manifests for the specific edge case of `std::size_t` tags on weighted measures. Most usage patterns (count tags, non-`std::size_t` measures) are unaffected.

**Proposed Fix**:

Replace the fast-path check with a stricter guard or add an `is_count_measure` predicate. For now, simplest fix is to remove the fast path and always flatten for safety:

```diff
// In trees/src/smd/tree/finger_tree.hpp, around line 1145
auto split_at_index(std::size_t index) const -> SplitAt
{
  if (index == 0U) {
    return SplitAt{empty(), *this};
  }
  if (index >= breadth()) {
    return SplitAt{*this, empty()};
  }
  // Remove the fast-path optimization for std::size_t; always use flatten-and-rebuild.
  // This is safe for now until we have a way to verify that Tag semantically means element count.
  auto vec = flatten();
  auto clamped = index > vec.size() ? vec.size() : index;
  std::vector<T> lv(vec.begin(),
    vec.begin() + static_cast<std::ptrdiff_t>(clamped));
  std::vector<T> rv(
    vec.begin() + static_cast<std::ptrdiff_t>(clamped), vec.end());
  return SplitAt{from_sequence(std::move(lv)),
                 from_sequence(std::move(rv))};
}
```

**Alternative (Longer-term)**:
Add a concept/constraint to require that `std::size_t` measures implement a `count_measure` marker or static `is_count: true`, and check it in the fast path.

**Action Items**:
- [ ] Apply the conservative fix (always flatten) to remove the incorrect fast path.
- [ ] Add regression test: `TEST_CASE("FingerTreeTest - SplitAtIndexWithWeightedSize_t")` in [trees/src/smd/tree/finger_tree.t.cpp](trees/src/smd/tree/finger_tree.t.cpp) that builds a `FingerTree<int, std::size_t, WeightedMeasure>` and verifies `split_at_index` splits by element count, not cumulative weight.
- [ ] Comment the fix to explain why the fast path was removed.

---

### 2. [REPO-HEALTH] Orphan dead test file masking broken include (MEDIUM)

**Location**: [trees/tests/ziplist.t.cpp](trees/tests/ziplist.t.cpp)

**Issue**:
```cpp
#include <smd/typeclass/zip_list.hpp>

int main() { return 0; }
```

This file:
- Lives in `trees/tests/` instead of alongside the real tests at `trees/src/smd/ziplist/`.
- **No longer** references a valid header (the real ziplist tests in `trees/src/smd/ziplist/zip_list_applicative.t.cpp` exist and are wired into the build via [trees/src/smd/ziplist/CMakeLists.txt](trees/src/smd/ziplist/CMakeLists.txt#L19)).
- Contains only a no-op `main()`.
- Build succeeds because `ziplist_tests` executable is not discovered from this directory; the real tests run via CMake's `catch_discover_tests()` from the `src/smd/ziplist/` target.

**Why It Matters**:
This is a **build-masked dead file** — it compiles but is never executed. Future maintainers may:
1. Assume all tests under `trees/tests/` are live.
2. Try to add tests here, unaware they won't run.
3. Wonder why the file exists at all.

**Action Items**:
- [ ] **Delete** [trees/tests/ziplist.t.cpp](trees/tests/ziplist.t.cpp) entirely.
- [ ] Confirm real ziplist tests still run: `make TOOLCHAIN=clang test 2>&1 | grep -i ziplist`.
- [ ] Commit with message: `Remove orphan dead test file trees/tests/ziplist.t.cpp; real tests are in src/smd/ziplist/`.

---

### 3. [REPO-HEALTH] Broad formatting churn in vendored Catch2 (LOW-TO-MEDIUM)

**Location**: [trees/vendor/catch2/](trees/vendor/catch2/) (~403 files changed)

**Issue**:
Extensive automated formatting changes across vendored Catch2 code:
- CMake files: brace/indentation styles, function call reformatting.
- C++ examples: pointer/reference spacing (`T&` → `T &`), brace placement.
- Docs: trailing whitespace, line breaks.

**Why It Matters**:
1. **Subtree conflicts**: If Catch2 is updated via `git subtree`, these formatting changes will complicate diffs and merges.
2. **Diff noise**: Large formatting PRs obscure real project changes in review.
3. **Maintenance burden**: Carrying divergent style from upstream makes syncing harder.

**Severity**: Low to medium operationally, but signals that the repo may be applying a formatter globally without excluding vendor/ directories.

**Action Items**:
- [ ] Check `.clang-format`, `.prettierrc`, or equivalent formatter config to confirm if vendor/ is properly excluded.
- [ ] If formatter is not excluding vendor/, update `.clang-format` (or equivalent) to add:
  ```
  IgnoreDirectories: [vendor]
  ```
  (or equivalent in your formatter).
- [ ] Consider reverting vendor/ changes in the next commit to maintain upstream tracking:
  ```bash
  git checkout HEAD~1 -- trees/vendor/catch2/
  git add -- trees/vendor/catch2/
  git commit -m "Revert vendor/catch2 formatting to upstream state"
  ```
- [ ] Document in a CI/pre-commit hook or contributor guide that vendor/ is not formatted locally.

---

### 4. [MINOR] Benign CMake and C++ formatting changes

**Locations**: 
- [trees/src/smd/ziplist/CMakeLists.txt](trees/src/smd/ziplist/CMakeLists.txt) — multi-line indentation (no semantic change).
- [trees/src/smd/typeclass/](trees/src/smd/typeclass/), [trees/src/smd/tree/](trees/src/smd/tree/), etc. — consistent pointer/reference and brace reformatting across all `.hpp` and `.t.cpp` files.

**Assessment**: These are cosmetic and appear to be from a single pass of an automated formatter (likely clang-format). No semantic or correctness impact.

---

### 5. [MINOR] Test-helper alternate-core declaration reorder (LOW)

**Location**: [trees/src/smd/typeclass/traversable.t.cpp](trees/src/smd/typeclass/traversable.t.cpp) (NullOptMap class)

**Issue**: The `using` declarations in test helper `NullOptMap` were reordered from `{ using Impl::pure; using Impl::apply; }` to `{ using Impl::apply; using Impl::pure; }`.

**Assessment**: 
- **CLAUDE.md** emphasizes that `using` declarations in Map classes implement the alternate-core pattern: whichever operation is listed first is the primitive; the base class derives the other.
- For `NullOptMap` (a test-only fixture), both `pure` and `apply` are equally primitive, so reordering has **no semantic impact**.
- However, the reorder suggests the editor may not be aware of the alternate-core contract, so it's worth a code review comment.

**Action**: Low priority; consider adding a brief inline comment in test helpers to document the alternate-core pattern:
```cpp
struct NullOptMap : Applicative<NullOptApplicativeImpl> {
  // Alternate-core: both pure and apply are primitives (symmetric).
  using NullOptApplicativeImpl::apply;
  using NullOptApplicativeImpl::pure;
};
```

---

## Summary of Action Items

| Item | File(s) | Priority | Action |
|------|---------|----------|--------|
| split_at_index fast-path bug | finger_tree.hpp, finger_tree.t.cpp | MEDIUM | Apply conservative fix (remove fast path); add regression test |
| Orphan ziplist.t.cpp | trees/tests/ziplist.t.cpp | MEDIUM | Delete file |
| Vendor formatting churn | trees/vendor/catch2/ (~403 files) | LOW–MED | Exclude vendor/ from formatter config; consider reverting |
| CMake/C++ cosmetic formatting | src/smd/ | NONE | No action (cosmetic, committed) |
| Test-helper alternate-core docs | typeclass/traversable.t.cpp | LOW | Add inline comment (optional) |

---

## Validation

**Current Status**: ✅ **All 240 tests pass** under `make TOOLCHAIN=clang test`.

**Next Steps** (in priority order):
1. Apply split_at_index fix + regression test.
2. Delete orphan ziplist.t.cpp.
3. Update formatter config to exclude vendor/.
4. (Optional) Add alternate-core documentation comment in test helpers.

---

**Created**: 2026-05-02  
**Reviewed by**: GitHub Copilot  
**Ticket**: current-state-audit-2026-05-02
