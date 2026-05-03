# Session Notes: 2026-05-02/03

## Work Completed

### Build & Infrastructure
- **Default toolchain to clang**: Both Makefiles now default `TOOLCHAIN=clang` (clang++ 21, C++26). System GCC 13 is opt-in via `TOOLCHAIN=system`. Created missing `oop/etc/clang-toolchain.cmake`.
- **Pre-commit vendor exclusions**: All three `.pre-commit-config.yaml` files exclude `vendor/`, `infra/`, `reveal.js/`, `research/` globally.
- **clang-format pass**: Applied to 83 files under `trees/src/`. Added `.git-blame-ignore-revs` to ignore in blame.
- **ccache**: Required for oop build (`ctest --instrument` wraps compiler with ccache). Added to DevX dotfiles note.

### Presentation Accuracy (Feynman Audit)
- **Systematic audit**: 10 factual claims checked against source code. 2 fixed:
  - "five more operations" for Applicative → now lists key ops explicitly (actually 8+)
  - "fold can only produce a flat result" → "fold discards the original branching shape"
- **Cross-language name mapping slides**: New section with tables for Functor, Foldable, Applicative, Traversable, Monoid mapping C++ ↔ Haskell ↔ Scala Cats ↔ PureScript. Core operation markers.
- **Alternate cores slide**: Documents `using Impl::primitive;` pattern and connects to Haskell's `{-# MINIMAL #-}`.
- **Slide claim corrections**: Removed stale "currently flattens and rebuilds" hedging. Corrected priority queue from "dual-tree" to "single tree with combined PriorityTag measure". Stated actual complexities.

### Code Changes
- **Foldable alternate-core**: `fold_right` can now serve as the primitive (matching Haskell's `{-# MINIMAL foldMap | foldr #-}`). Derived `fold_map` in `Foldable<Impl>` base uses `requires { typename Impl::element_type; }` to activate only for fold_right-primitive Impls. 6 new tests.
- **Alternate-core comments**: Added one-line comments at each `using Impl::primitive;` site in Foldable, Applicative, Traversable, and NullOptMap test helper.
- **Include order fixes**: `traversable.t.cpp` and `zip_list_applicative.t.cpp` now include their component header first (header-under-test rule).

### Documentation Cleanup
- **All review-followups.org items DONE** with commit SHAs. Zero unchecked boxes.
- **All analytical-review-accuracy follow-ups DONE**.
- **Test coverage checklist**: Summary table marked COMPLETE with SHAs.
- **Reference materials organized**: Renamed/moved PDFs, tracked untracked docs, deleted duplicate reviewer-three.org.
- **Stale worktrees removed**: 14 merged worktrees cleaned up.
- **Merged local branches deleted**: 17 branches.

## Key Findings

### Code is already O(log n)
The review-followups had stale TODO items claiming random-access and priority-queue operations were O(n). Investigation showed:
- **RandomAccess**: `at()` uses `split()` → `d_pivot` (O(log n)). `update/insert/erase` use `split()` + `concat()` (Hinze-Paterson app3 is O(log(min(n,m)))).
- **PriorityQueue**: Single tree with combined `PriorityTag<T>` measure. `pop_min/pop_max` use measure-guided `split()` + `concat()`. O(log n).
- The old dual-tree approach (MinTree + MaxTree) was replaced; cross-measure split issue is gone.

### Haskell cross-check
- All slide claims verified against local GHC docs (`trees/docs/haskell/`).
- Naming follows `std::ranges` conventions deliberately (not Haskell): `any_of`, `all_of`, `fold_left`, `fold_right`, `to_vector`, `empty`.
- `invoke` is unique to this C++ library — no equivalent in Haskell/Cats/PureScript.

### Compile regression note
- `trees/docs/reference/finger_tree_compile_regression_notes.md` documents a GCC-16-specific runaway template instantiation. The current `kMaxDepth = 10` depth guard with `if constexpr` and `SpineTerminal` fallback is the fix. We cannot reproduce the GCC 16 failure in this environment (only have GCC 13 + Clang 21).

## Current State
- 247/247 tests pass with bare `make test`
- Local main = origin/main = github/main (all in sync)
- 1 active worktree: `review-2026-05-02` (another agent's stale WIP)
- Working tree clean

## Open Items (very minor)
- oop project has a pre-existing build issue (`#include <print>` requires C++23 libstdc++ headers not available with system GCC 13 headers)
- Reformat pass for non-src files (CMakeLists, docs) not done — only `trees/src/` was formatted
- `trees/docs/strict-fingertree.md` could use a preface clarifying its role (target design vs reference)
