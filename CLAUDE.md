# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build, test, lint

All workflows go through the top-level `Makefile`. The Python toolchain comes from `uv` (auto-creates `.venv` from `pyproject.toml`/`uv.lock`); CMake/CTest are invoked as `uv run cmake` / `uv run ctest`.

- `make test` — rebuilds and runs CTest (default target). Build dir is `.build/build-$(TOOLCHAIN)/`, default `TOOLCHAIN=clang`, default `CONFIG=Asan`. Generator is `Ninja Multi-Config` with configurations `RelWithDebInfo;Debug;Tsan;Asan;Gcov`.
- `make compile` — build only.
- `make compile-headers` — verifies interface header sets (`CMAKE_VERIFY_INTERFACE_HEADER_SETS=ON`); catches non-self-contained headers.
- `make coverage` / `make view-coverage` — Gcov + gcovr report at `$(_build_path)/coverage/coverage.html`.
- `make ctest` — run CTest without rebuilding. Pass a single test through CTest directly: `uv run ctest --test-dir .build/build-clang -C Asan -R <regex> --output-on-failure`.
- `make lint` — pre-commit-driven path used in CI (runs everything except clang-format, then batches clang-format).
- `make lint-local` — same checks without pre-commit hook environment downloads; installs `markdownlint`, `checkmake`, `gitleaks` into `.tools/`.
- `make compile_commands.json` — refresh the symlink to the active build's compile DB.
- `make realclean` — wipes `.build`, `.venv`, `.install`, `.emacs.d/elpa*`, `installtest/.build`.

CMakePresets (`gcc-debug`, `llvm-debug`, etc.) exist for IDE/CI workflows but the Makefile uses its own toolchain files under `etc/` and bypasses presets. Don't expect the two paths to share build dirs.

Default `_args` pins googletest to a local repo path (`/home/sdowney/bld/googletest/googletest.git`); active tests use Catch2 from `vendor/catch2`, not gtest.

## Presentation pipeline

The repo doubles as the source for a CppNow26 talk. `make presentation` regenerates `docs-refresh` (source snapshots + index), runs tests, and exports `foldable-applicative-traversable.org` to `.html`, `-slides.html`, and `.md` via batch Emacs (`.emacs.d/init.el`). Slide snippets are transcluded from real source files using UUID anchors — see "Slide and Transclusion Rules" in `docs/CODING_RULES.md`. The `slide_snippets_check` CMake target ensures presentation snippets stay compilable.

`make docs-live-src` and `make docs-live-src-strict` regenerate `docs/live-src-main.md` and `docs/live-src-main-built-targets.md` from `git HEAD` (not the working tree). The strict variant only includes files explicitly named in CMake `target_sources()`. These snapshots are excluded from most lint and codespell paths.

## Architecture: typeclass-object pattern

The repo is built around a single design idea: open extension with static dispatch via per-concept lookup objects. Read `docs/typeclass-object-pattern.md` before adding new instances or concepts — it is authoritative and supersedes older slideware notes.

- **Concept side** (`src/smd/typeclass/`): each concept (`functor`, `foldable`, `applicative`, `traversable`, `monad`, `monoid`) defines a generic user-facing algorithm (`smd::fmap`, `smd::fold_map`, `smd::invoke`, `smd::traverse`) that dispatches through a per-concept variable template named `*_typeclass<T>`.
- **Type side**: a datatype participates by specializing `*_typeclass<T>` in a separate adaptation header. Do **not** put Foldable/Applicative/Traversable specialization logic into the core data-structure header; it belongs in a designated adapter header.
- **Three lookup modes** must remain available for every typeclass:
  1. implicit lookup via the variable template,
  2. explicit object argument (`invoke_with(map, ...)`, `traverse_with(map, ...)`),
  3. NTTP pinning (`template <typename P, const auto& map = *_typeclass<P>>`).
- **Applicative contract**: minimal ops are `pure` + `apply`; `invoke(f, a, b, c)` is derived via terminating partial application in the `Applicative<Impl>` base. Implementations may override `invoke` for shape-aware semantics.
- **Algorithm composition pattern**: an algorithm that uses multiple ops can inherit from the looked-up `*_typeclass<T>` (stateless empty struct) to bring ops into unqualified scope. Working example: `src/smd/tree/fixpoint_tree_algorithm.hpp`.

`src/smd/conceptmap/` is a parallel historical surface using `*_concept_map<T>` and wrapper records (`Functor`, `Monoid`) with `std::false_type{}` defaults. It is teaching material, not the primary active surface — prefer `smd/typeclass/` for new work.

### Semantic defaults

These are project-wide invariants — older slideware that says otherwise is wrong:

- Default tree Applicative is monad-derived. Zip semantics are an *alternate* tree Applicative, not the default.
- Range Applicative models nondeterminism.
- `smd::zip_list<T>` Applicative is positional/parallel (Haskell `Control.Applicative` ZipList): `pure(x)` is infinite repetition; `apply`/`invoke` zips and truncates to the shortest finite input.

## Library targets

All `smd::*` libraries are header-only `INTERFACE` targets declared in `src/CMakeLists.txt` with file sets rooted at `src/`:

- `smd::typeclass` → concept algorithms + bases (depends on `beman.optional`)
- `smd::fixpoint` → `fix.hpp`, `cata.hpp`, `box.hpp` — the recursive-type machinery
- `smd::tree` → binary tree, fringe tree, fixpoint tree, finger tree (multiple versions `finger_tree2/3/4` plus wrappers, rope, priority queue, random-access, interval-index), plus per-tree `*_foldable.hpp` / `*_applicative.hpp` / `*_traversable.hpp` adapters
- `smd::ranges`, `smd::ziplist`, `smd::thunk` (`delay`, `memoize`)
- `smd::conceptmap` (historical surface)

Vendored deps: `vendor/catch2`, `vendor/optional` (beman.optional). `beman::optional` is the real dependency in production; `src/beman/optional/optional.hpp` exists only as a slide-isolation shim.

## Physical design (binding)

From `docs/CODING_RULES.md` and `README.md`:

- **Component trio**: each logical component is `<name>.hpp`, `<name>.cpp`, `<name>.t.cpp` (or `.test.cpp` for new standalone work), co-located under `src/<namespace-path>/`. No separate `include/`/`tests/` trees.
- **Include guards, never `#pragma once`**. Guard name follows the repo-relative path: `INCLUDE_SMD_TREE_FIX_TREE_HPP`.
- **Canonical includes**: project headers use angle brackets with the full namespace path (`<smd/tree/fix_tree.hpp>`). Never use `.` or `..` relative includes. Never rely on transitive includes.
- **Component header is included first** in each `.cpp`, and tests should `#include` the header under test **twice** to verify idempotent inclusion.
- **Functions out of line**, including template definitions in headers, with full namespace + class qualification. One exception: hidden friends for customization points.
- **No `using namespace` in headers**. Namespace levels mirror directory levels.
- **C++23** for new code; reach for C++26 facilities when the toolchain supports them. If an API can be `constexpr`, make it `constexpr` and add a compile-time test.
- **One sentence per line** in Markdown / Org / LaTeX prose; do not hard-wrap at fixed columns.

## Testing

- Catch2 only. The migration shim is gone — use native Catch2 includes and macros.
- Add law-focused tests before performance tests.
- Tree traversal order is part of the instance contract — write tests that would fail if fold order flips.
- For typeclass tests, prefer explicit map lookup (`const auto& map = smd::traversable_typeclass<Tree>;`) over deeply nested generic calls; the compile errors are clearer.

## Deadcode

`src/deadcode/` and `src/smd/tree/deadcode/` contain prior implementations kept for reference. They are excluded from the live-source snapshots and should not be cited as the current pattern. The active fix-tree implementation is `src/smd/tree/fixpoint_tree.hpp`, not `src/smd/tree/deadcode/fix_tree.hpp`.
