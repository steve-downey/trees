# Plan: Integrate the Proposal Set into One Beman Project

## Purpose

This note covers repository and codebase planning for the integrated Beman
Project implementation that will track the coordinated paper set.

This is separate from the paper-planning note.
The paper note explains what the proposals are trying to do.
This note explains what needs to happen in code and repository structure to
support that work in a Beman-conforming library.

Names drive the architecture here.
That is true in this repository, and it will be even more true in a Beman
library because the chosen library name determines the public include spelling,
namespace root, CMake target spelling, README identity, and how much stutter
users see at every call site.

The assumption here is:

- there will be one Beman Project library repository for the inter-related work
- that repository will track multiple papers
- code will be extracted from the current `trees` repository
- `trees` will continue to exist as the design, pedagogy, and exploration repo
- pedagogical material and explanatory implementations that justify design
  choices, including why certain representation techniques are necessary, may
  remain in `trees` even when the production implementation moves to Beman

## Naming Comes First

The first design decision for the Beman extraction should be naming, not file
movement.

If the name is wrong, everything downstream is wrong:

- include paths
- namespace spelling
- CMake package and target names
- test layout names
- example filenames
- paper wording about the implementation vehicle

This is especially important because Beman's standard include pattern can
produce visible stutter for small libraries.
The classic shape is:

- `#include <beman/optional/optional.hpp>`

That is tolerable for a very small and obvious library, but it becomes more
awkward for a larger proposal-tracking library if the public surface ends up
looking like repeated library names everywhere.

The working goal for this project should be:

- one library-name directory under `beman/`
- one semantic component filename underneath it
- no extra directory levels unless there is a strong technical reason

In other words, the target shape should be closer to:

- `#include <beman/XYZZY/plugh.hpp>`

and not:

- `#include <beman/XYZZY/trees/plugh.hpp>`
- `#include <beman/XYZZY/traversable/plugh.hpp>` when a flatter name would do
- `#include <beman/XYZZY/XYZZY.hpp>`

Additional filename detail is acceptable when needed.
Something like `plugh_meow.hpp` is fine when it avoids an extra directory layer
or disambiguates two closely related surfaces.

The default bias should therefore be:

- keep the library directory name short, specific, and stable
- keep public filenames semantic and flat
- avoid repeated words across directory, file, namespace, and target unless the
  repetition is actually carrying information

## Naming Rules for the Future Beman Library

The extracted Beman library should follow a small set of naming rules.

### 1. The library name must carry the conceptual grouping

The Beman short name should do the heavy lifting.
It should describe the family of facilities well enough that the next path
segment can name the specific header, not restate the family.

Good shape:

- `beman::<short_name>` is the family namespace
- `<beman/<short_name>/<component>.hpp>` names a concrete facility

Bad shape:

- the short name is so vague that every filename has to repeat the concept
- the short name is so narrow that companion proposal surfaces feel misplaced

### 2. Public headers should be flat by default

Most public headers should live directly under:

- `include/beman/<short_name>/`

Only add another public subdirectory when it expresses a real user-visible
partition that will matter at include time.

The default assumption should be that more levels are not necessary.

### 3. Filenames should name the facility, not the framework story

Users should include headers for the thing they want, not for the internal
organizational theory behind it.

That means filenames should usually prefer concrete facility names over broad
adapter-framework terms, unless the framework surface is itself public and
first-class.

### 4. Avoid duplicate nouns across directory and filename

If the directory already says the family name, the filename should usually say
the component.

Avoid patterns like:

- `<beman/<short_name>/<short_name>.hpp>`
- `<beman/<short_name>/tree.hpp>` when the library is already named `tree`
- `<beman/<short_name>/traversable/traversable.hpp>` unless there is no better
  decomposition

### 5. Prefer a slightly longer filename over another directory level

If the choice is between:

- `include/beman/<short_name>/foo/bar.hpp`

and:

- `include/beman/<short_name>/foo_bar.hpp`

the second form is probably better unless the subdirectory is doing real work.

This matches the goal of minimizing stutter and keeping include paths legible.

### 6. Internal organization does not need to mirror public organization

The public header tree should be optimized for readers and users.
The private implementation tree can be more structured.

That means it is fine if:

- public headers are flat
- internal sources use additional `detail/` or component directories

The public include path should not be forced to expose every internal axis of
organization.

## Consequences for Repository Naming

Because the directory name under `beman/` carries so much weight, the library
name should be chosen with explicit pressure from the desired header spellings.

The naming exercise should not start from an abstract repo title.
It should start from representative include lines and ask whether they look
like something people will actually want to type, teach, review, and keep.

The right test is not just whether a name is theoretically accurate.
The right test is whether examples like these look sane:

- `#include <beman/<short_name>/traverse.hpp>`
- `#include <beman/<short_name>/applicative.hpp>`
- `#include <beman/<short_name>/sequence.hpp>`
- `#include <beman/<short_name>/finger_tree.hpp>`

or whatever the eventual public surface becomes.

If those look clumsy, the short name is probably wrong.

## Consequences for Namespace Naming

The same anti-stutter rule should apply to namespaces.

The public namespace should likely be:

- `beman::<short_name>`

and then the next level should only be introduced if it carries useful public
structure.

The default should not be to mirror every internal taxonomy in the public
namespace tree.

## Consequences for Extraction

When code moves from `trees` into Beman, the extraction process should begin by
classifying public names, not just files.

For each candidate public header, decide:

- what the public include spelling should be
- whether the filename is too generic or too framework-specific
- whether the current name causes stutter under `beman/<short_name>/`
- whether the concept belongs as a top-level header or a suffixed sibling like
  `foo_meow.hpp`

This naming pass should happen before large-scale file moves.
Otherwise the repo will get reorganized twice.

## Naming Evaluation Rubric

Candidate Beman names should be evaluated against concrete examples rather than
in the abstract.

For each candidate short name, write out at least:

- one core algorithm header
- one container header
- one support header
- one example `find_package` and target line
- one namespace-qualified use site

For example, test shapes like:

- `#include <beman/<short_name>/traverse.hpp>`
- `#include <beman/<short_name>/applicative.hpp>`
- `#include <beman/<short_name>/finger_tree.hpp>`
- `target_link_libraries(x PRIVATE beman::<short_name>)`
- `beman::<short_name>::traverse(...)`

Then score each candidate against these questions:

### 1. Does the include path stutter?

If the directory and filename feel like they are saying the same word twice,
the name is weak.

### 2. Does the library name carry enough semantic weight?

If every filename has to compensate for a vague top-level name, the name is too
empty.

### 3. Does the library name overconstrain the paper set?

If the name sounds perfect for one paper but awkward for its companion papers,
it is probably too narrow.

### 4. Can the public headers remain flat?

If a candidate name seems to force extra public subdirectories just to avoid
collisions, that is a bad sign.

### 5. Does the namespace read cleanly in examples?

The namespace spelling should look like something people will actually want to
put in slides, papers, docs, and code review.

### 6. Does the CMake target name read cleanly?

`beman::<short_name>` should look natural, not like an internal codename or a
temporary sketch.

### 7. Does the name still work if one surface becomes more important?

The chosen name should survive shifts in emphasis between traversal,
applicative operations, extension-point machinery, and container structures.

## Immediate Naming Task

Before any extraction work starts, produce a short list of candidate Beman
names and evaluate them using the rubric above.

That should result in:

- the preferred short name
- rejected alternatives and why they fail
- a small set of canonical public header spellings to preserve during
  extraction

That output becomes the naming contract for the rest of the migration.

## First-Pass Candidate Short Names

This is a first-pass shortlist, not a final verdict.
The goal is to identify names that can plausibly host:

- traversal and applicative-style algorithm headers
- extension-point machinery
- finger-tree-based containers and wrappers
- recursive tree surfaces

The candidates below are scored qualitatively against the anti-stutter and
breadth requirements.

### Candidate: `structure`

Representative spellings:

- `#include <beman/structure/traverse.hpp>`
- `#include <beman/structure/applicative.hpp>`
- `#include <beman/structure/finger_tree.hpp>`
- `#include <beman/structure/fixpoint_tree.hpp>`
- `beman::structure::traverse(...)`
- `target_link_libraries(x PRIVATE beman::structure)`

Assessment:

- strongest current umbrella candidate
- broad enough to cover traversal plus recursive and persistent structures
- flat headers read cleanly
- little visible stutter
- does not force the library to be only about trees or only about effects

Weaknesses:

- somewhat plain
- risks sounding more like a repository category than a sharply branded library

Current status:

- preferred first-pass umbrella

### Candidate: `shape`

Representative spellings:

- `#include <beman/shape/traverse.hpp>`
- `#include <beman/shape/applicative.hpp>`
- `#include <beman/shape/finger_tree.hpp>`
- `beman::shape::traverse(...)`

Assessment:

- short and low-stutter
- aligns with the idea that traversal preserves shape while sequencing effects
- keeps public paths compact

Weaknesses:

- more category-theory-coded and less obvious to general library users
- weaker fit for container wrappers than `structure`
- may read as metaphorical rather than literal

Current status:

- plausible alternate if a shorter, more theory-flavored umbrella is desired

### Candidate: `functional`

Representative spellings:

- `#include <beman/functional/traverse.hpp>`
- `#include <beman/functional/finger_tree.hpp>`
- `beman::functional::applicative_typeclass<...>`

Assessment:

- broad enough to hold both algorithms and containers
- public paths remain flat

Weaknesses:

- too generic
- says little about persistent structures or recursive trees specifically
- risks sounding like a general style label rather than a library identity

Current status:

- acceptable breadth, weak specificity

### Candidate: `context`

Representative spellings:

- `#include <beman/context/traverse.hpp>`
- `#include <beman/context/applicative.hpp>`
- `#include <beman/context/finger_tree.hpp>`

Assessment:

- very good for traversal and applicative operations
- clean header spellings for the algorithm side

Weaknesses:

- too effect-centric for the container and tree side
- makes `finger_tree.hpp` and `fixpoint_tree.hpp` feel bolted on

Current status:

- good paper-mechanism name, weak whole-library name

### Candidate: `tree`

Representative spellings:

- `#include <beman/tree/finger_tree.hpp>`
- `#include <beman/tree/fixpoint_tree.hpp>`
- `#include <beman/tree/traverse.hpp>`

Assessment:

- excellent for the recursive/container side
- compact and obvious

Weaknesses:

- overconstrains the library to one data-structure family
- makes applicative and traversal headers feel misplaced
- poor fit if non-tree surfaces become first-class

Current status:

- reject as umbrella name

### Candidate: `sequence`

Representative spellings:

- `#include <beman/sequence/finger_tree.hpp>`
- `#include <beman/sequence/traverse.hpp>`
- `#include <beman/sequence/fixpoint_tree.hpp>`

Assessment:

- good for persistent sequence containers
- good fit for random access, rope, and related wrappers

Weaknesses:

- too narrow for recursive trees
- traversal and applicative surfaces read as secondary add-ons

Current status:

- reject as umbrella name

## First-Pass Naming Recommendation

If the project needs one umbrella Beman short name today, `structure` is the
best first-pass choice.

Reason:

- it is broad enough to hold the coordinated paper set
- it supports flat public headers cleanly
- it keeps `traverse.hpp`, `applicative.hpp`, `finger_tree.hpp`, and
  `fixpoint_tree.hpp` all plausible in one namespace
- it avoids the worst forms of Beman path stutter

If a shorter and more theory-forward spelling is preferred, `shape` is the
best alternate.

## First-Pass Public Header Map

This section translates the current `smd` surface into a flatter Beman public
header set.
It is intentionally not a one-to-one file move.

The main rule is:

- versioned implementation headers stay internal
- public headers are unversioned and semantic
- adapter glue should be merged into the public facade where practical rather
  than exported as separate `*_foldable.hpp` or `*_traversable.hpp` headers

The examples below use `structure` as the working short name.

### Likely top-level public algorithm headers

- `include/beman/structure/applicative.hpp`
  Source starting point: `src/smd/typeclass/applicative.hpp`
  Notes: likely public as-is, subject to naming cleanup.

- `include/beman/structure/traverse.hpp`
  Source starting point: `src/smd/typeclass/traversable.hpp`
  Notes: rename from the category name to the operation users actually reach
  for.

- `include/beman/structure/foldable.hpp` or `include/beman/structure/fold.hpp`
  Source starting point: `src/smd/typeclass/foldable.hpp`
  Notes: unresolved; `fold.hpp` is cleaner for users, `foldable.hpp` matches
  the current framework vocabulary.

- `include/beman/structure/monoid.hpp`
  Source starting point: `src/smd/typeclass/monoid.hpp`
  Notes: probably public if the traversal/fold story relies on it explicitly.

### Likely later or advanced algorithm headers

- `include/beman/structure/functor.hpp`
  Source starting point: `src/smd/typeclass/functor.hpp`
  Notes: probably not the first public header to lead with.

- `include/beman/structure/monad.hpp`
  Source starting point: `src/smd/typeclass/monad.hpp`
  Notes: likely later, depending on paper scope.

- `include/beman/structure/dual_monoid.hpp`
  Source starting point: `src/smd/typeclass/dual_monoid.hpp`
  Notes: likely advanced support header, not front-door API.

### Headers that should stay internal

- `src/smd/typeclass/typeclass_base.hpp`
  Proposed Beman status: internal detail support, not direct public include.

### Finger-tree public headers

- `include/beman/structure/finger_tree.hpp`
  Source starting point: `src/smd/tree/finger_tree5.hpp`
  Notes: public header should drop the version suffix.
  The current `finger_tree5` name is an implementation-generation marker, not a
  Beman surface name.

- `include/beman/structure/random_access.hpp`
  Source starting point: `src/smd/tree/finger_tree_random_access.hpp`
  Notes: keep flat and name the capability, not the underlying structure.

- `include/beman/structure/rope.hpp`
  Source starting point: `src/smd/tree/finger_tree_rope.hpp`
  Notes: public wrapper header should not repeat `finger_tree` if the type is
  sold as a rope.

- `include/beman/structure/priority_queue.hpp`
  Source starting point: `src/smd/tree/finger_tree_priority_queue.hpp`
  Notes: same principle as `rope.hpp`.

- `include/beman/structure/interval_index.hpp`
  Source starting point: `src/smd/tree/finger_tree_interval_index.hpp`
  Notes: capability-facing name is preferable.

- `include/beman/structure/finger_tree_wrappers.hpp`
  Source starting point: `src/smd/tree/finger_tree_wrappers.hpp`
  Notes: optional convenience omnibus; useful only if Beman wants an aggregate
  include.

### Finger-tree headers that should become internal

- `src/smd/tree/finger_tree5_iterator.hpp`
  Proposed Beman status: internal support for `finger_tree.hpp`, unless the
  iterator type itself must be named publicly.

- `src/smd/tree/finger_tree5_foldable.hpp`
  Proposed Beman status: fold support should be wired in through the main
  public facade rather than shipped as a separate user header.

- `src/smd/tree/finger_tree5_traversable.hpp`
  Proposed Beman status: same as above.

- any retained `detail/finger_tree_*` implementation headers
  Proposed Beman status: internal only.

### Recursive-tree public headers

- `include/beman/structure/fixpoint_tree.hpp`
  Source starting point: `src/smd/tree/fixpoint_tree.hpp`
  Notes: public viability depends on whether the current expression-specific
  presentation is generalized into a reusable tree surface.

- `include/beman/structure/binary_tree.hpp`
  Source starting point: `src/smd/tree/binary_tree.hpp`
  Notes: plausible if binary tree remains part of the public motivating set.

- `include/beman/structure/fringe_tree.hpp`
  Source starting point: `src/smd/tree/fringe_tree.hpp`
  Notes: probably secondary unless it carries unique standardization value.

### Recursive-tree adapter headers that should not remain public

- `binary_tree_applicative.hpp`
- `binary_tree_foldable.hpp`
- `binary_tree_traversable.hpp`
- `fringe_tree_applicative.hpp`
- `fringe_tree_foldable.hpp`
- `fringe_tree_traversable.hpp`
- `fixpoint_tree_foldable.hpp`
- `fixpoint_tree_traversable.hpp`

Proposed Beman status:

- merge adapter exposure into the main public tree headers where practical
- keep extra split headers only if compile-time isolation or layering makes
  them materially useful to users

## Proposed Public Include Tree

This section turns the first-pass header map into a concrete proposed public
header tree.
The point is not that every file below must exist on day one.
The point is to define the shape the library should be trying to converge on.

The example below uses the current preferred umbrella name:

- `structure`

### Target day-one shape

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

This is the smallest coherent public tree that still demonstrates the intended
shape:

- one umbrella directory
- flat semantic headers
- no version suffixes in public names
- no adapter-specific public headers where the adapter can be incorporated into
  the main surface

### Likely later public additions

```text
include/
  beman/
    structure/
      functor.hpp
      monad.hpp
      dual_monoid.hpp
      fixpoint_tree.hpp
      binary_tree.hpp
      fringe_tree.hpp
      finger_tree_wrappers.hpp
```

These are reasonable later additions, but they should not all be treated as
mandatory day-one exports.

Reason:

- some are framework-facing rather than user-front-door headers
- some are support headers
- some depend on whether the public paper story actually needs those surfaces
- some may stay in `trees` until generalized enough to deserve a stable Beman
  name

### Headers that should stay internal from the start

```text
src/beman/structure/detail/
  typeclass_base.hpp
  finger_tree_iterator.hpp
  finger_tree_impl.hpp
  finger_tree_measure.hpp
  finger_tree_node.hpp
  ...
```

The exact internal filenames can vary, but the main point is stable:

- version numbers should disappear from public names
- implementation-generation markers should disappear from public names
- adapter plumbing should usually disappear from public names
- internal refactoring freedom should increase once the public tree is flat and
  semantic

One refinement matters here.
In a Beman library, these files are still technically public headers in the
sense that they ship with the project and are reachable by include path.
But they are not intended as front-door user-facing API.

The right treatment for parts that are too large, too structural, or too tied
to implementation mechanics is:

- place them under `include/beman/<short_name>/detail/`
- place their entities under `beman::<short_name>::detail`
- treat them as supported implementation detail rather than tutorial-facing API

Representative shape:

- `#include <beman/XYZZY/detail/internal_node_thing.hpp>`

That is a better fit than either:

- forcing large structural helpers into the top-level public header set, or
- pretending they are private in a way that prevents practical factoring and
  testing

So the real distinction is not “installed” versus “not installed”.
The real distinction is:

- front-door headers for normal users
- `detail/` headers for large implementation-support surfaces that must exist
  but should not dominate the public story

## Suggested Source-to-Header Translation

This is the current best first-pass translation from the `trees` repo into the
proposed public include tree.

### Algorithm surface

- `src/smd/typeclass/applicative.hpp`
  becomes public `include/beman/structure/applicative.hpp`

- `src/smd/typeclass/traversable.hpp`
  becomes public `include/beman/structure/traverse.hpp`

- `src/smd/typeclass/foldable.hpp`
  becomes public `include/beman/structure/fold.hpp` unless there is a strong
  reason to preserve the framework noun in the filename

- `src/smd/typeclass/monoid.hpp`
  becomes public `include/beman/structure/monoid.hpp`

- `src/smd/typeclass/functor.hpp`
  is a likely later public header `include/beman/structure/functor.hpp`

- `src/smd/typeclass/monad.hpp`
  is a likely later public header `include/beman/structure/monad.hpp`

- `src/smd/typeclass/dual_monoid.hpp`
  is likely advanced support and should only become public if the user-facing
  API or examples really need it

- `src/smd/typeclass/typeclass_base.hpp`
  should not become a public header

### Persistent finger-tree surface

- `src/smd/tree/finger_tree5.hpp`
  becomes public `include/beman/structure/finger_tree.hpp`

- `src/smd/tree/finger_tree_random_access.hpp`
  becomes public `include/beman/structure/random_access.hpp`

- `src/smd/tree/finger_tree_rope.hpp`
  becomes public `include/beman/structure/rope.hpp`

- `src/smd/tree/finger_tree_priority_queue.hpp`
  becomes public `include/beman/structure/priority_queue.hpp`

- `src/smd/tree/finger_tree_interval_index.hpp`
  becomes public `include/beman/structure/interval_index.hpp`

- `src/smd/tree/finger_tree_wrappers.hpp`
  remains optional and should only become public if there is a clear value in a
  convenience umbrella header

- `src/smd/tree/finger_tree5_iterator.hpp`
  should become internal support unless there is a deliberate reason to expose
  the iterator type by name

- `src/smd/tree/finger_tree5_foldable.hpp`
  should not remain a separate public header

- `src/smd/tree/finger_tree5_traversable.hpp`
  should not remain a separate public header

### Recursive-tree surface

- `src/smd/tree/fixpoint_tree.hpp`
  is a candidate for public `include/beman/structure/fixpoint_tree.hpp`, but
  only if generalized beyond the current expression-oriented presentation

- `src/smd/tree/binary_tree.hpp`
  is a candidate for public `include/beman/structure/binary_tree.hpp`

- `src/smd/tree/fringe_tree.hpp`
  is a candidate for public `include/beman/structure/fringe_tree.hpp`

- `src/smd/tree/*_applicative.hpp`
  should usually fold into the primary public type header or remain internal

- `src/smd/tree/*_foldable.hpp`
  should usually fold into the primary public type header or remain internal

- `src/smd/tree/*_traversable.hpp`
  should usually fold into the primary public type header or remain internal

## Export Policy for Day One

The initial Beman export set should be smaller than the eventual codebase.

Recommended day-one public exports:

- `applicative.hpp`
- `traverse.hpp`
- `fold.hpp`
- `monoid.hpp`
- `finger_tree.hpp`
- zero or more capability wrappers that are already clean and reviewable

Recommended not-day-one exports unless specifically needed:

- `functor.hpp`
- `monad.hpp`
- `dual_monoid.hpp`
- `finger_tree_wrappers.hpp`
- `binary_tree.hpp`
- `fringe_tree.hpp`
- `fixpoint_tree.hpp`

This smaller export set reduces naming churn and lets the Beman public surface
track the paper set that is actually under active review.

## Classification of Current `src/smd/...` Headers

This section classifies the current header inventory into three buckets:

- public facade
- `detail/` support
- stay in `trees`

The intent is not to freeze every judgment permanently.
The intent is to define a first-pass extraction policy.

### `src/smd/typeclass/`

#### Public facade candidates

- `applicative.hpp`
  Proposed Beman header: `include/beman/structure/applicative.hpp`
  Reason: core public algorithm surface.

- `traversable.hpp`
  Proposed Beman header: `include/beman/structure/traverse.hpp`
  Reason: central user-facing paper surface.

- `foldable.hpp`
  Proposed Beman header: `include/beman/structure/fold.hpp` or
  `include/beman/structure/foldable.hpp`
  Reason: likely public, though final filename remains open.

- `monoid.hpp`
  Proposed Beman header: `include/beman/structure/monoid.hpp`
  Reason: likely public support concept if fold/traverse exposure keeps it in
  the user story.

#### Later public facade candidates

- `functor.hpp`
  Proposed Beman header: `include/beman/structure/functor.hpp`
  Reason: plausible public surface, but not needed to define the first public
  cut.

- `monad.hpp`
  Proposed Beman header: `include/beman/structure/monad.hpp`
  Reason: same as above.

- `dual_monoid.hpp`
  Proposed Beman header: `include/beman/structure/dual_monoid.hpp`
  Reason: advanced support header; public only if examples and papers actually
  need users to name it.

#### `detail/` support candidates

- `typeclass_base.hpp`
  Proposed Beman header:
  `include/beman/structure/detail/typeclass_base.hpp`
  Reason: foundational machinery, but not a front-door user header.

### `src/smd/tree/`

#### Public facade candidates

- `finger_tree5.hpp`
  Proposed Beman header: `include/beman/structure/finger_tree.hpp`
  Reason: public implementation surface with version suffix removed.

- `finger_tree_random_access.hpp`
  Proposed Beman header: `include/beman/structure/random_access.hpp`
  Reason: capability-facing wrapper.

- `finger_tree_rope.hpp`
  Proposed Beman header: `include/beman/structure/rope.hpp`
  Reason: capability-facing wrapper.

- `finger_tree_priority_queue.hpp`
  Proposed Beman header: `include/beman/structure/priority_queue.hpp`
  Reason: capability-facing wrapper.

- `finger_tree_interval_index.hpp`
  Proposed Beman header: `include/beman/structure/interval_index.hpp`
  Reason: capability-facing wrapper.

#### Possible later public facade candidates

- `fixpoint_tree.hpp`
  Proposed Beman header: `include/beman/structure/fixpoint_tree.hpp`
  Reason: likely only after generalization beyond the current expression-heavy
  presentation.

- `binary_tree.hpp`
  Proposed Beman header: `include/beman/structure/binary_tree.hpp`
  Reason: plausible supporting tree surface if it remains part of the proposal
  story.

- `fringe_tree.hpp`
  Proposed Beman header: `include/beman/structure/fringe_tree.hpp`
  Reason: same as above, but lower priority.

- `finger_tree_wrappers.hpp`
  Proposed Beman header: `include/beman/structure/finger_tree_wrappers.hpp`
  Reason: optional umbrella include, not obviously a day-one need.

#### `detail/` support candidates

- `finger_tree5_iterator.hpp`
  Proposed Beman header:
  `include/beman/structure/detail/finger_tree_iterator.hpp`
  Reason: likely too structural and too coupled to the representation to act as
  a front-door header, but large enough that it may merit its own tested detail
  header.

- `fixpoint_tree_algorithm.hpp`
  Proposed Beman header:
  `include/beman/structure/detail/fixpoint_tree_algorithm.hpp`
  Reason: implementation-support algorithm layer unless it graduates into a
  stable named public utility.

- implementation partitions extracted from `finger_tree5.hpp`
  Proposed Beman headers:
  `include/beman/structure/detail/internal_node_thing.hpp`,
  `include/beman/structure/detail/finger_tree_measure.hpp`,
  `include/beman/structure/detail/finger_tree_concat.hpp`, and similar
  factored headers as needed.
  Reason: large structural machinery that must be separately testable without
  becoming top-level tutorial-facing API.

#### Headers that should usually not remain separate public headers

- `finger_tree5_foldable.hpp`
- `finger_tree5_traversable.hpp`
- `finger_tree_foldable.hpp`
- `finger_tree_traversable.hpp`
- `binary_tree_applicative.hpp`
- `binary_tree_foldable.hpp`
- `binary_tree_traversable.hpp`
- `fringe_tree_applicative.hpp`
- `fringe_tree_foldable.hpp`
- `fringe_tree_traversable.hpp`
- `fixpoint_tree_foldable.hpp`
- `fixpoint_tree_traversable.hpp`

Default treatment:

- merge into the corresponding public facade header when that keeps the public
  surface cleaner, or
- move under `include/beman/<short_name>/detail/` if the split remains useful
  for testing or factoring

They should not automatically survive as top-level public headers just because
they are currently separate in the local repo.

#### Headers that should stay in `trees`

- `finger_tree2.hpp`
- `finger_tree3.hpp`
- `finger_tree4.hpp`
- their compile probes
- benchmark-specific support headers and exploratory implementations
- anything under `deadcode/`

Reason:

- these are implementation-history, comparison, exploration, or pedagogy
  artifacts rather than candidates for a clean Beman public surface

## Rule for `detail/` Headers

Use `detail/` when a component is all of these:

- real code that benefits from separate testing
- too large or structural to belong inline in a front-door header
- too implementation-oriented to deserve a top-level public name

That means the extraction policy should not force a false binary between:

- top-level public header, and
- not shipped at all

Instead, the extraction policy should allow:

- `include/beman/<short_name>/<user_header>.hpp`
- `include/beman/<short_name>/detail/<support_header>.hpp`

with matching namespaces:

- `beman::<short_name>`
- `beman::<short_name>::detail`

This is likely the right home for the larger internal node and measurement
machinery once `finger_tree5.hpp` is split into production-facing and
implementation-facing layers.

## Public Naming Consequences for Current Headers

The current `trees` layout contains several names that are appropriate locally
but should not survive into the Beman public surface.

### Names that should lose version suffixes

- `finger_tree5.hpp` -> `finger_tree.hpp`

The version is an implementation-history marker, not a public API concept.

### Names that should lose implementation nouns

- `finger_tree_random_access.hpp` -> `random_access.hpp`
- `finger_tree_rope.hpp` -> `rope.hpp`
- `finger_tree_priority_queue.hpp` -> `priority_queue.hpp`
- `finger_tree_interval_index.hpp` -> `interval_index.hpp`

If the public type is being sold as a rope or priority queue, the include path
should name that thing directly.

### Names that should lose adapter suffixes

- `*_foldable.hpp`
- `*_traversable.hpp`
- `*_applicative.hpp`

Those names are implementation-organization names.
They should only remain public if separate adapter headers prove materially
valuable to users rather than merely convenient for local organization.

## Copier-Based Bootstrap Note

Repository bootstrapping should assume the Copier-based exemplar workflow,
not the older one-shot stamping model.

Relevant upstream work:

- `bemanproject/exemplar` PR #393 proposes the Copier migration
- `steve-downey/exemplar` branch `copier` carries the working implementation

Current planning implication:

- use the Copier-capable exemplar flow as the intended bootstrap model because
  it leaves behind `.copier-answers.yml` metadata and enables future
  `copier update --trust`
- this is a better fit for a proposal-tracking repository that will need to
  stay aligned with evolving Beman template practice

Current status note:

- reviews have been positive
- final agreement, cutover direction, and conflict resolution are still pending
- those conflicts are out of scope for this repository plan

Practical recommendation:

- when the integrated Beman project is actually created, prefer stamping from a
  Copier-capable exemplar base rather than from the older template flow
- if upstream cutover is still pending at creation time, use the Copier-aware
  branch or equivalent baseline intentionally so the new repository is not
  trapped on the older one-shot path

## Context

The current `trees` repository follows a merged-src local style described in
[docs/codestyle.org](/workarea/trees/docs/codestyle.org).
That style prefers:

- public headers under `src/`
- tests next to the code they verify
- examples near the implementation subtree
- local component ownership and co-located test files

The Beman Standard and exemplar template instead expect a split layout with:

- public headers in `include/beman/<short_name>/`
- implementation files in top-level `src/`
- tests in top-level `tests/`
- examples in top-level `examples/`
- documentation in top-level `docs/`
- paper sources in top-level `papers/`

This is the main structural difference to plan around.

## Core Planning Decision

The Beman implementation should be treated as the production-facing,
standardization-tracking library.
The `trees` repository should remain the broader design and pedagogy workspace.

That means the Beman repository should contain:

- the code needed to implement and demonstrate the evolving paper set
- examples and tests that support standardization review
- docs that explain usage, build, integration, and current paper tracking

That also means the Beman repository should not try to absorb everything from
`trees`.

The `trees` repository should continue to hold:

- pedagogical and exploratory documents
- alternate designs and historical experiments
- explanatory scaffolding showing why specific implementation choices are
  needed
- material useful for talks, design exploration, and code archaeology

## Primary Repository Questions

Before extraction begins, several repository-level choices must be made.

### 1. Library identity

The Beman Standard expects one repository per library with a single Beman
library name.
This proposal set is inter-related enough that one repository is sensible, but
the library still needs a single coherent identity.

This is not just branding.
It is the decision that determines whether the public surface feels clean or
annoying.

Open question:

- what is the Beman library's short name?

This name should be broad enough to hold the coordinated paper set without
being so vague that it says nothing.

It should also be short enough, and non-redundant enough, that the resulting
headers do not suffer from avoidable Beman-path stutter.

Candidates may depend on which paper becomes the public anchor.
Until that is decided, the naming question should be tracked explicitly.

### 2. Scope of one repository

One Beman repository can track multiple papers, but that does not mean every
idea from `trees` belongs in it.

The Beman repository should include only the code needed for:

- the facilities being proposed
- the examples needed to review them
- the tests and benchmarks needed to support the claims

It should not become the dumping ground for all historical and pedagogical
variants.

### 3. Paper tracking in README and papers/

The Beman README convention wants an “Implements” section listing the papers.
Because this repository will track multiple papers, that section must be kept
current and explicit.

In addition, Beman expects paper sources to live under top-level `papers/`.
That aligns well with the coordinated-paper plan.

## Structural Migration Plan

### Current `trees` style

Representative current style:

- `src/smd/typeclass/*.hpp`
- `src/smd/typeclass/*.t.cpp`
- `src/smd/tree/*.hpp`
- `src/smd/tree/*.t.cpp`
- exploratory and explanatory docs mixed into `docs/`

### Target Beman style

Representative target style:

- `include/beman/<short_name>/...`
- `src/beman/<short_name>/...`
- `tests/beman/<short_name>/...`
- `examples/...`
- `docs/...`
- `papers/...`

### Mapping guidance

#### Public headers

Current public headers under `src/smd/...` that belong in the library surface
must move to:

- `include/beman/<short_name>/...`

This is not just a file move.
It implies a public include spelling and namespace story consistent with the
Beman library identity.
It also implies an explicit flattening pass so the extracted headers are named
for users rather than for the current local source-tree structure.

#### Non-public implementation headers and sources

Non-public implementation material should move to:

- `src/beman/<short_name>/...`

Implementation headers that are not part of the public interface should either:

- use `detail_` naming, or
- live under a `detail/` directory

#### Tests

Current `*.t.cpp` tests need to move into top-level `tests/` and should be
renamed to `*.test.cpp` to align with the Beman Standard.

This is one of the clearest mechanical differences from the local `trees`
style.

#### Examples

Examples that demonstrate use of the Beman library should live in top-level
`examples/`.

Slide-specific, pedagogical, or “why this design exists” examples can remain in
`trees` if they are not part of the production library story.

#### Documentation

Top-level `docs/` in the Beman repository should contain usage, development,
and project documentation relevant to the library.

Long-form design essays, exploratory notes, and talk-oriented narrative docs do
not all need to move over.

#### Paper sources

All evolving paper material intended to ship with the Beman library should live
under top-level `papers/`.

The current paper-planning documents may stay in `trees` while the plan is
still fluid, but final drafting work should assume Beman's `papers/` layout.

## Code Extraction Categories

Not all code in `trees` should be moved as-is.
It should be classified first.

### Category A: production-candidate code

This includes:

- code that directly implements the proposed library surface
- tests that validate proposal semantics
- examples that demonstrate intended public use

This category should be extracted into the Beman repository.

### Category B: support and proving-ground code

This includes:

- implementation experiments
- alternate versions of the same structure
- performance exploration code
- deadcode or reference implementations

This category should usually remain in `trees`, or only be copied selectively.

### Category C: pedagogical and explanatory material

This includes:

- documents showing why a particular representation or abstraction choice is
  necessary
- comparative variants used for talks or design explanation
- material that teaches the design rather than defines the production surface

This category should mostly remain in `trees`.

This is especially important for things like:

- explaining why type-erasure or flattening choices in a finger-tree-like spine
  are necessary in the chosen design
- preserving historical and pedagogical context without bloating the production
  repository

## Beman-Specific Work Items

### 1. Choose the library name and namespace

Need to decide:

- repository name
- Beman library name
- namespace root under `beman::<short_name>`
- CMake project name and alias target name

This should be aligned with the anchor-paper story.
It should also be validated against the actual public include spellings the
project expects users to write.

Representative spellings should be written down before the final name is
chosen.

### 2. Re-layout the source tree

Need to:

- identify public headers
- identify implementation-only headers
- identify which tests migrate and which stay behind
- identify which examples are production-facing versus pedagogical
- move code into Beman split layout

### 3. Normalize file naming and test naming

Need to:

- rename `*.t.cpp` to `*.test.cpp` in the Beman repo
- ensure public header names align with Beman file naming and include spelling
- ensure generated or config headers follow Beman conventions

### 4. Establish top-level Beman repo files

Need to provide:

- root `CMakeLists.txt`
- `LICENSE`
- `README.md`
- `.github/CODEOWNERS`
- appropriate workflow and lint configuration

Likely easiest path:

- start from Copier-capable `bemanproject/exemplar`
- if upstream cutover is not complete yet, use the Copier-capable fork/branch
  as the bootstrap baseline deliberately
- stamp it with the chosen project name and paper metadata
- then layer in extracted code rather than hand-assembling the structure from
  scratch

### 5. Adapt build system shape

The `trees` repo uses a Makefile-centered workflow with a merged local build
story.
The Beman repo should conform to Beman's CMake-first expectations.

Need to:

- move to a root CMake workflow that builds/tests the library directly
- provide `BEMAN_<short_name>_BUILD_TESTS` and
  `BEMAN_<short_name>_BUILD_EXAMPLES` options
- create the exported package config and `beman::<short_name>` target
- avoid flag forking and passive-target violations

### 6. Handle feature-conditional code the Beman way

The current repo uses modern C++ aggressively and may use compile-time feature
selection directly.
The Beman Standard is stricter about flag forking and feature-test-macro use.

Need to:

- identify direct uses of feature-test macros or compiler-flag-sensitive code
- decide which features require generated config headers
- generate `config.hpp` / `config_generated.hpp` style wrappers where needed

This matters especially if the integrated library uses newer language features
that may need controlled fallback or configuration.

### 7. Split usage docs from design docs

Need to decide what goes into Beman docs:

- README usage and build guidance
- library-specific docs under `docs/`
- paper drafts under `papers/`

And what stays in `trees`:

- design essays
- pedagogical comparisons
- large exploratory notes

### 8. Add Beman-style examples and integration documentation

The Beman repo should include examples that are runnable, installable, and easy
to point to in review.

Need:

- at least one small example per main proposal surface
- README links to those examples
- install and `find_package` documentation
- if practical, a Compiler Explorer example badge and maintained example

### 9. Papers directory and coordinated paper tracking

Need to:

- create `papers/` structure for the coordinated paper set
- decide whether each paper gets its own subdirectory or shared drafting layout
- ensure README “Implements” section reflects the active set accurately
- keep the public anchor paper visible in the repo metadata

### 10. Decide maturity and release posture

The Beman README and release model require an explicit maturity story.

Need to decide:

- initial maturity label
- when the project becomes “production ready” versus “under development”
- release cadence relative to paper revisions

Because this repo is paper-tracking and evolving, “under development” is the
likely initial status.

## Comparison: Local `trees` Style vs Beman Style

### Layout

`trees` preference:

- merged-src
- interface, implementation, and tests together
- examples near code

Beman requirement:

- split include/src/tests/examples/docs/papers layout

Migration implication:

- extraction is not a direct copy; the component boundaries may remain similar,
  but physical layout changes substantially

### Public include identity

`trees` preference:

- canonical includes from `src/` path, currently `smd/...`

Beman requirement:

- public headers under `include/beman/<short_name>/...`
- public namespace should align with `beman::<short_name>`

Migration implication:

- public include spelling and probably namespace spelling need a deliberate
  translation plan

### Tests

`trees` preference:

- tests co-located and often named `*.t.cpp`

Beman requirement:

- tests in `tests/`
- named `*.test.cpp`

Migration implication:

- mechanical rename and move, plus possible regrouping by public component

### Papers

`trees` today:

- planning and long-form design notes live under `docs/`

Beman requirement:

- paper artifacts belong under top-level `papers/`

Migration implication:

- paper drafting needs a dedicated home distinct from library docs

## Recommended Extraction Strategy

### Phase 1: repository framing

- choose the Beman library identity
- write representative public include spellings and reject names that stutter
- stamp `exemplar`
- establish README, LICENSE, CODEOWNERS, workflow skeleton, and CMake shell

### Phase 2: minimal production surface

- extract the anchor-paper-facing code first
- add tests and examples that support the anchor story
- get one coherent library package building cleanly in Beman layout

### Phase 3: companion surfaces

- add the extension-point framework pieces
- add companion tests and examples
- add paper scaffolding under `papers/`

### Phase 4: container and tree expansion

- extract the persistent measured sequence implementation pieces
- add any recursive-tree support that belongs in the Beman repo
- keep pedagogy-heavy variants in `trees`

### Phase 5: documentation split and stabilization

- move only production-facing docs into Beman
- leave design-essay and exploratory docs in `trees`
- ensure README and paper tracking reflect the coordinated set accurately

## What Should Stay in `trees`

The following should generally remain in `trees` unless a clear production need
appears:

- alternate implementations kept for explanation or comparison
- deadcode and historical variants
- talk-oriented documents
- long-form rationale documents explaining why a chosen representation is
  necessary
- exploratory performance notes that are not part of the Beman library story

This is important because the Beman repo should be reviewable as a library.
The `trees` repo can remain the place where the full design journey is visible.

## Cross-Reference Guidance

The paper-planning note should reference this file as the repository-planning
companion.
This note should remain focused on repo migration, layout, extraction, and
Beman conformance rather than on paper strategy.

## Summary

The integrated Beman project should be treated as the production-tracking home
for the coordinated paper set, while `trees` remains the broader design and
pedagogy workspace.

The largest migration issue is not code correctness.
It is structural translation:

- merged-src to split Beman layout
- `smd/...` public identity to `beman/<short_name>/...`
- co-located `*.t.cpp` tests to top-level `tests/.../*.test.cpp`
- mixed docs to a cleaner split between production docs and pedagogical docs

The right path is to stamp from exemplar, extract deliberately, and keep the
pedagogical parts in `trees` unless they are necessary to review the Beman
library itself.