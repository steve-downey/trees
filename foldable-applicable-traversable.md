- [Algorithms for Trees](#org0f1dc21)
  - [Abstract](#orge3b34d1)
  - [Foldable](#org5098ab1)
  - [Applicative](#org0ee48b1)
  - [Traversable](#org2b00f25)
  - [Not Monadic](#orgff7f92a)
- [Ranges Flatten the World](#orgd421469)
    - [Linearization as a Design Assumption](#orge86e0af)
    - [Where Structure Carries Meaning](#org8a50a77)
    - [Trees That Are Not Sequences](#orgfe4aacf)
- [Visitors, Pattern Matching, and the Missing Syntax](#orgcd2711d)
    - [Visitor as Manual Recursion Control](#org34feee5)
    - [Pattern Matching as the Intended Interface](#org32c2b37)
    - [Designing Today for Tomorrow's Syntax](#org386f9ab)
- [Recursion Schemes You Can Actually Use](#org228009d)
    - [F-Algebras: How to Collapse One Layer](#org9912345)
    - [Catamorphisms as a Principled Fold](#orgcce76d6)
    - [Separating Recursion from Business Logic](#org1712d91)
- [The Typeclass Object Pattern](#orgb849d92)
    - [Typeclass Lookup: One Object Per Concept](#orge597202)
    - [Three Lookup Modes](#orgce8b425)
    - [NTTP Pinning Example](#org63af0fe)
    - [Implementors: One Hook, Many Derived Operations](#org1627a3f)
    - [CRTP and Deducing This](#org7728518)
- [Functor: The Foundation](#org8f45e0e)
    - [Functor Interface](#org26d6b38)
    - [Functor: Derived replace](#org1f26191)
    - [Functor Identity Law in Tests](#orgfd8675e)
- [Preserving Shape: Traversable and Friends](#org0b36703)
    - [Foldable vs. Traversable: Sequence vs. Shape](#org5753bcf)
    - [Crisp Contrast: Flatten vs. Preserve Shape](#org9fc266f)
    - [Same Algorithm, Two Tree Representations](#org655b69b)
- [Foldable](#orgd0b88df)
    - [Monoid: The Glue fold\_map Needs](#org65edcf5)
    - [Foldable API: One Hook, Derived from fold\_map](#orgad299f0)
    - [Foldable Proof: Tests as Examples](#org1774fa1)
- [Applicative](#org73bdc8b)
    - [Applicative Model: Pure Function over Effectful Arguments](#org79b3968)
    - [Applicative in Use](#org9887ac6)
    - [Applicative: invoke in Tests](#orga904722)
    - [Applicative: Short-Circuit on Absent](#org6d0f6dc)
    - [Applicative: invoke via Terminating Partial Application](#orgd7d7f48)
    - [Applicative Law: Interchange](#org8f5f6ad)
    - [Applicative Law: Composition](#org81853ae)
- [Traversable](#org2596761)
    - [Traversable Model: Commute Shape and Effect](#org9c27628)
    - [Traversable in Use: Success](#org9eba4e2)
    - [Traversable in Use: Failure Propagates](#org00f8db5)
    - [Traversable API: sequence](#org7c14975)
    - [Traversable Proof: sequence in Tests](#orgf533911)
    - [Traversable Law: Naturality](#org52dd5a4)
    - [Traversable Commute: Range and ZipList](#org96217e1)
    - [Laws That Keep This Honest](#orgc32e6d6)
- [Monoids and Measured Trees](#org54334ab)
    - [Monoid Interface](#org93c1274)
    - [Monoid: Count Specialization](#orge54659f)
    - [Monoid: Generic Helpers](#orge68d4e8)
    - [Monoid in Tests](#org762dd46)
    - [Monoid Identity Law in Tests](#org845879f)
    - [Associativity as Algorithmic Leverage](#org334d2c1)
    - [Annotations as Summaries](#orgfbd637c)
    - [Search and Split Driven by Measures](#orgb631d8d)
- [Finger Trees as a Case Study](#org79fc147)
    - [Persistent Concatenation and Splitting](#org5e2d240)
    - [One Structure, Many Interpretations](#orgc3f05b3)
    - [Why This Belongs in Modern C++](#orga34bfe0)
- [Designing APIs That Won't Age Poorly](#orgb13aa3c)
    - [Library Abstractions Anticipating Language Features](#orgdf86dc1)
    - [Avoiding the `std::bind` vs. Lambda Overlap](#org0d0cf85)
    - [Keeping the Good Path Obvious](#orgf9b1cb6)



<a id="org0f1dc21"></a>

# Algorithms for Trees

-   Foldable.
-   Applicative.
-   Traversable.


<a id="orge3b34d1"></a>

## Abstract

-   Functor and monad patterns are proven; Foldable, Applicative, and Traversable are the next step.
-   Trees and structured data lose too much information when flattened to a sequence.
-   Monoid underpins efficient tree algorithms; the three typeclasses compose on top of it.
-   This talk proposes what a standard library `fingertree` API should look like.


<a id="org5098ab1"></a>

## Foldable

-   Opt-in hook: `fold_map` — provides the algorithmic power of `std::ranges`.
-   Decouples algorithm from representation.
-   No flattening required.


<a id="org0ee48b1"></a>

## Applicative

-   Apply a pure function to independent effectful arguments.
-   Two hooks: `pure` (lift) and `apply` (sequence). `invoke` is the C++ user API.
-   Less sequencing machinery than monad for independent effects.


<a id="org2b00f25"></a>

## Traversable

-   Generalizes Foldable: maps with effects while rebuilding the container shape.
-   A tree stays a tree; a fold can only produce a flat result.
-   Commutes containers: a range of effects becomes an effect of a range.


<a id="orgff7f92a"></a>

## Not Monadic

-   This talk stops short of Monad deliberately.
-   Monad adds sequencing and dependency between effects. Most tree operations do not need it.
-   Applicative covers the independent-effect cases where the whole structure is known up front.


<a id="orgd421469"></a>

# Ranges Flatten the World


<a id="orge86e0af"></a>

### Linearization as a Design Assumption

-   Ranges are a great default when the structure is inherently sequential.
-   Many generic algorithms quietly assume that flattening first is semantically neutral.
-   For trees, flattening throws away parent/child relationships and subtree boundaries.

<div class="notes" id="org9c55e57">
<p>
This is the setup: flattening is a design choice, not a law of nature.
The talk is about recovering algorithms that preserve structure when structure matters.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="org8a50a77"></a>

### Where Structure Carries Meaning

-   Search paths, balancing, and decomposition points are part of the meaning.
-   The same inorder sequence can come from many different trees.
-   If we flatten too early, we lose algorithmic leverage.

<div class="notes" id="org0f62a57">
<p>
The argument is practical: preserving shape enables better APIs for split/search/relabel.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="orgfe4aacf"></a>

### Trees That Are Not Sequences

-   Expression trees: hierarchy controls precedence and rewrite legality.
-   Syntax trees: children have roles, not just positions.
-   Measured trees: internal summaries define split/search interfaces and drive optimization.

<div class="notes" id="org37186a4">
<p>
A range view is still useful, but it should be derived, not the primary model.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="orgcd2711d"></a>

# Visitors, Pattern Matching, and the Missing Syntax


<a id="org34feee5"></a>

### Visitor as Manual Recursion Control

-   Visitor centralizes recursion, but at the cost of ceremony and indirection.
-   Every new operation requires another visitor type or lambda nest.
-   The control flow is explicit, but often noisy.

<div class="notes" id="orgaca1b83">
<p>
Visitor is not wrong; it is just too low-level for everyday algebraic operations.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="org32c2b37"></a>

### Pattern Matching as the Intended Interface

-   Pattern matching expresses what cases exist directly.
-   C++ is moving in this direction, but we still need practical libraries now.
-   Typeclass-style APIs can encode the same intent with today's language.

<div class="notes" id="orge9609e5">
<p>
Design now so the API maps naturally to future language features.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="org386f9ab"></a>

### Designing Today for Tomorrow's Syntax

-   Keep recursion control in library algorithms, not business code.
-   Expose a small vocabulary: `fold_map`, `invoke`, `traverse`.
-   Make call sites read like intent, not machinery.

<div class="notes" id="org9b825f1">
<p>
The point is migration-friendly design, not speculative syntax tricks.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="org228009d"></a>

# Recursion Schemes You Can Actually Use


<a id="org9912345"></a>

### F-Algebras: How to Collapse One Layer

-   Think of an algebra as consuming one layer and summarizing it.
-   The recursion pattern stays fixed while business logic changes.
-   This separation makes tree algorithms easier to reason about.

<div class="notes" id="org5aaee8c">
<p>
I only need the intuition here, not full categorical development.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="orgcce76d6"></a>

### Catamorphisms as a Principled Fold

-   Catamorphism: apply the algebra recursively until the structure is collapsed.
-   In C++, this corresponds to a disciplined fold over a recursive representation.
-   You get reuse without hardcoding each algorithm into the node type.

<div class="notes" id="org5eaf855">
<p>
Foldable is the operational entry point for this in everyday code.
Source: (Steve Downey, 2026).
</p>

</div>

1.  FixTree fold\_map: The Catamorphism

    ```cpp
    template <class F>
    auto fold_map(this auto&& self, F&& f, const smd::tree::FixTree<T>& t)
    {
        if (t.is_leaf()) {
            return std::invoke(f, t.value());
        }
    
        auto lhs = self.fold_map(f, t.left());
        auto rhs = self.fold_map(f, t.right());
    
        using Result = std::remove_cvref_t<decltype(lhs)>;
        return smd::typeclass::monoid_v<Result>.combine(lhs, rhs);
    }
    ```
    
    <div class="notes" id="org75320b0">
    <p>
    Recursion control lives in fold_map; business logic goes in f.
    monoid_v&lt;Result&gt; looks up the Monoid instance for whatever type f returns.
    combine merges two sub-results; the tree is never flattened.
    </p>
    
    </div>


<a id="org1712d91"></a>

### Separating Recursion from Business Logic

-   Business logic should answer how to combine results, not how to recurse.
-   This yields smaller tests and more reusable algorithms.
-   It also creates a natural place to enforce laws.

<div class="notes" id="orgc464461">
<p>
When recursion is abstracted, law tests become executable documentation.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="orgb849d92"></a>

# The Typeclass Object Pattern


<a id="orge597202"></a>

### Typeclass Lookup: One Object Per Concept

-   Each concept has a variable template: `foldable_typeclass<T>`, `applicative_typeclass<T>`, `traversable_typeclass<T>`.
-   The looked-up object provides all operations for that concept on `T`.
-   New types opt in by specializing the variable template — no inheritance required.
-   Instances are open-world: add one close to the type, not in a central registry.

<div class="notes" id="org84b45bc">
<p>
This replaces concept maps from C++0x with a simpler, working mechanism.
Source: (Steve Downey, 2026).
</p>

</div>

1.  Specializing the Variable Template

    ```cpp
    template <class T>
    inline constexpr auto foldable_typeclass<smd::tree::FixTree<T> > =
        FixTreeFoldableMap<T>{};
    ```
    
    <div class="notes" id="orgfcc5b0c">
    <p>
    Three lines of opt-in. No registry, no inheritance, no base class modification.
    The specialization can live next to the type or in any adapter header.
    </p>
    
    </div>


<a id="orgce8b425"></a>

### Three Lookup Modes

-   Implicit: `const auto& f = smd::foldable_typeclass<Tree>;` then call `f.method(...)`
-   Explicit object argument: pass a custom instance directly — local policy override.
-   NTTP pinning: `template <const auto& F = foldable_typeclass<Tree>>` — lookup bound at instantiation.
-   All three produce the same dispatch; the choice is about stability and explicitness.

<div class="notes" id="orgfeffe3e">
<p>
NTTP pinning is demonstrated in conceptmap functor tests (testP, testP2).
It proves that a generic helper's lookup is stable even when callers pass different instances.
</p>

</div>


<a id="org63af0fe"></a>

### NTTP Pinning Example

```cpp
template <class STRUCTURE,
          const auto& FOLDABLE = smd::foldable_typeclass<STRUCTURE> >
auto sum_with_nttp_lookup(const STRUCTURE& structure)
{
    return FOLDABLE.fold_map([](int x) { return x; }, structure);
}
```

<div class="notes" id="org8ef48b3">
<p>
The FOLDABLE parameter defaults to the variable template lookup.
Callers can supply a custom instance to change behavior for a specific call site.
</p>

</div>


<a id="org1627a3f"></a>

### Implementors: One Hook, Many Derived Operations

-   Implement one minimal hook per concept; all derived operations come for free.
-   Foldable: implement `fold_map` → counting, folding, predicates, collection.
-   Applicative: implement `pure` + `apply` → `invoke` and five more operations.
-   Traversable: implement `traverse` → `for_each`, `sequence`, and override variants.

<div class="notes" id="org1f2a63d">
<p>
The implementor surface is small; the user-facing surface is rich.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="org7728518"></a>

### CRTP and Deducing This

-   Each concept wrapper is a CRTP base (`Foldable<Impl>`, `Applicative<Impl>`, `Traversable<Impl>`).
-   `this auto&& self` preserves value category and constness through all wrapper calls.
-   Derived operations call back into the Impl via `self`; overrides are detected by `requires`.
-   Dispatch stays fully static — no virtual calls, no type erasure.

<div class="notes" id="org79f7778">
<p>
CRTP supplies structure; deducing this keeps wrappers generic without losing type information.
</p>

</div>


<a id="org8f45e0e"></a>

# Functor: The Foundation


<a id="org26d6b38"></a>

### Functor Interface

-   Minimal hook: `fmap(F, container)` — apply a pure function inside a context.
-   Derived: `replace(container, value)` — overwrite all elements with a constant.
-   Instances provided: `std::optional`, `beman::optional`, `std::vector`.
-   Lookup: `smd::functor_typeclass<std::optional<int>>`.

<div class="notes" id="orgfb2247a">
<p>
Functor is the base on which Applicative and Traversable are built.
</p>

</div>


<a id="org1f26191"></a>

### Functor: Derived replace

```cpp
template <class T, class U>
auto replace(this auto&& self, T&& value, U&& replacement)
{
    return self.fmap(
        [replacement = std::forward<U>(replacement)](const auto&) {
            return replacement;
        },
        std::forward<T>(value));
}
```

<div class="notes" id="orgc3dd60f">
<p>
replace is derived from fmap with a constant function — no extra instance work required.
</p>

</div>


<a id="orgfd8675e"></a>

### Functor Identity Law in Tests

-   fmap(id, x) == x for every instance and every shape.

```cpp
{
    const auto& functor = smd::functor_typeclass<std::optional<int> >;
    CHECK(functor.fmap(id, std::optional<int>{42}) == std::optional<int>{42});
    CHECK(functor.fmap(id, std::optional<int>{}) == std::optional<int>{});
}
```

<div class="notes" id="org42df77a">
<p>
Tests that encode laws document intent more durably than comments.
</p>

</div>


<a id="org0b36703"></a>

# Preserving Shape: Traversable and Friends


<a id="org5753bcf"></a>

### Foldable vs. Traversable: Sequence vs. Shape

-   Foldable consumes structure into a summary.
-   Traversable maps with effects while rebuilding the same outer shape.
-   For trees, this is the difference between counting nodes and relabeling them in place.

<div class="notes" id="org4c02cba">
<p>
This section shows concrete code now, before the formal typeclass introductions.
The goal is to build intuition: Foldable = collapse to value, Traversable = transform in place.
Formal treatment of each typeclass follows in the next three sections.
</p>

</div>


<a id="org9fc266f"></a>

### Crisp Contrast: Flatten vs. Preserve Shape

-   Two differently shaped trees can flatten to the same sequence under Foldable.
-   Traversable can map values and keep the original branching shape.

1.  Foldable Flattens and Loses Shape Identity

    ```cpp
    auto left_flat = foldable.to_vector(left_heavy);
    auto right_flat = foldable.to_vector(right_heavy);
    ```

2.  Traversable Maps While Preserving Shape

    ```cpp
    auto mapped = traversable.traverse(
        [](int x) -> optional<int> { return optional<int>{x + 10}; },
        tree);
    ```
    
    <div class="notes" id="org04e37cf">
    <p>
    Foldable can collapse two different shapes to the same flat view.
    Traversable keeps the tree skeleton and only transforms payloads.
    </p>
    
    </div>


<a id="org655b69b"></a>

### Same Algorithm, Two Tree Representations

-   Fixpoint tree and shared\_ptr binary tree can share the same Foldable call shape.
-   The representation changes; the typeclass API and algorithm intent stay the same.

<div class="notes" id="orgd54c05e">
<p>
The call site reads identically across all three representations.
This is the key payoff of typeclass lookup: the algorithm is written once against an interface, not once per type.
</p>

</div>

1.  Fixpoint Tree

    ```cpp
    auto n = foldable.length(tree);
    ```
    
    <div class="notes" id="orgba7950d">
    <p>
    FixTree&lt;int&gt;. fold_map and to_vector dispatch through foldable_typeclass&lt;FixTree&lt;int&gt;&gt;.
    </p>
    
    </div>

2.  shared\_ptr Binary Tree

    ```cpp
    auto n = foldable.length(tree);
    ```
    
    <div class="notes" id="org49fc244">
    <p>
    BinaryTree&lt;int&gt;. Different type, different fold_map implementation — same call site.
    </p>
    
    </div>

3.  FringeTree (Simplified FingerTree)

    ```cpp
    auto n = foldable.length(tree);
    ```
    
    <div class="notes" id="org527cde6">
    <p>
    FringeTree: a variant-based tree (Empty | Leaf | Branch). Same API, third representation.
    </p>
    
    </div>

4.  FringeTree: Traversable Also Preserves Shape

    ```cpp
    using beman::optional::optional;
    
    auto relabelled = traversable.traverse(
        [](int x) -> optional<int> {
            return x >= 0 ? optional<int>{x + 1} : optional<int>{};
        },
        tree);
    ```
    
    <div class="notes" id="org612825a">
    <p>
    The same FringeTree that folded to {1,2,3} under Foldable now maps values and comes back as a FringeTree.
    The variant structure (Empty | Leaf | Branch) is intact; only the leaf values changed.
    </p>
    
    </div>


<a id="orgd0b88df"></a>

# Foldable


<a id="org65edcf5"></a>

### Monoid: The Glue fold\_map Needs

-   `fold_map` maps each element to some type, then folds the results into one.
-   That result type must support two operations: a neutral starting value and an associative merge.
-   In other words: a Monoid. Counting uses `Count{0}` + addition. Collecting uses `vector{}` + append.

<div class="notes" id="org5605300">
<p>
This is a brief primer so the fold_map code makes sense immediately.
Full treatment — specialization, law tests, measured trees, finger tree policies — is in "Monoids and Measured Trees".
</p>

</div>


<a id="orgad299f0"></a>

### Foldable API: One Hook, Derived from fold\_map

-   Minimal hook: `fold_map(F, container)` — apply F to each element, combine results.
-   Derived: counting, folding left/right, collecting, predicates — all from one hook.
-   No traversal order is mandated; the instance chooses and must be consistent.

1.  fold\_map → length

    ```cpp
    template <class T>
    auto length(this auto&& self, T&& value) -> std::size_t
    {
      const auto count = self.fold_map(
        [](const auto&) { return typeclass::Count{1}; },
        std::forward<T>(value));
      return count.d_value;
    }
    ```

2.  fold\_map → to\_vector

    ```cpp
    template <class T>
    auto to_vector(this auto&& self, T&& value)
    {
      return self.fold_map(
        [](const auto& x) {
          using ValueType = remove_cvref_t<decltype(x)>;
          return std::vector<ValueType>{x};
        },
        std::forward<T>(value));
    }
    ```
    
    <div class="notes" id="org331ffe8">
    <p>
    Every derived operation is implemented by specializing what fold_map collects.
    fold_left and fold_right use a function-composition monoid internally.
    </p>
    
    </div>


<a id="org1774fa1"></a>

### Foldable Proof: Tests as Examples

-   Derived operations are verified directly against concrete inputs.

```cpp
using IntSequence = smd::typeclass::test::Sequence<int>;
auto sequence = IntSequence{{1, 2, 3}};
const auto& int_foldable = smd::foldable_typeclass<IntSequence>;

const auto as_vector = int_foldable.to_vector(sequence);
CHECK(as_vector == (std::vector<int>{1, 2, 3}));
```

<div class="notes" id="org7ddf0f7">
<p>
The test encodes a semantic claim: to_vector of {1,2,3} is exactly {1,2,3}.
That claim would catch a traversal-order regression.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="org73bdc8b"></a>

# Applicative


<a id="org79b3968"></a>

### Applicative Model: Pure Function over Effectful Arguments

-   Applicative captures applying a pure function to independent effectful arguments.
-   Minimal hooks: `pure` (lift a value) and `apply` (apply a contextual function).
-   User API: `invoke` — matches the mental model of `std::invoke` over effectful values.
-   Less sequencing machinery than monadic formulations for independent effects.

<div class="notes" id="orgbed7c57">
<p>
McBride's "applicative style" paper is the primary reference.
apply_pure is a teaching alias that retains FP bracket notation [| f a b c |] for Haskell audiences; invoke is the preferred C++ spelling.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="org9887ac6"></a>

### Applicative in Use

```cpp
auto sum = applicative.invoke(
    [](int a, int b, int c) { return a + b + c; },
    ax,
    ay,
    az);
```

<div class="notes" id="org7e0cd79">
<p>
Three independent optional arguments. If any is absent the whole computation short-circuits.
</p>

</div>


<a id="orga904722"></a>

### Applicative: invoke in Tests

```cpp
std::optional<int> ax{10};
std::optional<int> ay{5};
const auto& applicative = smd::applicative_typeclass<std::optional<int> >;

auto result = applicative.invoke([](int a, int b) { return a - b; }, ax, ay);
REQUIRE(result.has_value());
CHECK(*result == 5);
```

<div class="notes" id="org8269c59">
<p>
invoke works the same at arity 2, 3, or more — no per-call-site plumbing.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="org6d0f6dc"></a>

### Applicative: Short-Circuit on Absent

```cpp
std::optional<int> ax{1};
std::optional<int> ay{};
auto invoke_result = applicative.invoke([](int a, int b) { return a + b; }, ax, ay);
CHECK_FALSE(invoke_result.has_value());
```

<div class="notes" id="org6e9dc4f">
<p>
ax is present; ay is absent. invoke short-circuits: f is never called.
This is the core contract of optional-as-applicative.
</p>

</div>


<a id="orgd7d7f48"></a>

### Applicative: invoke via Terminating Partial Application

-   `invoke` is derived: `pure(partial(f))` lifts f; each `apply` peels off one contextual argument.
-   Implementations provide only `pure` + `apply`; `invoke` can be overridden for custom semantics.

```cpp
auto partial = smd::detail::make_terminating_partial(
    [](int a, int b, int c) { return a * 100 + b * 10 + c; });

auto partial2 = partial(1);
auto partial3 = partial2(2);
CHECK(partial3(3) == 123);
```

<div class="notes" id="org250501d">
<p>
make_terminating_partial wraps f; each call either invokes f if all args are present or returns a new partial.
This avoids std::bind complexity while handling arbitrary arity uniformly.
Also derived from pure + apply: map, lift, ap, zip_with, discard_first, discard_second, invoke_with.
</p>

</div>


<a id="org8f5f6ad"></a>

### Applicative Law: Interchange

-   Interchange: `ap(u, pure(y)) == ap(pure(λf. f(y)), u)`
-   Applying a contextual function to a pure value is symmetric.

```cpp
const auto& ap = smd::applicative_typeclass<std::optional<int> >;
std::optional<Fn> u{[](int x) { return x * 3; }};

auto lhs = ap.ap(u, ap.pure(y));
auto rhs = ap.ap(ap.pure([](const Fn& fn) { return fn(y); }), u);

REQUIRE(lhs.has_value());
CHECK(*lhs == 21);
CHECK(lhs == rhs);
```

<div class="notes" id="org89da614">
<p>
The interchange law is the trickiest to build intuition for.
It constrains how pure values interact with contextual functions.
</p>

</div>


<a id="org81853ae"></a>

### Applicative Law: Composition

-   Composition: `ap(invoke(compose, u, v), w) == ap(u, ap(v, w))`
-   Composing effectful functions then applying equals sequencing the applications.

```cpp
const auto& ap = smd::applicative_typeclass<std::optional<int> >;
std::optional<Fn> u{[](int x) { return x + 10; }};
std::optional<Fn> v{[](int x) { return x * 2; }};
std::optional<int> w{3};

auto lhs = ap.ap(ap.invoke(compose, u, v), w);
auto rhs = ap.ap(u, ap.ap(v, w));

REQUIRE(lhs.has_value());
CHECK(*lhs == 16);  // (3 * 2) + 10
CHECK(lhs == rhs);
```

<div class="notes" id="orgad2d263">
<p>
The composition law ensures that effectful function composition is associative.
w = 3, v doubles to 6, u adds 10: result is 16.
</p>

</div>


<a id="org2596761"></a>

# Traversable


<a id="org9c27628"></a>

### Traversable Model: Commute Shape and Effect

-   Traversal commutes shape and effect: from a structure of effects to an effect of a structure.
-   This gives a generic path from many small checks to one checked result.
-   Traversable strictly generalizes Foldable: it can rebuild the container, not just collapse it.
-   Use this to model validation, partial relabeling, and structured transformations.


<a id="org9eba4e2"></a>

### Traversable in Use: Success

```cpp
auto values = smd::ranges::from_vector(std::vector<int>{1, 2, 3});
const auto& traversable = smd::traversable_typeclass<decltype(values)>;

auto traversed = traversable.traverse(
    [](int value) -> std::optional<int> {
        return std::optional<int>{value + 1};
    },
    values);

REQUIRE(traversed.has_value());
CHECK(collect(*traversed) == (std::vector<int>{2, 3, 4}));
```

<div class="notes" id="orga42e57f">
<p>
Every element transforms successfully. The optional wrapping is removed and a new range is returned.
</p>

</div>


<a id="org00f8db5"></a>

### Traversable in Use: Failure Propagates

```cpp
auto values = smd::ranges::from_vector(std::vector<int>{1, -2, 3});
const auto& traversable = smd::traversable_typeclass<decltype(values)>;

auto traversed = traversable.traverse(
    [](int value) -> std::optional<int> {
        return value >= 0 ? std::optional<int>{value + 1}
                          : std::optional<int>{};
    },
    values);

CHECK_FALSE(traversed.has_value());
```

<div class="notes" id="orgc3bf1ec">
<p>
One absent result poisons the whole traversal. No partial range is returned.
This is the short-circuit behavior that distinguishes traverse from map.
</p>

</div>


<a id="org7c14975"></a>

### Traversable API: sequence

```cpp
template <class T>
auto sequence(this auto&& self, T&& value)
{
    return self.traverse(
        [](auto&& x) { return std::forward<decltype(x)>(x); },
        std::forward<T>(value));
}
```

<div class="notes" id="orgf8c4e88">
<p>
sequence commutes a container of effects into an effect of a container.
The identity function here means "the effect IS the structure": traverse(id, t).
Also derived: for_each, which is traverse with its arguments flipped (container before function).
</p>

</div>


<a id="orgf533911"></a>

### Traversable Proof: sequence in Tests

```cpp
using IdentityOpt = smd::typeclass::test::Identity<std::optional<int> >;
auto identity = IdentityOpt{std::optional<int>{1}};
const auto& traversable = smd::traversable_typeclass<IdentityOpt>;

auto sequenced = traversable.sequence(identity);
REQUIRE(sequenced.has_value());
CHECK(sequenced->value == 1);
```

<div class="notes" id="org1d1c70a">
<p>
sequence converts Identity&lt;optional&lt;int&gt;&gt; into optional&lt;Identity&lt;int&gt;&gt;.
The shape is preserved; the effect wraps the whole result.
</p>

</div>


<a id="org52dd5a4"></a>

### Traversable Law: Naturality

-   If you have a function that converts between applicatives and respects `pure` and `ap`, traversal commutes through it.
-   Concretely: converting `optional<B>` → `beman::optional<B>` after traversal gives the same result as composing the conversion into `f` before traversal.

```cpp
{
    auto value = Identity{3};
    CHECK(eta(traversable.traverse(f, value)) ==
          traversable.traverse(eta_f, value));
}
```

<div class="notes" id="org302c0ba">
<p>
Formal law: for applicative morphism η (commutes with pure and ap), η(traverse f t) == traverse (η∘f) t.
eta here converts optional&lt;Identity&lt;int&gt;&gt; to beman::optional&lt;Identity&lt;int&gt;&gt;.
eta_f is eta composed with f on the value side.
Both sides produce beman::optional&lt;Identity&lt;int&gt;&gt;{Identity{6}}.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="org96217e1"></a>

### Traversable Commute: Range and ZipList

-   Traversable commutes a range of ZipLists into a ZipList of ranges.
-   The inverse matrix view (ZipList of vectors to vector of ZipLists) is also tested.

1.  Range of ZipLists → ZipList of Ranges

    ```cpp
    auto sequenced = traversable.sequence(values);
    
    REQUIRE(sequenced.data.size() == 2U);
    CHECK(collect(sequenced.data[0]) == (std::vector<int>{1, 10, 100}));
    CHECK(collect(sequenced.data[1]) == (std::vector<int>{2, 20, 200}));
    ```

2.  ZipList of Vectors → Vector of ZipLists (matrix transpose)

    ```cpp
    smd::zip_list<std::vector<int> > zip_of_vectors{
        {{1, 10, 100}, {2, 20, 200}}};
    
    auto as_rows = to_vector_of_ziplists(zip_of_vectors);
    
    REQUIRE(as_rows.size() == 3U);
    CHECK(as_rows[0].data == (std::vector<int>{1, 2}));
    CHECK(as_rows[1].data == (std::vector<int>{10, 20}));
    CHECK(as_rows[2].data == (std::vector<int>{100, 200}));
    ```
    
    <div class="notes" id="org6ca41bb">
    <p>
    This helper (<code>to_vector_of_ziplists</code>) is hand-coded to illustrate the inverse transpose concept.
    The Traversable version is the range-of-ZipLists → ZipList-of-ranges test on the previous slide.
    Key law intuition: preserve shape and evaluation order discipline.
    Source: (Steve Downey, 2026).
    </p>
    
    </div>


<a id="orgc32e6d6"></a>

### Laws That Keep This Honest

-   Applicative: identity, homomorphism, interchange, composition — all automated.
-   Traversable: identity, naturality, composition — all automated.
-   Foldable: all derived operations exercised directly against `fold_map`.
-   If these fail, abstractions become accidental APIs rather than reliable interfaces.

<div class="notes" id="org71a4b68">
<p>
If these fail, abstractions become accidental APIs rather than reliable interfaces.
Note: tree applicative (applying a tree of functions to a tree of values) is a policy choice, not the core applicative story.
The core teaching value of Applicative is visible in optional, range, and ZipList examples.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="org54334ab"></a>

# Monoids and Measured Trees


<a id="org93c1274"></a>

### Monoid Interface

-   The full contract: `identity()` (neutral element) and `combine(lhs, rhs)` (associative merge).
-   Associativity: `combine(combine(a,b),c) == combine(a,combine(b,c))` — regrouping changes nothing.
-   Left and right identity: `combine(identity(), x) == x == combine(x, identity())`.
-   Lookup via `monoid_v<T>`; extend by specializing `Monoid<T>`.


<a id="orge54659f"></a>

### Monoid: Count Specialization

```cpp
template <>
struct Monoid<Count> {
    constexpr auto identity() const -> Count { return Count{0}; }

    constexpr auto combine(const Count& lhs, const Count& rhs) const -> Count
    {
        return Count{lhs.d_value + rhs.d_value};
    }
};
```

<div class="notes" id="org6e10e69">
<p>
Count is the canonical monoid for counting elements.
identity is 0; combine is addition — the simplest possible monoid.
</p>

</div>


<a id="orge68d4e8"></a>

### Monoid: Generic Helpers

```cpp
template <class VALUE_TYPE>
auto monoid_identity() -> VALUE_TYPE
{
    return typeclass::monoid_v<VALUE_TYPE>.identity();
}

template <class VALUE_TYPE>
auto monoid_combine(const VALUE_TYPE& lhs, const VALUE_TYPE& rhs) -> VALUE_TYPE
{
    return typeclass::monoid_v<VALUE_TYPE>.combine(lhs, rhs);
}
```

<div class="notes" id="org0511c19">
<p>
monoid_v&lt;T&gt; is the canonical lookup object; monoid_combine and monoid_identity are free-function helpers.
These are the call shapes used by fold_map and all derived Foldable operations.
</p>

</div>


<a id="org762dd46"></a>

### Monoid in Tests

```cpp
const smd::typeclass::Count one{1};
const smd::typeclass::Count two{2};

const auto result = smd::monoid_combine(one, two);
CHECK(result.d_value == 3U);
```

<div class="notes" id="org56df9d7">
<p>
monoid_combine dispatches through monoid_v&lt;Count&gt;. The test is mechanical, but it pins the specialization.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="org845879f"></a>

### Monoid Identity Law in Tests

```cpp
{
    const auto& m = smd::typeclass::monoid_v<int>;
    CHECK(m.combine(m.identity(), 42) == 42);
    CHECK(m.combine(42, m.identity()) == 42);
}
```

<div class="notes" id="org496be99">
<p>
The identity law is what makes identity() useful for initializing fold accumulators.
If this fails the Monoid is not a monoid.
</p>

</div>


<a id="org334d2c1"></a>

### Associativity as Algorithmic Leverage

-   Associativity lets us regroup work without changing results.
-   Measured trees exploit this to maintain summaries incrementally.
-   This is the bridge from algebra to explicit performance contracts.

<div class="notes" id="org7211070">
<p>
If the measure is a monoid, split/search become compositional.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="orgfbd637c"></a>

### Annotations as Summaries

-   Each node caches a measure of its subtree.
-   Measures are domain-specific: size, min priority, span, or cost.
-   Updating structure updates summaries locally.

<div class="notes" id="orgeffc441">
<p>
The data structure stays the same while behavior changes with the monoid.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="orgb631d8d"></a>

### Search and Split Driven by Measures

-   Design: each node carries a cumulative measure; split navigates by predicate on that measure.
-   Current status: correct with linear-time bounds — split scans the flattened sequence.
-   The API shape is already what O(log n) split would need; optimization does not change call sites.

<div class="notes" id="orgedb9eae">
<p>
The original finger-tree papers promise amortized O(1) at the ends,
O(log(min(n,m))) concatenation, and O(log n) split/search.
The current implementation keeps the same API shape but does not yet meet those bounds.
The point for the talk: the abstraction boundary is in the right place even before the optimization lands.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="org79fc147"></a>

# Finger Trees as a Case Study


<a id="org5e2d240"></a>

### Persistent Concatenation and Splitting

-   Current prototype provides efficient, persistence-friendly concatenation.
-   Current split/search paths are correct with linear-time upper bounds.
-   The API is designed so split/search can be optimized later without changing call sites.
-   The API naturally composes with foldable/traversable abstractions.

<div class="notes" id="org07bb4f3">
<p>
This is where abstractions meet implementation reality.
The paper-level target bounds remain the north star.
The current prototype contract is explicit linear split/search.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="orgc3f05b3"></a>

### One Structure, Many Interpretations

-   Change the monoid, change the interpretation.
-   Same implementation can model sequence, priority queue, or rope.
-   Reuse is semantic, not just syntactic.

<div class="notes" id="org30c8964">
<p>
This is the strongest argument for measured trees in a standard library context.
Source: (Steve Downey, 2026).
</p>

</div>

1.  Sequence: O(log n) Random Access

    ```cpp
    using Seq = smd::tree::FingerTreeRandomAccess<int>;
    
    auto seq = Seq::from_sequence({1, 2, 3});
    REQUIRE(seq.at(0).has_value());
    CHECK(*seq.at(0) == 1);
    CHECK_FALSE(seq.at(99).has_value());
    
    auto edited = seq.push_back(4).push_front(0).insert(2, 9).update(3, 7).erase(1);
    CHECK(edited.to_vector() == (std::vector<int>{0, 9, 7, 3, 4}));
    ```
    
    <div class="notes" id="org460e915">
    <p>
    Monoid: size. The measure at each node is the count of elements below it.
    push_front/push_back/insert/update/erase all work by measure-guided split and rejoin.
    </p>
    
    </div>

2.  Priority Queue: Min and Max in One Structure

    ```cpp
    auto q = Queue::from_values({5, 2, 8, 2, 7});
    REQUIRE(q.min().has_value());
    REQUIRE(q.max().has_value());
    CHECK(*q.min() == 2);
    CHECK(*q.max() == 8);
    ```
    
    <div class="notes" id="org611a8a9">
    <p>
    Two FingerTrees, one keyed by min, one by max. The same element lives in both.
    Monoid: (Min, Max) — a pair that combines by taking component-wise extrema.
    </p>
    
    </div>

3.  Rope: Character Buffer with Efficient Editing

    ```cpp
    auto rope = Rope::from_text("abCDxy", 2)
                    .insert(2, "--")
                    .erase(5, 2)
                    .replace(0, 2, "AB");
    
    CHECK(rope.to_string() == "AB--Cy");
    CHECK(rope.size_bytes() == 6U);
    ```
    
    <div class="notes" id="org005289e">
    <p>
    Monoid: byte-length. The measure at each node is the byte count of its chunk subtree.
    insert, erase, and replace all navigate by cumulative byte offset using split.
    </p>
    
    </div>


<a id="orga34bfe0"></a>

### Why This Belongs in Modern C++

-   Adding Traversable to an existing type requires no modification to the type itself — one specialization in a header.
-   The Rope, priority queue, and sequence share a Traversable implementation; no per-type code was written.
-   The abstraction is a library choice today; it maps cleanly to pattern matching and richer generic facilities when those arrive.

<div class="notes" id="org6600b2f">
<p>
This is the concrete payoff: the design composes correctly across independent extension points.
No monkey-patching, no reopening of classes, no central registry.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="orgb13aa3c"></a>

# Designing APIs That Won't Age Poorly


<a id="orgdf86dc1"></a>

### Library Abstractions Anticipating Language Features

-   Favor explicit, composable operations over magical overload sets.
-   Keep extension points separate from core type definitions.
-   Make future language support a simplification, not a rewrite.

<div class="notes" id="org2e8bfac">
<p>
Pattern matching and richer generic facilities should refine this API, not replace it.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="org0d0cf85"></a>

### Avoiding the `std::bind` vs. Lambda Overlap

-   Avoid parallel abstractions that solve the same use case differently.
-   Choose one clear good path per concept.
-   For Applicative, that path is `invoke`; `apply_pure` remains a teaching aid.

<div class="notes" id="orgd3b02f3">
<p>
The goal is reducing cognitive branching in generic code.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="orgf9b1cb6"></a>

### Keeping the Good Path Obvious

-   Make lawful defaults easy and alternative policies explicit.
-   Keep naming consistent across concepts.
-   Back claims with executable law tests.

<div class="notes" id="orgf3023fd">
<p>
The best API docs in this space are tests that encode the laws.
Source: (Steve Downey, 2026).
</p>

</div>
