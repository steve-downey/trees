# Copilot / Claude implementation rules

You are assisting with a C++ talk artifact and implementation proof-of-concept. The code must compile, tests must pass, and slide-transcluded examples must stay executable.

## Style and physical structure

Follow Bloomberg BDE-style conventions unless explicitly amended here.

Project amendments:

- Multiple namespace levels are allowed.
- Namespace levels correspond to directories.
- All project includes use canonical angle-bracket form, even within the same directory.
  - Good: `#include <smd/tree/fix_tree.hpp>`
  - Bad: `#include "fix_tree.hpp"`
- Use classical include guards, never `#pragma once`.
- Guard names use the full repo-relative path pattern:
  - `INCLUDE_SMD_TREE_FIX_TREE_HPP`
  - `INCLUDE_SMD_TYPECLASS_FOLDABLE_HPP`
- Avoid forward declarations except where strictly necessary for recursive type implementation.
- Prefer including the needed header over hand-written forward declarations.
- Keep functions generally out of line.
- For templates, define them out of class in the header.
- Definitions should be explicitly namespace-scoped so they visibly match declarations.

## Typeclass design

- Typeclass interfaces are concept-map-like records of named operations.
- Lookup is via variable-template-selected typeclass objects.
- Generic algorithms may also accept explicit or NTTP instance objects later.
- The datatype and its typeclass adaptations are separate concerns.
- Do not put Foldable/Applicative/Traversable specialization code directly into `fix_tree.hpp`.

## Foldable

- `fold_map` is the semantic center.
- `length`, `to_vector`, `fold_left`, and `fold_right` should be derived where practical.
- Tree order is part of the instance contract. Do not leave it implicit.

## Applicative

- The public surface is `invoke(f, ax, ay, ...)`.
- Implementor-facing primitives may be `pure` and `apply`.
- `beman::optional::optional` is the baseline Applicative test instance.
- Avoid using `std::optional` directly in this project unless explicitly requested.
- Tree Applicative semantics must be explicit. The primary slideware instance is shape-preserving zip-style behavior.
- Semantics that flatten, duplicate, expand, or reorder structure may be useful as alternate maps, but not as the primary tree Applicative.

## Traversable

- `traverse` is the minimal operation.
- Traversal preserves shape.
- Traversal uses the same documented order as Foldable.
- Effect order is observable and therefore part of the contract.

## Tests

- New tests use Catch2.
- Each test translation unit includes the header under test twice, adjacent, to confirm idempotency.
- Add law tests before adding performance tests.
- Keep optional Applicative test support separate from monoid test support.

## Slide transclusion

- Code shown in slides is extracted from real source using UUID anchors.
- Do not transclude include guards, duplicate includes, or physical boilerplate.
- Do transclude short executable examples.
- One UUID block should represent one slide concept.
- Do not nest UUID blocks.
- Do not invent illustrative code that does not compile.

## When uncertain

Choose the smaller, compiling, law-oriented implementation. This project is an existence proof; correctness and clarity beat cleverness.


## org-re-reveal and transclusion

- Keep slide code transclusion-ready with UUID comment anchors.
- Real source files keep include guards and physical boilerplate.
- Transcluded snippets should show the semantic core only.
- Prefer example files under `src/smd/typeclass/examples/` for slide snippets rather than transcluding production headers directly.
