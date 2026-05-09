<!-- markdownlint-disable MD013 -->

# Typeclass Object Pattern in This Repository

This repository uses a typeclass-object lookup approach to approximate the intent of C++03 concept maps.

The short version:

- A concept is represented by a typeclass object interface.
- A concrete type chooses behavior by specializing a lookup object.
- Generic algorithms call through that lookup object instead of relying on ADL overload sets.

## Why this exists

The goal is open extension with static dispatch:

- no virtual dispatch
- no mandatory type erasure
- explicit instance selection when needed
- predictable, testable instance lookup by type

This is close to the old concept-map idea.
For a given type and concept, pick a specific implementation map.

## Two surfaces in this repo

There are two related implementations:

1. `src/smd/conceptmap/`

- Uses per-concept variable templates such as `functor_concept_map<T>` and `monoid_concept_map<T>`.
- Uses wrapper records (`Functor`, `Monoid`) to provide defaults.
- Defaults lookup to `std::false_type{}` and specializes for supported types.
- In current workspace state, this surface is mostly historical/teaching material and is not the primary active test surface.

1. `src/smd/typeclass/`

- Uses per-concept implementation objects named `*_typeclass<T>` for lookup and override.
- Generic algorithms dispatch through those looked-up objects.

Both are expressing the same design idea.
Behavior is selected by typed map object at compile time.

## Lookup modes (important)

The typeclass implementation object is an object and should be usable in three ways:

1. Implicit lookup by variable template.

- Example shape: `functor_concept_map<T>` in the conceptmap surface.

1. Explicit object argument.

- Call a generic algorithm with a specific map object when you want local policy control.

1. Non-type template parameter (NTTP) pinning.

- A generic function can bind the looked-up map object as an NTTP default and use it directly.

This is not a cosmetic detail.
It keeps instance selection explicit, testable, and overridable while preserving static dispatch.

The conceptmap tests already demonstrate the NTTP style in practice:

- `template <typename P, const auto& functor = functor_concept_map<P>>`
- See `testP` and `testP2` in `src/smd/conceptmap/functor.t.cpp`.

For future work in `src/smd/typeclass/`, preserve this spirit through `*_typeclass<T>` lookup objects.
If additional wrapper APIs are introduced, they should keep explicit object override and NTTP pinning available for generic code.

In the current typeclass headers this is exposed directly via helper APIs such as:

- `invoke_with(map, ...)` and `invoke_with<map>(...)` in Applicative
- `traverse_with(map, ...)` in Traversable
- NTTP-pinned helper templates in foldable tests and tree tests

Current naming convention in this subtree is `*_typeclass<T>` for implementation objects.

## Core mechanics

### Concept side

A concept contributes:

- a per-concept lookup variable template in the typeclass subtree
- generic user-facing algorithms (`fmap`, `length`, `invoke`, `traverse`)
- optional defaults in a wrapper record (conceptmap subtree)

### Type side

A type participates by specializing the per-concept lookup object:

- conceptmap style: specialize `*_concept_map<T>` to an instance object
- typeclass style: specialize `*_typeclass<T>` to an instance object

### Call side

Generic code calls concept algorithms, not type members directly:

- `smd::fmap(f, value)`
- `smd::fold_map(f, value)`
- `smd::invoke(fn, ax, ay)`
- `smd::traverse(f, tree)`

That call performs compile-time lookup to the right typeclass object specialization.

## How to add a new instance

1. Decide the concept and the concrete type or type family.
1. Implement the `*_typeclass<T>` specialization close to the type adapter header.
1. Keep operation names aligned with existing concept APIs.
1. Add a `.t.cpp` test file with at least:

- a breathing test
- one semantic behavior test
- one type-level expectation when meaningful

1. If snippets are used in slides, add a small example in `src/smd/typeclass/examples/` and anchor it with a UUID transclude marker.

## How to add a new concept

1. Add a new tag and generic entry-point API in `src/smd/typeclass/<concept>.hpp`.
1. Add `*_typeclass<T>` specializations for at least one concrete type.
1. Add tests in `src/smd/typeclass/<concept>.t.cpp`.
1. Add the header to `smd_typeclass` header file set in `src/smd/typeclass/CMakeLists.txt`.
1. If needed for talks, add an examples source file in `src/smd/typeclass/examples/` and include it in the slide examples object target.

## Testing and build wiring expectations

- Catch2 is the active and only test framework in active code paths.
- Typeclass-object behavior is tested not only in `src/smd/typeclass/*.t.cpp`, but also in tree, ranges, and ziplist test suites that instantiate and use `*_typeclass<T>` maps.
- The compatibility shim used during migration has been removed.
- Tests should use native Catch2 includes and macros.
- Slide snippet sources are compiled via `smd_typeclass_slide_examples`.
- Top-level target `slide_snippets_check` ensures presentation snippets remain compilable.

## Algorithm objects: Inheriting from typeclass instances

An algorithm that composes multiple typeclass operations can inherit from the typeclass instance, bringing those operations into unqualified scope as inherited members.

The typeclass objects are stateless empty structs, so inheritance adds no data and slicing is not a concern.

### Pattern

```cpp
namespace detail {

template <class T,
          const auto& TC = smd::traversable_typeclass<smd::remove_cvref_t<T>>>
struct validate_impl : smd::remove_cvref_t<decltype(TC)> {
    template <class Pred>
    auto call(Pred&& pred, const T& value) const {
        // for_each is an inherited member — no qualification needed
        return this->for_each(value, [&](const auto& elem) -> std::optional<...> {
            if (pred(elem)) return {elem};
            return std::nullopt;
        });
    }
};

} // namespace detail

// Public CPO: single object, deduces T from argument
struct validate_fn {
    template <class Pred, class T>
    auto operator()(Pred&& pred, T&& value) const {
        return detail::validate_impl<smd::remove_cvref_t<T>>{}.call(
            std::forward<Pred>(pred), std::forward<T>(value));
    }
};
inline constexpr validate_fn validate{};
```

### Multi-typeclass composition

When an algorithm needs both Foldable and Traversable, inherit from both:

```cpp
template <class T,
          const auto& FC = smd::foldable_typeclass<smd::remove_cvref_t<T>>,
          const auto& TC = smd::traversable_typeclass<smd::remove_cvref_t<T>>>
struct transform_if_large_impl
    : smd::remove_cvref_t<decltype(FC)>,
      smd::remove_cvref_t<decltype(TC)> {
    using foldable_base    = smd::remove_cvref_t<decltype(FC)>;
    using traversable_base = smd::remove_cvref_t<decltype(TC)>;

    template <class F>
    auto call(std::size_t min_size, F&& f, const T& value) const {
        if (this->foldable_base::length(value) < min_size)
            return std::optional<T>{};
        return this->traversable_base::for_each(value, ...);
    }
};
```

### Key points

- The implementation is in `detail` — callers see only the deducing `operator()` on the CPO.
- `decltype` on a `const auto&` NTTP gives a reference type; use `remove_cvref_t<decltype(TC)>` at base specifiers, nested type extraction, and qualified disambiguation calls.
- ADL is suppressed because the CPO is an object, not a function.
- The NTTP defaults pin the typeclass lookup; callers can override if needed.
- Working example: `src/smd/tree/fixpoint_tree_algorithm.hpp` with tests in `fixpoint_tree_algorithm.t.cpp`.

## Applicative: Derived invoke via terminating partial application

In Haskell, Applicative is minimal (`pure` and `(<*>)`).
`sequenceA` and `traverse` style usage is naturally expressed by applying pure functions to effectful arguments.

In this C++ codebase, we model the same intent by deriving `invoke` from `pure` and `apply` using a terminating partial-application adapter:

```text
invoke(f, a, b, c) == ap(ap(ap(pure(partial(f)), a), b), c)
```

The helper object stores already-bound values.
On each call it either invokes `f` when enough arguments have been collected, or returns a new callable waiting for the next argument.

That gives us a practical equivalent of Ben Deane's terminating partial-application technique while preserving the object-lookup model.

Contract summary:

- Minimal required operations for Applicative impl are `pure` and `apply`.
- `invoke` is provided by the base `Applicative<Impl>`.
- Implementations may still override `invoke` for custom semantics or performance, for example shape-aware structures.

## Traps and corrections from tree-instance implementation

1. Keep effect sequencing and shape preservation explicit in Traversable.

- For tree traversals, tests caught regressions where effects were accidentally duplicated or shape changed.
- Practical rule: recurse structurally, combine in one place, and assert both shape and value behavior in tests.

1. Prefer explicit map lookup in tests for readability and diagnostics.

- `const auto& map = smd::traversable_typeclass<Tree>;` gives clearer compile errors than deeply nested generic calls.
- NTTP pinning is excellent for proving lookup stability in generic helpers.

1. Be deliberate about fold-order semantics.

- `fold_map` on trees depends on traversal order chosen by the instance.
- Derived `fold_left` and `fold_right` behavior should be validated with tests that would fail if order flips.

1. Distinguish required operations from convenience operations.

- Applicative contract is `pure` plus `apply`.
- Everything else is derived unless explicitly overridden.
- Do not silently add alternative dispatch paths, for example ADL customizations, that bypass lookup objects.

1. Migration lesson: convert tests directly to native Catch2.

- Mixed macro styles in one file are easy to miss and produce confusing build failures.
- During migration, convert includes and assertion or test-case macros in the same edit pass.

## Notes for future cleanup

- Keep one consistent naming story between `conceptmap` and `typeclass` surfaces to avoid conceptual drift.
- Prefer explicit object lookup usage in tests when comparing alternate semantics.
- Preserve the lookup-first model.
- Avoid introducing parallel ADL-only customization paths for the same concept.
