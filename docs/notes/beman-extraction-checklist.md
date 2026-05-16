# Checklist: First-Pass Beman Extraction

## Purpose

This checklist turns the Beman integration plan into a concrete extraction
sequence.

It is intentionally narrower than the broader planning notes.
Its job is to answer:

- what gets moved first
- what gets renamed at extraction time
- what becomes a top-level public header
- what becomes a `detail/` header
- which tests move with each component
- which examples move with each component
- what stays in `trees`

This checklist assumes the current working Beman short name is:

- `structure`

If that name changes, the checklist still mostly stands, but all target paths
need to be rewritten.

## Phase 0: Bootstrap the Repository Correctly

- choose the final Beman short name before moving code
- stamp from a Copier-capable exemplar baseline
- preserve `.copier-answers.yml` so later `copier update --trust` is possible
- create the Beman repository with the final namespace, package, and target
  spellings already chosen

## Phase 1: Freeze the Public Naming Contract

- confirm the day-one public headers under `include/beman/structure/`
- confirm which filenames use operation names rather than framework nouns
- confirm that version suffixes do not survive in public headers
- confirm which current wrapper names lose `finger_tree_` in public headers
- confirm that large structural helper code may move under
  `include/beman/structure/detail/`

Day-one target public tree:

```text
include/
  beman/
    structure/
      applicative.hpp
      traverse.hpp
      fold.hpp
      monoid.hpp
      finger_tree.hpp
      random_access.hpp
      rope.hpp
      priority_queue.hpp
      interval_index.hpp
```

    Component rule:

    - no component is considered extracted until its tests move with it
    - examples are part of the component story, not optional garnish
    - code without tests is incomplete

## Phase 2: Extract the Day-One Algorithm Surface

- move `src/smd/typeclass/applicative.hpp` to
  `include/beman/structure/applicative.hpp`
- move `src/smd/typeclass/traversable.hpp` to
  `include/beman/structure/traverse.hpp`
- move `src/smd/typeclass/foldable.hpp` to either
  `include/beman/structure/fold.hpp` or
  `include/beman/structure/foldable.hpp`
- move `src/smd/typeclass/monoid.hpp` to
  `include/beman/structure/monoid.hpp`
- move `src/smd/typeclass/typeclass_base.hpp` to
  `include/beman/structure/detail/typeclass_base.hpp`

Checks:

- public includes no longer mention `smd/`
- public includes are flat under `beman/structure/`
- support machinery no longer appears as a top-level front-door header

## Phase 3: Extract the Day-One Finger-Tree Surface

- move `src/smd/tree/finger_tree5.hpp` to
  `include/beman/structure/finger_tree.hpp`
- remove the version suffix from the public header name
- move `src/smd/tree/finger_tree_random_access.hpp` to
  `include/beman/structure/random_access.hpp`
- move `src/smd/tree/finger_tree_rope.hpp` to
  `include/beman/structure/rope.hpp`
- move `src/smd/tree/finger_tree_priority_queue.hpp` to
  `include/beman/structure/priority_queue.hpp`
- move `src/smd/tree/finger_tree_interval_index.hpp` to
  `include/beman/structure/interval_index.hpp`

Checks:

- public wrapper headers name the capability, not the underlying structure
- no public header exposes `finger_tree5` as a user-facing concept
- no public wrapper header repeats `finger_tree_` unless that repetition is
  intentionally preserved

## Phase 4: Split Large Support Code into `detail/`

- move `src/smd/tree/finger_tree5_iterator.hpp` to
  `include/beman/structure/detail/finger_tree_iterator.hpp` unless the
  iterator type is deliberately made front-door API
- factor large node, measure, concat, or representation helpers out of the
  monolithic finger-tree header into `detail/` headers as needed
- keep these support headers installed and testable, but not part of the main
  tutorial-facing include set

Representative target shapes:

```text
include/
  beman/
    structure/
      detail/
        typeclass_base.hpp
        finger_tree_iterator.hpp
        internal_node_thing.hpp
        finger_tree_measure.hpp
        finger_tree_concat.hpp
```

Checks:

- support headers use `beman::structure::detail`
- factoring improves testability without bloating the top-level include tree
- no `detail/` header is presented as a day-one user entry point

## Phase 5: Collapse Adapter-Split Public Headers

- do not automatically export `*_foldable.hpp`
- do not automatically export `*_traversable.hpp`
- do not automatically export `*_applicative.hpp`
- merge adapter exposure into the corresponding main public facade where that
  keeps the public story cleaner
- only keep split adapter headers if they remain useful enough to live under
  `detail/`

Current files likely affected:

- `src/smd/tree/finger_tree5_foldable.hpp`
- `src/smd/tree/finger_tree5_traversable.hpp`
- `src/smd/tree/finger_tree_foldable.hpp`
- `src/smd/tree/finger_tree_traversable.hpp`
- `src/smd/tree/binary_tree_applicative.hpp`
- `src/smd/tree/binary_tree_foldable.hpp`
- `src/smd/tree/binary_tree_traversable.hpp`
- `src/smd/tree/fringe_tree_applicative.hpp`
- `src/smd/tree/fringe_tree_foldable.hpp`
- `src/smd/tree/fringe_tree_traversable.hpp`
- `src/smd/tree/fixpoint_tree_foldable.hpp`
- `src/smd/tree/fixpoint_tree_traversable.hpp`

## Phase 6: Defer or Promote Secondary Surfaces Deliberately

- defer `functor.hpp` unless it earns a day-one public role
- defer `monad.hpp` unless it earns a day-one public role
- defer `dual_monoid.hpp` unless users must name it directly
- defer `fixpoint_tree.hpp` until the public name and scope are stable enough
- defer `binary_tree.hpp` and `fringe_tree.hpp` until they are clearly part of
  the first public story
- treat `finger_tree_wrappers.hpp` as optional convenience only

## Phase 7: Keep the Right Things in `trees`

- keep `finger_tree2.hpp` in `trees`
- keep `finger_tree3.hpp` in `trees`
- keep `finger_tree4.hpp` in `trees`
- keep compile probes in `trees` unless a specific Beman regression suite needs
  a rewritten form
- keep benchmark-specific support and exploratory implementations in `trees`
- keep `deadcode/` in `trees`
- keep talk-oriented and pedagogy-heavy explanation material in `trees`

## Phase 8: Tests and Examples

- move tests into `tests/` with `*.test.cpp` naming
- create top-level examples for the day-one public headers
- ensure examples use only the approved public include spellings
- add tests for any `detail/` headers that remain large enough to deserve their
  own factoring

More specifically:

- every promoted public facade header must have at least one corresponding test
  target in `tests/`
- every top-level capability header should have at least one runnable example or
  usage test demonstrating the intended public include spelling
- every large `detail/` header that exists because it is too large to inline
  must also have tests appropriate to that factored support surface
- migration should move tests and examples in the same phase as the component,
  not as a cleanup pass later

Representative Beman test and example tree:

```text
tests/
  beman/
    structure/
      applicative.test.cpp
      traverse.test.cpp
      fold.test.cpp
      monoid.test.cpp
      finger_tree.test.cpp
      random_access.test.cpp
      rope.test.cpp
      priority_queue.test.cpp
      interval_index.test.cpp
      detail/
        typeclass_base.test.cpp
        finger_tree_iterator.test.cpp

examples/
  applicative_example.cpp
  traverse_example.cpp
  fold_example.cpp
  finger_tree_example.cpp
  random_access_example.cpp
  rope_example.cpp
  priority_queue_example.cpp
  interval_index_example.cpp
```

The exact filenames can vary.
The important rule is that each promoted component carries its own proof of
behavior and at least one clear usage path.

## File-by-File First Pass

### Day-one public facade moves

- `src/smd/typeclass/applicative.hpp` ->
  `include/beman/structure/applicative.hpp`
- `src/smd/typeclass/traversable.hpp` ->
  `include/beman/structure/traverse.hpp`
- `src/smd/typeclass/foldable.hpp` ->
  `include/beman/structure/fold.hpp`
- `src/smd/typeclass/monoid.hpp` ->
  `include/beman/structure/monoid.hpp`
- `src/smd/tree/finger_tree5.hpp` ->
  `include/beman/structure/finger_tree.hpp`
- `src/smd/tree/finger_tree_random_access.hpp` ->
  `include/beman/structure/random_access.hpp`
- `src/smd/tree/finger_tree_rope.hpp` ->
  `include/beman/structure/rope.hpp`
- `src/smd/tree/finger_tree_priority_queue.hpp` ->
  `include/beman/structure/priority_queue.hpp`
- `src/smd/tree/finger_tree_interval_index.hpp` ->
  `include/beman/structure/interval_index.hpp`

### First-pass `detail/` moves

- `src/smd/typeclass/typeclass_base.hpp` ->
  `include/beman/structure/detail/typeclass_base.hpp`
- `src/smd/tree/finger_tree5_iterator.hpp` ->
  `include/beman/structure/detail/finger_tree_iterator.hpp`

### Day-one tests that should move with the component

- `src/smd/typeclass/applicative.t.cpp` ->
  `tests/beman/structure/applicative.test.cpp`
- `src/smd/typeclass/traversable.t.cpp` ->
  `tests/beman/structure/traverse.test.cpp`
- `src/smd/typeclass/foldable.t.cpp` ->
  `tests/beman/structure/fold.test.cpp`
- `src/smd/typeclass/monoid.t.cpp` ->
  `tests/beman/structure/monoid.test.cpp`
- `src/smd/typeclass/typeclass_base.t.cpp` ->
  `tests/beman/structure/detail/typeclass_base.test.cpp`
- `src/smd/tree/finger_tree5.t.cpp` ->
  `tests/beman/structure/finger_tree.test.cpp`
- `src/smd/tree/finger_tree5_iterator.t.cpp` ->
  `tests/beman/structure/detail/finger_tree_iterator.test.cpp`
- `src/smd/tree/finger_tree_random_access.t.cpp` ->
  `tests/beman/structure/random_access.test.cpp`
- `src/smd/tree/finger_tree_priority_queue.t.cpp` ->
  `tests/beman/structure/priority_queue.test.cpp`
- `src/smd/tree/finger_tree_interval_index.t.cpp` ->
  `tests/beman/structure/interval_index.test.cpp`
- `src/smd/tree/finger_tree_rope.t.cpp` ->
  `tests/beman/structure/rope.test.cpp`

Wrapper-note checks:

- where there are both generic-wrapper tests and FT5-specific wrapper tests,
  decide whether the Beman component wants one canonical test file or both
- `*_ft5.t.cpp` names should not survive in the Beman surface unless they are
  clearly internal regression tests

Current files affected by that decision:

- `src/smd/tree/finger_tree_interval_index_ft5.t.cpp`
- `src/smd/tree/finger_tree_priority_queue_ft5.t.cpp`
- `src/smd/tree/finger_tree_random_access_ft5.t.cpp`
- `src/smd/tree/finger_tree_rope_ft5.t.cpp`

### Day-one examples that should move with the component

- `src/smd/typeclass/examples/applicative_examples.cpp` ->
  `examples/applicative_example.cpp`
- `src/smd/typeclass/examples/traversable_examples.cpp` ->
  `examples/traverse_example.cpp`
- `src/smd/typeclass/examples/foldable_examples.cpp` ->
  `examples/fold_example.cpp`
- `src/examples/fixpoint_tree_example.cpp` -> keep in `trees` for now unless
  `fixpoint_tree.hpp` is promoted into the Beman day-one set

Likely stay in `trees` for now:

- `src/smd/typeclass/examples/blog_fixpoint_examples.cpp`
- `src/smd/typeclass/examples/blog_typeclass_examples.cpp`
- `src/smd/typeclass/examples/lookup_modes_examples.cpp`
- `src/examples/cpo_example.cpp`
- `src/examples/map_example.cpp`
- `src/examples/main.cpp`

Reason:

- these are blog, pedagogy, or local demonstration assets rather than obvious
  day-one Beman examples

### Defer pending design stabilization

- `src/smd/typeclass/functor.hpp`
- `src/smd/typeclass/monad.hpp`
- `src/smd/typeclass/dual_monoid.hpp`
- `src/smd/tree/fixpoint_tree.hpp`
- `src/smd/tree/binary_tree.hpp`
- `src/smd/tree/fringe_tree.hpp`
- `src/smd/tree/finger_tree_wrappers.hpp`

Matching tests/examples also defer unless the associated surface is promoted.

### Keep in `trees`

- `src/smd/tree/finger_tree2.hpp`
- `src/smd/tree/finger_tree3.hpp`
- `src/smd/tree/finger_tree4.hpp`
- `src/smd/tree/deadcode/`
- benchmark-only and comparison-only support code

## Completion Criteria for the First Extraction Pass

- the Beman repo builds with the Copier-capable template baseline
- the day-one include tree exists under `include/beman/structure/`
- the public API no longer exposes versioned finger-tree names
- the public API no longer exposes adapter-split headers unless intentionally
  preserved
- large support code lives under `detail/` where appropriate
- every promoted component has tests in the Beman repo
- every top-level public surface has at least one example or equivalent usage
  demonstration in the Beman repo
- tests and examples use only final public include spellings
- exploratory and pedagogical variants remain in `trees`