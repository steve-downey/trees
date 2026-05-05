# Colophon

-   Slideware: [reveal.js](https://revealjs.com/)
-   Slide Preparation: [org-re-reveal](https://gitlab.com/oer/org-re-reveal)
-   Fonts: [Atkinson Hyperlegible](https://www.brailleinstitute.org/freefont/) Next and Mono
-   Color Themes: [Modus](https://github.com/protesilaos/modus-themes) Vivendi and Operandi Tinted

Intended to conform to [Web Content Accessibility Guidelines Level AAA](https://www.w3.org/WAI/WCAG2AAA-Conformance)

<div class="notes" id="orgcef790a">
<p>
Clock:
</p>

<ul class="org-ul">
<li>now <code>00:03</code></li>
<li>next <code>00:07</code></li>
</ul>

<p>
Quick timer guide:
</p>

<ul class="org-ul">
<li><code>00:07</code> problem statement</li>
<li><code>00:18</code> recursion schemes</li>
<li><code>00:34</code> typeclass object pattern complete</li>
<li><code>00:45</code> shape-preservation section complete</li>
<li><code>00:58</code> Applicative complete</li>
<li><code>01:06</code> Traversable complete</li>
<li><code>01:15</code> FingerTree case study complete</li>
<li><code>01:19</code> Tree Core API complete</li>
<li><code>01:22</code> questions</li>
<li><code>01:30</code> hard stop</li>
</ul>

<p>
Please try to be considerate when making presentations. Accessibility helps everyone.
</p>

<p>
Try to present working code, even in slideware.
</p>

</div>


# Algorithms for Trees

<div class="notes" id="org2bc802c">
<p>
Clock:
</p>

<ul class="org-ul">
<li>about <code>00:04</code></li>
<li>keep: 15s</li>
</ul>

<p>
Agenda slide. Name the three ideas once, then move.
</p>

</div>

-   Foldable.
-   Applicative.
-   Traversable.


## Abstract

<div class="notes" id="orgadb29e2">
<p>
Clock:
</p>

<ul class="org-ul">
<li>about <code>00:05</code></li>
</ul>

<p>
One-slide thesis: trees carry shape, and these abstractions let me keep it.
</p>

</div>

-   Functor and Monad patterns have proven themselves in practice; Foldable, Applicative, and Traversable are the next step.
-   Trees and structured data can lose important information when flattened to a sequence.
-   Monoid underpins efficient tree algorithms; the three typeclasses compose on top of it.
-   This talk sketches one route to a standard-library `fingertree` API.


## Foldable

<div class="notes" id="orgc67ec59">
<p>
Clock:
</p>

<ul class="org-ul">
<li>about <code>00:05</code></li>
</ul>

<p>
Summary only. Collapse to a result without flattening first.
</p>

</div>

-   Opt-in hook: `fold_map` — provides the algorithmic power of `std::ranges`.
-   Decouples algorithm from representation.
-   No flattening required.


## Applicative

<div class="notes" id="orgae3d357">
<p>
Clock:
</p>

<ul class="org-ul">
<li>about <code>00:06</code></li>
</ul>

<p>
Independent effects, not sequencing.
</p>

</div>

-   Apply a pure function to independent effectful arguments.
-   Two hooks: `pure` (lift) and `apply` (contextual application). `invoke` is the C++ user API.
-   Less machinery than Monad when the effects are independent.


## Traversable

<div class="notes" id="orge775ac4">
<p>
Clock:
</p>

<ul class="org-ul">
<li>about <code>00:06</code></li>
</ul>

<p>
This is the headliner: a tree stays a tree.
</p>

</div>

-   Generalizes Foldable: maps with effects while rebuilding the container shape.
-   A tree stays a tree; a fold discards the original branching shape.
-   Commutes containers: a range of effects becomes an effect of a range.


## Not Monadic

<div class="notes" id="orgc3b00c4">
<p>
Clock:
</p>

<ul class="org-ul">
<li>about <code>00:06</code></li>
</ul>

<p>
Scope control. Monad is important, but not needed for this story.
</p>

</div>

-   This talk stops short of Monad deliberately.
-   Monad adds sequencing and dependency between effects; most tree operations do not need this overhead.
-   Applicative covers the independent-effect cases where the whole structure is known upfront.


# Ranges Flatten the World

<div class="notes" id="org306f1ac">
<p>
Clock:
</p>

<ul class="org-ul">
<li>about <code>00:07</code></li>
<li>keep: 10s</li>
</ul>

<p>
Section marker. Move straight into the first concrete claim.
</p>

</div>


### Linearization as a Design Assumption

-   Ranges are a great default when the structure is inherently sequential.
-   Many generic algorithms quietly assume that flattening first is semantically neutral.
-   For trees, flattening throws away parent/child relationships and subtree boundaries.

<div class="notes" id="orgceb0865">
<p>
Clock:
</p>

<ul class="org-ul">
<li>in <code>00:07</code></li>
<li>out <code>00:13</code></li>
<li>cut: next 2 examples</li>
</ul>

<p>
This is the setup.
Flattening is a design choice, not a law of nature.
The rest of the talk is about keeping structure when structure carries meaning.
</p>

</div>


### Where Structure Carries Meaning

-   Search paths, balancing, and decomposition points are part of the meaning.
-   The same inorder sequence can come from many different trees.
-   If we flatten too early, we lose algorithmic leverage.

<div class="notes" id="orgeda0176">
<p>
The point here is practical, not philosophical.
If I preserve shape, I get better split, search, and relabel APIs.
</p>

</div>


### Trees That Are Not Sequences

-   Expression trees: hierarchy controls precedence and rewrite legality.
-   Syntax trees: children have roles, not just positions.
-   Measured trees: internal summaries define split/search interfaces and drive optimization.

<div class="notes" id="orgbc1ea14">
<p>
A range view is still useful, but it should be derived, not the primary model.
</p>

</div>


# Visitors, Pattern Matching, and the Missing Syntax

<div class="notes" id="orge64eb1d">
<p>
Clock:
</p>

<ul class="org-ul">
<li>about <code>00:13</code></li>
<li>keep: 10s</li>
</ul>

<p>
Bridge from the problem to the interface story.
</p>

</div>


### Visitor as Manual Recursion Control

-   Visitor centralizes recursion, but at the cost of ceremony and indirection.
-   Each new operation typically requires another visitor type or nested lambda structure.
-   The control flow is explicit, but often noisy.

<div class="notes" id="orgeaa8cd4">
<p>
Clock:
</p>

<ul class="org-ul">
<li>in <code>00:13</code></li>
<li>out <code>00:18</code></li>
</ul>

<p>
Visitor is not wrong.
It is just too low-level and too ceremonial for this kind of everyday algebraic work.
</p>

</div>


### Pattern Matching as the Intended Interface

-   Pattern matching expresses what cases exist directly.
-   C++ has active pattern-matching proposals, but no standardized feature yet.
-   Typeclass-style APIs can encode the same intent with today's language.

<div class="notes" id="org8ee2d90">
<p>
The design goal is not to predict syntax.
It is to land on APIs that migrate cleanly when the language catches up.
</p>

</div>


### Designing Today for Tomorrow's Syntax

-   Keep recursion control in library algorithms, not business code.
-   Expose a small vocabulary: `fold_map`, `invoke`, `traverse`.
-   Make call sites read like intent, not machinery.

<div class="notes" id="org24a7204">
<p>
This is about migration-friendly design, not speculative syntax tricks.
</p>

</div>


# Recursion Schemes You Can Actually Use

<div class="notes" id="org9f0b134">
<p>
Clock:
</p>

<ul class="org-ul">
<li>about <code>00:18</code></li>
<li>keep: 10s</li>
</ul>

<p>
Bridge slide. Recursion control moves into reusable library structure.
</p>

</div>


### F-Algebras: How to Collapse One Layer

-   Think of an algebra as consuming one layer and summarizing it.
-   The recursion pattern stays fixed while business logic changes.
-   This separation makes tree algorithms easier to reason about.

<div class="notes" id="org9c588e4">
<p>
Clock:
</p>

<ul class="org-ul">
<li>in <code>00:18</code></li>
<li>out <code>00:25</code></li>
<li>keep cata moving</li>
</ul>

<p>
I only need the operational intuition here.
I do not need a semester of category theory.
</p>

</div>


### Catamorphisms as a Principled Fold

-   Catamorphism: apply the algebra recursively until the structure is collapsed.
-   In C++, this corresponds to a disciplined fold over a recursive representation.
-   This yields reuse without hardcoding each algorithm into the node type.

<div class="notes" id="orga5c3064">
<p>
Foldable is the operational entry point for this in everyday code.
</p>

</div>

-   Fixpoint Expression Tree: eval\_algebra and cata

    ```cpp
    inline auto eval_algebra(const ExprF<double> &expr) -> double {
        return std::visit(
            smd::fixpoint::overloaded{
                [](const ExprConst<double> &c) { return c.value; },
                [](const ExprAdd<double> &a) { return *a.left + *a.right; },
                [](const ExprMul<double> &m) { return *m.left * *m.right; },
            },
            expr);
    }
    
    inline auto eval(const Expr &expr) -> double {
        return smd::fixpoint::cata<double>(eval_algebra, fmap_expr_fn, expr);
    }
    ```
    
    <div class="notes" id="org34a2546">
    <p>
    eval_algebra consumes one already-processed layer.
    Constants return their value.
    Binary nodes combine the folded children.
    cata supplies the recursion.
    That separation is the point.
    </p>
    
    </div>


### Separating Recursion from Business Logic

-   Business logic should answer how to combine results, not how to recurse.
-   This yields smaller tests and more reusable algorithms.
-   It also creates a natural place to enforce laws.

<div class="notes" id="org0eee4ec">
<p>
Once recursion is abstracted away, the laws read like executable documentation.
</p>

</div>


# The Typeclass Object Pattern

<div class="notes" id="orgb4f8cf1">
<p>
Clock:
</p>

<ul class="org-ul">
<li>about <code>00:25</code></li>
<li>keep: 10s</li>
</ul>

<p>
Section marker. This is the implementation spine of the talk.
</p>

</div>


### Typeclass Lookup: One Object Per Concept

-   Each concept has a variable template: `foldable_typeclass<T>`, `applicative_typeclass<T>`, `traversable_typeclass<T>`.
-   The looked-up object provides all operations for that concept on `T`.
-   New types opt in by specializing the variable template — no inheritance required.
-   Instances are open-world: add one close to the type, not in a central registry.

<div class="notes" id="org97600b2">
<p>
Clock:
</p>

<ul class="org-ul">
<li>in <code>00:25</code></li>
<li>out <code>00:34</code></li>
<li>cut: NTTP example if needed</li>
</ul>

<p>
This is the same basic impulse as concept maps.
The difference is that this version is smaller and works with today's language.
</p>

<p>
This also splits the naming bikeshed in two.
We argue once about typeclass operation names (<code>fold_map</code>, <code>traverse</code>, <code>invoke</code>) —
and it is OK that they are all terrible, because they name abstract generic operations.
Particular types and templates can use good, domain-specific names:
<code>push_back</code>, <code>pop_min</code>, <code>insert</code>, <code>to_string</code>.
The typeclass layer is plumbing; the user-facing API is porcelain.
</p>

</div>

-   Specializing the Variable Template

    ```cpp
    template <>
    inline constexpr auto foldable_typeclass<smd::fixpoint::Fix<smd::tree::ExprF>> =
        FixpointTreeFoldableMap{};
    ```
    
    <div class="notes" id="orgd5e9bbb">
    <p>
    Three lines of opt-in. No registry, no inheritance, no base-class modification.
    The specialization can live next to the type or in any adapter header.
    </p>
    
    </div>


### Three Lookup Modes

-   Implicit: `const auto& f = smd::foldable_typeclass<Tree>;` then call `f.method(...)`
-   Explicit object argument: pass a custom instance directly — local policy override.
-   NTTP pinning: `template <const auto& F = foldable_typeclass<Tree>>` — lookup bound at instantiation.
-   All three produce the same dispatch; the choice is about stability and explicitness.

<div class="notes" id="org5a22f68">
<p>
The interesting part is not the syntax.
The interesting part is that lookup is stable inside a generic helper.
That matters once you start parameterizing policy.
</p>

</div>


### NTTP Pinning Example

```cpp
template <class STRUCTURE,
          const auto &FOLDABLE = smd::foldable_typeclass<STRUCTURE>>
auto sum_with_nttp_lookup(const STRUCTURE &structure) {
    return FOLDABLE.fold_map([](int x) { return x; }, structure);
}
```

<div class="notes" id="orgc61a767">
<p>
The FOLDABLE parameter defaults to the variable template lookup.
Callers can supply a custom instance to change behavior for a specific call site.
</p>

</div>


### Implementers: One Hook, Many Derived Operations

-   Implement one minimal hook per concept; all derived operations are provided automatically.
-   Foldable: implement `fold_map` → counting, folding, predicates, collection.
-   Applicative: implement `pure` + `apply` → `invoke`, `map`, `ap`, `zip_with`, and more.
-   Traversable: implement `traverse` → `for_each`, `sequence`, and override variants.

<div class="notes" id="orge46fe50">
<p>
Small implementer surface.
Larger user surface.
That is the trade I want.
</p>

</div>


### CRTP and Deducing This

-   Each concept wrapper is a CRTP base (`Foldable<Impl>`, `Applicative<Impl>`, `Traversable<Impl>`).
-   `this auto&& self` (C++23 explicit object parameter) preserves value category and constness through all wrapper calls.
-   Derived operations call back into the Impl via `self`; overrides are detected by `requires`.
-   Dispatch stays fully static — no virtual calls, no type erasure.

<div class="notes" id="orgf588099">
<p>
CRTP supplies structure; deducing this keeps wrappers generic without losing type information.
</p>

</div>


# Functor: The Foundation

<div class="notes" id="org3ac317a">
<p>
Clock:
</p>

<ul class="org-ul">
<li>about <code>00:34</code></li>
<li>keep: 10s</li>
</ul>

<p>
Short bridge. Functor is the base layer, not the destination.
</p>

</div>


### Functor Interface

-   Minimal hook: `fmap(F, container)` — apply a pure function inside a context.
-   Derived: `replace(container, value)` — overwrite all elements with a constant.
-   Instances: `std::optional`, `beman::optional`, `std::vector`.
-   Lookup: `smd::functor_typeclass<std::optional<int>>`.

<div class="notes" id="org190a9f2">
<p>
Clock:
</p>

<ul class="org-ul">
<li>in <code>00:34</code></li>
<li>out <code>00:37</code></li>
</ul>

<p>
Functor is the base on which Applicative and Traversable are built.
</p>

</div>


### Functor: Derived replace

```cpp
template <class T, class U>
auto replace(this auto &&self, T &&value, U &&replacement) {
        return self.fmap([replacement = std::forward<U>(replacement)](
                             const auto &) { return replacement; },
                         std::forward<T>(value));
}
```

<div class="notes" id="org977499a">
<p>
replace is derived from fmap with a constant function — no extra instance work required.
</p>

</div>


### Functor Identity Law in Tests

-   fmap(id, x) == x for every instance and every shape.

```cpp
{
        const auto &functor = smd::functor_typeclass<std::optional<int>>;
        CHECK(functor.fmap(id, std::optional<int>{42}) ==
              std::optional<int>{42});
        CHECK(functor.fmap(id, std::optional<int>{}) == std::optional<int>{});
}
```

<div class="notes" id="org0905797">
<p>
Tests that encode laws document intent more durably than comments.
</p>

</div>


# Preserving Shape: Traversable and Friends

<div class="notes" id="org3f53454">
<p>
Clock:
</p>

<ul class="org-ul">
<li>about <code>00:37</code></li>
<li>keep: 10s</li>
</ul>

<p>
Section marker. This is where fold versus traverse becomes concrete.
</p>

</div>


### Foldable vs. Traversable: Sequence vs. Shape

-   Foldable consumes structure into a summary.
-   Traversable maps with effects while rebuilding the same outer shape.
-   For trees, this is the difference between counting nodes and relabeling them in place.

<div class="notes" id="org80767f1">
<p>
Clock:
</p>

<ul class="org-ul">
<li>in <code>00:37</code></li>
<li>out <code>00:45</code></li>
</ul>

<p>
I want the intuition first.
Foldable collapses.
Traversable transforms in place.
Then I can come back and make that formal.
</p>

</div>


### Crisp Contrast: Flatten vs. Preserve Shape

<div class="notes" id="orge9c5a54">
<p>
Clock:
</p>

<ul class="org-ul">
<li>about <code>00:40</code></li>
</ul>

<p>
Set up the before/after contrast, then let the code do the work.
</p>

</div>

-   Two differently shaped trees can flatten to the same sequence under Foldable.
-   Traversable can map values and keep the original branching shape.

-   Foldable Flattens and Loses Shape Identity

    <div class="notes" id="orgf34571a">
    <p>
    Clock:
    </p>
    
    <ul class="org-ul">
    <li>about <code>00:41</code></li>
    </ul>
    
    <p>
    Stress same flat result, different original shape.
    </p>
    
    </div>
    
    ```cpp
    auto right_flat = foldable.to_vector(right_deep);
    auto left_flat = foldable.to_vector(left_deep);
    ```

-   Traversable Maps While Preserving Shape

    ```cpp
    auto mapped = smd::traverse(
            [](double x) -> optional<double> { return optional<double>{x + 10.0}; },
            tree);
    ```
    
    <div class="notes" id="org71f3739">
    <p>
    Foldable can collapse two different shapes to the same flat view.
    Traversable keeps the tree skeleton and only transforms payloads.
    </p>
    
    </div>


### Same Algorithm, Three Tree Representations

-   Fixpoint tree and shared\_ptr binary tree can share the same Foldable call shape.
-   The representation changes; the typeclass API and algorithm intent stay the same.

<div class="notes" id="org5fbcc56">
<p>
The call site stays the same across all three representations.
That is the payoff.
I write the algorithm against the interface once, not once per tree type.
</p>

</div>

-   Fixpoint Tree

    ```cpp
    auto n = foldable.length(tree);
    ```
    
    <div class="notes" id="orgb904b0d">
    <p>
    Expr (Fix&lt;ExprF&gt;). length dispatches through foldable_typeclass&lt;Expr&gt;; counts leaf constants.
    </p>
    
    </div>

-   shared\_ptr Binary Tree

    ```cpp
    auto n = foldable.length(tree);
    ```
    
    <div class="notes" id="orgcdc8363">
    <p>
    BinaryTree&lt;int&gt;. Different type, different fold_map implementation — same call site.
    </p>
    
    </div>

-   FringeTree (Simplified FingerTree)

    ```cpp
    auto n = foldable.length(tree);
    ```
    
    <div class="notes" id="org24d6f68">
    <p>
    FringeTree: a variant-based tree (Empty | Leaf | Branch). Same API, third representation.
    </p>
    
    </div>

-   FringeTree: Traversable Also Preserves Shape

    ```cpp
    using beman::optional::optional;
    
    auto relabelled = smd::traverse(
            [](int x) -> optional<int> {
                return x >= 0 ? optional<int>{x + 1} : optional<int>{};
            },
            tree);
    ```
    
    <div class="notes" id="orgd3aaba1">
    <p>
    The same FringeTree that folded to {1,2,3} under Foldable now maps values and comes back as a FringeTree.
    The variant structure (Empty | Leaf | Branch) is intact; only the leaf values changed.
    </p>
    
    </div>


# Foldable

<div class="notes" id="org5cb6625">
<p>
Clock:
</p>

<ul class="org-ul">
<li>about <code>00:45</code></li>
<li>keep: 10s</li>
</ul>

<p>
Section marker. Now formalize the collapse story.
</p>

</div>


### Monoid: The Glue fold\_map Needs

-   `fold_map` maps each element to some type, then folds the results into one.
-   That result type must support two operations: a neutral starting value and an associative merge.
-   In other words: a Monoid. Counting uses `Count{0}` + addition. Collecting uses `vector{}` + append.

<div class="notes" id="org0777d97">
<p>
Clock:
</p>

<ul class="org-ul">
<li>in <code>00:45</code></li>
<li>out <code>00:50</code></li>
<li>cut: 1 derived example</li>
</ul>

<p>
This is just enough Monoid to make fold_map make sense.
I will come back to the full story later.
</p>

</div>


### Foldable API: One Hook, Derived from fold\_map

<div class="notes" id="orgf3c6580">
<p>
Clock:
</p>

<ul class="org-ul">
<li>about <code>00:47</code></li>
</ul>

<p>
One primitive, many derived operations.
</p>

</div>

-   Minimal hook: `fold_map(F, container)` — apply F to each element, combine results.
-   Derived: counting, folding left/right, collecting, predicates — all from one hook.
-   No traversal order is mandated; the instance chooses and must be consistent.

-   fold\_map → length

    <div class="notes" id="org040e180">
    <p>
    Clock:
    </p>
    
    <ul class="org-ul">
    <li>about <code>00:48</code></li>
    </ul>
    
    <p>
    Smallest derived example. Count by mapping each element to one.
    </p>
    
    </div>
    
    ```cpp
    template <class T>
    auto length(this auto &&self, T &&value) -> std::size_t {
            const auto count =
                self.fold_map([](const auto &) { return typeclass::Count{1}; },
                              std::forward<T>(value));
            return count.d_value;
    }
    ```

-   fold\_map → to\_vector

    ```cpp
    template <class T>
    auto to_vector(this auto &&self, T &&value) {
            return self.fold_map(
                [](const auto &x) {
                    using ValueType = remove_cvref_t<decltype(x)>;
                    return std::vector<ValueType>{x};
                },
                std::forward<T>(value));
    }
    ```
    
    <div class="notes" id="org14ba71a">
    <p>
    Every derived operation is implemented by specializing what fold_map collects.
    fold_left and fold_right use a function-composition monoid internally.
    </p>
    
    </div>


### Foldable Proof: Tests as Examples

-   Derived operations are verified directly against concrete inputs.

```cpp
using IntSequence = smd::typeclass::test::Sequence<int>;
auto sequence = IntSequence{{1, 2, 3}};
const auto &int_foldable = smd::foldable_typeclass<IntSequence>;

const auto as_vector = int_foldable.to_vector(sequence);
CHECK(as_vector == (std::vector<int>{1, 2, 3}));
```

<div class="notes" id="org5392ae9">
<p>
This is not a toy check.
It pins down traversal order.
If that order changes, this test fails.
</p>

</div>


# Applicative

<div class="notes" id="orga639000">
<p>
Clock:
</p>

<ul class="org-ul">
<li>about <code>00:50</code></li>
<li>keep: 10s</li>
</ul>

<p>
Section marker. Shift from summaries to effectful application.
</p>

</div>


### Applicative Model: Pure Function over Effectful Arguments

-   Applicative captures applying a pure function to independent effectful arguments.
-   Minimal hooks: `pure` (lift a value) and `apply` (apply a contextual function).
-   User API: `invoke` — matches the mental model of `std::invoke` over effectful values.
-   Less machinery than monadic formulations when the effects are independent.

<div class="notes" id="org8e44cdc">
<p>
Clock:
</p>

<ul class="org-ul">
<li>in <code>00:50</code></li>
<li>out <code>00:58</code></li>
<li>keep law slides brisk</li>
</ul>

<p>
McBride and Paterson are the reference here (Conor McBride and Ross Paterson, 2008).
apply_pure is just a teaching alias.
invoke is the spelling I actually want people to use in C++.
</p>

</div>


### Applicative in Use

```cpp
auto sum = applicative.invoke([](int a, int b, int c) { return a + b + c; },
                                  ax, ay, az);
```

<div class="notes" id="org4a7c047">
<p>
Three independent optional arguments.
If any one is absent, the whole computation short-circuits.
</p>

</div>


### Applicative: invoke in Tests

```cpp
std::optional<int> ax{10};
std::optional<int> ay{5};
const auto &applicative = smd::applicative_typeclass<std::optional<int>>;

auto result =
        applicative.invoke([](int a, int b) { return a - b; }, ax, ay);
REQUIRE(result.has_value());
CHECK(*result == 5);
```

<div class="notes" id="org2d009b7">
<p>
invoke works the same at arity 2, 3, or more — no per-call-site plumbing.
</p>

</div>


### Applicative: Short-Circuit on Absent

```cpp
std::optional<int> ax{1};
std::optional<int> ay{};
auto invoke_result =
        applicative.invoke([](int a, int b) { return a + b; }, ax, ay);
CHECK_FALSE(invoke_result.has_value());
```

<div class="notes" id="orgc37efcb">
<p>
ax is present; ay is absent. invoke short-circuits: f is never called.
This is the core contract of optional-as-applicative.
</p>

</div>


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

<div class="notes" id="org3feae2b">
<p>
make_terminating_partial wraps the function.
Each apply either calls it or returns the next partial.
This avoids std::bind complexity and gives one story for arbitrary arity.
</p>

</div>


### Applicative Law: Interchange

-   Interchange: `ap(u, pure(y)) == ap(pure(λf. f(y)), u)`
-   Applying a contextual function to a pure value is symmetric.

```cpp
const auto &ap = smd::applicative_typeclass<std::optional<int>>;
std::optional<Fn> u{[](int x) { return x * 3; }};

auto lhs = ap.ap(u, ap.pure(y));
auto rhs = ap.ap(ap.pure([](const Fn &fn) { return fn(y); }), u);

REQUIRE(lhs.has_value());
CHECK(*lhs == 21);
CHECK(lhs == rhs);
```

<div class="notes" id="orgc14388c">
<p>
Interchange is usually the law that needs the most explanation.
It tells you how a pure value and a contextual function are allowed to meet.
</p>

</div>


### Applicative Law: Composition

-   Composition: `ap(invoke(compose, u, v), w) == ap(u, ap(v, w))`
-   Composing effectful functions then applying equals sequencing the applications.

```cpp
const auto &ap = smd::applicative_typeclass<std::optional<int>>;
std::optional<Fn> u{[](int x) { return x + 10; }};
std::optional<Fn> v{[](int x) { return x * 2; }};
std::optional<int> w{3};

auto lhs = ap.ap(ap.invoke(compose, u, v), w);
auto rhs = ap.ap(u, ap.ap(v, w));

REQUIRE(lhs.has_value());
CHECK(*lhs == 16); // (3 * 2) + 10
CHECK(lhs == rhs);
```

<div class="notes" id="org06fd6bb">
<p>
Composition is the payoff law.
It says I can compose in the effectful world without changing the result.
Here the numbers make that concrete: 3, then 6, then 16.
</p>

</div>


# Traversable

<div class="notes" id="org25cc285">
<p>
Clock:
</p>

<ul class="org-ul">
<li>about <code>00:58</code></li>
<li>keep: 10s</li>
</ul>

<p>
Section marker. This is the payoff abstraction.
</p>

</div>


### Traversable Model: Commute Shape and Effect

<div class="notes" id="org660170f">
<p>
Clock:
</p>

<ul class="org-ul">
<li>about <code>00:58</code></li>
</ul>

<p>
Give the one-line slogan before the examples.
</p>

</div>

-   Traversal commutes shape and effect: from a structure of effects to an effect of a structure.
-   This gives a generic path from many small checks to one checked result.
-   Traversable strictly generalizes Foldable: it can rebuild the container, not just collapse it (Jeremy Gibbons and Bruno C. d. S. Oliveira, 2006).
-   Use this to model validation, partial relabeling, and structured transformations.


### Traversable in Use: Success

```cpp
auto values = smd::ranges::from_vector(std::vector<int>{1, 2, 3});

auto traversed = smd::traverse(
        [](int value) -> std::optional<int> {
            return std::optional<int>{value + 1};
        },
        values);

REQUIRE(traversed.has_value());
CHECK(collect(*traversed) == (std::vector<int>{2, 3, 4}));
```

<div class="notes" id="org89e0385">
<p>
Clock:
</p>

<ul class="org-ul">
<li>in <code>00:58</code></li>
<li>out <code>01:06</code></li>
<li>cut: naturality detail first</li>
</ul>

<p>
Every element transforms successfully. The optional wrapping is removed and a new range is returned.
</p>

</div>


### Traversable in Use: Failure Propagates

```cpp
auto values = smd::ranges::from_vector(std::vector<int>{1, -2, 3});

auto traversed = smd::traverse(
        [](int value) -> std::optional<int> {
            return value >= 0 ? std::optional<int>{value + 1}
                              : std::optional<int>{};
        },
        values);

CHECK_FALSE(traversed.has_value());
```

<div class="notes" id="org8c7dca0">
<p>
One absent result poisons the whole traversal. No partial range is returned.
This is the short-circuit behavior that distinguishes traverse from map.
</p>

</div>


### Traversable API: sequence

```cpp
template <class T>
auto sequence(this auto &&self, T &&value) {
        using Context = element_type;
        const auto &applicative = smd::applicative_typeclass<Context>;
        return self.traverse(
            applicative, [](auto &&x) { return std::forward<decltype(x)>(x); },
            std::forward<T>(value));
}
```

<div class="notes" id="org0de457f">
<p>
sequence commutes a container of effects into an effect of a container.
The identity function here means "the effect IS the structure": traverse(id, t).
Also derived: for_each, which is traverse with its arguments flipped (container before function).
</p>

</div>


### Traversable Proof: sequence in Tests

```cpp
using IdentityOpt = smd::typeclass::test::Identity<std::optional<int>>;
auto identity = IdentityOpt{std::optional<int>{1}};
const auto &traversable = smd::traversable_typeclass<IdentityOpt>;

auto sequenced = traversable.sequence(identity);
REQUIRE(sequenced.has_value());
CHECK(sequenced->value == 1);
```

<div class="notes" id="org9548436">
<p>
sequence converts Identity&lt;optional&lt;int&gt;&gt; into optional&lt;Identity&lt;int&gt;&gt;.
The shape is preserved; the effect wraps the whole result.
</p>

</div>


### Traversable Law: Naturality

-   If you have a function that converts between applicatives and respects `pure` and `ap`, traversal commutes through it.
-   Concretely: converting `optional<B>` → `beman::optional<B>` after traversal gives the same result as composing the conversion into `f` before traversal.

```cpp
{
        auto value = Identity{3};
        CHECK(to_beman(smd::traverse(f, value)) ==
              smd::traverse(f_returning_beman, value));
}
```

<div class="notes" id="orgf704bf5">
<p>
This is the most abstract law in the talk, so I want to narrate it slowly.
Change applicatives after traversal, or bake that change into the function first.
You should get the same answer either way.
That is what this test is checking.
</p>

</div>


### Traversable Commute: Range and ZipList

<div class="notes" id="orge0226f1">
<p>
Clock:
</p>

<ul class="org-ul">
<li>about <code>01:04</code></li>
</ul>

<p>
Use the transpose picture; do not get lost in the mechanics.
</p>

</div>

-   Traversable commutes a range of ZipLists into a ZipList of ranges.
-   The inverse matrix view (ZipList of vectors to vector of ZipLists) is also tested.

-   Range of ZipLists → ZipList of Ranges

    <div class="notes" id="orgb28a4f2">
    <p>
    Clock:
    </p>
    
    <ul class="org-ul">
    <li>about <code>01:04</code></li>
    </ul>
    
    <p>
    Range outside, ZipList inside; after commute, ZipList outside.
    </p>
    
    </div>
    
    ```cpp
    auto sequenced = traversable.sequence(values);
    
    REQUIRE(sequenced.data.size() == 2U);
    CHECK(collect(sequenced.data[0]) == (std::vector<int>{1, 10, 100}));
    CHECK(collect(sequenced.data[1]) == (std::vector<int>{2, 20, 200}));
    ```

-   ZipList of Vectors → Vector of ZipLists (matrix transpose)

    ```cpp
    smd::zip_list<std::vector<int>> zip_of_vectors{
            {{1, 10, 100}, {2, 20, 200}}};
    
    auto as_rows = to_vector_of_ziplists(zip_of_vectors);
    
    REQUIRE(as_rows.size() == 3U);
    CHECK(as_rows[0].data == (std::vector<int>{1, 2}));
    CHECK(as_rows[1].data == (std::vector<int>{10, 20}));
    CHECK(as_rows[2].data == (std::vector<int>{100, 200}));
    ```
    
    <div class="notes" id="orgb033760">
    <p>
    This helper is hand-coded on purpose.
    It gives a familiar matrix-transpose picture before I point back to the generic Traversable version.
    The law intuition is: preserve shape, preserve evaluation order discipline.
    </p>
    
    </div>


### Laws That Keep This Honest

-   Applicative: identity, homomorphism, interchange, composition — all automated.
-   Traversable: identity, naturality, composition — all automated.
-   Foldable: all derived operations exercised directly against `fold_map`.
-   If these fail, abstractions become accidental APIs rather than reliable interfaces.

<div class="notes" id="orgca9ae05">
<p>
If the laws fail, the abstraction is just an accident of naming.
Also, tree applicative is a policy choice.
The clean teaching examples are optional, range, and ZipList.
</p>

</div>


# Monoids and Measured Trees

<div class="notes" id="org17872fb">
<p>
Clock:
</p>

<ul class="org-ul">
<li>about <code>01:06</code></li>
<li>keep: 10s</li>
</ul>

<p>
Section marker. Now the algebra becomes data-structure leverage.
</p>

</div>


### Monoid Interface

<div class="notes" id="org00fe270">
<p>
Clock:
</p>

<ul class="org-ul">
<li>about <code>01:06</code></li>
</ul>

<p>
Say the contract once: identity plus associative combine.
</p>

</div>

-   The full contract: `identity()` (neutral element) and `combine(lhs, rhs)` (associative merge).
-   Associativity: `combine(combine(a,b),c) == combine(a,combine(b,c))` — regrouping changes nothing.
-   Left and right identity: `combine(identity(), x) == x == combine(x, identity())`.
-   Lookup via `monoid_v<T>`; extend by specializing `Monoid<T>`.


### Monoid: Count Specialization

```cpp
template <>
struct Monoid<Count> {
    constexpr auto identity() const -> Count { return Count{0}; }

    constexpr auto combine(const Count &lhs, const Count &rhs) const -> Count {
        return Count{lhs.d_value + rhs.d_value};
    }
};
```

<div class="notes" id="org43336ae">
<p>
Clock:
</p>

<ul class="org-ul">
<li>in <code>01:06</code></li>
<li>out <code>01:11</code></li>
</ul>

<p>
Count is the canonical monoid for counting elements.
identity is 0; combine is addition — the simplest possible monoid.
</p>

</div>


### Monoid: Generic Helpers

```cpp
template <class VALUE_TYPE>
auto monoid_identity() -> VALUE_TYPE {
    return typeclass::monoid_v<VALUE_TYPE>.identity();
}

template <class VALUE_TYPE>
auto monoid_combine(const VALUE_TYPE &lhs, const VALUE_TYPE &rhs)
    -> VALUE_TYPE {
    return typeclass::monoid_v<VALUE_TYPE>.combine(lhs, rhs);
}
```

<div class="notes" id="org8a1f2c9">
<p>
monoid_v&lt;T&gt; is the canonical lookup object; monoid_combine and monoid_identity are free-function helpers.
These are the call shapes used by fold_map and all derived Foldable operations.
</p>

</div>


### Monoid in Tests

```cpp
const smd::typeclass::Count one{1};
const smd::typeclass::Count two{2};

const auto result = smd::monoid_combine(one, two);
CHECK(result.d_value == 3U);
```

<div class="notes" id="orgeea9031">
<p>
monoid_combine dispatches through monoid_v&lt;Count&gt;. The test is mechanical, but it pins the specialization.
</p>

</div>


### Monoid Identity Law in Tests

```cpp
{
        const auto &m = smd::typeclass::monoid_v<int>;
        CHECK(m.combine(m.identity(), 42) == 42);
        CHECK(m.combine(42, m.identity()) == 42);
}
```

<div class="notes" id="orga8f0c8a">
<p>
The identity law is what makes identity() useful for initializing fold accumulators.
If this fails the Monoid is not a monoid.
</p>

</div>


### Associativity as Algorithmic Leverage

-   Associativity lets us regroup work without changing results.
-   Measured trees exploit this to maintain summaries incrementally.
-   This is the bridge from algebra to explicit performance contracts.

<div class="notes" id="org857f062">
<p>
This is where the algebra starts paying rent.
If the measure is a monoid, split and search become compositional (Ralf Hinze and Ross Paterson, 2006).
</p>

</div>


### Annotations as Summaries

-   Each node caches a measure of its subtree.
-   Measures are domain-specific: size, minimum priority, span, or cost.
-   Updating structure updates summaries locally.

<div class="notes" id="orga849deb">
<p>
The data structure stays the same while behavior changes with the monoid.
</p>

</div>


### Search and Split Driven by Measures

-   Each node carries a cumulative measure; split navigates by predicate on that measure.
-   Predicate-based core split navigates tree structure in O(log n).
-   `split_at_index()` uses the structural path for count-measure trees; non-count measures fall back to flatten/rebuild.

<div class="notes" id="orgbe35e01">
<p>
The Hinze-Paterson paper gives the headline costs (Ralf Hinze and Ross Paterson, 2006).
In this implementation, the important thing to say out loud is where the structural fast paths are, and where they are not.
The core split is structural.
split_at_index is structural for count measures, and falls back when it has to preserve index semantics for non-count measures.
The wrapper operations build on that split-plus-concat story.
</p>

</div>


# Finger Trees as a Case Study

<div class="notes" id="org9b73b25">
<p>
Clock:
</p>

<ul class="org-ul">
<li>about <code>01:11</code></li>
<li>keep: 10s</li>
</ul>

<p>
Section marker. This is the concrete proof-of-concept data structure.
</p>

</div>


### Persistent Concatenation and Splitting

-   Persistent concatenation in O(log(min(n,m))).
-   Predicate-based split navigates tree structure in O(log n); wrapper operations use split + concat.
-   The API composes naturally with foldable/traversable abstractions.

<div class="notes" id="org9ba7a77">
<p>
Clock:
</p>

<ul class="org-ul">
<li>in <code>01:11</code></li>
<li>out <code>01:15</code></li>
<li>cut: do 1 interpretation in detail</li>
</ul>

<p>
Concatenation uses Hinze-Paterson app3: O(log(min(n,m))).
Split navigates the tree structurally in O(log n) and returns left, pivot, right.
Wrapper operations (random-access, priority queue, rope) use split + concat directly.
</p>

</div>


### One Structure, Many Interpretations

-   Change the monoid, change the interpretation.
-   The same implementation can model sequence, priority queue, or rope.
-   Reuse is semantic, not merely syntactic.

<div class="notes" id="org660d01d">
<p>
This is the standard-library argument.
One structure, different meanings, without rewriting the whole implementation.
</p>

</div>

-   Sequence: O(log n) Random Access

    ```cpp
    using Seq = smd::tree::FingerTreeRandomAccess<int>;
    
    auto seq = Seq::from_sequence({1, 2, 3});
    REQUIRE(seq.at(0).has_value());
    CHECK(*seq.at(0) == 1);
    CHECK_FALSE(seq.at(99).has_value());
    
    auto edited =
            seq.push_back(4).push_front(0).insert(2, 9).update(3, 7).erase(1);
    CHECK(edited.to_vector() == (std::vector<int>{0, 9, 7, 3, 4}));
    ```
    
    <div class="notes" id="orgd343bba">
    <p>
    Monoid: size. The measure at each node is the count of elements below it.
    push_front/push_back are O(1) amortized.
    at/insert/erase/update use structural split O(log n) + concat O(log(min(n,m))).
    </p>
    
    </div>

-   Priority Queue: Min and Max in One Structure

    ```cpp
    auto q = Queue::from_values({5, 2, 8, 2, 7});
    REQUIRE(q.min().has_value());
    REQUIRE(q.max().has_value());
    CHECK(*q.min() == 2);
    CHECK(*q.max() == 8);
    ```
    
    <div class="notes" id="org4ce4787">
    <p>
    One FingerTree with a combined PriorityTag measure tracking both min and max simultaneously.
    Monoid: (Min, Max) — a pair that combines by taking component-wise extrema.
    push is O(1) amortized; pop_min/pop_max use measure-guided split O(log n) + concat O(log(min(n,m))).
    </p>
    
    </div>

-   Rope: Character Buffer with Efficient Editing

    ```cpp
    auto rope = Rope::from_text("abCDxy", 2)
                        .insert(2, "--")
                        .erase(5, 2)
                        .replace(0, 2, "AB");
    
    CHECK(rope.to_string() == "AB--Cy");
    CHECK(rope.size_bytes() == 6U);
    ```
    
    <div class="notes" id="org572daec">
    <p>
    Monoid: byte-length. The measure at each node is the byte count of its chunk subtree.
    insert, erase, and replace all navigate by cumulative byte offset using split.
    </p>
    
    </div>


### Why This Belongs in Modern C++

-   Adding Traversable to an existing type requires no modification to the type itself — one specialization in a header.
-   The Rope, priority queue, and sequence expose the same Traversable interface, with separate per-type specializations in the current codebase.
-   The abstraction is a library choice today; it maps cleanly to pattern matching and richer generic facilities when those arrive.

<div class="notes" id="org73ee146">
<p>
This is the concrete payoff.
Independent extension points still compose.
No monkey-patching, no reopening classes, no central registry.
</p>

</div>


# Tree Core API

<div class="notes" id="org1c25a48">
<p>
Clock:
</p>

<ul class="org-ul">
<li>about <code>01:15</code></li>
<li>keep: 10s</li>
</ul>

<p>
Section marker. Move from implementation to standard-library surface area.
</p>

</div>


### What a `std::tree` Should Provide

-   Containers have a standard vocabulary: `empty()`, `size()`, `front()`, `push_back()`, `begin()`.
-   Trees need an equivalent: construction, decomposition, persistent updates, bulk traversal.
-   FringeTree and FingerTree share this interface — differing in performance, not in surface.

<div class="notes" id="org46b8820">
<p>
Clock:
</p>

<ul class="org-ul">
<li>in <code>01:15</code></li>
<li>out <code>01:19</code></li>
<li>cut: summarize tables</li>
</ul>

<p>
This is the surface you would expect from a standard tree type.
It parallels the container requirements tables, but for persistent tree structures.
All operations are functional — they return new trees, leaving the original unchanged.
</p>

</div>


### Construction and Queries

| Operation          | Description                    | Container Parallel |
|------------------ |------------------------------ |------------------ |
| `empty()`          | Create an empty tree           | default ctor       |
| `leaf(x)`          | Single-element tree            | initializer ctor   |
| `from_sequence(v)` | Build from a vector            | range ctor         |
| `is_empty()`       | True if no elements            | `empty()`          |
| `measure()`        | Aggregate metric (count, etc.) | `size()`           |
| `breadth()`        | Number of leaf elements        | `size()`           |

<div class="notes" id="org5f5ed78">
<p>
<code>measure</code> generalizes <code>size</code> — the metric can be count, priority, or byte-length, depending on the monoid.
<code>breadth</code> always returns the element count regardless of measure policy.
</p>

</div>


### Views and Persistent Updates

| Operation       | Description                           | Container Parallel    |
|--------------- |------------------------------------- |--------------------- |
| `view_l()`      | Leftmost element + rest               | —                     |
| `view_r()`      | Rightmost element + rest              | —                     |
| `head()`        | Leftmost element                      | `front()`             |
| `last()`        | Rightmost element                     | `back()`              |
| `tail()`        | Drop leftmost                         | `pop_front()` (pure)  |
| `init()`        | Drop rightmost                        | `pop_back()` (pure)   |
| `cons(x)`       | Prepend element                       | `push_front()` (pure) |
| `snoc(x)`       | Append element                        | `push_back()` (pure)  |
| `append(other)` | Concatenate two trees                 | —                     |
| `flatten()`     | Extract all elements as vector        | iterator + copy       |
| `for_each(f)`   | Visit each element without allocation | `for_each` algorithm  |

<div class="notes" id="org9ca51ac">
<p>
Views are the key novelty: <code>view_l</code> returns both the head and the rest in one operation.
This is the functional equivalent of <code>front()</code> + <code>pop_front()</code>, but non-destructive.
All updates return new trees — structural sharing makes this efficient.
</p>

</div>


### Traversable Fills the Gaps

-   Foldable gives `fold_map`, `length`, `to_vector`, `any_of`, `all_of`, `fold_left`, `fold_right` — derived from one hook.
-   Traversable gives `traverse`, `sequence` — shape-preserving traversal with effects.
-   These work on *any* tree (FringeTree, FingerTree, BinaryTree, FixTree) via typeclass specialization.
-   A tree does not need the full core API to participate in generic algorithms.

<div class="notes" id="org360bb35">
<p>
A new tree type needs only a <code>traverse</code> implementation and an <code>element_type</code> declaration.
Everything else is derived.
The core API above is for direct tree manipulation;
the typeclass layer is for generic algorithms that work across all tree shapes.
</p>

</div>


# Cross-Language Name Mapping

<div class="notes" id="org0a641c8">
<p>
Clock:
</p>

<ul class="org-ul">
<li>about <code>01:19</code></li>
<li>keep: 10s</li>
</ul>

<p>
Short comparison section. Trim here before trimming questions.
</p>

</div>


### Functor

| C++       | Haskell | Cats (Scala) | PureScript  | Core |
|--------- |------- |------------ |----------- |---- |
| `fmap`    | `fmap`  | `map`        | `map`       | ✓    |
| `replace` | `(<$)`  | `as`         | `voidRight` |      |

<div class="notes" id="org3ee183c">
<p>
Clock:
</p>

<ul class="org-ul">
<li>in <code>01:19</code></li>
<li>out <code>01:21</code></li>
<li>cut: tables first</li>
</ul>

<p>
Functor is minimal everywhere: one operation. C++ keeps the Haskell name.
Cats and PureScript prefer <code>map</code>; C++ reserves <code>map</code> for the Applicative-derived version.
</p>

</div>


### Foldable

| C++           | Haskell   | Cats (Scala) | PureScript | Core |
|------------- |--------- |------------ |---------- |---- |
| `fold_map`    | `foldMap` | `foldMap`    | `foldMap`  | ✓    |
| `fold_right`  | `foldr`   | `foldRight`  | `foldr`    | ✓    |
| `fold_left`   | `foldl'`  | `foldLeft`   | `foldl`    | ✓    |
| `length`      | `length`  | `size`       | `length`   |      |
| `to_vector`   | `toList`  | `toList`     | —          |      |
| `empty`       | `null`    | `isEmpty`    | `null`     |      |
| `any_of`      | `any`     | `exists`     | `any`      |      |
| `all_of`      | `all`     | `forall`     | `all`      |      |
| `find_first`  | `find`    | `find`       | `find`     |      |
| `fold`        | `fold`    | `fold`       | `fold`     |      |
| `combine_all` | `fold`    | `combineAll` | `fold`     |      |

<div class="notes" id="orgb0365db">
<p>
C++ names follow <code>std::ranges</code> conventions: <code>any_of</code>, <code>all_of</code>, <code>fold_left</code>, <code>fold_right</code>.
<code>to_vector</code> instead of <code>toList</code> — the natural C++ materialization target.
<code>empty</code> instead of <code>null</code> — avoids pointer connotation.
Three different alternate cores across languages: Haskell allows <code>foldMap</code> or <code>foldr</code>;
Cats requires <code>foldLeft</code> + <code>foldRight</code>; PureScript requires all three.
This library defaults to <code>fold_map</code> as the primitive; the alternate-core pattern supports <code>fold_right</code>.
</p>

</div>


### Applicative

| C++              | Haskell  | Cats (Scala) | PureScript | Core |
|---------------- |-------- |------------ |---------- |---- |
| `pure`           | `pure`   | `pure`       | `pure`     | ✓    |
| `apply`          | `(<*>)`  | `ap`         | `apply`    | ✓    |
| `zip_with`       | `liftA2` | `map2`       | —          | ✓    |
| `invoke`         | —        | —            | —          |      |
| `map`            | `fmap`   | `map`        | `map`      |      |
| `lift`           | `pure`   | `pure`       | `pure`     |      |
| `ap`             | `(<*>)`  | `ap`         | `apply`    |      |
| `discard_first`  | `(*>)`   | `productR`   | —          |      |
| `discard_second` | `(<*)`   | `productL`   | —          |      |
| `apply_pure`     | —        | —            | —          |      |

<div class="notes" id="orgcd7ec4b">
<p>
<code>invoke</code> is unique to this C++ library: applies a pure function to effectful arguments
without requiring manual currying. It replaces what Haskell expresses with <code>f &lt;$&gt; a &lt;*&gt; b &lt;*&gt; c</code>.
<code>apply_pure</code> is a teaching alias for <code>invoke</code> that preserves bracket notation for FP audiences.
Haskell allows <code>pure + (&lt;*&gt;)</code> or <code>pure + liftA2</code> as alternate cores.
Cats requires <code>pure + ap</code>. PureScript requires <code>pure + apply</code>.
This library defaults to <code>pure + apply</code>; the alternate-core pattern supports <code>zip_with</code>.
</p>

</div>


### Traversable

| C++             | Haskell     | Cats (Scala) | PureScript | Core |
|--------------- |----------- |------------ |---------- |---- |
| `traverse`      | `traverse`  | `traverse`   | `traverse` | ✓    |
| `sequence`      | `sequenceA` | `sequence`   | `sequence` | ✓    |
| `for_each`      | `for`       | —            | `for`      |      |
| `traverse_with` | —           | —            | —          |      |
| `sequence_with` | —           | —            | —          |      |

<div class="notes" id="orgaac58fd">
<p>
<code>sequence</code> drops the <code>A</code> suffix (for Applicative) since this library omits the monadic <code>sequence</code>.
<code>for_each</code> is the flipped form: container before function.
<code>traverse_with</code> and <code>sequence_with</code> are C++ innovations: explicit-object override for
the applicative instance used during traversal. Haskell, Cats, and PureScript dispatch
implicitly through their typeclass mechanisms; in C++ the lookup object is a value you can replace.
Haskell allows <code>traverse</code> or <code>sequenceA</code> as alternate cores.
Cats requires <code>traverse</code> only. PureScript requires both <code>traverse</code> and <code>sequence</code>.
This library defaults to <code>traverse</code>; the alternate-core pattern supports <code>sequence</code>.
</p>

</div>


### Monoid

| C++        | Haskell  | Cats (Scala) | PureScript | Core |
|---------- |-------- |------------ |---------- |---- |
| `identity` | `mempty` | `empty`      | `mempty`   | ✓    |
| `combine`  | `(<>)`   | `combine`    | `append`   | ✓    |

<div class="notes" id="orgf97b5b1">
<p>
All languages agree on two operations; naming varies.
<code>identity</code> is chosen over <code>mempty</code> for mathematical clarity.
<code>combine</code> matches Cats; <code>(&lt;&gt;)</code> is the Haskell operator (from Semigroup); PureScript uses <code>append</code>.
Haskell and PureScript separate Semigroup (<code>&lt;&gt;</code>) from Monoid (<code>mempty</code>); this library does not.
</p>

</div>


### Alternate Cores: Why This Matters

-   Haskell's `{-# MINIMAL foldMap | foldr #-}` pragma lets instances choose which operation to provide.
-   This C++ library supports the same idea via `using Impl::primitive;` in the Map class.
-   Different types naturally express different primitives: a tree may find `fold_map` natural; a stream may prefer `fold_right`.
-   The `using` declaration selects the core; the base class derives everything else.

<div class="notes" id="org4a18503">
<p>
This is not decorative boilerplate.
The alternate-core pattern is load-bearing in the implementation you just saw.
The <code>using Impl::traverse;</code> declarations are how that choice is made.
</p>

</div>


# Designing APIs That Won't Age Poorly

<div class="notes" id="org3c35cc7">
<p>
Clock:
</p>

<ul class="org-ul">
<li>about <code>01:21</code></li>
<li>keep: 10s</li>
</ul>

<p>
Final synthesis before Q&amp;A.
</p>

</div>


### Library Abstractions Anticipating Language Features

-   Favor explicit, composable operations over magical overload sets.
-   Keep extension points separate from core type definitions.
-   Make future language support a simplification, not a rewrite.

<div class="notes" id="orgc7e970a">
<p>
Clock:
</p>

<ul class="org-ul">
<li>in <code>01:21</code></li>
<li>out <code>01:22</code></li>
</ul>

<p>
If the language gets better, this API should get simpler.
It should not need to be replaced.
</p>

</div>


### Avoiding the `std::bind` vs. Lambda Overlap

-   Avoid parallel abstractions that solve the same use case differently.
-   Choose one clear good path per concept.
-   For Applicative, that path is `invoke`.

<div class="notes" id="org1ab6438">
<p>
The goal is to reduce cognitive branching in generic code.
One good path beats three almost-equivalent ones.
</p>

</div>


### Keeping the Good Path Obvious

-   Make lawful defaults easy and alternative policies explicit.
-   Keep naming consistent across concepts.
-   Back claims with executable law tests.

<div class="notes" id="orgf194be8">
<p>
The best documentation here is still the tests.
They say what the interface is allowed to mean.
</p>

</div>


# One more thing


### Monad: When the Next Step Depends on the Previous Result

-   Applicative covers independent effects; Monad adds *dependent* sequencing.
-   Minimal hook: `bind` — the result of one computation determines the next.
-   Derived `apply` from `bind + pure` — every Monad is an Applicative.
-   The typeclass pattern extends naturally: one hook, many derived operations.

<div class="notes" id="orgff031ec">
<p>
Bonus slide.
This is not part of the talk's thesis — the talk deliberately stops at Applicative.
But the pattern extends to Monad without design changes: delegation to the
applicative typeclass object, synthesized <code>apply</code> from <code>bind + pure</code>, law tests.
The Monad instance for <code>optional</code> is exactly <code>and_then</code> from C++23.
</p>

</div>


# Questions?

-   **A Question:** is where YOU want more information from ME.
-   **A Question:** goes up at the end.

<div class="notes" id="orgd7ce32e">
<p>
Clock:
</p>

<ul class="org-ul">
<li>Q&amp;A <code>01:22</code></li>
<li>stop <code>01:30</code></li>
<li>no questions: revisit payoff slides</li>
</ul>

</div>

> "More of a comment than a question &hellip;" hold them for a moment. I want to discuss this all with everyone, and you know where to find me.


# Comments?

<div class="notes" id="org22a8ad0">
<p>
Clock:
</p>

<ul class="org-ul">
<li>after <code>01:30</code> if used</li>
</ul>

<p>
Overflow slide for hallway-track discussion, not scheduled time.
</p>

</div>


# Thank You!

<div class="notes" id="orgb11795f">
<p>
Clock:
</p>

<ul class="org-ul">
<li>after <code>01:30</code> if used</li>
</ul>

<p>
End cleanly and invite follow-up offline.
</p>

</div>


# Bibliography

<div class="notes" id="orge73863c">
<p>
Hidden bibliography slide.
</p>

</div>

Conor McBride and Ross Paterson (2008). *Applicative Programming with Effects*, Journal of Functional Programming.

Jeremy Gibbons and Bruno C. d. S. Oliveira (2006). *The Essence of the Iterator Pattern*.

Ralf Hinze and Ross Paterson (2006). *Finger Trees: A Simple General-Purpose Data Structure*, Journal of Functional Programming.
