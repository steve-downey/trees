---
name: FT5 improvement campaign — complete
description: Four-phase campaign on FingerTree5 is fully done and pushed to origin/main; 488 tests pass; ready for Beman extraction
type: project
originSessionId: febb364f-d8bd-42ed-a1af-9c37e8675e7e
---
The FT5 improvement campaign (Phases 1–4) is complete and pushed to `origin/main`.

**Phase 1 — Code quality** (branch `ft5-code-quality`):
C++20 redundancies removed, `std::unreachable()` at assert sites, `[[nodiscard]]` on all pure ops, `SpinePtr{}` as typed null, `digit_to_vec` eliminated from `app3`, `overloaded` promoted to component with `consteval` exhaustiveness checking.

**Phase 2 — Benchmarks** (branch `ft5-benchmarks`):
Six-category std-comparison benchmark suite in `finger_tree_std_compare.bench.cpp`.

**Phase 3 — Performance** (branch `ft5-perf`):
H7 `flatten()` reserve, H6 `inplace_vector<EP,12>` in `app3`, H5 bottom-up `from_sequence`, H9 `head_ref()`/`last_ref()`.

**Phase 4 — Container/AllocatorAware/PMR** (branch `ft5-phase4`, merged as single --no-ff commit):
Full Container + ReversibleContainer named requirements, `bool empty() const` (default ctor resolves factory naming conflict), `AllocatorAware` with Lakos-rule coherency enforcement, `pmr::FingerTree5` alias, spine shell uses placement new for arena-safe allocation, allocator propagates through ALL sub-tree operations (view_l/r, tail, init, split, reversed), PMR probe proves zero global heap allocations.

**Current state**:
- `origin/main` at `6e0ce77` — clean, 488/488 tests pass
- All campaign worktrees still exist at `/workarea/ft5-*` but are merged and no longer needed
- The `codestyle.org` rule: Co-Authored-By trailer IS included in this project (user explicitly said "following codestyle.org")

**Why:** The full phase history is in the commit log with detailed messages explaining every design decision (allocator coherency, spine shell placement new, etc.). Do NOT rebase or squash this history — it is standardization evidence.

**Next up:** Beman extraction — see `docs/notes/beman-extraction-gaps.md` and `docs/notes/beman-migration-plan-v2.md` for the full pre-extraction checklist.
