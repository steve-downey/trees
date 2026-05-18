---
name: Beman extraction — pending, pre-requisites identified
description: The Beman Project extraction has 7 pre-requisites that must complete before any file moves; detailed plans exist in docs/notes/
type: project
originSessionId: febb364f-d8bd-42ed-a1af-9c37e8675e7e
---
The Beman extraction of `trees` into a Beman Project library (`beman::structure`) has not started yet.

**Key planning documents** (all in `docs/notes/`):
- `beman-extraction-gaps.md` — seven concrete gaps, each a pre-requisite
- `beman-migration-plan-v2.md` — the updated full extraction plan
- `beman-integration-plan.md` — the original plan (still useful for background)

**The 7 pre-requisites (from beman-extraction-gaps.md):**
1. Choose history-preservation mechanism (recommend `git filter-repo` with path renames — preserves all Phase 1–4 commit messages that explain design decisions to LEWG reviewers)
2. Decide PMR header surface (top-level `finger_tree_pmr.hpp` vs folded alias)
3. Add PMR probe to mapping (`finger_tree5_pmr_probe.cpp` → `tests/beman/structure/finger_tree_pmr_probe.cpp` — the zero-global-alloc proof)
4. Add `docs/finger-tree5-allocator-design.md` to mapping → Beman `docs/`
5. Add all of `src/smd/fixpoint/` to mapping (`overloaded.hpp` is a direct dependency of `finger_tree5.hpp`)
6. Add benchmark suite to mapping
7. Design `recursive_fold.hpp` / `recursive_build.hpp` interfaces (these are API design, not file copies)

**Also needed before moves:**
- Mechanical rewrite script (include guards, namespaces, include paths, type names) — must be tested before any file moves
- Stable non-versioned names for measure policy types (`UnitMeasure5` → `unit_measure`, etc.)
- Two-repo sync policy (documented in beman-migration-plan-v2.md §0.7)
- `*_ft5.t.cpp` merge decision (recommend: merge into canonical wrapper tests)

**Working short name:** `structure` (as in `beman::structure`, `include/beman/structure/`)
**P3200 reserved** for Paper A (traversal/customization anchor paper)

**Key file mappings** (decided):
- `finger_tree5.hpp` → `include/beman/structure/finger_tree.hpp` (drop version suffix)
- `finger_tree5_iterator.hpp` → `include/beman/structure/detail/finger_tree_iterator.hpp`
- `finger_tree_random_access.hpp` → `include/beman/structure/random_access.hpp`
- `finger_tree_rope.hpp` → `include/beman/structure/rope.hpp`
- FT2/FT3/FT4 stay in `trees` (pedagogy/archaeology)

**Standardization docs published:**
- `docs/proposal-strategy.org` — the inverted-pyramid strategy document in org-mode (published 2026-05-18)
- `docs/notes/standardization-inverted-triangle-plan.md` — rewritten with correct inverted pyramid structure
