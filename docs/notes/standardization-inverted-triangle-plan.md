# Inverted Triangle Plan: From Design Thesis to Papers

## Purpose

This note captures the current standardization plan in inverted-triangle form:
start from the broadest design claim, narrow through proposal families, and
end at concrete candidate papers.

The point is not to freeze the paper sequence.
The point is to keep the broad thesis, the tactical decomposition, and the
paper-sized units aligned.

This is a planning note, not paper text.
It may include framing useful for hallway-track conversation or discussion with
 collaborators, even where the final papers should use a calmer and more
impersonal tone.

Repository extraction and Beman-conformance planning are tracked separately in
[docs/notes/beman-integration-plan.md](/workarea/trees/docs/notes/beman-integration-plan.md).
This note stays focused on paper architecture and proposal strategy.

Current planning assumption:

- the paper set should be published as a coordinated group, not one at a time
- each paper should still be independently reviewable and independently useful
- overlap between papers is acceptable when it is controlled and explicit
- the most likely anchor paper is now the traversal/applicative problem paper
  rather than the container paper or the pure extension-point paper

## Top of the Triangle: Broad Thesis

The standard library has strong support for sequence-oriented generic
programming centered on external iteration.
That remains valuable.
But it is not the only credible foundation for generic algorithms.

For recursive, persistent, and effect-aware structures, a different center of
gravity is often better:

- internal traversal rather than external cursor choreography
- structural reconstruction rather than elementwise mutation
- explicit adaptation objects rather than duck-typed naming conventions
- minimal primitive adaptation plus derived defaults
- overrideable derived operations when the default is not efficient enough

The design space here is now mature enough to talk about standardization.
The goal is not to replace STL-style iteration.
The goal is to standardize a complementary generic style for structures and
algorithms that do not fit the iterator model especially well.

## Second Layer: Architectural Claims

Several architectural claims currently hang together.
They should remain conceptually connected even if they are standardized, if at
all, in separate papers.

### 1. Typeclass-object adaptation is a useful C++ extension-point pattern

The typeclass-object pattern provides:

- a bundle of related operations rather than one CPO per operation
- derivable defaults from a small primitive core
- explicit adaptation of types that do not share member names
- static dispatch with open extension
- explicit override points for better implementations

This is not only a dispatch mechanism.
It is also an adaptation framework in the same general spirit as iterator
facades and `view_interface`: implement a small semantic core, get a coherent
surface, override where needed.

### 2. The naming problem should be split, not globally solved

Abstract generic operations need stable names.
Concrete domains need names that fit local semantics.

The active repository position is already close to the right one:

- abstract names live in the adaptation layer
- domain names live on concrete types
- generic code targets the adaptation layer, not the type's end-user surface

This avoids forcing misleading duck-typed names like `transform` onto every
type that happens to support a functor-like mapping operation.

### 3. Traversal and effect transposition are the most compelling user-facing story

The strongest motivation currently is not the abstract typeclass hierarchy by
itself.
It is the problem of transposing structure and context while preserving shape.

Examples:

- `vector<sender<T>> <-> sender<vector<T>>`
- `vector<simd<T>> <-> simd<vector<T>>` as an analogy for zipped/lanewise
  independent composition
- `vector<optional<T>> -> optional<vector<T>>`

This makes `Traversable` the most visible user problem and makes the
supporting applicative machinery easier to explain as intent rather than as
currying mechanics.

### 4. Persistent measured sequences are a meaningful standard-library target

Finger trees matter not because they are elegant or old, but because they are
a mature implementation family for:

- persistent sequence structure
- amortized front/back edits
- efficient concatenation
- splitting by accumulated measure
- generic support for indexed or weighted search

The user-facing case should likely be stated in terms of capabilities rather
than in terms of “standardize finger trees by name”.

### 5. Recursive tree structures benefit from the same machinery

Fixpoint trees and related recursive structures are strong internal evidence
that the same adaptation model scales beyond flat sequences.
They may or may not become the first standard library target.
But they are important as:

- motivating examples for traversal and fold structure
- proof that internal iteration is a real generic style in C++
- evidence that the typeclass-object pattern is not sequence-only

## Third Layer: Proposal Families

The broad thesis currently narrows into four proposal families.
These are related, but they should not be forced into one paper.

Because the intended publication mode is now a coordinated set rather than a
drip sequence, the right question is not “which paper strictly comes first?”
but “which paper is the public anchor, and how do the others remain independent
without pretending there is no overlap?”

### Family A: Adaptation framework and naming model

Core question:
How should C++ let a type participate in a bundle of related generic
operations with derivable defaults and explicit override points?

Likely contents:

- variable-template or equivalent object lookup model
- primitive vs derived operation model
- implementer-surface versus user-surface naming split
- explicit-object and NTTP-pinned lookup modes

This family is mostly about generic library design.
It overlaps intentionally with the traversal/applicative family, because the
extension-point framework is currently the least-worst practical encoding of
the applicative/traversable story.

### Family B: Traversal, sequencing, and contextual application

Core question:
How should C++ express shape-preserving traversal and transposition between a
container and a computational context?

Likely contents:

- traversal as the user-facing operation
- sequence/transpose operations
- supporting “apply pure function in context” model
- examples involving senders, `optional`, SIMD analogies, and trees

This family is the strongest user-facing motivation for the adaptation model.
It is currently the best candidate for the anchor paper because it can solve
problems involving standard-library-relevant components today.

### Family C: Persistent measured sequence / tree-like container

Core question:
What user-facing abstraction best captures the capabilities often implemented
with finger trees?

Likely contents:

- persistent sequence semantics
- concatenation and front/back operations
- splitting and search by position or measure
- iterator/container integration where appropriate

This family should likely be phrased in terms of library utility and
capabilities, not just literature lineage.

### Family D: Recursive tree structures and algorithms

Core question:
What recursive tree abstraction, if any, deserves standard-library treatment,
and what generic algorithms should be expressed over it?

Possible shapes:

- fixpoint-tree-oriented design work
- a rose-tree or immutable tree vocabulary type
- an algorithm paper rather than a concrete container paper

This family may stay as design support longer than the others.

## Fourth Layer: Candidate Paper Sequence

The current best decomposition is a stack of papers where each one pays for
itself.
The later papers benefit from earlier ones, but none should require the entire
stack to be accepted at once.

That said, the working publication plan is now to release the papers as a
coordinated set.
So the sequence below is logical rather than chronological.
It describes roles and boundaries, not necessarily submission order.

### Paper A: Traversal and contextual application

Working purpose:
be the anchor paper for the whole effort.

Why this is currently the strongest anchor:

- it addresses a live problem over familiar or standard-adjacent components
- it explains why the rest of the design exists
- it does not require prior sympathy for trees or recursive representations
- it provides the clearest public-facing “why now?”

Working scope:

- shape-preserving traversal
- sequencing and transposition of structure and context
- applicative-style independent contextual composition as the semantic basis
- user-facing API vocabulary that does not require leading with category-theory
  names

Representative motivating examples:

- `vector<optional<T>> -> optional<vector<T>>`
- `vector<sender<T>> <-> sender<vector<T>>`
- `vector<simd<T>> <-> simd<vector<T>>` as a strong explanatory analogy for
  zipped or lanewise independent composition

What this paper should not try to do:

- standardize the full algebraic hierarchy
- introduce tree containers as its primary reason to exist
- depend normatively on the extension-point paper, even if it benefits from it

Relationship to companion papers:

- can say that the companion extension-point paper provides a uniform
  customization framework for these operations
- should remain independently meaningful if that companion paper changes,
  stalls, or is rejected

### Paper B: Typeclass objects as an extension-point pattern

Working purpose:
establish the adaptation and derivation framework as a serious C++ design
technique.

Likely scope:

- bundles of operations
- primitive/derived split
- overrideable defaults
- explicit lookup objects
- comparison to ADL, CPOs, `tag_invoke`, traits, and facade-style frameworks

Additional focus, now that overlap with the anchor paper is explicit:

- explain why applicative/traversable need more than one-CPO-per-operation
  customization
- explain how the primitive/derived split lowers adoption cost for user types
- explain why the same mechanism naturally extends to nearby families such as
  monoid, foldable, monad, alternative, and bifunctor

Role in the coordinated set:

- this is the mechanism paper, not the anchor problem paper
- it should present applicative/traversable as the first serious clients
- it should use the broader abstraction family as evidence of scalability, not
  as first-wave standardization scope

This paper is about generic library architecture, not about trees first.

What this paper should not do:

- force the whole cluster to be reviewed as an FP vocabulary package
- pretend to be fully independent of the traversal motivation
- require the tree papers for justification

### Paper C: Persistent measured sequence container

Working purpose:
turn the finger-tree capability story into a standalone library proposal.

Likely scope:

- persistent sequence semantics
- front/back operations under persistence
- concatenation
- prefix search and split by position or accumulated measure
- iterator and container integration where appropriate

Working naming guidance:

- prefer capability-oriented naming over implementation-family naming
- the paper may describe finger trees as the primary lineage or likely
  implementation family, but the title should probably focus on the user-facing
  abstraction

Relationship to companion papers:

- benefits from the adaptation paper for generic integration
- benefits from the traversal paper for algorithmic vocabulary
- must still stand as a container paper even if neither companion is adopted

### Paper D: Recursive tree abstraction or recursive tree algorithms

Working purpose:
capture the recursive-tree side once the traversal and adaptation story is more
settled.

Likely scope:

- tree algorithms over a recursive structure
- or a concrete immutable tree vocabulary type
- or a fixpoint-tree-oriented design paper if the use case becomes strong

This is currently the least fixed slot.
It may remain more design-heavy for longer than the other three.

## Fifth Layer: Paper Boundaries and Managed Overlap

Because the papers are now intended to ship together, overlap is not only
acceptable but necessary.
The key is to make the overlap disciplined.

### Traversal paper vs extension-point paper

These two papers overlap the most.
That overlap is a feature, not a mistake.

The correct split is:

- the traversal paper is the problem paper
- the extension-point paper is the mechanism paper

More concretely:

- the traversal paper answers: why do we need this capability?
- the extension-point paper answers: why is this the right adaptation model for
  expressing it in C++?

The traversal paper should treat applicative and traversable as the first-class
motivation.
The extension-point paper should treat applicative and traversable as its first
serious client and use the larger family only as evidence of generality.

### Shared background that may appear in both papers

Some material will naturally appear in both papers in abbreviated form:

- the distinction between abstract adapter vocabulary and domain-specific names
- the primitive/derived split
- the value of overrideable defaults
- examples showing structure/context transposition

This shared material should be concise and tailored to each paper's role.
It should not try to make either paper completely self-sufficient on all
technical detail.

### Material that should stay mostly in the traversal paper

- why transposition of structure and context is a real standard-library problem
- sender, optional, and SIMD-flavored motivation
- user-facing vocabulary and semantics of traversal/sequence operations
- the public-facing case that this solves something useful today

### Material that should stay mostly in the extension-point paper

- lookup-object mechanics
- variable-template adaptation model
- comparison with ADL, CPOs, `tag_invoke`, and trait-only approaches
- primitive/derived machinery across multiple abstraction families
- the argument that user-defined types can start minimal and optimize later

### Material that should stay mostly in the container paper

- semantic contract of the persistent measured sequence
- complexity guarantees
- iterator/container compliance
- measured split/search operations
- relation to other standard containers and views

### Material that should stay mostly in the tree paper

- recursive-structure motivation
- fixpoint or immutable tree design issues
- tree-specific algorithms and recursive traversal semantics
- evidence that internal iteration is a serious generic style in C++

## Sixth Layer: Coordinated Publication Plan

The working publication model is now:

- publish the papers as a coordinated set
- let each paper name the others explicitly as related work or in-flight support
- avoid hard normative dependence where possible
- allow progress on any paper if another becomes controversial or delayed

That means each paper should contain a short section of the form:

- what this paper covers on its own
- what companion papers provide if adopted
- what remains valid even if companion papers do not progress at the same pace

This is especially important for the anchor paper and its closest companion.

## Seventh Layer: Review-Control Strategy

One explicit lesson from prior proposal work is that review goes better when
the paper clearly classifies its decisions instead of leaving every design
choice looking equally open to debate.

This matters for this paper set because the design space is large and unfamiliar
enough that unmanaged discussion will otherwise spread into secondary questions.

### Working rule

Each paper should explicitly identify which choices are:

- arbitrary but necessary
- motivated trade-offs
- direct consequences of the primary design goal
- intentionally deferred or delegated to companion papers

This is not cosmetic.
It is a way to keep review energy focused on the core claims rather than on
every surrounding design detail.

### Why this is useful

- reviewers can see that a choice was made deliberately rather than casually
- not every choice gets treated as a first-principles design debate
- companion-paper boundaries become clearer
- the paper is better able to say what it is and is not attempting to solve

### Suggested drafting pattern

For major design areas, include short sections or paragraphs of the form:

- Design decision
- Alternatives considered
- Why this choice for this revision
- What remains intentionally open or deferred

For each paper as a whole, front-load the proposal shape as well:

- the abstract must identify both the problem and the general shape of the
  solution
- the early “Before / After” table must strongly motivate the problem being
  solved, not merely summarize mechanics
- the reader should not need to get halfway through the paper to discover what
  is actually being proposed

This is important not only for first reading but for re-reading.
Papers are often revisited after weeks or months.
The abstract and early before/after material must be strong enough to bring the
proposal back to the front of the reader's mind quickly.

This pattern is especially useful when a choice is not uniquely optimal but a
decision still has to be made.

### Paper-set application

The same strategy should be applied across the coordinated set, not just within
one paper.
Each paper should say:

- which decisions are local to this paper
- which trade-offs are accepted because of this paper's scope
- which issues are intentionally handled in a companion paper
- which issues are left open because they are not necessary to make progress

## Eighth Layer: P3200 Reservation

## Seventh Layer: P3200 Reservation

`P3200` is currently being held for one of these papers.
The placeholder associated with that reservation was:

- `Trees for the standard library`

but no `D3200R0` or `P3200R0` was published under that title.

Because a paper number is needed before publication, the reserved number cannot
stay abstract forever.
It must be assigned to the paper that is expected to act as the public anchor
of the coordinated set.

Working purpose:
pick the anchor paper early enough to enable publication logistics without
letting numbering alone dictate the proposal architecture.

### Current recommendation

Assign `P3200` to the traversal/context-transposition paper.

Reasoning:

- it has the clearest “problem exists today” story
- it can stand on current standard and standard-adjacent components
- it explains why the companion extension-point paper exists
- it gives the coordinated set a public-facing center that is not dependent on
  immediate sympathy for tree containers

### Why not assign P3200 to the extension-point paper?

Because the extension-point framework is best motivated by its clients, and the
strongest current client is applicative/traversable-style traversal.
The mechanism paper is important, but it is better understood as companion
infrastructure than as the public flagship.

### Why not assign P3200 to the container paper?

Because the container paper is currently less universal in its motivation than
the traversal paper.
It may yet become a major paper in the set, but it is not the best anchor for
explaining the whole coordinated release.

### Why not assign P3200 to the tree paper?

Because the recursive-tree paper is still the least fixed in scope and the most
likely to evolve substantially before publication.

## Ninth Layer: More Detailed Per-Paper Plans

### Paper A plan: traversal and contextual application

Primary claim:
C++ needs a standard way to express shape-preserving traversal and
transposition between a structure and a computational context.

Essential sections:

1. Problem statement grounded in current components.
2. Examples showing independent contextual composition.
3. User-facing operations: traverse, sequence, and lifted contextual
   application.
4. Semantics: independence vs sequencing.
5. Customization story, likely brief, with companion-paper cross-reference.
6. Why the abstraction is broader than any one domain.

Success criterion:
reviewers can care about the problem even if they do not yet care about trees
or about the general extension-point framework.

Review-control guidance for this paper:

- classify the public-facing naming choices as either necessary terminology
  choices or deferred naming refinements
- state explicitly which contextual examples are normative motivation and which
  are explanatory analogies
- mark the companion extension-point framework as supporting machinery rather
  than as an implicit prerequisite
- make the abstract and early before/after material immediately answer two
  questions: what real problem exists today, and what kind of operation is
  being proposed to solve it

### Paper B plan: bundled extension-point objects with derived defaults

Primary claim:
C++ benefits from a customization mechanism that bundles related operations,
derives defaults from a small primitive core, and leaves every derived
operation open to override.

Essential sections:

1. Problem with per-operation customization alone.
2. Primitive/derived split and low-friction adaptation story.
3. Explicit lookup objects and override modes.
4. Comparison with CPOs, `tag_invoke`, traits, and facade patterns.
5. Applicative/traversable as the motivating case study.
6. Broader family as evidence of scalability.

Success criterion:
reviewers can see this as a serious library-design technique rather than as a
Haskell import exercise.

Review-control guidance for this paper:

- identify which aspects of the lookup-object design are essential and which
  are one workable encoding choice
- classify primitive-vs-derived splits as deliberate trade-offs, not as the
  only mathematically possible decomposition
- state clearly which parts of the larger abstraction family are examples of
  extensibility rather than first-wave standardization scope
- make the early before/after material show why current C++ customization tools
  are insufficient for this family of operations, and what the proposed
  framework changes structurally

### Paper C plan: persistent measured sequence

Primary claim:
the standard library lacks a persistent sequence abstraction supporting
concatenation and prefix-based split/search under general accumulated measures.

Essential sections:

1. User problem and capability matrix.
2. Public semantics of the sequence abstraction.
3. Complexity and iterator/container model.
4. Measure-based split and search.
5. Persistent semantics and structural sharing.
6. Implementation notes and relationship to finger trees.

Success criterion:
the paper is reviewable as a container proposal even if the rest of the paper
set moves more slowly.

Review-control guidance for this paper:

- classify user-visible semantic choices separately from implementation-lineage
  choices
- make clear which operations are fundamental to the abstraction and which are
  convenience or integration features
- identify any arbitrary-but-necessary API choices early so they do not become
  proxy debates about the whole container
- make the early before/after material show the missing capability combination,
  not just that finger trees are interesting or elegant

### Paper D plan: recursive tree abstraction or algorithms

Primary claim:
recursive trees expose generic structure and traversal problems not naturally
captured by sequence-centric iterator models alone.

Essential sections:

1. Why recursive structure deserves direct treatment.
2. Candidate vocabulary type or algorithm family.
3. Relationship to internal traversal and reconstruction.
4. Adaptation and traversal integration where useful.
5. Examples from fixpoint and immutable tree representations.

Success criterion:
the paper clarifies the recursive-tree side of the design without requiring the
entire set to be adopted first.

Review-control guidance for this paper:

- distinguish sharply between what is being proposed as a vocabulary type,
  what is only implementation technique, and what is only motivating example
- call out where the current revision is making a pragmatic representational
  choice rather than claiming a uniquely best tree encoding
- identify what is intentionally not being solved in first-wave scope
- make the abstract and early before/after material explain why recursive tree
  structure is the problem space, rather than assuming readers already share
  that premise

## Fifth Layer: Near-Term Decisions

Several decisions still control how the paper sequence narrows.

### Decision 1: Is the traversal paper the anchor paper?

Two plausible openings exist:

- lead with the extension-point pattern, then motivate traversal as a client
- lead with traversal as the visible problem, then explain the adaptation model
  as the enabling mechanism

Current bias: traversal should be the anchor paper, with the extension-point
paper as a close companion rather than as the flagship.

### Decision 2: Is the sequence paper called “finger tree” or something else?

Current bias: prefer capability-oriented naming over implementation-oriented
naming.

Examples of the likely shape:

- persistent measured sequence
- concatenable persistent sequence
- split/search sequence with accumulated measure

Finger trees remain the key technical lineage, but not necessarily the paper
title.

### Decision 3: Is fixpoint tree a paper, a major example, or just internal design support?

This is not yet settled.
Fixpoint trees clearly matter for the design story.
It is less clear that they should be an early standalone standardization target.

### Decision 4: How much algebraic naming appears in titles and lead sections?

Current bias:

- keep algebraic names in the technical/core discussion
- prefer C++-native problem statements and user-facing terminology in titles,
  abstracts, and early motivation sections

## Bottom of the Triangle: Concrete Paper Candidates

These are the concrete candidates the broader plan currently narrows toward.
They are intentionally provisional.

### Candidate A: Traversal anchor paper

Possible title direction:

- Shape-Preserving Traversal for Contextual Computations
- Transposing Structures and Computational Contexts
- Traversal and Sequencing for Contextual Values

Role:
public anchor paper for the coordinated set, and current recommended home for
`P3200`.

### Candidate B: Extension-point paper

Possible title direction:

- Typeclass Objects as a C++ Extension-Point Pattern
- Bundled Generic Customization Objects with Derived Defaults

Role:
companion foundation paper for the implementation/adaptation story.

### Candidate C: Sequence/container paper

Possible title direction:

- A Persistent Measured Sequence for the Standard Library
- Concatenable Persistent Sequences with Prefix Search and Split

Role:
container proposal grounded in finger-tree capabilities.

### Candidate D: Tree paper

Possible title direction:

- Recursive Tree Algorithms for the Standard Library
- Immutable Tree Structures and Traversal
- Fixpoint Trees in the Standard Library

Role:
currently the least fixed and most dependent on later confidence.

## Tenth Layer: Current Paper Sketches

These are not draft papers.
They are working sketches for the current conception of each paper:

- tentative title
- abstract shape
- what the leading Before / After table must accomplish

They should be revised as the proposal set sharpens, but they are meant to be
concrete enough to guide drafting.

### Paper A sketch: traversal anchor paper

#### Tentative title

Shape-Preserving Traversal for Contextual Computations

Alternative title directions:

- Transposing Structures and Computational Contexts
- Traversal and Sequencing for Contextual Values

#### Abstract sketch

C++ has vocabulary types and computational contexts that are individually well
understood, but it lacks a uniform way to traverse a structure while producing
contextual results and then transpose the result into a single outer context.
This problem appears today in combinations such as containers of optionals,
containers of senders, and lanewise or zipped structured computation.

This paper proposes a shape-preserving traversal facility together with
sequencing operations that convert a structure of contextual values into a
contextual structure.
The proposal is based on independent contextual composition rather than on
general sequential dependence, which makes it suitable for effect aggregation,
batching, and transposition-style algorithms.

The paper focuses on the problem being solved and the user-facing operation
set.
Companion work may provide a more general customization framework for adapting
additional structures, but the traversal problem and the proposed API are
independently meaningful.

#### Leading Before / After table should show

The table should make the problem visible immediately.
It should not begin with algebraic terminology.

The “Before” column should show:

- ad hoc loops for `vector<optional<T>>` or similar structures
- manual state threading or repeated early-exit logic
- bespoke code to convert `structure<context<T>>` into `context<structure<T>>`
- no common abstraction for shape-preserving contextual traversal

The “After” column should show:

- one traversal operation over a familiar structure
- one sequencing/transposition operation
- the same shape preserved on success
- the context moved to the outside in a single expression

The table must answer, immediately:

- what concrete pain exists today
- what kind of operation is being proposed
- why this is more than just a helper for one type

#### Core claim

C++ needs a standard way to express shape-preserving traversal and sequencing
of contextual values, because this is already a real problem over standard and
standard-adjacent components, and current practice is fragmented and ad hoc.

#### Evidence and examples needed before drafting starts

- at least three examples from different domains that all instantiate the same
  traversal problem shape
- at least one example over current standard vocabulary types
- one example that makes independent contextual composition feel modern and
  performance-relevant rather than academic
- a compact explanation of why this is not just `transform` plus manual glue

Preferred examples:

- `optional`
- sender-like async composition
- SIMD or Zip-like lanewise structure as an explanatory analogy

#### Likely objections

- “Why is this not just a helper algorithm over existing containers?”
- “Why isn’t this `transform`, `ranges`, or `zip` by another name?”
- “Why is applicative-style structure needed instead of ordinary sequential
  chaining?”
- “Is this too abstract for the standard library?”

#### What the implementation must show to be acceptable

Assume a Beman Project implementation exists and evolves with the paper.
The question is therefore not basic executability but acceptability and
evidence quality.

The implementation should demonstrate:

- one coherent user-facing API for traversal and sequence operations
- operation over more than one concrete structure, including at least one
  standard or standard-adjacent type
- operation over more than one context family, so the design does not look
  single-domain
- that shape preservation is a semantic invariant, not an accidental property
- that the API is teachable without first teaching the whole algebraic
  hierarchy
- that the examples in the paper are actually representative of the design, not
  hand-picked one-offs

### Paper B sketch: extension-point framework paper

#### Tentative title

Bundled Generic Customization Objects with Derived Defaults

Alternative title directions:

- Typeclass Objects as a C++ Extension-Point Pattern
- Bundled Extension-Point Objects for Generic Libraries

#### Abstract sketch

C++ customization mechanisms such as ADL, customization point objects,
`tag_invoke`, and trait structures are effective for individual operations, but
they do not directly provide a uniform way to adapt a type to a family of
related operations with derivable defaults and explicit override points.

This paper proposes a bundled customization model in which a type participates
through an explicitly looked-up object that defines a small primitive core and
receives a larger derived surface by default.
The model supports static dispatch, open extension, explicit override, and
incremental adaptation from minimal conformance to optimized specialization.

Applicative and traversable-style operations provide the primary case study for
the design, and nearby abstraction families such as monoid, foldable, monad,
alternative, and bifunctor demonstrate that the mechanism scales beyond a
single client.

The paper is about the adaptation framework itself.
Companion papers may define particular operation families or data structures
that use it.

#### Leading Before / After table should show

The table should make the mechanism gap visible before any deep machinery is
introduced.

The “Before” column should show:

- one CPO or ADL hook per operation
- duplicated customization work across a family of related operations
- no shared place for defaults derived from primitives
- weak support for “start small, optimize later” adaptation

The “After” column should show:

- one looked-up customization object for a concept family
- a small primitive surface
- a larger derived surface obtained automatically
- the ability to replace any derived operation with a better implementation

The table must answer, immediately:

- what current extension-point tools do not do well enough
- why this is a framework problem rather than just a local library trick
- why the proposal is about adaptation and defaults, not only dispatch

#### Core claim

Some C++ abstraction families need a bundled adaptation object with minimal
primitives, derived defaults, and explicit override points, because per-
operation customization mechanisms do not provide enough structure for the
family as a whole.

#### Evidence and examples needed before drafting starts

- one primary case study where one-CPO-per-operation or traits-only adaptation
  becomes visibly awkward
- a clear primitive-vs-derived table for the main client family
- at least one second family showing that the mechanism is not overfit to a
  single concept pair
- comparison material against ADL, CPOs, `tag_invoke`, traits, and facade-like
  frameworks

Preferred case studies:

- applicative and traversable as the main client
- foldable or monoid as nearby evidence
- one additional family such as bifunctor or alternative only if it helps more
  than it distracts

#### Likely objections

- “Why is this not just traits plus free functions?”
- “Why is this not just a set of CPOs?”
- “Is this too much new machinery for the standard library?”
- “Why should the standard library standardize a framework rather than a set of
  concrete facilities?”
- “Is the design too FP-shaped for ordinary C++ practice?”

#### What the implementation must show to be acceptable

The implementation should demonstrate:

- a small primitive core yielding a noticeably larger derived surface
- explicit override of a derived operation for better semantics or efficiency
- incremental adaptation: start with the primitive core, then refine
- use across multiple concept families, not just one class template or one
  algorithm
- readable user code that benefits from the framework rather than merely making
  the implementation cleverer
- that the framework makes later papers easier to express, not harder to read

### Paper C sketch: persistent measured sequence paper

#### Tentative title

A Persistent Measured Sequence for the Standard Library

Alternative title directions:

- Concatenable Persistent Sequences with Prefix Search and Split
- Persistent Sequences with Measured Split and Search

#### Abstract sketch

The standard library provides strong support for mutable flat sequences, but it
does not provide a persistent sequence abstraction that combines efficient
front and back operations, concatenation, and splitting or searching by
accumulated measure.
These capabilities appear in editors, batching pipelines, incremental
processing, and other domains where structural sharing and non-destructive
updates are useful.

This paper proposes a persistent measured sequence abstraction with efficient
concatenation, prefix-based search, and split operations.
The design emphasizes semantic capabilities rather than a single implementation
name, while drawing on the long-studied literature and practice behind
finger-tree-like structures.

The paper focuses on the container abstraction, its semantics, and its
complexity guarantees.
Companion work may supply generic traversal vocabulary or adaptation
infrastructure, but the sequence abstraction is independently motivated.

#### Leading Before / After table should show

The table should foreground the missing combination of capabilities.

The “Before” column should show:

- `vector`, `deque`, list-like containers, rope-like ad hoc structures, or
  paired container compositions each covering only part of the problem
- destructive updates or copying where persistence would be valuable
- no standard abstraction combining concatenation with prefix split/search

The “After” column should show:

- one persistent sequence abstraction
- explicit support for concatenation and measured split/search
- structural sharing rather than forced mutation
- a clear capability combination not present in today's standard containers

The table must answer, immediately:

- what capability set is missing today
- why persistence matters here
- why the proposal is about a sequence abstraction, not just about an exotic
  data structure

#### Core claim

The standard library lacks a persistent sequence abstraction that combines
structural sharing, concatenation, and measured split/search in one coherent
facility, and that missing combination matters in real workloads.

#### Evidence and examples needed before drafting starts

- a capability matrix against existing standard containers and views
- at least two motivating workloads where the combined capability set matters
- clear semantic explanation of persistence and structural sharing
- concrete complexity story for the key operations
- enough user-facing examples that the abstraction can be understood without
  implementation details

Preferred examples:

- text or editing-style structure
- batching or chunked processing pipeline
- weighted or indexed split/search use case

#### Likely objections

- “Why is this not served well enough by `vector`, `deque`, `list`, rope-like
  library types, or views?”
- “Why should persistence be in the standard library?”
- “Is the measured split/search story too specialized?”
- “Is the proposal exposing an implementation lineage instead of a user-facing
  abstraction?”
- “Does this fit ordinary container expectations?”

#### What the implementation must show to be acceptable

The implementation should demonstrate:

- the full capability combination the paper claims, not only a subset
- persistence and structural sharing as real semantics, not just copy-heavy
  emulation
- stable and understandable iterator/container behavior
- container ergonomics sufficient for reviewers to imagine real use
- examples of measured split/search that feel integral, not bolted on
- performance and complexity evidence strong enough to justify the abstraction
  boundary, even though mere “working code” is assumed

### Paper D sketch: recursive tree abstraction or algorithms paper

#### Tentative title

Recursive Tree Algorithms for the Standard Library

Alternative title directions:

- Immutable Trees and Structural Traversal
- Recursive Tree Structures and Algorithms in the Standard Library
- Fixpoint Trees and Recursive Traversal in C++

#### Abstract sketch

Recursive tree structures are common in parsing, transformation pipelines,
symbolic representations, and hierarchical data processing, but the standard
library offers little direct support for generic algorithms centered on
recursive structure rather than on flat external iteration.

This paper proposes either a recursive tree vocabulary type, a focused family
of recursive tree algorithms, or both, depending on final scope.
The unifying theme is that recursive trees are more naturally served by
structure-directed traversal and reconstruction than by cursor-oriented generic
interfaces alone.

The paper is intended to make the recursive-tree side of the design space
reviewable on its own.
It can draw on companion work for traversal vocabulary or adaptation patterns,
but its primary goal is to make the recursive problem space explicit and
concrete.

#### Leading Before / After table should show

The table should justify why recursive trees deserve their own treatment.

The “Before” column should show:

- recursive structures handled through bespoke one-off algorithms
- iterator-centric interfaces that expose traversal mechanics rather than tree
  structure
- limited reuse across different recursive tree representations

The “After” column should show:

- one vocabulary type or one reusable algorithm family over recursive trees
- structure-directed traversal or reconstruction
- a generic story that works across more than one tree representation

The table must answer, immediately:

- why recursive trees are the problem space
- what generic capability is missing today
- whether the proposal is primarily a type, an algorithm family, or a bridge
  between both

#### Core claim

Recursive trees are an important enough structural family that C++ should offer
either a reusable vocabulary abstraction, a reusable algorithm layer, or both,
rather than leaving every recursive-tree design to bespoke local solutions.

#### Evidence and examples needed before drafting starts

- clear scoping decision: vocabulary type, algorithms, or hybrid
- at least two distinct recursive-tree use cases that are not merely the same
  representation with different names
- explanation of why sequence-centric interfaces are not the right generic
  center here
- examples showing that recursive traversal and reconstruction are reusable
  beyond one concrete tree implementation

Preferred examples:

- expression or syntax trees
- immutable tree transformation
- one fixpoint-based example if it clarifies the design rather than dominating
  it

#### Likely objections

- “Why does the standard library need a tree abstraction here at all?”
- “Is this too theoretical or too representation-driven?”
- “Why isn’t this better left to user code or third-party libraries?”
- “Is fixpoint machinery necessary, or merely one implementation technique?”
- “Why not just expose iterators and let algorithms do the rest?”

#### What the implementation must show to be acceptable

The implementation should demonstrate:

- that the proposed abstraction works across more than one recursive tree shape
- that the generic layer is meaningfully reusable
- that structure-directed traversal or reconstruction is clearer than bespoke
  hand-written recursion at call sites
- that the proposal is not secretly just standardizing one local representation
- that the public API can be understood without first buying into the entire
  supporting theory

## Eleventh Layer: Acceptance-Oriented Implementation Guidance

Because a Beman Project implementation is assumed to track the papers, the bar
for implementation evidence is not “does something compile and run?”
The more important question is what the implementation must make clear to be
convincing in standardization review.

Across the coordinated set, the implementation work should show:

- that the proposed APIs are coherent and teachable
- that the examples used in the papers are representative rather than narrow
  demonstrations
- that customization and adaptation scale beyond one hand-crafted example
- that primitive/derived decompositions are practical rather than merely elegant
- that important semantics such as shape preservation, persistence, or explicit
  override are visible in use
- that the design remains stable enough under evolution to support paper review
  across revisions

Where performance matters, implementation evidence should support the claimed
abstraction boundary.
Where performance is not the primary point, the implementation should still
demonstrate ergonomic credibility and semantic clarity.

## P3200 Reservation

`P3200` is currently being held for one of these papers.
The placeholder associated with that reservation was:

- `Trees for the standard library`

but no `D3200R0` or `P3200R0` was published under that title.

The paper number should therefore be treated as reserved but unassigned in
substance.

Current guidance:

- the number now needs to be assigned before publication can proceed
- the assignment should follow the anchor-paper decision, not historical
  placeholder text
- current best assignment is the traversal anchor paper

## Summary

The current plan narrows as follows:

1. Broad thesis: C++ needs a complementary generic style for recursive,
   persistent, and effect-aware structures.
2. Architectural claims: explicit adaptation objects, split naming layers,
   traversal as the strongest user story, measured persistent sequences as a
   plausible library target.
3. Proposal families: adaptation, traversal, persistent sequence, recursive
  trees.
4. Coordinated paper set: traversal anchor, extension-point companion,
  sequence/container paper, and tree paper.
5. Current working assignment: the traversal paper is the best public anchor
  and the best current home for the reserved `P3200` slot.

Until that anchor is clearer, the correct move is not to collapse the stack
into one mega-paper.
The correct move is to keep narrowing the sequence until each paper has one
clear reason to exist.