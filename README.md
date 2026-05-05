# CppNow26 Foldable/Applicative/Traversable slideware scaffold

This package contains a small, compile-oriented scaffold for the talk plan around implementing `Foldable`, `Applicative`, and `Traversable` in C++ using the concept-map style.

The examples are intentionally small. They are meant to be transcluded into slides, compiled as ordinary source, and replaced incrementally by the real implementation in the `cppnow26/trees` repository.

## Physical design rules

- Use classical include guards, not `#pragma once`.
- Guard names follow the repo-relative path, for example `INCLUDE_SMD_TREE_FIX_TREE_HPP`.
- Project includes use canonical angle-bracket form, for example `<smd/tree/fix_tree.hpp>`.
- Avoid forward declarations except where strictly necessary for recursive type definitions.
- Keep typeclass adaptation headers separate from the datatype header.
- Keep functions generally out of line, including template definitions in headers.
- Prefer explicit namespace-scoped definitions so declarations and definitions stay visibly aligned.
- Tests include the header under test twice to verify idempotent inclusion.

## Transclusion rule

Real source files keep all physical correctness details. Slide snippets should be extracted with UUID anchors and omit include guards and duplicate include checks.

Example:

```org
#+transclude: [[file:src/smd/typeclass/examples/applicative_examples.cpp::3f0c8d0e-9a6b-4a3e-9c2a-0c1e9c3d4f11]]
:lines 2- :src cpp :end "3f0c8d0e-9a6b-4a3e-9c2a-0c1e9c3d4f11 end"
```

## Included examples

- `foldable_examples.cpp`: generic `smd::length(tree)` payoff.
- `applicative_examples.cpp`: `smd::invoke` over `beman::optional::optional`.
- `traversable_examples.cpp`: tree traversal through an optional-like Applicative.
- `applicative_bad.cpp`: slide example of a compiling but semantically wrong primary tree Applicative.

## ZipList semantics

- `smd::zip_list<T>` follows the standard ZipList Applicative semantics (`Control.Applicative`).
- `pure(x)` is represented as an infinite repetition of `x`.
- `apply` and `invoke` zip positionally, truncating to the shortest finite input.

## Beman optional note

This package includes a tiny compatibility header at `src/beman/optional/optional.hpp` so the examples compile in isolation. In the real repo, prefer the real `beman::optional` dependency.

## Lint workflow

- Use `make lint` for the pre-commit-driven lint path used by CI.
- Use `make lint-local` when you need the same checks without pre-commit hook environment downloads.

## Deck entry point

- `example.org`: org-re-reveal presentation entry point intended to drop into the `example` template project.

## Contributor handoff

- `docs/typeclass-object-pattern.md`: how the typeclass-object lookup pattern works in this repo, and how to extend it safely.
