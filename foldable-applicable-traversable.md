- [Algorithms for Trees](#orgd3bce4e)
  - [Abstract](#orgccfa834)
  - [Foldable](#org683f1a6)
  - [Applicative](#orgac0c04b)
  - [Traversable](#orgabd68b1)
  - [Not Monadic](#orge35010d)
- [Ranges Flatten the World](#org37339f7)
    - [Linearization as a Design Assumption](#orga9eb4d0)
    - [Where Structure Carries Meaning](#orgeb8eafd)
    - [Trees That Are Not Sequences](#org3a39270)
- [Visitors, Pattern Matching, and the Missing Syntax](#org1b0a180)
    - [Visitor as Manual Recursion Control](#orgd4ea5a5)
    - [Pattern Matching as the Intended Interface](#orgf933d16)
    - [Designing Today for Tomorrow's Syntax](#org36e2bf9)
- [Recursion Schemes You Can Actually Use](#orgb19739c)
    - [F-Algebras: How to Collapse One Layer](#orgb1d65ef)
    - [Catamorphisms as a Principled Fold](#org3f5f166)
    - [Separating Recursion from Business Logic](#orge073e5f)
- [The Typeclass Object Pattern](#orgb492dea)
    - [Typeclass Lookup: One Object Per Concept](#org0891070)
    - [Three Lookup Modes](#org3eb092b)
    - [NTTP Pinning Example](#orgd6054df)
    - [Implementors: One Hook, Many Derived Operations](#orgd1aa92f)
    - [CRTP and Deducing This](#orgce2c464)
- [Functor: The Foundation](#orgcfaab6f)
    - [Functor Interface](#org19cd7d2)
    - [Functor: Derived replace](#org0a6f388)
    - [Functor Identity Law in Tests](#org58fd64d)
- [Preserving Shape: Traversable and Friends](#org31339e1)
    - [Foldable vs. Traversable: Sequence vs. Shape](#org704877d)
    - [Crisp Contrast: Flatten vs. Preserve Shape](#orgc0963f8)
    - [Same Algorithm, Two Tree Representations](#org4cee177)
- [Foldable](#org7c67419)
    - [Monoid: The Glue fold\_map Needs](#orgcea5e8a)
    - [Foldable API: One Hook, Derived from fold\_map](#org45f7ff6)
    - [Foldable Proof: Tests as Examples](#org13232e0)
- [Applicative](#orgc383fce)
    - [Applicative Model: Pure Function over Effectful Arguments](#org898cb53)
    - [Applicative in Use](#orgaf2deea)
    - [Applicative: invoke in Tests](#org73bb43e)
    - [Applicative: Short-Circuit on Absent](#org46d276b)
    - [Applicative: invoke via Terminating Partial Application](#org71e21b7)
    - [Applicative Law: Interchange](#orgeb46ad2)
    - [Applicative Law: Composition](#orgae89d31)
- [Traversable](#org0f2cca9)
    - [Traversable Model: Commute Shape and Effect](#org2e8da61)
    - [Traversable in Use: Success](#org76103dd)
    - [Traversable in Use: Failure Propagates](#orgc259923)
    - [Traversable API: sequence](#org88d3729)
    - [Traversable Proof: sequence in Tests](#org2b7479b)
    - [Traversable Law: Naturality](#org1195886)
    - [Traversable Commute: Range and ZipList](#orge98de12)
    - [Laws That Keep This Honest](#org2e37194)
- [Monoids and Measured Trees](#org09e450c)
    - [Monoid Interface](#org12a776d)
    - [Monoid: Count Specialization](#org6f0d480)
    - [Monoid: Generic Helpers](#org7ebb3cb)
    - [Monoid in Tests](#org232f234)
    - [Monoid Identity Law in Tests](#orga263379)
    - [Associativity as Algorithmic Leverage](#orged103e2)
    - [Annotations as Summaries](#orgd25e477)
    - [Search and Split Driven by Measures](#orge5aadb1)
- [Finger Trees as a Case Study](#orgc2060c1)
    - [Persistent Concatenation and Splitting](#org1ae14e7)
    - [One Structure, Many Interpretations](#org1a5098f)
    - [Why This Belongs in Modern C++](#org318b6b9)
- [Designing APIs That Won't Age Poorly](#orgd307f52)
    - [Library Abstractions Anticipating Language Features](#orga2271c8)
    - [Avoiding the `std::bind` vs. Lambda Overlap](#org0adac7f)
    - [Keeping the Good Path Obvious](#orga3f7e56)



<a id="orgd3bce4e"></a>

# Algorithms for Trees

-   Foldable.
-   Applicative.
-   Traversable.


<a id="orgccfa834"></a>

## Abstract

-   Functor and monad patterns have proven themselves in practice; Foldable, Applicative, and Traversable are the next step.
-   Trees and structured data can lose important information when flattened to a sequence.
-   Monoid underpins efficient tree algorithms; the three typeclasses compose on top of it.
-   This talk illustrates one approach to designing a standard library `fingertree` API.


<a id="org683f1a6"></a>

## Foldable

-   Opt-in hook: `fold_map` — provides the algorithmic power of `std::ranges`.
-   Decouples algorithm from representation.
-   No flattening required.


<a id="orgac0c04b"></a>

## Applicative

-   Apply a pure function to independent effectful arguments.
-   Two hooks: `pure` (lift) and `apply` (sequence). `invoke` is the C++ user API.
-   Less sequencing machinery than monad for independent effects.


<a id="orgabd68b1"></a>

## Traversable

-   Generalizes Foldable: maps with effects while rebuilding the container shape.
-   A tree stays a tree; a fold can only produce a flat result.
-   Commutes containers: a range of effects becomes an effect of a range.


<a id="orge35010d"></a>

## Not Monadic

-   This talk stops short of Monad deliberately.
-   Monad adds sequencing and dependency between effects; most tree operations do not need this overhead.
-   Applicative covers the independent-effect cases where the whole structure is known upfront.


<a id="org37339f7"></a>

# Ranges Flatten the World


<a id="orga9eb4d0"></a>

### Linearization as a Design Assumption

-   Ranges are a great default when the structure is inherently sequential.
-   Many generic algorithms quietly assume that flattening first is semantically neutral.
-   For trees, flattening throws away parent/child relationships and subtree boundaries.

<div class="notes" id="orgde25247">
<p>
This is the setup: flattening is a design choice, not a law of nature.
The talk is about recovering algorithms that preserve structure when structure matters.
</p>

</div>


<a id="orgeb8eafd"></a>

### Where Structure Carries Meaning

-   Search paths, balancing, and decomposition points are part of the meaning.
-   The same inorder sequence can come from many different trees.
-   If we flatten too early, we lose algorithmic leverage.

<div class="notes" id="org02de8b7">
<p>
The argument is practical: preserving shape enables better APIs for split/search/relabel.
</p>

</div>


<a id="org3a39270"></a>

### Trees That Are Not Sequences

-   Expression trees: hierarchy controls precedence and rewrite legality.
-   Syntax trees: children have roles, not just positions.
-   Measured trees: internal summaries define split/search interfaces and drive optimization.

<div class="notes" id="org097d178">
<p>
A range view is still useful, but it should be derived, not the primary model.
</p>

</div>


<a id="org1b0a180"></a>

# Visitors, Pattern Matching, and the Missing Syntax


<a id="orgd4ea5a5"></a>

### Visitor as Manual Recursion Control

-   Visitor centralizes recursion, but at the cost of ceremony and indirection.
-   Each new operation typically requires another visitor type or nested lambda structure.
-   The control flow is explicit, but often noisy.

<div class="notes" id="org080a644">
<p>
Visitor is not wrong; it is just too low-level for everyday algebraic operations.
</p>

</div>


<a id="orgf933d16"></a>

### Pattern Matching as the Intended Interface

-   Pattern matching expresses what cases exist directly.
-   C++ has active pattern-matching proposals, but no standardized feature yet.
-   Typeclass-style APIs can encode the same intent with today's language.

<div class="notes" id="org48309e6">
<p>
Design now so the API maps naturally to future language features.
</p>

</div>


<a id="org36e2bf9"></a>

### Designing Today for Tomorrow's Syntax

-   Keep recursion control in library algorithms, not business code.
-   Expose a small vocabulary: `fold_map`, `invoke`, `traverse`.
-   Make call sites read like intent, not machinery.

<div class="notes" id="org1598359">
<p>
The point is migration-friendly design, not speculative syntax tricks.
</p>

</div>


<a id="orgb19739c"></a>

# Recursion Schemes You Can Actually Use


<a id="orgb1d65ef"></a>

### F-Algebras: How to Collapse One Layer

-   Think of an algebra as consuming one layer and summarizing it.
-   The recursion pattern stays fixed while business logic changes.
-   This separation makes tree algorithms easier to reason about.

<div class="notes" id="org354bfe5">
<p>
I only need the intuition here, not full categorical development.
</p>

</div>


<a id="org3f5f166"></a>

### Catamorphisms as a Principled Fold

-   Catamorphism: apply the algebra recursively until the structure is collapsed.
-   In C++, this corresponds to a disciplined fold over a recursive representation.
-   This yields reuse without hardcoding each algorithm into the node type.

<div class="notes" id="org53e766e">
<p>
Foldable is the operational entry point for this in everyday code.
</p>

</div>

1.  Fixpoint Expression Tree: eval\_algebra and cata

    <div class="notes" id="org451f337">
    <p>
    eval_algebra consumes one fully-evaluated layer: constants return their value,
    binary nodes combine the already-folded children.  cata supplies the recursion.
    Separating the algebra from the recursion is the whole point.
    </p>
    
    </div>


<a id="orge073e5f"></a>

### Separating Recursion from Business Logic

-   Business logic should answer how to combine results, not how to recurse.
-   This yields smaller tests and more reusable algorithms.
-   It also creates a natural place to enforce laws.

<div class="notes" id="orgf96b26f">
<p>
When recursion is abstracted, law tests become executable documentation.
</p>

</div>


<a id="orgb492dea"></a>

# The Typeclass Object Pattern


<a id="org0891070"></a>

### Typeclass Lookup: One Object Per Concept

-   Each concept has a variable template: `foldable_typeclass<T>`, `applicative_typeclass<T>`, `traversable_typeclass<T>`.
-   The looked-up object provides all operations for that concept on `T`.
-   New types opt in by specializing the variable template — no inheritance required.
-   Instances are open-world: add one close to the type, not in a central registry.

<div class="notes" id="org3ab59ae">
<p>
This replaces concept maps from C++0x with a simpler, working mechanism.
</p>

</div>

1.  Specializing the Variable Template

    <div class="notes" id="orgdde5f42">
    <p>
    Three lines of opt-in. No registry, no inheritance, no base class modification.
    The specialization can live next to the type or in any adapter header.
    </p>
    
    </div>


<a id="org3eb092b"></a>

### Three Lookup Modes

-   Implicit: `const auto& f = smd::foldable_typeclass<Tree>;` then call `f.method(...)`
-   Explicit object argument: pass a custom instance directly — local policy override.
-   NTTP pinning: `template <const auto& F = foldable_typeclass<Tree>>` — lookup bound at instantiation.
-   All three produce the same dispatch; the choice is about stability and explicitness.

<div class="notes" id="org5646aa5">
<p>
NTTP pinning is demonstrated in conceptmap functor tests (testP, testP2).
It proves that a generic helper's lookup is stable even when callers pass different instances.
</p>

</div>


<a id="orgd6054df"></a>

### NTTP Pinning Example

<div class="notes" id="org5e7fe5d">
<p>
The FOLDABLE parameter defaults to the variable template lookup.
Callers can supply a custom instance to change behavior for a specific call site.
</p>

</div>


<a id="orgd1aa92f"></a>

### Implementors: One Hook, Many Derived Operations

-   Implement one minimal hook per concept; all derived operations are provided automatically.
-   Foldable: implement `fold_map` → counting, folding, predicates, collection.
-   Applicative: implement `pure` + `apply` → `invoke` and five more operations.
-   Traversable: implement `traverse` → `for_each`, `sequence`, and override variants.

<div class="notes" id="org8beefe9">
<p>
The implementor surface is small; the user-facing surface is rich.
</p>

</div>


<a id="orgce2c464"></a>

### CRTP and Deducing This

-   Each concept wrapper is a CRTP base (`Foldable<Impl>`, `Applicative<Impl>`, `Traversable<Impl>`).
-   `this auto&& self` (C++23 explicit object parameter) preserves value category and constness through all wrapper calls.
-   Derived operations call back into the Impl via `self`; overrides are detected by `requires`.
-   Dispatch stays fully static — no virtual calls, no type erasure.

<div class="notes" id="orga2acac3">
<p>
CRTP supplies structure; deducing this keeps wrappers generic without losing type information.
</p>

</div>


<a id="orgcfaab6f"></a>

# Functor: The Foundation


<a id="org19cd7d2"></a>

### Functor Interface

-   Minimal hook: `fmap(F, container)` — apply a pure function inside a context.
-   Derived: `replace(container, value)` — overwrite all elements with a constant.
-   Instances: `std::optional`, `beman::optional`, `std::vector`.
-   Lookup: `smd::functor_typeclass<std::optional<int>>`.

<div class="notes" id="orgcc135ad">
<p>
Functor is the base on which Applicative and Traversable are built.
</p>

</div>


<a id="org0a6f388"></a>

### Functor: Derived replace

<div class="notes" id="org907cc85">
<p>
replace is derived from fmap with a constant function — no extra instance work required.
</p>

</div>


<a id="org58fd64d"></a>

### Functor Identity Law in Tests

-   fmap(id, x) == x for every instance and every shape.

<div class="notes" id="org406942e">
<p>
Tests that encode laws document intent more durably than comments.
</p>

</div>

**Preservation**: This C++ encoding of Functor preserves the identity law in `functor.t.cpp`. Every instance (optional, beman::optional, vector) passes the law test; deviation signals a contract violation.


<a id="org31339e1"></a>

# Preserving Shape: Traversable and Friends


<a id="org704877d"></a>

### Foldable vs. Traversable: Sequence vs. Shape

-   Foldable consumes structure into a summary.
-   Traversable maps with effects while rebuilding the same outer shape.
-   For trees, this is the difference between counting nodes and relabeling them in place.

<div class="notes" id="org5aef040">
<p>
This section shows concrete code now, before the formal typeclass introductions.
The goal is to build intuition: Foldable = collapse to value, Traversable = transform in place.
Formal treatment of each typeclass follows in the next three sections.
</p>

</div>


<a id="orgc0963f8"></a>

### Crisp Contrast: Flatten vs. Preserve Shape

-   Two differently shaped trees can flatten to the same sequence under Foldable.
-   Traversable can map values and keep the original branching shape.

1.  Foldable Flattens and Loses Shape Identity

2.  Traversable Maps While Preserving Shape

    <div class="notes" id="org86bebed">
    <p>
    Foldable can collapse two different shapes to the same flat view.
    Traversable keeps the tree skeleton and only transforms payloads.
    </p>
    
    </div>


<a id="org4cee177"></a>

### Same Algorithm, Two Tree Representations

-   Fixpoint tree and shared\_ptr binary tree can share the same Foldable call shape.
-   The representation changes; the typeclass API and algorithm intent stay the same.

<div class="notes" id="org788c121">
<p>
The call site reads identically across all three representations.
This is the key payoff of typeclass lookup: the algorithm is written once against an interface, not once per type.
</p>

</div>

1.  Fixpoint Tree

    <div class="notes" id="org6636c4e">
    <p>
    Expr (Fix&lt;ExprF&gt;). length dispatches through foldable_typeclass&lt;Expr&gt;; counts leaf constants.
    </p>
    
    </div>

2.  shared\_ptr Binary Tree

    <div class="notes" id="org3f23f93">
    <p>
    BinaryTree&lt;int&gt;. Different type, different fold_map implementation — same call site.
    </p>
    
    </div>

3.  FringeTree (Simplified FingerTree)

    <div class="notes" id="org78e6390">
    <p>
    FringeTree: a variant-based tree (Empty | Leaf | Branch). Same API, third representation.
    </p>
    
    </div>

4.  FringeTree: Traversable Also Preserves Shape

    <div class="notes" id="org9531416">
    <p>
    The same FringeTree that folded to {1,2,3} under Foldable now maps values and comes back as a FringeTree.
    The variant structure (Empty | Leaf | Branch) is intact; only the leaf values changed.
    </p>
    
    </div>


<a id="org7c67419"></a>

# Foldable


<a id="orgcea5e8a"></a>

### Monoid: The Glue fold\_map Needs

-   `fold_map` maps each element to some type, then folds the results into one.
-   That result type must support two operations: a neutral starting value and an associative merge.
-   In other words: a Monoid. Counting uses `Count{0}` + addition. Collecting uses `vector{}` + append.

<div class="notes" id="orgd733e1e">
<p>
This is a brief primer so the fold_map code makes sense immediately.
Full treatment — specialization, law tests, measured trees, finger tree policies — is in "Monoids and Measured Trees".
</p>

</div>


<a id="org45f7ff6"></a>

### Foldable API: One Hook, Derived from fold\_map

-   Minimal hook: `fold_map(F, container)` — apply F to each element, combine results.
-   Derived: counting, folding left/right, collecting, predicates — all from one hook.
-   No traversal order is mandated; the instance chooses and must be consistent.

1.  fold\_map → length

2.  fold\_map → to\_vector

    <div class="notes" id="orgf8bef94">
    <p>
    Every derived operation is implemented by specializing what fold_map collects.
    fold_left and fold_right use a function-composition monoid internally.
    </p>
    
    </div>


<a id="org13232e0"></a>

### Foldable Proof: Tests as Examples

-   Derived operations are verified directly against concrete inputs.

<div class="notes" id="orgbd0c16b">
<p>
The test encodes a semantic claim: to_vector of {1,2,3} is exactly {1,2,3}.
That claim would catch a traversal-order regression.
</p>

</div>


<a id="orgc383fce"></a>

# Applicative


<a id="org898cb53"></a>

### Applicative Model: Pure Function over Effectful Arguments

-   Applicative captures applying a pure function to independent effectful arguments.
-   Minimal hooks: `pure` (lift a value) and `apply` (apply a contextual function).
-   User API: `invoke` — matches the mental model of `std::invoke` over effectful values.
-   Less sequencing machinery than monadic formulations for independent effects.

<div class="notes" id="org909be69">
<p>
McBride's "applicative style" paper is the primary reference (Conor McBride and Ross Paterson, 2008).
apply_pure is a teaching alias that retains FP bracket notation [| f a b c |] for Haskell audiences; invoke is the preferred C++ spelling.
</p>

</div>


<a id="orgaf2deea"></a>

### Applicative in Use

<div class="notes" id="org2cdf250">
<p>
Three independent optional arguments. If any is absent the whole computation short-circuits.
</p>

</div>


<a id="org73bb43e"></a>

### Applicative: invoke in Tests

<div class="notes" id="org154c21a">
<p>
invoke works the same at arity 2, 3, or more — no per-call-site plumbing.
</p>

</div>


<a id="org46d276b"></a>

### Applicative: Short-Circuit on Absent

<div class="notes" id="org69e6c60">
<p>
ax is present; ay is absent. invoke short-circuits: f is never called.
This is the core contract of optional-as-applicative.
</p>

</div>


<a id="org71e21b7"></a>

### Applicative: invoke via Terminating Partial Application

-   `invoke` is derived: `pure(partial(f))` lifts f; each `apply` peels off one contextual argument.
-   Implementations provide only `pure` + `apply`; `invoke` can be overridden for custom semantics.

<div class="notes" id="orgc9d7688">
<p>
make_terminating_partial wraps f; each call either invokes f if all args are present or returns a new partial.
This avoids std::bind complexity while handling arbitrary arity uniformly.
Also derived from pure + apply: map, lift, ap, zip_with, discard_first, discard_second, invoke_with.
</p>

</div>


<a id="orgeb46ad2"></a>

### Applicative Law: Interchange

-   Interchange: `ap(u, pure(y)) == ap(pure(λf. f(y)), u)`
-   Applying a contextual function to a pure value is symmetric.

<div class="notes" id="org75892ff">
<p>
The interchange law is the trickiest to build intuition for.
It constrains how pure values interact with contextual functions.
</p>

</div>


<a id="orgae89d31"></a>

### Applicative Law: Composition

-   Composition: `ap(invoke(compose, u, v), w) == ap(u, ap(v, w))`
-   Composing effectful functions then applying equals sequencing the applications.

<div class="notes" id="org4f300cc">
<p>
The composition law ensures that effectful function composition is associative.
w = 3, v doubles to 6, u adds 10: result is 16.
</p>

</div>

**Preservation**: This C++ encoding of Applicative preserves all four laws (identity, homomorphism, interchange, composition) via executable tests in `applicative.t.cpp`. The semantic claim that effectful function application is associative survives the encoding as a statically-verified contract.


<a id="org0f2cca9"></a>

# Traversable


<a id="org2e8da61"></a>

### Traversable Model: Commute Shape and Effect

-   Traversal commutes shape and effect: from a structure of effects to an effect of a structure.
-   This gives a generic path from many small checks to one checked result.
-   Traversable strictly generalizes Foldable: it can rebuild the container, not just collapse it.
-   Use this to model validation, partial relabeling, and structured transformations.


<a id="org76103dd"></a>

### Traversable in Use: Success

<div class="notes" id="orge67e114">
<p>
Every element transforms successfully. The optional wrapping is removed and a new range is returned.
</p>

</div>


<a id="orgc259923"></a>

### Traversable in Use: Failure Propagates

<div class="notes" id="org32bd513">
<p>
One absent result poisons the whole traversal. No partial range is returned.
This is the short-circuit behavior that distinguishes traverse from map.
</p>

</div>


<a id="org88d3729"></a>

### Traversable API: sequence

<div class="notes" id="org1939d8f">
<p>
sequence commutes a container of effects into an effect of a container.
The identity function here means "the effect IS the structure": traverse(id, t).
Also derived: for_each, which is traverse with its arguments flipped (container before function).
</p>

</div>


<a id="org2b7479b"></a>

### Traversable Proof: sequence in Tests

<div class="notes" id="org769f02a">
<p>
sequence converts Identity&lt;optional&lt;int&gt;&gt; into optional&lt;Identity&lt;int&gt;&gt;.
The shape is preserved; the effect wraps the whole result.
</p>

</div>


<a id="org1195886"></a>

### Traversable Law: Naturality

-   If you have a function that converts between applicatives and respects `pure` and `ap`, traversal commutes through it.
-   Concretely: converting `optional<B>` → `beman::optional<B>` after traversal gives the same result as composing the conversion into `f` before traversal.

<div class="notes" id="org4741a6c">
<p>
Formal law: for an applicative morphism φ (commutes with pure and ap), φ(traverse f t) == traverse (φ∘f) t.
to_beman is the morphism: converts std::optional&lt;Identity&lt;int&gt;&gt; to beman::optional&lt;Identity&lt;int&gt;&gt;.
f_returning_beman is f with its return type changed to beman::optional — equivalent to composing to_beman with f at the value level.
Both sides produce beman::optional&lt;Identity&lt;int&gt;&gt;{Identity{6}}.
</p>

</div>


<a id="orge98de12"></a>

### Traversable Commute: Range and ZipList

-   Traversable commutes a range of ZipLists into a ZipList of ranges.
-   The inverse matrix view (ZipList of vectors to vector of ZipLists) is also tested.

1.  Range of ZipLists → ZipList of Ranges

2.  ZipList of Vectors → Vector of ZipLists (matrix transpose)

    <div class="notes" id="orgb516454">
    <p>
    This helper (<code>to_vector_of_ziplists</code>) is hand-coded to illustrate the inverse transpose concept.
    The Traversable version is the range-of-ZipLists → ZipList-of-ranges test on the previous slide.
    Key law intuition: preserve shape and evaluation order discipline.
    </p>
    
    </div>


<a id="org2e37194"></a>

### Laws That Keep This Honest

-   Applicative: identity, homomorphism, interchange, composition — all automated.
-   Traversable: identity, naturality, composition — all automated.
-   Foldable: all derived operations exercised directly against `fold_map`.
-   If these fail, abstractions become accidental APIs rather than reliable interfaces.

<div class="notes" id="org2013492">
<p>
If these fail, abstractions become accidental APIs rather than reliable interfaces.
Note: tree applicative (applying a tree of functions to a tree of values) is a policy choice, not the core applicative story.
The core teaching value of Applicative is visible in optional, range, and ZipList examples.
</p>

</div>

**Preservation**: This C++ encoding of Traversable preserves identity and naturality laws in `traversable.t.cpp`. Composition is verified through range-of-ZipLists and ZipList-of-ranges commutation tests (matrix transpose). The semantic claim that traversal commutes shape and effect survives unbroken from theory to executable code.


<a id="org09e450c"></a>

# Monoids and Measured Trees


<a id="org12a776d"></a>

### Monoid Interface

-   The full contract: `identity()` (neutral element) and `combine(lhs, rhs)` (associative merge).
-   Associativity: `combine(combine(a,b),c) == combine(a,combine(b,c))` — regrouping changes nothing.
-   Left and right identity: `combine(identity(), x) == x == combine(x, identity())`.
-   Lookup via `monoid_v<T>`; extend by specializing `Monoid<T>`.


<a id="org6f0d480"></a>

### Monoid: Count Specialization

<div class="notes" id="orgfbf5ba0">
<p>
Count is the canonical monoid for counting elements.
identity is 0; combine is addition — the simplest possible monoid.
</p>

</div>


<a id="org7ebb3cb"></a>

### Monoid: Generic Helpers

<div class="notes" id="org7950f4b">
<p>
monoid_v&lt;T&gt; is the canonical lookup object; monoid_combine and monoid_identity are free-function helpers.
These are the call shapes used by fold_map and all derived Foldable operations.
</p>

</div>


<a id="org232f234"></a>

### Monoid in Tests

<div class="notes" id="org5ba2a9c">
<p>
monoid_combine dispatches through monoid_v&lt;Count&gt;. The test is mechanical, but it pins the specialization.
</p>

</div>


<a id="orga263379"></a>

### Monoid Identity Law in Tests

<div class="notes" id="org340ebfc">
<p>
The identity law is what makes identity() useful for initializing fold accumulators.
If this fails the Monoid is not a monoid.
</p>

</div>

**Preservation**: This C++ encoding of Monoid preserves identity and associativity laws in `monoid.t.cpp`. Every specialization (Count, std::string, std::vector) must satisfy both laws to qualify as a lawful measure. This unlocks the algorithmic guarantee: regrouping combine operations changes nothing.


<a id="orged103e2"></a>

### Associativity as Algorithmic Leverage

-   Associativity lets us regroup work without changing results.
-   Measured trees exploit this to maintain summaries incrementally.
-   This is the bridge from algebra to explicit performance contracts.

<div class="notes" id="orgfbaacb4">
<p>
If the measure is a monoid, split/search become compositional (Ralf Hinze and Ross Paterson, 2006).
</p>

</div>


<a id="orgd25e477"></a>

### Annotations as Summaries

-   Each node caches a measure of its subtree.
-   Measures are domain-specific: size, minimum priority, span, or cost.
-   Updating structure updates summaries locally.

<div class="notes" id="org3c05e5a">
<p>
The data structure stays the same while behavior changes with the monoid.
</p>

</div>


<a id="orge5aadb1"></a>

### Search and Split Driven by Measures

-   Each node carries a cumulative measure; split navigates by predicate on that measure.
-   Predicate-based core split navigates tree structure in O(log n).
-   `split_at_index()` uses the structural path for count-measure trees; non-count measures fall back to flatten/rebuild.

<div class="notes" id="orgb992012">
<p>
The original finger-tree paper promises amortized O(1) at the ends,
O(log(min(n,m))) concatenation, and O(log n) split/search (Ralf Hinze and Ross Paterson, 2006).
The predicate-based core split (<code>split()</code> / <code>split_at()</code>) navigates tree structure in O(log n).
<code>split_at_index()</code> uses a structural fast path when the measure is a count (UnitMeasure with size_t tag);
non-count measures fall back to flatten/rebuild to preserve index semantics.
Wrapper operations (random-access at/insert/erase/update, priority-queue pop) use structural split + concat and are O(log n).
</p>

</div>


<a id="orgc2060c1"></a>

# Finger Trees as a Case Study


<a id="org1ae14e7"></a>

### Persistent Concatenation and Splitting

-   Persistent, persistence-friendly concatenation in O(log(min(n,m))).
-   Predicate-based split navigates tree structure in O(log n); wrapper operations use split + concat.
-   The API composes naturally with foldable/traversable abstractions.

<div class="notes" id="org4d08ad4">
<p>
Concatenation uses Hinze-Paterson app3: O(log(min(n,m))).
Split navigates the tree structurally in O(log n) and returns left, pivot, right.
Wrapper operations (random-access, priority queue, rope) use split + concat directly.
</p>

</div>


<a id="org1a5098f"></a>

### One Structure, Many Interpretations

-   Change the monoid, change the interpretation.
-   The same implementation can model sequence, priority queue, or rope.
-   Reuse is semantic, not merely syntactic.

<div class="notes" id="orgc7508ea">
<p>
This is the strongest argument for measured trees in a standard library context.
</p>

</div>

1.  Sequence: O(log n) Random Access

    <div class="notes" id="org6d7da98">
    <p>
    Monoid: size. The measure at each node is the count of elements below it.
    push_front/push_back are O(1) amortized.
    at/insert/erase/update use structural split O(log n) + concat O(log(min(n,m))).
    </p>
    
    </div>

2.  Priority Queue: Min and Max in One Structure

    <div class="notes" id="org8c687fe">
    <p>
    One FingerTree with a combined PriorityTag measure tracking both min and max simultaneously.
    Monoid: (Min, Max) — a pair that combines by taking component-wise extrema.
    push is O(1) amortized; pop_min/pop_max use measure-guided split O(log n) + concat O(log(min(n,m))).
    </p>
    
    </div>

3.  Rope: Character Buffer with Efficient Editing

    <div class="notes" id="orgb957405">
    <p>
    Monoid: byte-length. The measure at each node is the byte count of its chunk subtree.
    insert, erase, and replace all navigate by cumulative byte offset using split.
    </p>
    
    </div>


<a id="org318b6b9"></a>

### Why This Belongs in Modern C++

-   Adding Traversable to an existing type requires no modification to the type itself — one specialization in a header.
-   The Rope, priority queue, and sequence expose the same Traversable interface, with separate per-type specializations in the current codebase.
-   The abstraction is a library choice today; it maps cleanly to pattern matching and richer generic facilities when those arrive.

<div class="notes" id="orgdd56aef">
<p>
This is the concrete payoff: the design composes correctly across independent extension points.
No monkey-patching, no reopening of classes, no central registry.
</p>

</div>

**Preservation**: All five finger tree specializations (raw, random-access, priority-queue, rope, interval-index) satisfy the Traversable laws and compose with Foldable, Monoid, and Applicative correctly. The semantic claim that shape preservation and measure-guided search are compositional survives from theory through C++ encoding to real test suites.


<a id="orgd307f52"></a>

# Designing APIs That Won't Age Poorly


<a id="orga2271c8"></a>

### Library Abstractions Anticipating Language Features

-   Favor explicit, composable operations over magical overload sets.
-   Keep extension points separate from core type definitions.
-   Make future language support a simplification, not a rewrite.

<div class="notes" id="org9f1451a">
<p>
Pattern matching and richer generic facilities should refine this API, not replace it.
</p>

</div>


<a id="org0adac7f"></a>

### Avoiding the `std::bind` vs. Lambda Overlap

-   Avoid parallel abstractions that solve the same use case differently.
-   Choose one clear good path per concept.
-   For Applicative, that path is `invoke`; `apply_pure` remains a teaching aid.

<div class="notes" id="orgfa04382">
<p>
The goal is reducing cognitive branching in generic code.
</p>

</div>


<a id="orga3f7e56"></a>

### Keeping the Good Path Obvious

-   Make lawful defaults easy and alternative policies explicit.
-   Keep naming consistent across concepts.
-   Back claims with executable law tests.

<div class="notes" id="org24e1685">
<p>
The best API docs in this space are tests that encode the laws.
</p>

</div>
