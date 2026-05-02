# Re-Review: May 2 Finger Tree Merge Chunks

Date: 2026-05-02  
Scope: latest merged chunks on main (`df3ef80`, `687e5cb`) and backing commits (`35aa4c9`, `a29a623`)  
Primary files reviewed:
- `trees/src/smd/tree/finger_tree.hpp`
- `trees/docs/notes/slide-snapshot-still-relevant-todos.org`

## Summary

The two chunks are directionally good:
- Internal type scaffolding moved to `smd::tree::detail` (API hygiene improvement).
- `split_at_index` gained a structural path when measure can guide index split.

However, one correctness guard is missing and one review note now overstates guarantees.

## Findings (ordered by severity)

## 1) High: `split_at_index` optimization can be wrong for custom `size_t` measure policies

Location:
- `trees/src/smd/tree/finger_tree.hpp` around `split_at_index` (current branch condition uses only `Tag == std::size_t`).

Problem:
- The fast path assumes prefix measure equals element count.
- Current condition is `std::is_same_v<Tag, std::size_t>`, which is insufficient.
- A custom measure policy can also use `std::size_t` while representing weighted totals, not element positions.
- In that case, `split_at([index](std::size_t prefix){ return prefix > index; })` becomes semantically incorrect for index splits.

Impact:
- `split_at_index(n)` may split at the wrong boundary for weighted `size_t` tags.
- This is a silent logic bug because most existing tests use count-based/default measure.

Recommended fix:
- Restrict the structural fast path to the known count-measure case only.
- Keep flatten fallback for all other measure policies.

## 2) Medium: no regression test locks this corner case

Location:
- `trees/src/smd/tree/finger_tree.t.cpp`

Problem:
- Existing tests cover `split_at_index` for default/count measure trees.
- No test asserts index semantics under a non-count `std::size_t` measure policy.

Impact:
- Future refactors can reintroduce the bug unnoticed.

Recommended fix:
- Add a regression test with a weighted `size_t` measure policy that verifies `split_at_index` still splits by element index.

## 3) Low: review note wording overstates optimization scope and test count is stale

Location:
- `trees/docs/notes/slide-snapshot-still-relevant-todos.org` in DONE item `wrapper-impl-measure-exploitation`.

Problem:
- Current note implies the new structural branch is generally tied to `TAG_TYPE = std::size_t`; that can be misread as always safe.
- It also states `215/215 tests pass`, but current suite is 240 tests.

Impact:
- Documentation drift / potential future misunderstanding.

Recommended fix:
- Clarify that the fast path is valid only for count semantics.
- Update stale test-count statement.

---

## Worker Patch Set

Apply these patches in order.

### Patch 1: Guard fast path by count measure semantics

File: `trees/src/smd/tree/finger_tree.hpp`

```diff
@@
-    if constexpr (std::is_same_v<Tag, std::size_t>) {
-      // Prefix measure = running element count; navigate structurally, no flatten.
+    if constexpr (std::is_same_v<Tag, std::size_t>
+                  && std::is_same_v<MeasurePolicy, UnitMeasure<T, Tag>>) {
+      // Prefix measure is exactly running element count; navigate structurally,
+      // no flatten.
       return split_at([index](std::size_t prefix) { return prefix > index; });
     } else {
-      // Non-size_t tags don't accumulate position; fall back to flatten-and-rebuild.
+      // For non-count measures (including weighted size_t), split_at_index must
+      // preserve index semantics, so fall back to flatten-and-rebuild.
       auto vec = flatten();
```

Why this is safe:
- Keeps optimization for the canonical count-measure case.
- Preserves correctness for all custom measure policies.

### Patch 2: Add regression test for weighted `size_t` measure policy

File: `trees/src/smd/tree/finger_tree.t.cpp`

```diff
@@
 TEST_CASE("FingerTreeTest - SplitAtIndexConvenience") {
@@
 }
+
+TEST_CASE("FingerTreeTest - SplitAtIndexUsesElementIndexForWeightedSizeTMeasure") {
+    struct WeightedSizeMeasure {
+        auto operator()(int value) const -> std::size_t {
+            return static_cast<std::size_t>(value * 10);
+        }
+    };
+
+    using WeightedTree =
+        smd::tree::FingerTree<int, std::size_t, WeightedSizeMeasure>;
+
+    auto tree = WeightedTree::from_sequence({1, 2, 3, 4});
+
+    // split_at_index(2) must split by element position, not accumulated measure.
+    auto split = tree.split_at_index(2U);
+    CHECK(split.d_left.flatten() == (std::vector<int>{1, 2}));
+    CHECK(split.d_right.flatten() == (std::vector<int>{3, 4}));
+}
```

Notes:
- This test will fail with the current broad `Tag == std::size_t` fast path.
- It should pass after Patch 1.

### Patch 3: Update DONE-note wording and stale test count

File: `trees/docs/notes/slide-snapshot-still-relevant-todos.org`

```diff
@@
-Resolution: ~FingerTreeRandomAccess~ ~at~, ~insert~, ~erase~, ~update~ rewritten to use structural
-~split()~ / ~split_at()~ with predicate ~prefix > index~ — O(log n) instead of O(n).  ~split_at_index()~
-in ~finger_tree.hpp~ gained an ~if constexpr~ branch: when ~TAG_TYPE = std::size_t~, it delegates
-to ~split_at()~ (no flatten); non-size_t tags retain the flatten fallback.
+Resolution: ~FingerTreeRandomAccess~ ~at~, ~insert~, ~erase~, ~update~ rewritten to use structural
+~split()~ / ~split_at()~ with predicate ~prefix > index~ — O(log n) instead of O(n).  ~split_at_index()~
+in ~finger_tree.hpp~ gained an ~if constexpr~ branch for the count-measure case; non-count tags/
+policies retain the flatten fallback to preserve index semantics.
@@
-215/215 tests pass.
+240/240 tests pass.
```

---

## Validation checklist for worker agent

1. Run:
- `make TOOLCHAIN=clang test`

2. Confirm:
- New regression test is present and passing.
- No behavior change in existing split/index tests.

3. Optional sanity check:
- `make TOOLCHAIN=clang presentation`

---

## Reviewer acceptance criteria

- Correctness: weighted `size_t` measure no longer mis-splits by index.
- Coverage: regression test prevents reintroduction.
- Documentation: DONE note reflects actual scope and current test totals.
