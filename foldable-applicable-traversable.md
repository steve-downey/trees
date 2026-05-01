- [Algorithms For Trees](#org75341ce)
  - [Abstract](#org813cdf0)
  - [Foldable](#orgf889d2c)
  - [Applicative](#orgbcb4681)
  - [Traversable](#orgbff1762)
  - [Not Monadic](#org0d4779a)
- [Ranges Flatten the World](#orgda67de6)
    - [Linearization as a design assumption](#org9734ebc)
    - [Where structure carries meaning](#org6304542)
    - [Trees that are not sequences](#org2aecea6)
- [Visitors, Pattern Matching, and the Missing Syntax](#org090ec35)
    - [Visitor as manual recursion control](#org26526cb)
    - [Pattern matching as the intended interface](#org56573e2)
    - [Designing today for tomorrow’s syntax](#orge95c14d)
- [Recursion Schemes You Can Actually Use](#org39d309b)
    - [F-algebras: how to collapse one layer](#org72f8c01)
    - [Catamorphisms as principled fold](#org487d07a)
    - [Separating recursion from business logic](#orgbb1c1c1)
- [Preserving Shape: Traversable and Friends](#org8330312)
    - [Foldable vs Traversable: sequence vs shape](#orga011d15)
    - [Crisp contrast: flatten vs preserve shape](#org96e9f54)
    - [Typeclass object lookup in three calls](#org3f915c9)
    - [Typeclass object for implementors](#orgd0858c4)
    - [How the implementation works: CRTP and deducing this](#org5dd7d7e)
    - [Same algorithm, two tree representations](#org90ec02b)
    - [Foldable API: one primitive, many derived operations](#orgdbffbd5)
    - [Foldable proof: derived operations hold in tests](#orgc0933bc)
    - [Applicative model: pure function over effectful arguments](#org04f2495)
    - [Applicative API: minimal core, user-facing invoke](#org4401e1e)
    - [Applicative proof: n-ary use in tests](#org5cc3857)
    - [Traversable model: commute shape and effect](#org4c8bc93)
    - [Traversable API: one primitive, many derived operations](#org5b0539e)
    - [Traversable proof: derived operations hold in tests](#org376a618)
    - [Traversable commute: Range and ZipList](#orgb425adc)
    - [Laws that keep this honest](#orgd445e86)
    - [Tree Applicative as optional appendix](#org3f386dd)
- [Monoids and Measured Trees](#org715517e)
    - [Associativity as algorithmic leverage](#orgf87e0e2)
    - [Annotations as summaries](#orgeba2a5c)
    - [Search and split driven by measures](#orgee18d59)
- [Finger Trees as a Case Study](#org380c5d5)
    - [Persistent concatenation and splitting](#org61bd5a3)
    - [One structure, many interpretations](#orgab9ecf2)
    - [Why this belongs in modern C++](#org3875930)
- [Designing APIs That Won’t Age Poorly](#org046984c)
    - [Library abstractions anticipating language features](#orga10bca3)
    - [Avoiding the `std::bind` vs lambda overlap](#org293b48b)
    - [Keeping the good path obvious](#org737b61c)



<a id="org75341ce"></a>

# Algorithms For Trees

-   Foldable
-   Applicative
-   Traversable


<a id="org813cdf0"></a>

## Abstract

The use of the functor and monad patterns in ranges, sender-receiver, optional, and expected has been broadly and widely successful. There are other type classes that C++ can profitably adopt for use in generic programming that have proven to be useful in other languages and ecosystems in the last decade.

In particular, I am interested in better support for algorithms over trees, and other data structures, where flattening into a sequence loses too much information. In this talk, I will focus on Foldable, Applicative, and Traversable type classes, as well as Monoid, as it provides capabilities for a number of tree algorithms.

The eventual goal of this work is to provide `fingertree` to the standard library, as well as support for application domain trees in use today, such as expression evaluators and syntax trees.


<a id="orgf889d2c"></a>

## Foldable

-   **Foldables:** are types which can be made to look like a sequence of some sort, or a range, and support the basic `fold` operation which provides much of the power of std::ranges. Providing opt-in hooks for making a type Foldable rather than a Range is useful.


<a id="orgbcb4681"></a>

## Applicative

-   **Applicatives:** were introduced to provide the pattern of 'pure function applied to funny arguments', where a type "supports its own peculiar way of giving meaning to the usual [notion of function invocation]." The implementation details of partially applied functions in a container turn out to be a distraction from understanding. They turn out to be widely relevant in contexts such as data parallel operations, and often require less sequencing machinery than monadic formulations for independent effects.


<a id="orgbff1762"></a>

## Traversable

-   **Traversables:** are generalizations of Foldables which allow preservation of the "shape" of a container, where a Foldable can only see the ordered sequence. A binary tree can be traversed and maintain the parent child relationships, where a fold can at most produce a range. Traversable also provides the ability to "commute" containers, generically, providing the ability to convert a range of tasks into a task producing a range.


<a id="org0d4779a"></a>

## Not Monadic

sorry


<a id="orgda67de6"></a>

# Ranges Flatten the World


<a id="org9734ebc"></a>

### Linearization as a design assumption

-   Ranges are a great default when the structure is inherently sequential.
-   Many generic algorithms quietly assume that flattening first is semantically neutral.
-   For trees, flattening throws away parent/child relationships and subtree boundaries.

<div class="notes" id="org5aa1706">
<p>
This is the setup: flattening is a design choice, not a law of nature.
The talk is about recovering algorithms that preserve structure when structure matters.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="org6304542"></a>

### Where structure carries meaning

-   Search paths, balancing, and decomposition points are part of the meaning.
-   The same inorder sequence can come from many different trees.
-   If we flatten too early, we lose algorithmic leverage.

<div class="notes" id="org0c99293">
<p>
The argument is practical: preserving shape enables better APIs for split/search/relabel.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="org2aecea6"></a>

### Trees that are not sequences

-   Expression trees: hierarchy controls precedence and rewrite legality.
-   Syntax trees: children have roles, not just positions.
-   Measured trees: internal summaries drive efficient split/search.
-   Measured trees: internal summaries define split/search interfaces and optimization direction.

<div class="notes" id="orgd453c21">
<p>
A range view is still useful, but it should be derived, not the primary model.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="org090ec35"></a>

# Visitors, Pattern Matching, and the Missing Syntax


<a id="org26526cb"></a>

### Visitor as manual recursion control

-   Visitor centralizes recursion, but at the cost of ceremony and indirection.
-   Every new operation requires another visitor type or lambda nest.
-   The control flow is explicit, but often noisy.

<div class="notes" id="orgea5c020">
<p>
Visitor is not wrong; it is just too low-level for everyday algebraic operations.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="org56573e2"></a>

### Pattern matching as the intended interface

-   Pattern matching expresses what cases exist directly.
-   C++ is moving in this direction, but we still need practical libraries now.
-   Typeclass-style APIs can encode the same intent with today's language.

<div class="notes" id="orgda94fa2">
<p>
Design now so the API maps naturally to future language features.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="orge95c14d"></a>

### Designing today for tomorrow’s syntax

-   Keep recursion control in library algorithms, not business code.
-   Expose a small vocabulary: `fold_map`, `invoke`, `traverse`.
-   Make call sites read like intent, not machinery.

<div class="notes" id="org4e4d3a8">
<p>
The point is migration-friendly design, not speculative syntax tricks.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="org39d309b"></a>

# Recursion Schemes You Can Actually Use


<a id="org72f8c01"></a>

### F-algebras: how to collapse one layer

-   Think of an algebra as consume one layer and summarize it.
-   The recursion pattern stays fixed while business logic changes.
-   This separation makes tree algorithms easier to reason about.

<div class="notes" id="orgcf18762">
<p>
I only need the intuition here, not full categorical development.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="org487d07a"></a>

### TODO Catamorphisms as principled fold

-   Catamorphism: apply the algebra recursively until the structure is collapsed.
-   In C++, this corresponds to a disciplined fold over a recursive representation.
-   You get reuse without hardcoding each algorithm into the node type.

<div class="notes" id="org445a701">
<p>
Foldable is the operational entry point for this in everyday code.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="orgbb1c1c1"></a>

### Separating recursion from business logic

-   Business logic should answer how to combine results, not how to recurse.
-   This yields smaller tests and more reusable algorithms.
-   It also creates a natural place to enforce laws.

<div class="notes" id="org18de2bc">
<p>
When recursion is abstracted, law tests become executable documentation.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="org8330312"></a>

# Preserving Shape: Traversable and Friends


<a id="orga011d15"></a>

### Foldable vs Traversable: sequence vs shape

-   Foldable consumes structure into a summary.
-   Traversable maps with effects while rebuilding the same outer shape.
-   For trees, this is the difference between count nodes and relabel nodes in place.


<a id="org96e9f54"></a>

### Crisp contrast: flatten vs preserve shape

-   Two differently shaped trees can flatten to the same sequence under Foldable.
-   Traversable can map values and keep the original branching shape.

1.  Foldable flattens and loses shape identity

    ```cpp
    auto left_flat = foldable.to_vector(left_heavy);
    auto right_flat = foldable.to_vector(right_heavy);
    ```

2.  Traversable maps while preserving shape

    ```cpp
    auto mapped = traversable.traverse(
        [](int x) -> optional<int> { return optional<int>{x + 10}; },
        tree);
    ```
    
    <div class="notes" id="org9100ddd">
    <p>
    Use this as the one-slide intuition.
    Foldable can collapse two different shapes to the same flat view.
    Traversable keeps the tree skeleton and only transforms payloads.
    </p>
    
    </div>


<a id="org3f915c9"></a>

### Typeclass object lookup in three calls

-   User code calls the looked-up object, not a node method.
-   Lookup is a variable-template selection such as `foldable_typeclass<Tree>`.
-   The same pattern applies to `applicative_typeclass<Context>` and `traversable_typeclass<Tree>`.
-   You can use implicit lookup, explicit object arguments, or NTTP pinning for tests and policy control.

<div class="notes" id="org9854a8f">
<p>
This replaces a long historical detour with one operational model.
Call site intent stays stable while the representation changes.
</p>

</div>


<a id="orgd0858c4"></a>

### Typeclass object for implementors

-   Implement one minimal hook per concept and inherit derived operations.
-   Foldable implements `fold_map` and gets `length`, `fold_left`, `fold_right`, and `to_vector`.
-   Applicative implements `pure` and `apply` and gets user-facing `invoke`.
-   Traversable implements `traverse` and gets `for_each` and `sequence` helpers.
-   Keep traversal order and shape-preservation choices explicit in instance tests.

<div class="notes" id="org96c9d3f">
<p>
This is the key split.
Implementor surface is small, and user surface is rich.
</p>

</div>


<a id="org5dd7d7e"></a>

### How the implementation works: CRTP and deducing this

-   Each concept wrapper is a CRTP base that exposes derived API in terms of minimal hooks.
-   `this auto&& self` preserves value category and constness through wrapper calls.
-   The wrapper can call either default derived behavior or an instance override when provided.
-   This keeps dispatch static and local while avoiding repetitive forwarding boilerplate.

<div class="notes" id="org1e16597">
<p>
CRTP supplies structure.
Deducing this keeps wrappers generic without losing type information.
</p>

</div>


<a id="org90ec02b"></a>

### Same algorithm, two tree representations

-   Fixpoint tree and shared\_ptr binary tree can share the same Foldable call shape.
-   The representation changes; the typeclass API and algorithm intent stay the same.

1.  Fixpoint tree

    ```cpp
    auto n = foldable.length(tree);
    ```

2.  shared\_ptr binary tree

    ```cpp
    auto n = foldable.length(tree);
    ```

3.  fringetree (simplified fingertree)

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


<a id="orgdbffbd5"></a>

### Foldable API: one primitive, many derived operations

-   In this repository, the minimal implementation hook is `fold_map`.
-   User-facing operations like `length`, `fold_left`, `fold_right`, `fold`, and `to_vector` are derived.

```cpp

template <class T>
auto length(this auto&& self, T&& value) -> std::size_t
{
  const auto count = self.fold_map(
    [](const auto&) { return typeclass::Count{1}; },
    std::forward<T>(value));
  return count.d_value;
}

template <class T, class STATE, class F>
auto fold_left(this auto&& self, T&& value, STATE initial_state, F&& function)
{
  using StateType = remove_cvref_t<STATE>;
  auto step = std::forward<F>(function);

  const auto program = self.fold_map(
    [&step](const auto& x) {
        using ValueType = remove_cvref_t<decltype(x)>;
        return detail::LeftFoldProgram<StateType>{
          [x_copy = ValueType(x), &step](StateType s) {
            return std::invoke(step, std::move(s), x_copy);
          }};
    },
    std::forward<T>(value));

  return program(StateType(std::move(initial_state)));
}

template <class T, class STATE, class F>
auto fold_right(this auto&& self, T&& value, STATE initial_state, F&& function)
{
  using StateType = remove_cvref_t<STATE>;
  auto step = std::forward<F>(function);

  const auto program = self.fold_map(
    [&step](const auto& x) {
        using ValueType = remove_cvref_t<decltype(x)>;
        return detail::RightFoldProgram<StateType>{
          [x_copy = ValueType(x), &step](StateType s) {
            return std::invoke(step, x_copy, std::move(s));
          }};
    },
    std::forward<T>(value));

  return program(StateType(std::move(initial_state)));
}

template <class T>
auto combine_all(this auto&& self, T&& value)
{
  return self.fold_map([](const auto& x) { return x; },
               std::forward<T>(value));
}

template <class T>
auto fold(this auto&& self, T&& value)
{
  return self.combine_all(std::forward<T>(value));
}

template <class T, class PREDICATE>
auto any_of(this auto&& self, T&& value, PREDICATE&& predicate) -> bool
{
  const auto result = self.fold_map(
    [&predicate](const auto& x) {
        return detail::Any{std::invoke(predicate, x)};
    },
    std::forward<T>(value));

  return result.d_value;
}

template <class T, class PREDICATE>
auto all_of(this auto&& self, T&& value, PREDICATE&& predicate) -> bool
{
  const auto result = self.fold_map(
    [&predicate](const auto& x) {
        return detail::All{std::invoke(predicate, x)};
    },
    std::forward<T>(value));

  return result.d_value;
}

template <class T>
auto empty(this auto&& self, T&& value) -> bool
{
  return !self.any_of(std::forward<T>(value), [](const auto&) {
    return true;
  });
}

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


<a id="orgc0933bc"></a>

### Foldable proof: derived operations hold in tests

-   Derived operations agree operationally with the `fold_map` contract.

```cpp
using IntSequence = smd::typeclass::test::Sequence<int>;
auto sequence = IntSequence{{1, 2, 3}};
const auto& int_foldable = smd::foldable_typeclass<IntSequence>;

const auto as_vector = int_foldable.to_vector(sequence);
CHECK(as_vector == (std::vector<int>{1, 2, 3}));

using VectorSequence = smd::typeclass::test::Sequence<std::vector<int> >;
auto vectors = VectorSequence{{{1, 2}, {3}}};
const auto& vector_foldable = smd::foldable_typeclass<VectorSequence>;
const auto combined = vector_foldable.combine_all(vectors);
CHECK(combined == (std::vector<int>{1, 2, 3}));

const auto folded = vector_foldable.fold(vectors);
CHECK(folded == (std::vector<int>{1, 2, 3}));
```


<a id="org04f2495"></a>

### Applicative model: pure function over effectful arguments

-   Applicative captures independent effectful arguments without full monadic sequencing.
-   It is enough for lawful traversal in many practical cases.
-   In this talk, the convincing applicative instances are optional, ranges, and ZipList.
-   Tree applicative semantics are possible, but not the main motivation here.
-   Public API should read as ordinary invocation over effectful values.
-   Prefer `invoke` for C++ audiences; it aligns with the `std::invoke` mental model.
-   This follows McBride's "apply pure functions in effectful contexts" reading, with normalization to `pure` + `apply` under the covers.
-   `apply_pure(f, a, b, c)` is retained only as a teaching alias for audiences coming from FP notation like `[| f a b c |]`.

```cpp
auto sum = applicative.invoke(
        [](int a, int b, int c) { return a + b + c; },
        ax,
        ay,
        az);
```


<a id="org4401e1e"></a>

### Applicative API: minimal core, user-facing invoke

-   In this repository, the minimal implementation hooks are `pure` and `apply`.
-   Future-facing user spelling is `invoke` over effectful arguments.
-   `apply_pure` is retained as a pedagogical alias.

```cpp
// Teaching-friendly alias for "apply pure function to effectful arguments".
// Prefer invoke as the primary C++ spelling (std::invoke model).
template <class FUNCTION, class FIRST_ARGUMENT, class... REST_ARGUMENTS>
auto apply_pure(this auto&& self,
                  FUNCTION&& function,
                  FIRST_ARGUMENT&& first_argument,
                  REST_ARGUMENTS&&... rest_arguments)
{
  return self.invoke(std::forward<FUNCTION>(function),
                       std::forward<FIRST_ARGUMENT>(first_argument),
                       std::forward<REST_ARGUMENTS>(rest_arguments)...);
}

template <class FUNCTION, class FIRST_ARGUMENT, class... REST_ARGUMENTS>
auto invoke(this auto&& self,
              FUNCTION&& function,
              FIRST_ARGUMENT&& first_argument,
              REST_ARGUMENTS&&... rest_arguments)
{
  using SELF = std::remove_reference_t<decltype(self)>;
  using IMPL_BASE =
    std::conditional_t<std::is_const_v<SELF>, const Impl, Impl>;

  if constexpr (requires(IMPL_BASE& impl) {
                    impl.invoke(std::forward<FUNCTION>(function),
                                std::forward<FIRST_ARGUMENT>(first_argument),
                                std::forward<REST_ARGUMENTS>(rest_arguments)...);
                  }) {
    return static_cast<IMPL_BASE&>(self).invoke(
        std::forward<FUNCTION>(function),
        std::forward<FIRST_ARGUMENT>(first_argument),
        std::forward<REST_ARGUMENTS>(rest_arguments)...);
  } else {
    auto lifted_function =
        self.pure(detail::make_terminating_partial(std::forward<FUNCTION>(function)));
    return self.apply_chain(
        self.ap(std::move(lifted_function), std::forward<FIRST_ARGUMENT>(first_argument)),
        std::forward<REST_ARGUMENTS>(rest_arguments)...);
  }
}
```


<a id="org5cc3857"></a>

### Applicative proof: n-ary use in tests

-   The same API handles arity > 2 without per-call-site plumbing.

```cpp
std::optional<int> ax{2};
std::optional<int> ay{3};
std::optional<int> az{4};
const auto& applicative = smd::applicative_typeclass<std::optional<int> >;

auto result = applicative.apply_pure(
        [](int a, int b, int c) { return a * b + c; },
        ax,
        ay,
        az);
REQUIRE(result.has_value());
CHECK(*result == 10);
```


<a id="org4c8bc93"></a>

### Traversable model: commute shape and effect

-   Traversal commutes shape and effect: from shape of effects to effect of shape.
-   This gives a generic path from many small checks to one checked result.
-   Use this to model validation, partial relabeling, and structured transformations.


<a id="org5b0539e"></a>

### Traversable API: one primitive, many derived operations

-   In this repository, the minimal implementation hook is `traverse`.
-   `for_each`, `sequence`, and `sequence_with` are derived helper operations.

```cpp

template <class T, class F>
auto for_each(this auto&& self, T&& value, F&& function)
{
        return self.traverse(std::forward<F>(function),
                             std::forward<T>(value));
}

template <class T>
auto sequence(this auto&& self, T&& value)
{
        return self.traverse(
            [](auto&& x) { return std::forward<decltype(x)>(x); },
            std::forward<T>(value));
}

template <class TRAVERSABLE_MAP, class T, class F>
auto traverse_with(this auto&&,
                       const TRAVERSABLE_MAP& traversable_map,
                       F&& function,
                       T&& value)
{
        return traversable_map.traverse(std::forward<F>(function),
                                        std::forward<T>(value));
}

template <class TRAVERSABLE_MAP, class T>
auto sequence_with(this auto&& self,
                       const TRAVERSABLE_MAP& traversable_map,
                       T&& value)
{
        return self.traverse_with(
            traversable_map,
            [](auto&& x) { return std::forward<decltype(x)>(x); },
            std::forward<T>(value));
}
```


<a id="org376a618"></a>

### Traversable proof: derived operations hold in tests

-   Sequencing works both via implicit lookup and explicit object selection.

```cpp
using IdentityOpt = smd::typeclass::test::Identity<std::optional<int> >;
auto identity = IdentityOpt{std::optional<int>{1}};
const auto& traversable = smd::traversable_typeclass<IdentityOpt>;

auto sequenced = traversable.sequence(identity);
REQUIRE(sequenced.has_value());
CHECK(sequenced->value == 1);

auto sequenced_with = traversable.sequence_with(traversable, identity);
REQUIRE(sequenced_with.has_value());
CHECK(sequenced_with->value == 1);
```


<a id="orgb425adc"></a>

### Traversable commute: Range and ZipList

-   Traversable commutes a range of ZipLists into a ZipList of ranges.
-   The inverse matrix view (ZipList of vectors to vector of ZipLists) is also tested.

```cpp
using Zip = smd::zip_list<int>;
auto values = smd::ranges::from_vector(std::vector<Zip>{
        Zip{{1, 2, 3}},
        Zip{{10, 20}},
        Zip{{100, 200, 300, 400}}});

const auto& traversable = smd::traversable_typeclass<decltype(values)>;
auto sequenced = traversable.sequence(values);

REQUIRE(sequenced.data.size() == 2U);
CHECK(collect(sequenced.data[0]) == (std::vector<int>{1, 10, 100}));
CHECK(collect(sequenced.data[1]) == (std::vector<int>{2, 20, 200}));
```

```cpp
smd::zip_list<std::vector<int> > zip_of_vectors{
        {{1, 10, 100}, {2, 20, 200}}};

auto as_rows = to_vector_of_ziplists(zip_of_vectors);

REQUIRE(as_rows.size() == 3U);
CHECK(as_rows[0].data == (std::vector<int>{1, 2}));
CHECK(as_rows[1].data == (std::vector<int>{10, 20}));
CHECK(as_rows[2].data == (std::vector<int>{100, 200}));
```

<div class="notes" id="org51751b0">
<p>
Key law intuition: preserve shape and evaluation order discipline.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="orgd445e86"></a>

### Laws that keep this honest

-   Target law set:
    -   Applicative: identity, composition, homomorphism, interchange.
    -   Traversable: identity, naturality, composition.
    -   Foldable: derived operations agree with `fold_map`.
-   Current automated checks in this repository:
    -   Applicative: identity, homomorphism, invoke/ap equivalence, plus ZipList interchange and composition.
    -   Traversable: identity, naturality, composition, traverse/for\_each/sequence coherence, and ZipList-Range commute examples.
    -   Foldable: derived operations (`length`, `fold_left`, `fold_right`, `to_vector`, predicates) are exercised against `fold_map`-based behavior.

<div class="notes" id="orgd310ee1">
<p>
If these fail, abstractions become accidental APIs rather than reliable interfaces.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="org3f386dd"></a>

### Tree Applicative as optional appendix

-   Treat tree applicative as a policy choice, not the core applicative story.
-   If presented, keep it brief and explicitly label semantics.
-   Mainline examples should stay with optional, ranges, and ZipList.

<div class="notes" id="org58c272a">
<p>
This avoids spending scarce slide time on semantics debates.
The core teaching value of Applicative is already visible in optional/range/ZipList examples.
</p>

</div>


<a id="org715517e"></a>

# Monoids and Measured Trees


<a id="orgf87e0e2"></a>

### Associativity as algorithmic leverage

-   Associativity lets us regroup work without changing results.
-   Measured trees exploit this to maintain summaries incrementally.
-   This is the bridge from algebra to explicit performance contracts.

<div class="notes" id="orgc1f2fd3">
<p>
If the measure is a monoid, split/search become compositional.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="orgeba2a5c"></a>

### Annotations as summaries

-   Each node caches a measure of its subtree.
-   Measures are domain-specific: size, min priority, span, or cost.
-   Updating structure updates summaries locally.

<div class="notes" id="orgbbc8342">
<p>
The data structure stays the same while behavior changes with the monoid.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="orgee18d59"></a>

### Search and split driven by measures

-   Search is currently implemented by linear scan over the flattened sequence.
-   Split currently follows the first predicate flip in that linear scan.
-   This yields one structure with many interpretations.

<div class="notes" id="org4bddbcb">
<p>
Sequence, priority queue, and rope are policy layers on one core tree.
Original finger-tree papers promise stronger asymptotics with measured search.
Target asymptotic story from those papers is amortized O(1) at the ends,
O(log(min(n,m))) concatenation, and O(log n) split/search.
Current repository implementation keeps the same API shape but does not yet
meet those split/search bounds.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="org380c5d5"></a>

# Finger Trees as a Case Study


<a id="org61bd5a3"></a>

### Persistent concatenation and splitting

-   Current prototype gives cheap persistence-friendly concatenation.
-   Current split/search paths are correct with linear-time upper bounds.
-   The API is designed so split/search can be optimized later without changing call sites.
-   The API naturally composes with foldable/traversable abstractions.

<div class="notes" id="org1ed387b">
<p>
This is where abstractions meet implementation reality.
Paper-level target bounds remain the north star.
Current prototype contract is explicit linear split/search.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="orgab9ecf2"></a>

### One structure, many interpretations

-   Change the monoid, change the interpretation.
-   Same implementation can model sequence, priority queue, or rope.
-   Reuse is semantic, not just syntactic.

<div class="notes" id="org22cd495">
<p>
This is the strongest argument for measured trees in a standard library context.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="org3875930"></a>

### Why this belongs in modern C++

-   Zero-cost abstractions and strong typing fit this design.
-   Multiple paradigms can coexist: value types, OO boundaries, generic algorithms.
-   This is not import Haskell; it is idiomatic modern C++ with better algebraic interfaces.

<div class="notes" id="org0de8139">
<p>
Pragmatic conclusion: values first, identity where required, and laws where possible.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="org046984c"></a>

# Designing APIs That Won’t Age Poorly


<a id="orga10bca3"></a>

### Library abstractions anticipating language features

-   Favor explicit, composable operations over magical overload sets.
-   Keep extension points separate from core type definitions.
-   Make future language support a simplification, not a rewrite.

<div class="notes" id="org3626cdd">
<p>
Pattern matching and richer generic facilities should refine this API, not replace it.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="org293b48b"></a>

### Avoiding the `std::bind` vs lambda overlap

-   Avoid parallel abstractions that solve the same use case differently.
-   Choose one clear good path per concept.
-   For Applicative, that path is `invoke`; `apply_pure` remains a teaching aid.

<div class="notes" id="orga54bfb9">
<p>
The goal is reducing cognitive branching in generic code.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="org737b61c"></a>

### Keeping the good path obvious

-   Make lawful defaults easy and alternate policies explicit.
-   Keep naming consistent across concepts.
-   Back claims with executable law tests.

<div class="notes" id="orgbd0f9dd">
<p>
The best API docs in this space are tests that encode the laws.
Source: (Steve Downey, 2026).
</p>

</div>
