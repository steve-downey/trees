- [Algorithms for Trees](#orgf8cfa4b)
  - [Abstract](#org596ae0c)
  - [Foldable](#org900ec11)
  - [Applicative](#org3f3b7ad)
  - [Traversable](#orgd12bf72)
  - [Not Monadic](#org03154f7)
- [Ranges Flatten the World](#orgb4180e2)
    - [Linearization as a Design Assumption](#org52cbe56)
    - [Where Structure Carries Meaning](#org8976385)
    - [Trees That Are Not Sequences](#org9fb6a31)
- [Visitors, Pattern Matching, and the Missing Syntax](#orgec16381)
    - [Visitor as Manual Recursion Control](#org1b6110f)
    - [Pattern Matching as the Intended Interface](#orgf948db1)
    - [Designing Today for Tomorrow's Syntax](#org1d5eb2c)
- [Recursion Schemes You Can Actually Use](#org7713148)
    - [F-Algebras: How to Collapse One Layer](#org3dbf071)
    - [Catamorphisms as a Principled Fold](#org5907ebe)
    - [Separating Recursion from Business Logic](#org227872d)
- [The Typeclass Object Pattern](#org1934b3f)
    - [Typeclass Lookup: One Object Per Concept](#orgad2d183)
    - [Three Lookup Modes](#orgb4dd1e3)
    - [NTTP Pinning Example](#orgb243f69)
    - [Implementors: One Hook, Many Derived Operations](#org2f728a0)
    - [CRTP and Deducing This](#org1a57c73)
- [Functor: The Foundation](#org855199a)
    - [Functor Interface](#org8b01b70)
    - [Functor: Derived replace](#org0a57d62)
    - [Functor Identity Law in Tests](#orgc05a457)
- [Preserving Shape: Traversable and Friends](#org850553c)
    - [Foldable vs. Traversable: Sequence vs. Shape](#org62c8095)
    - [Crisp Contrast: Flatten vs. Preserve Shape](#orgf5037e8)
    - [Same Algorithm, Two Tree Representations](#org7afacf4)
- [Foldable](#org66f5d1f)
    - [Foldable API: One Hook, Derived from fold\_map](#orga67e55c)
    - [Foldable Proof: Tests as Examples](#org5d53b29)
- [Applicative](#orgaab35ba)
    - [Applicative Model: Pure Function over Effectful Arguments](#orgebd615d)
    - [Applicative in Use](#org1e4b814)
    - [Applicative: invoke in Tests](#orgc103809)
    - [Applicative: invoke via Terminating Partial Application](#org138a09e)
    - [Applicative Law: Interchange](#org481e479)
    - [Applicative Law: Composition](#orga98e51b)
- [Traversable](#org12f4c9f)
    - [Traversable Model: Commute Shape and Effect](#orgc6636a4)
    - [Traversable API: for\_each](#orgad71fea)
    - [Traversable API: sequence](#org2d4dc05)
    - [Traversable Proof: sequence in Tests](#org34eca31)
    - [Traversable Law: Naturality](#org8a1b681)
    - [Traversable Naturality Law in Tests](#orgf030f80)
    - [Traversable Commute: Range and ZipList](#org93e187d)
    - [Laws That Keep This Honest](#orga021c7e)
    - [Tree Applicative as Optional Appendix](#orgc3a3ee7)
- [Monoids and Measured Trees](#org6854c87)
    - [Monoid Interface](#orgeda7f11)
    - [Monoid: Count Specialization](#orge3b4a02)
    - [Monoid: Generic Helpers](#org4f4e732)
    - [Monoid in Tests](#org5ac1188)
    - [Monoid Identity Law in Tests](#org60080ca)
    - [Associativity as Algorithmic Leverage](#orgaaac907)
    - [Annotations as Summaries](#org80c1113)
    - [Search and Split Driven by Measures](#orgea2dfac)
- [Finger Trees as a Case Study](#orgf24761a)
    - [Persistent Concatenation and Splitting](#orgd82c513)
    - [One Structure, Many Interpretations](#orgb916381)
    - [Why This Belongs in Modern C++](#org00f42d2)
- [Designing APIs That Won't Age Poorly](#org499f8f8)
    - [Library Abstractions Anticipating Language Features](#org85b0975)
    - [Avoiding the `std::bind` vs. Lambda Overlap](#org1bd0c2c)
    - [Keeping the Good Path Obvious](#org43401bd)



<a id="orgf8cfa4b"></a>

# Algorithms for Trees

-   Foldable.
-   Applicative.
-   Traversable.


<a id="org596ae0c"></a>

## Abstract

The functor and monad patterns have been broadly successful in ranges, sender-receiver, optional, and expected. Other typeclasses from functional programming have proven their value over the last decade and are ready for C++ adoption.

In particular, I am interested in better support for algorithms over trees and other data structures where flattening into a sequence loses too much information. This talk focuses on Foldable, Applicative, and Traversable, as well as Monoid, which underpins a number of efficient tree algorithms.

The eventual goal is to bring `fingertree` to the standard library, along with support for application-domain trees in use today, such as expression evaluators and syntax trees.


<a id="org900ec11"></a>

## Foldable

-   **Foldables:** are types that can be treated as a sequence or range and support a `fold_map` minimal hook, which provides much of the power of `std::ranges`. Providing opt-in hooks for making a type Foldable rather than a Range decouples the algorithm from the representation.


<a id="org3f3b7ad"></a>

## Applicative

-   **Applicatives:** model the pattern of applying a pure function to effectful arguments. The implementation details—partial application inside a container—are a distraction; what matters is the lawful interface. Applicatives are widely relevant for data-parallel operations and independent effects, and they require less sequencing machinery than monadic formulations.


<a id="orgd12bf72"></a>

## Traversable

-   **Traversables:** generalize Foldables by preserving the shape of a container rather than collapsing it to a sequence. A binary tree can be traversed while maintaining parent-child relationships, where a fold can produce at most a flat range. Traversable also provides the ability to commute containers generically—converting a range of tasks into a task that produces a range.


<a id="org03154f7"></a>

## Not Monadic

-   This talk stops short of Monad deliberately.
-   Monad adds sequencing and dependency between effects. Most tree operations do not need it.
-   Applicative covers the independent-effect cases where the whole structure is known up front.


<a id="orgb4180e2"></a>

# Ranges Flatten the World


<a id="org52cbe56"></a>

### Linearization as a Design Assumption

-   Ranges are a great default when the structure is inherently sequential.
-   Many generic algorithms quietly assume that flattening first is semantically neutral.
-   For trees, flattening throws away parent/child relationships and subtree boundaries.

<div class="notes" id="orga22158a">
<p>
This is the setup: flattening is a design choice, not a law of nature.
The talk is about recovering algorithms that preserve structure when structure matters.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="org8976385"></a>

### Where Structure Carries Meaning

-   Search paths, balancing, and decomposition points are part of the meaning.
-   The same inorder sequence can come from many different trees.
-   If we flatten too early, we lose algorithmic leverage.

<div class="notes" id="org7fb34fa">
<p>
The argument is practical: preserving shape enables better APIs for split/search/relabel.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="org9fb6a31"></a>

### Trees That Are Not Sequences

-   Expression trees: hierarchy controls precedence and rewrite legality.
-   Syntax trees: children have roles, not just positions.
-   Measured trees: internal summaries define split/search interfaces and drive optimization.

<div class="notes" id="orgd988879">
<p>
A range view is still useful, but it should be derived, not the primary model.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="orgec16381"></a>

# Visitors, Pattern Matching, and the Missing Syntax


<a id="org1b6110f"></a>

### Visitor as Manual Recursion Control

-   Visitor centralizes recursion, but at the cost of ceremony and indirection.
-   Every new operation requires another visitor type or lambda nest.
-   The control flow is explicit, but often noisy.

<div class="notes" id="org67335fc">
<p>
Visitor is not wrong; it is just too low-level for everyday algebraic operations.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="orgf948db1"></a>

### Pattern Matching as the Intended Interface

-   Pattern matching expresses what cases exist directly.
-   C++ is moving in this direction, but we still need practical libraries now.
-   Typeclass-style APIs can encode the same intent with today's language.

<div class="notes" id="org632f98c">
<p>
Design now so the API maps naturally to future language features.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="org1d5eb2c"></a>

### Designing Today for Tomorrow's Syntax

-   Keep recursion control in library algorithms, not business code.
-   Expose a small vocabulary: `fold_map`, `invoke`, `traverse`.
-   Make call sites read like intent, not machinery.

<div class="notes" id="org3ae88ba">
<p>
The point is migration-friendly design, not speculative syntax tricks.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="org7713148"></a>

# Recursion Schemes You Can Actually Use


<a id="org3dbf071"></a>

### F-Algebras: How to Collapse One Layer

-   Think of an algebra as consuming one layer and summarizing it.
-   The recursion pattern stays fixed while business logic changes.
-   This separation makes tree algorithms easier to reason about.

<div class="notes" id="orgd97e983">
<p>
I only need the intuition here, not full categorical development.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="org5907ebe"></a>

### Catamorphisms as a Principled Fold

-   Catamorphism: apply the algebra recursively until the structure is collapsed.
-   In C++, this corresponds to a disciplined fold over a recursive representation.
-   You get reuse without hardcoding each algorithm into the node type.

<div class="notes" id="orge3106f6">
<p>
Foldable is the operational entry point for this in everyday code.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="org227872d"></a>

### Separating Recursion from Business Logic

-   Business logic should answer how to combine results, not how to recurse.
-   This yields smaller tests and more reusable algorithms.
-   It also creates a natural place to enforce laws.

<div class="notes" id="org334120e">
<p>
When recursion is abstracted, law tests become executable documentation.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="org1934b3f"></a>

# The Typeclass Object Pattern


<a id="orgad2d183"></a>

### Typeclass Lookup: One Object Per Concept

-   Each concept has a variable template: `foldable_typeclass<T>`, `applicative_typeclass<T>`, `traversable_typeclass<T>`.
-   The looked-up object provides all operations for that concept on `T`.
-   New types opt in by specializing the variable template — no inheritance required.
-   Instances are open-world: add one close to the type, not in a central registry.

<div class="notes" id="orgdc2f475">
<p>
This replaces concept maps from C++0x with a simpler, working mechanism.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="orgb4dd1e3"></a>

### Three Lookup Modes

-   Implicit: `const auto& f = smd::foldable_typeclass<Tree>; f.method(...)`
-   Explicit object argument: pass a custom instance directly — local policy override.
-   NTTP pinning: bind the looked-up object as a template parameter at compile time.
-   All three produce the same dispatch; the choice is about stability and explicitness.

<div class="notes" id="orge403a31">
<p>
NTTP pinning is demonstrated in conceptmap functor tests (testP, testP2).
It proves that a generic helper's lookup is stable even when callers pass different instances.
</p>

</div>


<a id="orgb243f69"></a>

### NTTP Pinning Example

```cpp
template <class STRUCTURE,
          const auto& FOLDABLE = smd::foldable_typeclass<STRUCTURE> >
auto sum_with_nttp_lookup(const STRUCTURE& structure)
{
    return FOLDABLE.fold_map([](int x) { return x; }, structure);
}
```

<div class="notes" id="orgc8852ad">
<p>
The FOLDABLE parameter defaults to the variable template lookup.
Callers can supply a custom instance to change behavior for a specific call site.
</p>

</div>


<a id="org2f728a0"></a>

### Implementors: One Hook, Many Derived Operations

-   Implement one minimal hook per concept; all derived operations come for free.
-   Foldable: implement `fold_map` → get `length`, `fold_left`, `fold_right`, `to_vector`, `any_of`, `all_of`, `empty`, `find_first`.
-   Applicative: implement `pure` + `apply` → get `invoke`, `map`, `zip_with`, `discard_first`, `discard_second`.
-   Traversable: implement `traverse` → get `for_each`, `sequence`, `traverse_with`, `sequence_with`.

<div class="notes" id="org7ab6938">
<p>
The implementor surface is small; the user-facing surface is rich.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="org1a57c73"></a>

### CRTP and Deducing This

-   Each concept wrapper is a CRTP base (`Foldable<Impl>`, `Applicative<Impl>`, `Traversable<Impl>`).
-   `this auto&& self` preserves value category and constness through all wrapper calls.
-   Derived operations call back into the Impl via `self`; overrides are detected by `requires`.
-   Dispatch stays fully static — no virtual calls, no type erasure.

<div class="notes" id="org8bd8df1">
<p>
CRTP supplies structure; deducing this keeps wrappers generic without losing type information.
</p>

</div>


<a id="org855199a"></a>

# Functor: The Foundation


<a id="org8b01b70"></a>

### Functor Interface

-   Minimal hook: `fmap(F, container)` — apply a pure function inside a context.
-   Derived: `replace(container, value)` — overwrite all elements with a constant.
-   Instances provided: `std::optional`, `beman::optional`, `std::vector`.
-   Lookup: `smd::functor_typeclass<std::optional<int>>`.

<div class="notes" id="org9b5b122">
<p>
Functor is the base on which Applicative and Traversable are built.
</p>

</div>


<a id="org0a57d62"></a>

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

<div class="notes" id="org163ce44">
<p>
replace is derived from fmap with a constant function — no extra instance work required.
</p>

</div>


<a id="orgc05a457"></a>

### Functor Identity Law in Tests

-   fmap(id, x) == x for every instance and every shape.

```cpp
{
    const auto& functor = smd::functor_typeclass<std::optional<int> >;
    CHECK(functor.fmap(id, std::optional<int>{42}) == std::optional<int>{42});
    CHECK(functor.fmap(id, std::optional<int>{}) == std::optional<int>{});
}
```

<div class="notes" id="org45a329b">
<p>
Tests that encode laws document intent more durably than comments.
</p>

</div>


<a id="org850553c"></a>

# Preserving Shape: Traversable and Friends


<a id="org62c8095"></a>

### Foldable vs. Traversable: Sequence vs. Shape

-   Foldable consumes structure into a summary.
-   Traversable maps with effects while rebuilding the same outer shape.
-   For trees, this is the difference between counting nodes and relabeling them in place.


<a id="orgf5037e8"></a>

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
    
    <div class="notes" id="orgab7ea61">
    <p>
    Foldable can collapse two different shapes to the same flat view.
    Traversable keeps the tree skeleton and only transforms payloads.
    </p>
    
    </div>


<a id="org7afacf4"></a>

### Same Algorithm, Two Tree Representations

-   Fixpoint tree and shared\_ptr binary tree can share the same Foldable call shape.
-   The representation changes; the typeclass API and algorithm intent stay the same.

1.  Fixpoint Tree

    ```cpp
    auto n = foldable.length(tree);
    ```

2.  shared\_ptr Binary Tree

    ```cpp
    auto n = foldable.length(tree);
    ```

3.  FringeTree (Simplified FingerTree)

    ```cpp
    auto n = foldable.length(tree);
    ```
    
    ```cpp
    using beman::optional::optional;
    
    auto relabelled = traversable.traverse(
        [](int x) -> optional<int> {
            return x >= 0 ? optional<int>{x + 1} : optional<int>{};
        },
        tree);
    ```


<a id="org66f5d1f"></a>

# Foldable


<a id="orga67e55c"></a>

### Foldable API: One Hook, Derived from fold\_map

-   Minimal hook: `fold_map(F, container)` — apply F to each element, combine results.
-   Derived: `length`, `fold_left`, `fold_right`, `to_vector`, `any_of`, `all_of`, `empty`, `find_first`.
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
    
    <div class="notes" id="orgde7aee8">
    <p>
    Every derived operation is implemented by specializing what fold_map collects.
    fold_left and fold_right use a function-composition monoid internally.
    </p>
    
    </div>


<a id="org5d53b29"></a>

### Foldable Proof: Tests as Examples

-   Derived operations are verified directly against concrete inputs.

```cpp
using IntSequence = smd::typeclass::test::Sequence<int>;
auto sequence = IntSequence{{1, 2, 3}};
const auto& int_foldable = smd::foldable_typeclass<IntSequence>;

const auto as_vector = int_foldable.to_vector(sequence);
CHECK(as_vector == (std::vector<int>{1, 2, 3}));
```

<div class="notes" id="org9cc369f">
<p>
The test encodes a semantic claim: to_vector of {1,2,3} is exactly {1,2,3}.
That claim would catch a traversal-order regression.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="orgaab35ba"></a>

# Applicative


<a id="orgebd615d"></a>

### Applicative Model: Pure Function over Effectful Arguments

-   Applicative captures applying a pure function to independent effectful arguments.
-   Minimal hooks: `pure` (lift a value) and `apply` (apply a contextual function).
-   User API: `invoke` — matches the mental model of `std::invoke` over effectful values.
-   `apply_pure` is a teaching alias for FP audiences; `invoke` is the C++ spelling.
-   Less sequencing machinery than monadic formulations for independent effects.

<div class="notes" id="org66f26cd">
<p>
McBride's "applicative style" paper is the primary reference.
apply_pure retains FP bracket notation [| f a b c |] for audiences coming from Haskell.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="org1e4b814"></a>

### Applicative in Use

```cpp
auto sum = applicative.invoke(
    [](int a, int b, int c) { return a + b + c; },
    ax,
    ay,
    az);
```

<div class="notes" id="org3d9beee">
<p>
Three independent optional arguments. If any is absent the whole computation short-circuits.
</p>

</div>


<a id="orgc103809"></a>

### Applicative: invoke in Tests

```cpp
std::optional<int> ax{10};
std::optional<int> ay{5};
const auto& applicative = smd::applicative_typeclass<std::optional<int> >;

auto result = applicative.invoke([](int a, int b) { return a - b; }, ax, ay);
REQUIRE(result.has_value());
CHECK(*result == 5);
```

<div class="notes" id="org719f6de">
<p>
invoke works the same at arity 2, 3, or more — no per-call-site plumbing.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="org138a09e"></a>

### Applicative: invoke via Terminating Partial Application

-   `invoke` is derived: `pure(partial(f))` lifts f; each `apply` peels off one contextual argument.
-   Implementations provide only `pure` + `apply`; `invoke` can be overridden for custom semantics.
-   Also derived: `map`, `lift`, `ap`, `zip_with`, `discard_first`, `discard_second`, `invoke_with`.

```cpp
auto partial = smd::detail::make_terminating_partial(
    [](int a, int b, int c) { return a * 100 + b * 10 + c; });

auto partial2 = partial(1);
auto partial3 = partial2(2);
CHECK(partial3(3) == 123);
```

<div class="notes" id="orgd8ae7c0">
<p>
make_terminating_partial wraps f; each call either invokes f if all args are present or returns a new partial.
This avoids std::bind complexity while handling arbitrary arity uniformly.
</p>

</div>


<a id="org481e479"></a>

### Applicative Law: Interchange

-   Interchange: `ap(u, pure(y)) == ap(pure(λf. f(y)), u)`
-   Applying a contextual function to a pure value is symmetric.

```cpp
const auto& ap = smd::applicative_typeclass<std::optional<int> >;
std::optional<Fn> u{[](int x) { return x * 3; }};

auto lhs = ap.ap(u, ap.pure(y));
auto rhs = ap.ap(ap.pure([y](const Fn& fn) { return fn(y); }), u);

REQUIRE(lhs.has_value());
CHECK(*lhs == 21);
CHECK(lhs == rhs);
```

<div class="notes" id="org183c8a2">
<p>
The interchange law is the trickiest to build intuition for.
It constrains how pure values interact with contextual functions.
</p>

</div>


<a id="orga98e51b"></a>

### Applicative Law: Composition

-   Composition: `ap(invoke(∘, u, v), w) == ap(u, ap(v, w))`
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

<div class="notes" id="org3902127">
<p>
The composition law ensures that effectful function composition is associative.
w = 3, v doubles to 6, u adds 10: result is 16.
</p>

</div>


<a id="org12f4c9f"></a>

# Traversable


<a id="orgc6636a4"></a>

### Traversable Model: Commute Shape and Effect

-   Traversal commutes shape and effect: from a structure of effects to an effect of a structure.
-   This gives a generic path from many small checks to one checked result.
-   Traversable strictly generalizes Foldable: it can rebuild the container, not just collapse it.
-   Use this to model validation, partial relabeling, and structured transformations.


<a id="orgad71fea"></a>

### Traversable API: for\_each

```cpp
template <class T, class F>
auto for_each(this auto&& self, T&& value, F&& function)
{
    return self.traverse(std::forward<F>(function),
                         std::forward<T>(value));
}
```

<div class="notes" id="orgee4f36b">
<p>
for_each simply swaps the argument order of traverse.
</p>

</div>


<a id="org2d4dc05"></a>

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

<div class="notes" id="org094e2fd">
<p>
sequence commutes a container of effects into an effect of a container.
The identity function here means "the effect IS the structure": traverse(id, t).
</p>

</div>


<a id="org34eca31"></a>

### Traversable Proof: sequence in Tests

```cpp
using IdentityOpt = smd::typeclass::test::Identity<std::optional<int> >;
auto identity = IdentityOpt{std::optional<int>{1}};
const auto& traversable = smd::traversable_typeclass<IdentityOpt>;

auto sequenced = traversable.sequence(identity);
REQUIRE(sequenced.has_value());
CHECK(sequenced->value == 1);
```

<div class="notes" id="orgbe62006">
<p>
sequence converts Identity&lt;optional&lt;int&gt;&gt; into optional&lt;Identity&lt;int&gt;&gt;.
The shape is preserved; the effect wraps the whole result.
</p>

</div>


<a id="org8a1b681"></a>

### Traversable Law: Naturality

-   Naturality: for applicative morphism η (a map that commutes with `pure` and `ap`), `η(traverse f t) == traverse (η∘f) t`.
-   Concrete: η converts `optional<B>` → `beman::optional<B>`.
-   Applying η after traversal equals composing η with f before traversal.


<a id="orgf030f80"></a>

### Traversable Naturality Law in Tests

```cpp
{
    auto value = Identity{3};
    CHECK(eta(traversable.traverse(f, value)) ==
          traversable.traverse(eta_f, value));
}
```

<div class="notes" id="orgf744c0b">
<p>
eta converts optional&lt;Identity&lt;int&gt;&gt; to beman::optional&lt;Identity&lt;int&gt;&gt;.
eta_f is eta composed with f on the value side.
Both sides produce beman::optional&lt;Identity&lt;int&gt;&gt;{Identity{6}}.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="org93e187d"></a>

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
    
    <div class="notes" id="orgae72168">
    <p>
    Key law intuition: preserve shape and evaluation order discipline.
    Source: (Steve Downey, 2026).
    </p>
    
    </div>


<a id="orga021c7e"></a>

### Laws That Keep This Honest

-   Applicative: identity, homomorphism, interchange, composition — all automated.
-   Traversable: identity, naturality, composition — all automated.
-   Foldable: derived operations (`length`, `fold_left`, `fold_right`, `to_vector`, predicates) exercised against `fold_map`.
-   If these fail, abstractions become accidental APIs rather than reliable interfaces.

<div class="notes" id="org8fbc121">
<p>
If these fail, abstractions become accidental APIs rather than reliable interfaces.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="orgc3a3ee7"></a>

### Tree Applicative as Optional Appendix

-   Treat tree applicative as a policy choice, not the core applicative story.
-   Mainline examples stay with optional, ranges, and ZipList.
-   Keep it brief and explicitly label semantics if presented.

<div class="notes" id="orgc7f8f42">
<p>
The core teaching value of Applicative is visible in optional/range/ZipList examples.
</p>

</div>


<a id="org6854c87"></a>

# Monoids and Measured Trees


<a id="orgeda7f11"></a>

### Monoid Interface

-   A Monoid has two operations: `identity()` and `combine(lhs, rhs)`.
-   Associativity: `combine(combine(a,b),c) == combine(a,combine(b,c))`.
-   Left and right identity: `combine(identity(), x) == x == combine(x, identity())`.
-   Lookup via `monoid_v<T>`; extend by specializing `Monoid<T>`.


<a id="orge3b4a02"></a>

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

<div class="notes" id="org739dd1d">
<p>
Count is the canonical monoid for counting elements.
identity is 0; combine is addition — the simplest possible monoid.
</p>

</div>


<a id="org4f4e732"></a>

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

<div class="notes" id="orga2887d2">
<p>
monoid_v&lt;T&gt; is the canonical lookup object; monoid_combine and monoid_identity are free-function helpers.
These are the call shapes used by fold_map and all derived Foldable operations.
</p>

</div>


<a id="org5ac1188"></a>

### Monoid in Tests

```cpp
const smd::typeclass::Count one{1};
const smd::typeclass::Count two{2};

const auto result = smd::monoid_combine(one, two);
CHECK(result.d_value == 3U);
```

<div class="notes" id="orgc50b843">
<p>
monoid_combine dispatches through monoid_v&lt;Count&gt;. The test is mechanical, but it pins the specialization.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="org60080ca"></a>

### Monoid Identity Law in Tests

```cpp
{
    const auto& m = smd::typeclass::monoid_v<int>;
    CHECK(m.combine(m.identity(), 42) == 42);
    CHECK(m.combine(42, m.identity()) == 42);
}
```

<div class="notes" id="orgbf16d57">
<p>
The identity law is what makes identity() useful for initializing fold accumulators.
If this fails the Monoid is not a monoid.
</p>

</div>


<a id="orgaaac907"></a>

### Associativity as Algorithmic Leverage

-   Associativity lets us regroup work without changing results.
-   Measured trees exploit this to maintain summaries incrementally.
-   This is the bridge from algebra to explicit performance contracts.

<div class="notes" id="org728707b">
<p>
If the measure is a monoid, split/search become compositional.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="org80c1113"></a>

### Annotations as Summaries

-   Each node caches a measure of its subtree.
-   Measures are domain-specific: size, min priority, span, or cost.
-   Updating structure updates summaries locally.

<div class="notes" id="org8ddb1ce">
<p>
The data structure stays the same while behavior changes with the monoid.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="orgea2dfac"></a>

### Search and Split Driven by Measures

-   Search is currently implemented by linear scan over the flattened sequence.
-   Split currently follows the first predicate flip in that linear scan.
-   This yields one structure with many interpretations.

<div class="notes" id="org95233cf">
<p>
Sequence, priority queue, and rope are policy layers on one core tree.
The original finger-tree papers promise stronger asymptotics with measured search.
The target asymptotic story from those papers is amortized O(1) at the ends,
O(log(min(n,m))) concatenation, and O(log n) split/search.
The current repository implementation keeps the same API shape but does not yet
meet those split/search bounds.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="orgf24761a"></a>

# Finger Trees as a Case Study


<a id="orgd82c513"></a>

### Persistent Concatenation and Splitting

-   Current prototype provides efficient, persistence-friendly concatenation.
-   Current split/search paths are correct with linear-time upper bounds.
-   The API is designed so split/search can be optimized later without changing call sites.
-   The API naturally composes with foldable/traversable abstractions.

<div class="notes" id="org6c5d283">
<p>
This is where abstractions meet implementation reality.
The paper-level target bounds remain the north star.
The current prototype contract is explicit linear split/search.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="orgb916381"></a>

### One Structure, Many Interpretations

-   Change the monoid, change the interpretation.
-   Same implementation can model sequence, priority queue, or rope.
-   Reuse is semantic, not just syntactic.

<div class="notes" id="org79f26d1">
<p>
This is the strongest argument for measured trees in a standard library context.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="org00f42d2"></a>

### Why This Belongs in Modern C++

-   Zero-cost abstractions and strong typing fit this design.
-   Multiple paradigms can coexist: value types, OO boundaries, generic algorithms.
-   This is not importing Haskell; it is idiomatic modern C++ with better algebraic interfaces.

<div class="notes" id="orgb269fe4">
<p>
Pragmatic conclusion: values first, identity where required, and laws where possible.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="org499f8f8"></a>

# Designing APIs That Won't Age Poorly


<a id="org85b0975"></a>

### Library Abstractions Anticipating Language Features

-   Favor explicit, composable operations over magical overload sets.
-   Keep extension points separate from core type definitions.
-   Make future language support a simplification, not a rewrite.

<div class="notes" id="org7119b14">
<p>
Pattern matching and richer generic facilities should refine this API, not replace it.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="org1bd0c2c"></a>

### Avoiding the `std::bind` vs. Lambda Overlap

-   Avoid parallel abstractions that solve the same use case differently.
-   Choose one clear good path per concept.
-   For Applicative, that path is `invoke`; `apply_pure` remains a teaching aid.

<div class="notes" id="org2f399f8">
<p>
The goal is reducing cognitive branching in generic code.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="org43401bd"></a>

### Keeping the Good Path Obvious

-   Make lawful defaults easy and alternative policies explicit.
-   Keep naming consistent across concepts.
-   Back claims with executable law tests.

<div class="notes" id="org70a8af6">
<p>
The best API docs in this space are tests that encode the laws.
Source: (Steve Downey, 2026).
</p>

</div>
