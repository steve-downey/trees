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
- the most likely anchor paper is now a merged traversal-plus-customization
  paper rather than a traversal-only paper, a pure mechanism paper, or the
  container paper

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

### 1. Bundled customization objects are the right mechanism for traversal-like families

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

The recent `beman.monadics` critique sharpens this point.
Traditional trait packaging is not strong enough for applicative-style families
because it does not securely bundle primitives like `pure` and `apply` with
their derived operations in a coherent lock-and-key way.
If traversal is proposed without this bundled mechanism, review pressure will
likely drift toward trait-only or `tag_invoke`-heavy designs that do not hold
the family together well enough.

### 2. The naming problem should be split, not globally solved

Abstract generic operations need stable names.
Concrete domains need names that fit local semantics.

The active repository position is already close to the right one:

- abstract names live in the adaptation layer
- domain names live on concrete types
- generic code targets the adaptation layer, not the type's end-user surface

This avoids forcing misleading duck-typed names like `transform` onto every
type that happens to support a functor-like mapping operation.

It also implies a second naming rule for paper-facing API design:

- lead with action verbs in the public surface where possible
- confine academic nouns to specification text, rationale, or footnotes

For LEWG, names like `transpose`, `reduce`, and `build` are easier to carry
than terms like traversable sequencing, algebra, or coalgebra in lead sections.

### 3. Traversal and effect transposition are the most compelling user-facing story

The strongest motivation currently is not the abstract typeclass hierarchy by
itself.
It is the problem of transposing structure and context while preserving shape.

Examples:

- `vector<sender<T>> <-> sender<vector<T>>`
- `vector<simd<T>> <-> simd<vector<T>>` as an analogy for zipped/lanewise
  independent composition
- `vector<optional<T>> -> optional<vector<T>>`

This makes traversal the most visible user problem and makes the supporting
applicative machinery easier to explain as intent rather than as currying
mechanics.

It also implies that the public operation should be described in terms of
transposition, not “sequencing”, when the operation flips
`structure<context<T>>` into `context<structure<T>>`.
In C++, “sequence” already carries too much noun-shaped container baggage.

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

The strongest near-term pitch here is probably algorithmic rather than
vocabulary-typish:

- recursive fold/rebuild operations over variant-based recursive nodes
- reusable recursive algorithms rather than one blessed concrete tree layout

### 6. “Box” should not be the public mental model

For the contextual side of the story, “box” is increasingly the wrong mental
model.
It suggests containment and range-like structure, which invites false
equivalences.

For types like `optional`, `expected`, and `StatusOr`-style results, the public
story should prefer terms like:

- fallible value
- outcome
- contextual result

and avoid leaning on “box” terminology in the paper narrative.

## Third Layer: Proposal Families

The broad thesis now narrows into three primary proposal families.
These are related, but they should not be forced into one mega-paper.

Because the intended publication mode is now a coordinated set rather than a
drip sequence, the right question is not “which paper strictly comes first?”
but “which paper is the public anchor, and how do the others remain independent
without pretending there is no overlap?”

### Family A: Traversal, transposition, and bundled customization

Core question:
How should C++ express shape-preserving traversal and transposition between a
structure and a computational context, and what customization mechanism is
strong enough to make that coherent in C++?

Likely contents:

- traversal as the user-facing operation
- transpose operations for flipping structure and context
- supporting “apply pure function in context” model
- variable-template or equivalent object lookup model
- primitive vs derived operation model
- implementer-surface versus user-surface naming split
- explicit-object and NTTP-pinned lookup modes

This family is now explicitly both the problem paper and the mechanism paper.
That merge is intentional.

The reason is practical, not merely aesthetic:

- the traversal problem is the strongest visible motivation
- the bundled customization mechanism is the only current boilerplate-light
  mechanism that keeps applicative primitives and derived operations coherent
- separating them invites room-design of weaker trait-only or `tag_invoke`-only
  alternatives

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

### Family D: Recursive tree algorithms

Core question:
What reusable recursive algorithms should C++ offer over recursive,
variant-based node structures?

Possible shapes:

- recursive fold/rebuild operations over recursive nodes
- fixpoint-tree-oriented design work as motivating evidence
- an algorithms paper rather than a concrete container paper

This family should currently be treated as an algorithms paper, not as a
“one true tree” vocabulary proposal.

## Fourth Layer: Candidate Paper Sequence

The current best decomposition is a stack of papers where each one pays for
itself.
The later papers benefit from earlier ones, but none should require the entire
stack to be accepted at once.

That said, the working publication plan is now to release the papers as a
coordinated set.
So the sequence below is logical rather than chronological.
It describes roles and boundaries, not necessarily submission order.

### Paper A: Traversal, transposition, and bundled customization

Working purpose:
be the anchor paper for the whole effort.

Why this is currently the strongest anchor:

- it addresses a live problem over familiar or standard-adjacent components
- it explains why the rest of the design exists
- it does not require prior sympathy for trees or recursive representations
- it provides the clearest public-facing “why now?”
- it avoids letting LEWG invent a weaker customization model in the room

Working scope:

- shape-preserving traversal
- transposition of structure and context
- applicative-style independent contextual composition as the semantic basis
- bundled customization objects as the enabling mechanism
- user-facing API vocabulary that does not require leading with category-theory
  names
- action-verb naming for lead APIs and motivation

Representative motivating examples:

- `vector<optional<T>> -> optional<vector<T>>`
- `vector<sender<T>> <-> sender<vector<T>>`
- `vector<simd<T>> <-> simd<vector<T>>` as a strong explanatory analogy for
  zipped or lanewise independent composition

What this paper should not try to do:

- standardize the full algebraic hierarchy
- introduce tree containers as its primary reason to exist
- turn into a survey of every nearby abstraction family

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

- benefits from the merged traversal/customization paper for generic
  integration and vocabulary
- must still stand as a container paper even if the companion paper stalls

### Paper D: Recursive tree algorithms

Working purpose:
capture the recursive-tree side as an algorithms paper once the traversal and
adaptation story is more settled.

Likely scope:

- recursive fold/rebuild operations over recursive structure
- variant-based node examples and fixpoint-tree evidence
- generic recursive algorithms rather than one blessed tree vocabulary type

This is currently the least fixed slot, but its intended framing should now be
considered algorithmic rather than representational.

## Fifth Layer: Paper Boundaries and Managed Overlap

Because the papers are now intended to ship together, overlap is not only
acceptable but necessary.
The key is to make the overlap disciplined.

### Merged anchor paper vs companion papers

The strongest overlap used to be between the traversal problem statement and
the extension-point mechanism discussion.
That overlap should now be internalized into the merged anchor paper.

More concretely:

- the merged anchor paper answers both why traversal/transposition matters and
  why bundled customization is needed to express it coherently
- the companion papers can rely on that result without restating the entire
  mechanism debate

### Shared background that may appear across papers

Some material will naturally appear across papers in abbreviated form:

- the distinction between abstract adapter vocabulary and domain-specific names
- the primitive/derived split
- the value of overrideable defaults
- examples showing structure/context transposition

This shared material should be concise and tailored to each paper's role.
It should not try to make either paper completely self-sufficient on all
technical detail.

### Material that should stay mostly in the merged anchor paper

- why transposition of structure and context is a real standard-library problem
- sender, optional, and SIMD-flavored motivation
- user-facing vocabulary and semantics of traversal/transpose operations
- the public-facing case that this solves something useful today
- why traits-only or one-CPO-per-operation designs are insufficient for
  applicative coherence
- the minimal amount of bundled-customization machinery needed to make the API
  credible

### Material that should stay mostly in the container paper

- semantic contract of the persistent measured sequence
- complexity guarantees
- iterator/container compliance
- measured split/search operations
- relation to other standard containers and views

### Material that should stay mostly in the recursive-algorithms paper

- recursive-structure motivation
- fixpoint evidence and recursive-node design issues
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

This is especially important for the anchor paper and its companion container
and recursive-algorithm papers.

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

Assign `P3200` to the merged traversal/customization paper.

Reasoning:

- it has the clearest “problem exists today” story
- it can stand on current standard and standard-adjacent components
- it explains why bundled customization is part of the solution rather than an
  optional afterthought
- it gives the coordinated set a public-facing center that is not dependent on
  immediate sympathy for tree containers

### Why not assign P3200 to the container paper?

Because the container paper is currently less universal in its motivation than
the traversal paper.
It may yet become a major paper in the set, but it is not the best anchor for
explaining the whole coordinated release.

### Why not assign P3200 to the recursive-algorithms paper?

Because the recursive-algorithms paper is still the least fixed in scope and
the most likely to evolve substantially before publication.

## Ninth Layer: More Detailed Per-Paper Plans

### Paper A plan: traversal, transposition, and bundled customization

Primary claim:
C++ needs a standard way to express shape-preserving traversal and
transposition between a structure and a computational context, and it needs a
bundled customization mechanism strong enough to keep the primitive and derived
operations coherent.

Essential sections:

1. Problem statement grounded in current components.
2. Examples showing independent contextual composition.
3. User-facing operations: traverse, transpose, and lifted contextual
   application.
4. Why traits-only and one-CPO-per-operation customization are not strong
  enough.
5. Bundled customization objects and primitive/derived coherence.
6. Semantics: independence vs sequencing.
7. Why the abstraction is broader than any one domain.

Success criterion:
reviewers can care about the problem and accept that the mechanism belongs in
the same paper because it solves a real coherence failure rather than because
it is theoretically elegant.

Review-control guidance for this paper:

- classify the public-facing naming choices as either necessary terminology
  choices or deferred naming refinements
- state explicitly which contextual examples are normative motivation and which
  are explanatory analogies
- explain explicitly why bundled customization is part of the solution and not
  a companion afterthought
- make the abstract and early before/after material immediately answer two
  questions: what real problem exists today, and what kind of operation is
  being proposed to solve it

Public naming guidance for this paper:

- prefer action verbs in the front-door API and motivation
- use `transpose` rather than `sequence` for structure/context flipping
- avoid “box” as the leading noun for contextual result types
- treat category-theory nouns as specification/rationale vocabulary, not as the
  lead teaching surface

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

### Paper D plan: recursive tree algorithms

Primary claim:
recursive trees expose generic structure and traversal problems not naturally
captured by sequence-centric iterator models alone, and C++ would benefit more
from reusable recursive algorithms than from prematurely standardizing one
blessed tree vocabulary type.

Essential sections:

1. Why recursive structure deserves direct treatment.
2. Recursive algorithm family: reduce/build/fold/rebuild style operations.
3. Relationship to internal traversal and reconstruction.
4. Adaptation and traversal integration where useful.
5. Examples from fixpoint and immutable tree representations.

Success criterion:
the paper clarifies the recursive-tree side of the design as an algorithms
proposal without requiring agreement on one canonical tree layout.

Review-control guidance for this paper:

- distinguish sharply between the proposed recursive algorithms, the motivating
  node representations, and any non-normative vocabulary examples
- call out where the current revision is using a pragmatic representational
  example rather than claiming a uniquely best tree encoding
- identify what is intentionally not being solved in first-wave scope
- make the abstract and early before/after material explain why recursive tree
  structure is the problem space, rather than assuming readers already share
  that premise

## Fifth Layer: Near-Term Decisions

Several decisions still control how the paper sequence narrows.

### Decision 1: Is the merged traversal/customization paper the anchor paper?

Two plausible openings exist:

- lead with the mechanism first and hope the client justifies it
- lead with traversal/transposition as the visible problem and package the
  mechanism with it as the enabling solution

Current bias: the merged traversal/customization paper should be the anchor.

### Decision 2: Is the sequence paper called “finger tree” or something else?

Current bias: prefer capability-oriented naming over implementation-oriented
naming.

Examples of the likely shape:

- persistent measured sequence
- concatenable persistent sequence
- split/search sequence with accumulated measure

Finger trees remain the key technical lineage, but not necessarily the paper
title.

### Decision 3: Is fixpoint tree a major example or just internal design support?

This is not yet settled.
Fixpoint trees clearly matter for the design story.
It is less clear that they should be anything more than a major example for the
recursive-algorithms paper.

### Decision 4: How much algebraic naming appears in titles and lead sections?

Current bias:

- keep algebraic names in the technical/core discussion
- prefer C++-native problem statements and user-facing terminology in titles,
  abstracts, and early motivation sections

## Bottom of the Triangle: Concrete Paper Candidates

These are the concrete candidates the broader plan currently narrows toward.
They are intentionally provisional.

### Candidate A: Traversal/customization anchor paper

Possible title direction:

- Shape-Preserving Traversal and Transposition for Contextual Computations
- Transposing Structures and Computational Contexts
- Traversal and Bundled Customization for Contextual Values

Role:
public anchor paper for the coordinated set, and current recommended home for
`P3200`.

### Candidate C: Sequence/container paper

Possible title direction:

- A Persistent Measured Sequence for the Standard Library
- Concatenable Persistent Sequences with Prefix Search and Split

Role:
container proposal grounded in finger-tree capabilities.

### Candidate D: Recursive algorithms paper

Possible title direction:

- Recursive Tree Algorithms for the Standard Library
- Recursive Fold and Rebuild Algorithms for Variant-Based Trees
- Structure-Directed Recursive Algorithms in C++

Role:
currently the least fixed, but now intended as an algorithms paper rather than
as a vocabulary-tree proposal.

## Tenth Layer: Current Paper Sketches

These are not draft papers.
They are working sketches for the current conception of each paper:

- tentative title
- abstract shape
- what the leading Before / After table must accomplish

They should be revised as the proposal set sharpens, but they are meant to be
concrete enough to guide drafting.

### Paper A sketch: traversal/customization anchor paper

#### Tentative title

Shape-Preserving Traversal and Transposition for Contextual Computations

Alternative title directions:

- Transposing Structures and Computational Contexts
- Traversal and Bundled Customization for Contextual Values

#### Abstract sketch

C++ has vocabulary types and computational contexts that are individually well
understood, but it lacks a uniform way to traverse a structure while producing
contextual results and then transpose the result into a single outer context.
This problem appears today in combinations such as containers of optionals,
containers of senders, and lanewise or zipped structured computation.

This paper proposes a shape-preserving traversal facility together with
transpose operations that convert a structure of contextual values into a
contextual structure.
The proposal is based on independent contextual composition rather than on
general sequential dependence, which makes it suitable for effect aggregation,
batching, and transposition-style algorithms.

The paper also proposes a bundled customization model for these operations.
That mechanism is included because current trait-only and per-operation
customization techniques are not strong enough to bundle applicative primitives
and derived operations coherently.
The combined proposal focuses on the problem being solved, the user-facing
operation set, and the minimum customization machinery needed to make the
design reliable in C++.

#### Leading Before / After table should show

The table should make the problem visible immediately.
It should not begin with algebraic terminology.

The “Before” column should show:

- ad hoc loops for `vector<optional<T>>` or similar structures
- manual state threading or repeated early-exit logic
- bespoke code to convert `structure<context<T>>` into `context<structure<T>>`
- no common abstraction for shape-preserving contextual traversal
- no coherent way to bundle the required customization primitives and derived
  operations

The “After” column should show:

- one traversal operation over a familiar structure
- one transpose operation
- the same shape preserved on success
- the context moved to the outside in a single expression
- one bundled customization object rather than disconnected hooks

The table must answer, immediately:

- what concrete pain exists today
- what kind of operation is being proposed
- why this is more than just a helper for one type
- why the customization mechanism is part of the solution rather than an
  implementation footnote

#### Core claim

C++ needs a standard way to express shape-preserving traversal and transposition
of contextual values, and it needs a bundled customization mechanism to make
that facility coherent, because current practice is fragmented and current
customization styles do not reliably tie the primitive and derived operations
together.

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
- “Why is the customization mechanism in the same paper?”

#### What the implementation must show to be acceptable

Assume a Beman Project implementation exists and evolves with the paper.
The question is therefore not basic executability but acceptability and
evidence quality.

The implementation should demonstrate:

- one coherent user-facing API for traversal and transpose operations
- operation over more than one concrete structure, including at least one
  standard or standard-adjacent type
- operation over more than one context family, so the design does not look
  single-domain
- that shape preservation is a semantic invariant, not an accidental property
- that the API is teachable without first teaching the whole algebraic
  hierarchy
- that the bundled customization object really secures primitive/derived
  coherence better than traits-only or one-CPO-per-operation alternatives
- that the examples in the paper are actually representative of the design, not
  hand-picked one-offs

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
Today, standard containers primarily assume destructive update: mutation
invalidates existing views of the value unless the user pays with copying,
coordination, or bespoke sharing schemes.
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

The table should foreground the missing combination of capabilities and the
pain of destructive update.

The “Before” column should show:

- `vector`, `deque`, list-like containers, rope-like ad hoc structures, or
  paired container compositions each covering only part of the problem
- destructive updates that invalidate existing views of the value, or deep
  copies where persistence would be valuable
- no standard abstraction combining concatenation with prefix split/search

The “After” column should show:

- one persistent sequence abstraction
- explicit support for concatenation and measured split/search
- structural sharing rather than forced mutation or deep copying
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

### Paper D sketch: recursive tree algorithms paper

#### Tentative title

Recursive Tree Algorithms for the Standard Library

Alternative title directions:

- Recursive Fold and Rebuild Algorithms for Variant-Based Trees
- Structure-Directed Recursive Algorithms in C++
- Reduce/Build Algorithms for Recursive Nodes

#### Abstract sketch

Recursive tree structures are common in parsing, transformation pipelines,
symbolic representations, and hierarchical data processing, but the standard
library offers little direct support for generic algorithms centered on
recursive structure rather than on flat external iteration.

This paper proposes a focused family of recursive algorithms over recursive,
variant-based node structures.
The unifying theme is that recursive trees are more naturally served by
structure-directed traversal and reconstruction than by cursor-oriented generic
interfaces alone.

The paper is intended to make the recursive-tree side of the design space
reviewable on its own without first standardizing one canonical tree layout.
It can draw on companion work for traversal vocabulary or adaptation patterns,
but its primary goal is to make the recursive problem space explicit and
concrete through reusable verbs.

#### Leading Before / After table should show

The table should justify why recursive trees deserve their own treatment.

The “Before” column should show:

- recursive structures handled through bespoke one-off algorithms
- iterator-centric interfaces that expose traversal mechanics rather than tree
  structure
- limited reuse across different recursive tree representations

The “After” column should show:

- one reusable algorithm family over recursive trees
- structure-directed traversal or reconstruction
- a generic story that works across more than one tree representation

The table must answer, immediately:

- why recursive trees are the problem space
- what generic capability is missing today
- that the proposal is primarily an algorithm family, not a canonical tree type

#### Core claim

Recursive trees are an important enough structural family that C++ should offer
reusable recursive algorithms rather than leaving every recursive-tree design to
bespoke local folds and rebuild passes.

#### Evidence and examples needed before drafting starts

- clear scoping decision: algorithms first
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

- “Why does the standard library need recursive algorithms here at all?”
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
- current best assignment is the merged traversal/customization anchor paper

## Summary

The current plan narrows as follows:

1. Broad thesis: C++ needs a complementary generic style for recursive,
   persistent, and effect-aware structures.
2. Architectural claims: explicit adaptation objects, split naming layers,
   traversal as the strongest user story, measured persistent sequences as a
   plausible library target.
3. Proposal families: merged traversal/customization, persistent sequence, and
  recursive algorithms.
4. Coordinated paper set: merged anchor paper, sequence/container paper, and
  recursive-algorithm paper.
5. Current working assignment: the merged traversal/customization paper is the
  best public anchor and the best current home for the reserved `P3200` slot.

The correct move is therefore not to split the traversal problem from the only
coherent mechanism currently available to express it.
It is to keep the merged anchor paper tight, while letting the persistent
sequence paper and recursive-algorithm paper each keep one clear reason to
exist.