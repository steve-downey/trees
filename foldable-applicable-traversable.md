# Colophon

-   Slideware: [reveal.js](https://revealjs.com/)
-   Slide Preparation: [org-re-reveal](https://gitlab.com/oer/org-re-reveal)
-   Fonts: [Atkinson Hyperlegible](https://www.brailleinstitute.org/freefont/) Next and Mono
-   Color Themes: [Modus](https://github.com/protesilaos/modus-themes) Vivendi and Operandi Tinted

Intended to conform to [Web Content Accessibility Guidelines Level AAA](https://www.w3.org/WAI/WCAG2AAA-Conformance)

<div class="notes" id="org0d3acab">
<p>
Please try to be considerate when making presentations. Accessibility helps everyone.
</p>

<p>
Try to present working code, even in slideware.
</p>

</div>


# Algorithms for Trees

-   Foldable.
-   Applicative.
-   Traversable.


## Abstract

-   Functor and Monad patterns have proven themselves in practice; Foldable, Applicative, and Traversable are the next step.
-   Trees and structured data can lose important information when flattened to a sequence.
-   Monoid underpins efficient tree algorithms; the three typeclasses compose on top of it.
-   This talk sketches one route to a standard-library `fingertree` API.


## Foldable

-   Opt-in hook: `fold_map` — provides the algorithmic power of `std::ranges`.
-   Decouples algorithm from representation.
-   No flattening required.


## Applicative

-   Apply a pure function to independent effectful arguments.
-   Two hooks: `pure` (lift) and `apply` (contextual application). `invoke` is the C++ user API.
-   Less machinery than Monad when the effects are independent.


## Traversable

-   Generalizes Foldable: maps with effects while rebuilding the container shape.
-   A tree stays a tree; a fold discards the original branching shape.
-   Commutes containers: a range of effects becomes an effect of a range.


## Not Monadic

-   This talk stops short of Monad deliberately.
-   Monad adds sequencing and dependency between effects; most tree operations do not need this overhead.
-   Applicative covers the independent-effect cases where the whole structure is known upfront.


# Ranges Flatten the World


### Linearization as a Design Assumption

-   Ranges are a great default when the structure is inherently sequential.
-   Many generic algorithms quietly assume that flattening first is semantically neutral.
-   For trees, flattening throws away parent/child relationships and subtree boundaries.

<div class="notes" id="org5dbcf7c">
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

<div class="notes" id="orgb6b5be8">
<p>
The point here is practical, not philosophical.
If I preserve shape, I get better split, search, and relabel APIs.
</p>

</div>


### Trees That Are Not Sequences

-   Expression trees: hierarchy controls precedence and rewrite legality.
-   Syntax trees: children have roles, not just positions.
-   Measured trees: internal summaries define split/search interfaces and drive optimization.

<div class="notes" id="org2f7aa90">
<p>
A range view is still useful, but it should be derived, not the primary model.
</p>

</div>


# Visitors, Pattern Matching, and the Missing Syntax


### Visitor as Manual Recursion Control

-   Visitor centralizes recursion, but at the cost of ceremony and indirection.
-   Each new operation typically requires another visitor type or nested lambda structure.
-   The control flow is explicit, but often noisy.

<div class="notes" id="orgcaac07e">
<p>
Visitor is not wrong.
It is just too low-level and too ceremonial for this kind of everyday algebraic work.
</p>

</div>


### Pattern Matching as the Intended Interface

-   Pattern matching expresses what cases exist directly.
-   C++ has active pattern-matching proposals, but no standardized feature yet.
-   Typeclass-style APIs can encode the same intent with today's language.

<div class="notes" id="org4e8c060">
<p>
The design goal is not to predict syntax.
It is to land on APIs that migrate cleanly when the language catches up.
</p>

</div>


### Designing Today for Tomorrow's Syntax

-   Keep recursion control in library algorithms, not business code.
-   Expose a small vocabulary: `fold_map`, `invoke`, `traverse`.
-   Make call sites read like intent, not machinery.

<div class="notes" id="orgc0e7b11">
<p>
This is about migration-friendly design, not speculative syntax tricks.
</p>

</div>


# Recursion Schemes You Can Actually Use


### F-Algebras: How to Collapse One Layer

-   Think of an algebra as consuming one layer and summarizing it.
-   The recursion pattern stays fixed while business logic changes.
-   This separation makes tree algorithms easier to reason about.

<div class="notes" id="org63c49ed">
<p>
I only need the operational intuition here.
I do not need a semester of category theory.
</p>

</div>


### Catamorphisms as a Principled Fold

-   Catamorphism: apply the algebra recursively until the structure is collapsed.
-   In C++, this corresponds to a disciplined fold over a recursive representation.
-   This yields reuse without hardcoding each algorithm into the node type.

<div class="notes" id="org89ce70c">
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
    
    <div class="notes" id="orgdc6499d">
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

<div class="notes" id="org488ffdd">
<p>
Once recursion is abstracted away, the laws read like executable documentation.
</p>

</div>


# The Typeclass Object Pattern


### Typeclass Lookup: One Object Per Concept

-   Each concept has a variable template: `foldable_typeclass<T>`, `applicative_typeclass<T>`, `traversable_typeclass<T>`.
-   The looked-up object provides all operations for that concept on `T`.
-   New types opt in by specializing the variable template — no inheritance required.
-   Instances are open-world: add one close to the type, not in a central registry.

<div class="notes" id="org7d3df13">
<p>
This is the same basic impulse as concept maps.
The difference is that this version is smaller and works with today's language.
</p>

</div>

-   Specializing the Variable Template

    ```cpp
    template <>
    inline constexpr auto foldable_typeclass<smd::fixpoint::Fix<smd::tree::ExprF>> =
        FixpointTreeFoldableMap{};
    ```
    
    <div class="notes" id="org2fbf080">
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

<div class="notes" id="org14f3385">
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

<div class="notes" id="orge94353e">
<p>
The FOLDABLE parameter defaults to the variable template lookup.
Callers can supply a custom instance to change behavior for a specific call site.
</p>

</div>


### Implementors: One Hook, Many Derived Operations

-   Implement one minimal hook per concept; all derived operations are provided automatically.
-   Foldable: implement `fold_map` → counting, folding, predicates, collection.
-   Applicative: implement `pure` + `apply` → `invoke`, `map`, `ap`, `zip_with`, and more.
-   Traversable: implement `traverse` → `for_each`, `sequence`, and override variants.

<div class="notes" id="org59e7372">
<p>
Small implementor surface.
Larger user surface.
That is the trade I want.
</p>

</div>


### CRTP and Deducing This

-   Each concept wrapper is a CRTP base (`Foldable<Impl>`, `Applicative<Impl>`, `Traversable<Impl>`).
-   `this auto&& self` (C++23 explicit object parameter) preserves value category and constness through all wrapper calls.
-   Derived operations call back into the Impl via `self`; overrides are detected by `requires`.
-   Dispatch stays fully static — no virtual calls, no type erasure.

<div class="notes" id="org86d4d21">
<p>
CRTP supplies structure; deducing this keeps wrappers generic without losing type information.
</p>

</div>


# Functor: The Foundation


### Functor Interface

-   Minimal hook: `fmap(F, container)` — apply a pure function inside a context.
-   Derived: `replace(container, value)` — overwrite all elements with a constant.
-   Instances: `std::optional`, `beman::optional`, `std::vector`.
-   Lookup: `smd::functor_typeclass<std::optional<int>>`.

<div class="notes" id="orgb38a919">
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

<div class="notes" id="orge95edf1">
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

<div class="notes" id="org6e86bdc">
<p>
Tests that encode laws document intent more durably than comments.
</p>

</div>


# Preserving Shape: Traversable and Friends


### Foldable vs. Traversable: Sequence vs. Shape

-   Foldable consumes structure into a summary.
-   Traversable maps with effects while rebuilding the same outer shape.
-   For trees, this is the difference between counting nodes and relabeling them in place.

<div class="notes" id="org7626568">
<p>
I want the intuition first.
Foldable collapses.
Traversable transforms in place.
Then I can come back and make that formal.
</p>

</div>


### Crisp Contrast: Flatten vs. Preserve Shape

-   Two differently shaped trees can flatten to the same sequence under Foldable.
-   Traversable can map values and keep the original branching shape.

-   Foldable Flattens and Loses Shape Identity

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
    
    <div class="notes" id="orgdfab721">
    <p>
    Foldable can collapse two different shapes to the same flat view.
    Traversable keeps the tree skeleton and only transforms payloads.
    </p>
    
    </div>


### Same Algorithm, Two Tree Representations

-   Fixpoint tree and shared\_ptr binary tree can share the same Foldable call shape.
-   The representation changes; the typeclass API and algorithm intent stay the same.

<div class="notes" id="org1f35331">
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
    
    <div class="notes" id="org70a3349">
    <p>
    Expr (Fix&lt;ExprF&gt;). length dispatches through foldable_typeclass&lt;Expr&gt;; counts leaf constants.
    </p>
    
    </div>

-   shared\_ptr Binary Tree

    ```cpp
    auto n = foldable.length(tree);
    ```
    
    <div class="notes" id="org176aefd">
    <p>
    BinaryTree&lt;int&gt;. Different type, different fold_map implementation — same call site.
    </p>
    
    </div>

-   FringeTree (Simplified FingerTree)

    ```cpp
    auto n = foldable.length(tree);
    ```
    
    <div class="notes" id="orgd77ff8b">
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
    
    <div class="notes" id="orgd9bb2fe">
    <p>
    The same FringeTree that folded to {1,2,3} under Foldable now maps values and comes back as a FringeTree.
    The variant structure (Empty | Leaf | Branch) is intact; only the leaf values changed.
    </p>
    
    </div>


# Foldable


### Monoid: The Glue fold\_map Needs

-   `fold_map` maps each element to some type, then folds the results into one.
-   That result type must support two operations: a neutral starting value and an associative merge.
-   In other words: a Monoid. Counting uses `Count{0}` + addition. Collecting uses `vector{}` + append.

<div class="notes" id="orgcc31a66">
<p>
This is just enough Monoid to make fold_map make sense.
I will come back to the full story later.
</p>

</div>


### Foldable API: One Hook, Derived from fold\_map

-   Minimal hook: `fold_map(F, container)` — apply F to each element, combine results.
-   Derived: counting, folding left/right, collecting, predicates — all from one hook.
-   No traversal order is mandated; the instance chooses and must be consistent.

-   fold\_map → length

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
    
    <div class="notes" id="org14ac23e">
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

<div class="notes" id="orgd7e19a9">
<p>
This is not a toy check.
It pins down traversal order.
If that order changes, this test fails.
</p>

</div>


# Applicative


### Applicative Model: Pure Function over Effectful Arguments

-   Applicative captures applying a pure function to independent effectful arguments.
-   Minimal hooks: `pure` (lift a value) and `apply` (apply a contextual function).
-   User API: `invoke` — matches the mental model of `std::invoke` over effectful values.
-   Less machinery than monadic formulations when the effects are independent.

<div class="notes" id="org3e79d60">
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

<div class="notes" id="org45f8567">
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

<div class="notes" id="org3996ae9">
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

<div class="notes" id="org37653f9">
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

<div class="notes" id="orgdd8c4af">
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

<div class="notes" id="org4479367">
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

<div class="notes" id="org289a1b2">
<p>
Composition is the payoff law.
It says I can compose in the effectful world without changing the result.
Here the numbers make that concrete: 3, then 6, then 16.
</p>

</div>


# Traversable


### Traversable Model: Commute Shape and Effect

-   Traversal commutes shape and effect: from a structure of effects to an effect of a structure.
-   This gives a generic path from many small checks to one checked result.
-   Traversable is the stronger tool here: it can rebuild the container, not just collapse it.
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

<div class="notes" id="orgdafee1d">
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

<div class="notes" id="org7767bd6">
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

<div class="notes" id="orgd8fea7c">
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

<div class="notes" id="org9e49be9">
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

<div class="notes" id="org8c221cc">
<p>
This is the most abstract law in the talk, so I want to narrate it slowly.
Change applicatives after traversal, or bake that change into the function first.
You should get the same answer either way.
That is what this test is checking.
</p>

</div>


### Traversable Commute: Range and ZipList

-   Traversable commutes a range of ZipLists into a ZipList of ranges.
-   The inverse matrix view (ZipList of vectors to vector of ZipLists) is also tested.

-   Range of ZipLists → ZipList of Ranges

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
    
    <div class="notes" id="org0e336d3">
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

<div class="notes" id="org340c516">
<p>
If the laws fail, the abstraction is just an accident of naming.
Also, tree applicative is a policy choice.
The clean teaching examples are optional, range, and ZipList.
</p>

</div>


# Monoids and Measured Trees


### Monoid Interface

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

<div class="notes" id="org0012c83">
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

<div class="notes" id="orgf994d4e">
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

<div class="notes" id="orga1e8534">
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

<div class="notes" id="orgf2b6763">
<p>
The identity law is what makes identity() useful for initializing fold accumulators.
If this fails the Monoid is not a monoid.
</p>

</div>


### Associativity as Algorithmic Leverage

-   Associativity lets us regroup work without changing results.
-   Measured trees exploit this to maintain summaries incrementally.
-   This is the bridge from algebra to explicit performance contracts.

<div class="notes" id="orgee63767">
<p>
This is where the algebra starts paying rent.
If the measure is a monoid, split and search become compositional (Ralf Hinze and Ross Paterson, 2006).
</p>

</div>


### Annotations as Summaries

-   Each node caches a measure of its subtree.
-   Measures are domain-specific: size, minimum priority, span, or cost.
-   Updating structure updates summaries locally.

<div class="notes" id="orgda7a0d3">
<p>
The data structure stays the same while behavior changes with the monoid.
</p>

</div>


### Search and Split Driven by Measures

-   Each node carries a cumulative measure; split navigates by predicate on that measure.
-   Predicate-based core split navigates tree structure in O(log n).
-   `split_at_index()` uses the structural path for count-measure trees; non-count measures fall back to flatten/rebuild.

<div class="notes" id="orge525a54">
<p>
The Hinze-Paterson paper gives the headline costs (Ralf Hinze and Ross Paterson, 2006).
In this implementation, the important thing to say out loud is where the structural fast paths are, and where they are not.
The core split is structural.
split_at_index is structural for count measures, and falls back when it has to preserve index semantics for non-count measures.
The wrapper operations build on that split-plus-concat story.
</p>

</div>


# Finger Trees as a Case Study


### Persistent Concatenation and Splitting

-   Persistent concatenation in O(log(min(n,m))).
-   Predicate-based split navigates tree structure in O(log n); wrapper operations use split + concat.
-   The API composes naturally with foldable/traversable abstractions.

<div class="notes" id="org83a0342">
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

<div class="notes" id="org88250b5">
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
    
    <div class="notes" id="orge144a1e">
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
    
    <div class="notes" id="org64ee6f0">
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
    
    <div class="notes" id="org20c07f4">
    <p>
    Monoid: byte-length. The measure at each node is the byte count of its chunk subtree.
    insert, erase, and replace all navigate by cumulative byte offset using split.
    </p>
    
    </div>


### Why This Belongs in Modern C++

-   Adding Traversable to an existing type requires no modification to the type itself — one specialization in a header.
-   The Rope, priority queue, and sequence expose the same Traversable interface, with separate per-type specializations in the current codebase.
-   The abstraction is a library choice today; it maps cleanly to pattern matching and richer generic facilities when those arrive.

<div class="notes" id="org6335fe6">
<p>
This is the concrete payoff.
Independent extension points still compose.
No monkey-patching, no reopening classes, no central registry.
</p>

</div>


# Cross-Language Name Mapping


### Functor

| C++       | Haskell | Cats (Scala) | PureScript  | Core |
|--------- |------- |------------ |----------- |---- |
| `fmap`    | `fmap`  | `map`        | `map`       | ✓    |
| `replace` | `(<$)`  | `as`         | `voidRight` |      |

<div class="notes" id="orgd869b2d">
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

<div class="notes" id="orgdcfef5e">
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

<div class="notes" id="org7f5ceff">
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

<div class="notes" id="org8663a49">
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

<div class="notes" id="org8a5ace0">
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

<div class="notes" id="orgdd2b074">
<p>
This is not decorative boilerplate.
The alternate-core pattern is load-bearing in the implementation you just saw.
The <code>using Impl::traverse;</code> declarations are how that choice is made.
</p>

</div>


# Designing APIs That Won't Age Poorly


### Library Abstractions Anticipating Language Features

-   Favor explicit, composable operations over magical overload sets.
-   Keep extension points separate from core type definitions.
-   Make future language support a simplification, not a rewrite.

<div class="notes" id="orgfd1db89">
<p>
If the language gets better, this API should get simpler.
It should not need to be replaced.
</p>

</div>


### Avoiding the `std::bind` vs. Lambda Overlap

-   Avoid parallel abstractions that solve the same use case differently.
-   Choose one clear good path per concept.
-   For Applicative, that path is `invoke`.

<div class="notes" id="org2c45824">
<p>
The goal is to reduce cognitive branching in generic code.
One good path beats three almost-equivalent ones.
</p>

</div>


### Keeping the Good Path Obvious

-   Make lawful defaults easy and alternative policies explicit.
-   Keep naming consistent across concepts.
-   Back claims with executable law tests.

<div class="notes" id="orge17470d">
<p>
The best documentation here is still the tests.
They say what the interface is allowed to mean.
</p>

</div>


# Questions?

-   **A Question:** is where YOU want more information from ME.
-   **A Question:** goes up at the end.

> "More of a comment than a question &#x2026;" hold them for a moment. I want to discuss this all with everyone, and you know where to find me.


# Comments?


# Thank You!


# Bibliography

Conor McBride and Ross Paterson (2008). *Applicative Programming with Effects*.

Ralf Hinze and Ross Paterson (2006). *Finger Trees: A Simple General-Purpose Data Structure*, Journal of Functional Programming.
