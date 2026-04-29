# Typeclass Object Pattern in This Repository

This repository uses a typeclass-object lookup approach to approximate the intent of C++03 concept maps.

The short version:
- A concept is represented by a tag plus a map object interface.
- A concrete type chooses behavior by specializing a lookup object.
- Generic algorithms call through that lookup object instead of relying on ADL overload sets.

## Why this exists

The goal is open extension with static dispatch:
- no virtual dispatch
- no mandatory type erasure
- explicit instance selection when needed
- predictable, testable instance lookup by type

This is close to the old concept-map idea: for a given type and concept, pick a specific implementation map.

## Two surfaces in this repo

There are two related implementations:

1. `src/smd/conceptmap/`
- Uses per-concept variable templates such as `functor_concept_map<T>` and `monoid_concept_map<T>`.
- Uses wrapper records (`Functor`, `Monoid`) to provide defaults.
- Defaults lookup to `std::false_type{}` and specializes for supported types.

2. `src/smd/typeclass/`
- Uses `map<Tag, T>` specializations with tags like `functor_tag`, `foldable_tag`, `applicative_tag`, `traversable_tag`.
- Uses per-concept implementation objects named `*_typeclass<T>` for lookup and override.
- Generic entry points are free functions (`fmap`, `fold_map`, `invoke`, `traverse`) that dispatch through `map<Tag, T>`.

Both are expressing the same design idea: behavior by typed map object, selected at compile time.

## Lookup modes (important)

The typeclass implementation object is an object and should be usable in three ways:

1. Implicit lookup by variable template
- Example shape: `functor_concept_map<T>` in the conceptmap surface.

2. Explicit object argument
- Call a generic algorithm with a specific map object when you want local policy control.

3. Non-type template parameter (NTTP) pinning
- A generic function can bind the looked-up map object as an NTTP default and use it directly.

This is not a cosmetic detail. It is how we keep instance selection explicit, testable, and overrideable while preserving static dispatch.

The conceptmap tests already demonstrate the NTTP style in practice:
- `template <typename P, const auto& functor = functor_concept_map<P>>`
- See `testP` and `testP2` in `src/smd/conceptmap/functor.t.cpp`.

For future work in `src/smd/typeclass/`, preserve this spirit even when dispatch is expressed as `map<Tag, T>` specialization. If additional wrapper APIs are introduced, they should keep explicit object override and NTTP pinning available for generic code.

Current naming convention in this subtree is `*_typeclass<T>` for implementation objects.

## Core mechanics

### Concept side

A concept contributes:
- a tag (`functor_tag`, `applicative_tag`, ...), or a per-concept lookup variable template in the conceptmap subtree
- generic user-facing algorithms (`fmap`, `length`, `invoke`, `traverse`)
- optional defaults in a wrapper record (conceptmap subtree)

### Type side

A type participates by adding a map specialization:
- conceptmap style: specialize `*_concept_map<T>` to an instance object
- typeclass style: specialize `map<Tag, T>` and provide required static operations

### Call side

Generic code calls concept algorithms, not type members directly:
- `smd::fmap(f, value)`
- `smd::fold_map(f, value)`
- `smd::invoke(fn, ax, ay)`
- `smd::traverse(f, tree)`

That call performs compile-time lookup to the right map specialization.

## How to add a new instance

1. Decide the concept and the concrete type/family.
2. Implement the map specialization close to the type adapter header.
3. Keep operation names aligned with existing concept APIs.
4. Add a `.t.cpp` test file with at least:
- breathing test
- one semantic behavior test
- one type-level expectation when meaningful
5. If snippets are used in slides, add a small example in `src/smd/typeclass/examples/` and anchor it with a UUID transclude marker.

## How to add a new concept

1. Add a new tag and generic entry-point API in `src/smd/typeclass/<concept>.hpp`.
2. Add `map<new_tag, T>` specializations for at least one concrete type.
3. Add tests in `src/smd/typeclass/<concept>.t.cpp`.
4. Add the header to `smd_typeclass` header file set in `src/smd/typeclass/CMakeLists.txt`.
5. If needed for talks, add an examples source file in `src/smd/typeclass/examples/` and include it in the slide examples object target.

## Testing and build wiring expectations

- Typeclass tests live under `src/smd/typeclass/*.t.cpp`.
- They are wired by `src/smd/typeclass/CMakeLists.txt` into `smd_typeclass_tests` when `TREE_ENABLE_TESTING` and `GTest_FOUND` are true.
- Slide snippet sources are compiled via `smd_typeclass_slide_examples`.
- Top-level target `slide_snippets_check` ensures presentation snippets remain compilable.

## Applicative: Derived invoke via terminating partial application

In Haskell, Applicative is minimal (`pure` and `(<*>)`) and `sequenceA`/`traverse`-style usage is naturally expressed by applying pure functions to effectful arguments.

In this C++ codebase, we now model the same intent by deriving `invoke` from `pure` and `apply` using a terminating partial-application adapter:

```
invoke(f, a, b, c) == ap(ap(ap(pure(partial(f)), a), b), c)
```

The helper object stores already-bound values and, on each call:
- invokes `f` when enough arguments have been collected, or
- returns a new callable waiting for the next argument.

That gives us a practical equivalent of Ben Deane's terminating partial-application technique while preserving the object-map lookup model.

Contract summary:
- Minimal required operations for Applicative Impl are `pure` and `apply`.
- `invoke` is provided by the base `Applicative<Impl>`.
- Implementations may still override `invoke` for custom semantics/performance (for example, shape-aware structures).

## Notes for future cleanup

- Keep one consistent naming story between `conceptmap` and `typeclass` surfaces to avoid conceptual drift.
- Prefer explicit object-map usage in tests when comparing alternate semantics.
- Preserve the lookup-first model; avoid introducing parallel ADL-only customization paths for the same concept.
