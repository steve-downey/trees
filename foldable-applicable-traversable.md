- [Algorithms For Trees](#orgbeea6d1)
  - [Abstract](#org918edaa)
  - [Foldable](#org468d11c)
  - [Applicative](#org0449231)
  - [Traversable](#orgd5247f7)
  - [Not Monadic](#org170dcc6)
- [Ranges Flatten the World](#orge86fc8b)
    - [Linearization as a design assumption](#orge4bd83d)
    - [Where structure carries meaning](#org8822d78)
    - [Trees that are not sequences](#org6a2005d)
- [Visitors, Pattern Matching, and the Missing Syntax](#org3493940)
    - [Visitor as manual recursion control](#org0833591)
    - [Pattern matching as the intended interface](#orga455330)
    - [Designing today for tomorrow’s syntax](#org639d9af)
- [Recursion Schemes You Can Actually Use](#orge3c8483)
    - [F-algebras: how to collapse one layer](#orgc55fa83)
    - [Catamorphisms as principled fold](#orgeaebcf3)
    - [Separating recursion from business logic](#orgd2732a3)
- [Preserving Shape: Traversable and Friends](#org309b024)
    - [Foldable vs Traversable: sequence vs shape](#org9813d99)
    - [Crisp contrast: flatten vs preserve shape](#org0f1a851)
    - [Typeclass object lookup in three calls](#orgb90913f)
    - [Typeclass object for implementors](#orge9762cc)
    - [How the implementation works: CRTP and deducing this](#orgab0ccf0)
    - [Same algorithm, two tree representations](#org257508d)
    - [Foldable API: one primitive, many derived operations](#orgfda377f)
    - [Foldable proof: derived operations hold in tests](#orgd10ff86)
    - [Applicative model: pure function over effectful arguments](#org184fb34)
    - [Applicative API: minimal core, user-facing invoke](#org0b168f1)
    - [Applicative proof: n-ary use in tests](#org835d2a8)
    - [Traversable model: commute shape and effect](#org777d40d)
    - [Traversable API: one primitive, many derived operations](#org2b4d668)
    - [Traversable proof: derived operations hold in tests](#orgace1d0d)
    - [Traversable commute: Range and ZipList](#org7b5130b)
    - [Laws that keep this honest](#orgfea0044)
    - [Tree Applicative as optional appendix](#org8fc0a5b)
- [Monoids and Measured Trees](#org7a29ac0)
    - [Associativity as algorithmic leverage](#orgecc1be4)
    - [Annotations as summaries](#orgd885838)
    - [Search and split driven by measures](#org54dc779)
- [Finger Trees as a Case Study](#org12e0a7d)
    - [Persistent concatenation and splitting](#org1f1424f)
    - [One structure, many interpretations](#orgb70ae5c)
    - [Why this belongs in modern C++](#orgc67ccc6)
- [Designing APIs That Won’t Age Poorly](#org9f37f89)
    - [Library abstractions anticipating language features](#org844a0d1)
    - [Avoiding the `std::bind` vs lambda overlap](#org83b58f0)
    - [Keeping the good path obvious](#orga9600fc)



<a id="orgbeea6d1"></a>

# Algorithms For Trees

-   Foldable
-   Applicative
-   Traversable


<a id="org918edaa"></a>

## Abstract

The use of the functor and monad patterns in ranges, sender-receiver, optional, and expected has been broadly and widely successful. There are other type classes that C++ can profitably adopt for use in generic programming that have proven to be useful in other languages and ecosystems in the last decade.

In particular, I am interested in better support for algorithms over trees, and other data structures, where flattening into a sequence loses too much information. In this talk, I will focus on Foldable, Applicative, and Traversable type classes, as well as Monoid, as it provides capabilities for a number of tree algorithms.

The eventual goal of this work is to provide `fingertree` to the standard library, as well as support for application domain trees in use today, such as expression evaluators and syntax trees.


<a id="org468d11c"></a>

## Foldable

-   **Foldables:** are types which can be made to look like a sequence of some sort, or a range, and support the basic `fold` operation which provides much of the power of std::ranges. Providing opt-in hooks for making a type Foldable rather than a Range is useful.


<a id="org0449231"></a>

## Applicative

-   **Applicatives:** were introduced to provide the pattern of 'pure function applied to funny arguments', where a type "supports its own peculiar way of giving meaning to the usual [notion of function invocation]." The implementation details of partially applied functions in a container turn out to be a distraction from understanding. They turn out to be widely relevant in contexts such as data parallel operations, and with less overhead than monadic operations.


<a id="orgd5247f7"></a>

## Traversable

-   **Traversables:** are generalizations of Foldables which allow preservation of the "shape" of a container, where a Foldable can only see the ordered sequence. A binary tree can be traversed and maintain the parent child relationships, where a fold can at most produce a range. Traversable also provides the ability to "commute" containers, generically, providing the ability to convert a range of tasks into a task producing a range.


<a id="org170dcc6"></a>

## Not Monadic

sorry


<a id="orge86fc8b"></a>

# Ranges Flatten the World


<a id="orge4bd83d"></a>

### Linearization as a design assumption

-   Ranges are a great default when the structure is inherently sequential.
-   Many generic algorithms quietly assume that flattening first is semantically neutral.
-   For trees, flattening throws away parent/child relationships and subtree boundaries.

<div class="notes" id="orgdac17e8">
<p>
This is the setup: flattening is a design choice, not a law of nature.
The talk is about recovering algorithms that preserve structure when structure matters.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="org8822d78"></a>

### Where structure carries meaning

-   Search paths, balancing, and decomposition points are part of the meaning.
-   The same inorder sequence can come from many different trees.
-   If we flatten too early, we lose algorithmic leverage.

<div class="notes" id="org4c929ef">
<p>
The argument is practical: preserving shape enables better APIs for split/search/relabel.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="org6a2005d"></a>

### Trees that are not sequences

-   Expression trees: hierarchy controls precedence and rewrite legality.
-   Syntax trees: children have roles, not just positions.
-   Measured trees: internal summaries drive efficient split/search.
-   Measured trees: internal summaries define split/search interfaces and optimization direction.

<div class="notes" id="org5209d67">
<p>
A range view is still useful, but it should be derived, not the primary model.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="org3493940"></a>

# Visitors, Pattern Matching, and the Missing Syntax


<a id="org0833591"></a>

### Visitor as manual recursion control

-   Visitor centralizes recursion, but at the cost of ceremony and indirection.
-   Every new operation requires another visitor type or lambda nest.
-   The control flow is explicit, but often noisy.

<div class="notes" id="org13083fc">
<p>
Visitor is not wrong; it is just too low-level for everyday algebraic operations.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="orga455330"></a>

### Pattern matching as the intended interface

-   Pattern matching expresses what cases exist directly.
-   C++ is moving in this direction, but we still need practical libraries now.
-   Typeclass-style APIs can encode the same intent with today's language.

<div class="notes" id="org4e43904">
<p>
Design now so the API maps naturally to future language features.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="org639d9af"></a>

### Designing today for tomorrow’s syntax

-   Keep recursion control in library algorithms, not business code.
-   Expose a small vocabulary: `fold_map`, `invoke`, `traverse`.
-   Make call sites read like intent, not machinery.

<div class="notes" id="orgdf1b9ed">
<p>
The point is migration-friendly design, not speculative syntax tricks.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="orge3c8483"></a>

# Recursion Schemes You Can Actually Use


<a id="orgc55fa83"></a>

### F-algebras: how to collapse one layer

-   Think of an algebra as consume one layer and summarize it.
-   The recursion pattern stays fixed while business logic changes.
-   This separation makes tree algorithms easier to reason about.

<div class="notes" id="org82c7a81">
<p>
I only need the intuition here, not full categorical development.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="orgeaebcf3"></a>

### Catamorphisms as principled fold

-   Catamorphism: apply the algebra recursively until the structure is collapsed.
-   In C++, this corresponds to a disciplined fold over a recursive representation.
-   You get reuse without hardcoding each algorithm into the node type.

<div class="notes" id="org07eb0bc">
<p>
Foldable is the operational entry point for this in everyday code.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="orgd2732a3"></a>

### Separating recursion from business logic

-   Business logic should answer how to combine results, not how to recurse.
-   This yields smaller tests and more reusable algorithms.
-   It also creates a natural place to enforce laws.

<div class="notes" id="org833fffd">
<p>
When recursion is abstracted, law tests become executable documentation.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="org309b024"></a>

# Preserving Shape: Traversable and Friends


<a id="org9813d99"></a>

### Foldable vs Traversable: sequence vs shape

-   Foldable consumes structure into a summary.
-   Traversable maps with effects while rebuilding the same outer shape.
-   For trees, this is the difference between count nodes and relabel nodes in place.


<a id="org0f1a851"></a>

### Crisp contrast: flatten vs preserve shape

-   Two differently shaped trees can flatten to the same sequence under Foldable.
-   Traversable can map values and keep the original branching shape.

1.  Foldable flattens and loses shape identity

    */ b1fd4b92-b060-4c47-8c08-97328ec02329 auto left\_flat = foldable.to\_vector(left\_heavy); auto right\_flat = foldable.to\_vector(right\_heavy); /* b1fd4b92-b060-4c47-8c08-97328ec02329 end
    
    return left\_flat == right\_flat; }
    
    } // close namespace smd::typeclass::examples :lines 2- :src cpp :end "b1fd4b92-b060-4c47-8c08-97328ec02329 end"

2.  Traversable maps while preserving shape

    */ d804ec63-77d1-4fa0-99a6-9effce6f741b auto mapped = traversable.traverse( [](int x) -> optional<int> { return optional<int>{x + 10}; }, tree); /* d804ec63-77d1-4fa0-99a6-9effce6f741b end
    
    if (!mapped || mapped->is\_leaf()) { return false; }
    
    return mapped->left().is\_leaf() && mapped->left().value() `= 11 && !mapped->right().is_leaf() && mapped->right().left().is_leaf() && mapped->right().left().value() =` 12 && mapped->right().right().is\_leaf() && mapped->right().right().value() == 13; }
    
    } // close namespace smd::typeclass::examples :lines 2- :src cpp :end "d804ec63-77d1-4fa0-99a6-9effce6f741b end"
    
    <div class="notes" id="org08b11dc">
    <p>
    Use this as the one-slide intuition.
    Foldable can collapse two different shapes to the same flat view.
    Traversable keeps the tree skeleton and only transforms payloads.
    </p>
    
    </div>


<a id="orgb90913f"></a>

### Typeclass object lookup in three calls

-   User code calls the looked-up object, not a node method.
-   Lookup is a variable-template selection such as `foldable_typeclass<Tree>`.
-   The same pattern applies to `applicative_typeclass<Context>` and `traversable_typeclass<Tree>`.
-   You can use implicit lookup, explicit object arguments, or NTTP pinning for tests and policy control.

<div class="notes" id="org1f971cc">
<p>
This replaces a long historical detour with one operational model.
Call site intent stays stable while the representation changes.
</p>

</div>


<a id="orge9762cc"></a>

### Typeclass object for implementors

-   Implement one minimal hook per concept and inherit derived operations.
-   Foldable implements `fold_map` and gets `length`, `fold_left`, `fold_right`, and `to_vector`.
-   Applicative implements `pure` and `apply` and gets user-facing `invoke`.
-   Traversable implements `traverse` and gets `for_each` and `sequence` helpers.
-   Keep traversal order and shape-preservation choices explicit in instance tests.

<div class="notes" id="orgcb3bd77">
<p>
This is the key split.
Implementor surface is small, and user surface is rich.
</p>

</div>


<a id="orgab0ccf0"></a>

### How the implementation works: CRTP and deducing this

-   Each concept wrapper is a CRTP base that exposes derived API in terms of minimal hooks.
-   `this auto&& self` preserves value category and constness through wrapper calls.
-   The wrapper can call either default derived behavior or an instance override when provided.
-   This keeps dispatch static and local while avoiding repetitive forwarding boilerplate.

<div class="notes" id="org22ea513">
<p>
CRTP supplies structure.
Deducing this keeps wrappers generic without losing type information.
</p>

</div>


<a id="org257508d"></a>

### Same algorithm, two tree representations

-   Fixpoint tree and shared\_ptr binary tree can share the same Foldable call shape.
-   The representation changes; the typeclass API and algorithm intent stay the same.

1.  Fixpoint tree

    */ 9a1c4e2b-2c7e-4b1a-9f55-8b6a4d2e91aa auto n = foldable.length(tree); /* 9a1c4e2b-2c7e-4b1a-9f55-8b6a4d2e91aa end
    
    return n; }
    
    auto generic\_length\_binary\_tree\_example() -> std::size\_t { using IntBinaryTree = smd::tree::BinaryTree<int>; auto tree = IntBinaryTree::from\_children\_ptrs( 2, IntBinaryTree::make\_ptr(IntBinaryTree::leaf(1)), IntBinaryTree::make\_ptr(IntBinaryTree::from\_children\_ptrs( 3, {}, IntBinaryTree::make\_ptr(IntBinaryTree::leaf(4)))));
    
    const auto& foldable = smd::foldable\_typeclass<IntBinaryTree>;
    
    */ 53b9f5b4-3b3a-4e18-9b3c-07b7e2c980f4 auto n = foldable.length(tree); /* 53b9f5b4-3b3a-4e18-9b3c-07b7e2c980f4 end
    
    return n; }
    
    auto generic\_length\_fringe\_tree\_example() -> std::size\_t { using Fringe = smd::tree::FringeTree<int>; auto tree = Fringe::branch( Fringe::branch(Fringe::leaf(1), Fringe::leaf(2)), Fringe::leaf(3));
    
    const auto& foldable = smd::foldable\_typeclass<Fringe>;
    
    */ 7c2f11d9-ef09-45e2-80da-9229f3c8d82c auto n = foldable.length(tree); /* 7c2f11d9-ef09-45e2-80da-9229f3c8d82c end
    
    return n; }
    
    auto foldable\_flattens\_shape\_example() -> bool { using Tree = smd::tree::FixTree<int>; auto left\_heavy = Tree::branch( Tree::leaf(1), Tree::branch(Tree::leaf(2), Tree::leaf(3))); auto right\_heavy = Tree::branch( Tree::branch(Tree::leaf(1), Tree::leaf(2)), Tree::leaf(3));
    
    const auto& foldable = smd::foldable\_typeclass<Tree>;
    
    */ b1fd4b92-b060-4c47-8c08-97328ec02329 auto left\_flat = foldable.to\_vector(left\_heavy); auto right\_flat = foldable.to\_vector(right\_heavy); /* b1fd4b92-b060-4c47-8c08-97328ec02329 end
    
    return left\_flat == right\_flat; }
    
    } // close namespace smd::typeclass::examples :lines 2- :src cpp :end "9a1c4e2b-2c7e-4b1a-9f55-8b6a4d2e91aa end"

2.  shared\_ptr binary tree

    */ 53b9f5b4-3b3a-4e18-9b3c-07b7e2c980f4 auto n = foldable.length(tree); /* 53b9f5b4-3b3a-4e18-9b3c-07b7e2c980f4 end
    
    return n; }
    
    auto generic\_length\_fringe\_tree\_example() -> std::size\_t { using Fringe = smd::tree::FringeTree<int>; auto tree = Fringe::branch( Fringe::branch(Fringe::leaf(1), Fringe::leaf(2)), Fringe::leaf(3));
    
    const auto& foldable = smd::foldable\_typeclass<Fringe>;
    
    */ 7c2f11d9-ef09-45e2-80da-9229f3c8d82c auto n = foldable.length(tree); /* 7c2f11d9-ef09-45e2-80da-9229f3c8d82c end
    
    return n; }
    
    auto foldable\_flattens\_shape\_example() -> bool { using Tree = smd::tree::FixTree<int>; auto left\_heavy = Tree::branch( Tree::leaf(1), Tree::branch(Tree::leaf(2), Tree::leaf(3))); auto right\_heavy = Tree::branch( Tree::branch(Tree::leaf(1), Tree::leaf(2)), Tree::leaf(3));
    
    const auto& foldable = smd::foldable\_typeclass<Tree>;
    
    */ b1fd4b92-b060-4c47-8c08-97328ec02329 auto left\_flat = foldable.to\_vector(left\_heavy); auto right\_flat = foldable.to\_vector(right\_heavy); /* b1fd4b92-b060-4c47-8c08-97328ec02329 end
    
    return left\_flat == right\_flat; }
    
    } // close namespace smd::typeclass::examples :lines 2- :src cpp :end "53b9f5b4-3b3a-4e18-9b3c-07b7e2c980f4 end"

3.  fringetree (simplified fingertree)

    */ 7c2f11d9-ef09-45e2-80da-9229f3c8d82c auto n = foldable.length(tree); /* 7c2f11d9-ef09-45e2-80da-9229f3c8d82c end
    
    return n; }
    
    auto foldable\_flattens\_shape\_example() -> bool { using Tree = smd::tree::FixTree<int>; auto left\_heavy = Tree::branch( Tree::leaf(1), Tree::branch(Tree::leaf(2), Tree::leaf(3))); auto right\_heavy = Tree::branch( Tree::branch(Tree::leaf(1), Tree::leaf(2)), Tree::leaf(3));
    
    const auto& foldable = smd::foldable\_typeclass<Tree>;
    
    */ b1fd4b92-b060-4c47-8c08-97328ec02329 auto left\_flat = foldable.to\_vector(left\_heavy); auto right\_flat = foldable.to\_vector(right\_heavy); /* b1fd4b92-b060-4c47-8c08-97328ec02329 end
    
    return left\_flat == right\_flat; }
    
    } // close namespace smd::typeclass::examples :lines 2- :src cpp :end "7c2f11d9-ef09-45e2-80da-9229f3c8d82c end"
    
    // 5c6b2d3e-7a44-4c8a-9c31-3d1e2a9b77c2 using beman::optional::optional;
    
    auto relabelled = traversable.traverse( [](int x) -> optional<int> { return x >= 0 ? optional<int>{x + 1} : optional<int>{}; }, tree); // 5c6b2d3e-7a44-4c8a-9c31-3d1e2a9b77c2 end
    
    if (!relabelled) { return {}; }
    
    const auto& foldable = smd::foldable\_typeclass<IntTree>; return foldable.length(\*relabelled); }
    
    auto traversable\_preserves\_shape\_example() -> bool { using IntTree = smd::tree::FixTree<int>; using beman::optional::optional;
    
    auto tree = IntTree::branch( IntTree::leaf(1), IntTree::branch(IntTree::leaf(2), IntTree::leaf(3))); const auto& traversable = smd::traversable\_typeclass<IntTree>;
    
    */ d804ec63-77d1-4fa0-99a6-9effce6f741b auto mapped = traversable.traverse( [](int x) -> optional<int> { return optional<int>{x + 10}; }, tree); /* d804ec63-77d1-4fa0-99a6-9effce6f741b end
    
    if (!mapped || mapped->is\_leaf()) { return false; }
    
    return mapped->left().is\_leaf() && mapped->left().value() `= 11 && !mapped->right().is_leaf() && mapped->right().left().is_leaf() && mapped->right().left().value() =` 12 && mapped->right().right().is\_leaf() && mapped->right().right().value() == 13; }
    
    } // close namespace smd::typeclass::examples :lines 2- :src cpp :end "5c6b2d3e-7a44-4c8a-9c31-3d1e2a9b77c2 end"


<a id="orgfda377f"></a>

### Foldable API: one primitive, many derived operations

-   Minimal implementation hook: `fold_map`.
-   User-facing operations like `length`, `fold_left`, `fold_right`, `fold`, and `to_vector` are derived.
    
    // e3a1b1a2-6adf-4cb9-8c85-c0e39a7b98f2
    
    template <class T> auto length(this auto&& self, T&& value) -> std::size\_t { const auto count = self.fold\_map( [](const auto&) { return typeclass::Count{1}; }, std::forward<T>(value)); return count.d\_value; }
    
    template <class T, class STATE, class F> auto fold\_left(this auto&& self, T&& value, STATE initial\_state, F&& function) { using StateType = remove\_cvref\_t<STATE>; auto step = std::forward<F>(function);
    
    const auto program = self.fold\_map( [&step](const auto& x) { using ValueType = remove\_cvref\_t<decltype(x)>; return detail::LeftFoldProgram<StateType>{ [x\_copy = ValueType(x), &step](StateType s) { return std::invoke(step, std::move(s), x\_copy); }}; }, std::forward<T>(value));
    
    return program(StateType(std::move(initial\_state))); }
    
    template <class T, class STATE, class F> auto fold\_right(this auto&& self, T&& value, STATE initial\_state, F&& function) { using StateType = remove\_cvref\_t<STATE>; auto step = std::forward<F>(function);
    
    const auto program = self.fold\_map( [&step](const auto& x) { using ValueType = remove\_cvref\_t<decltype(x)>; return detail::RightFoldProgram<StateType>{ [x\_copy = ValueType(x), &step](StateType s) { return std::invoke(step, x\_copy, std::move(s)); }}; }, std::forward<T>(value));
    
    return program(StateType(std::move(initial\_state))); }
    
    template <class T> auto combine\_all(this auto&& self, T&& value) { return self.fold\_map([](const auto& x) { return x; }, std::forward<T>(value)); }
    
    template <class T> auto fold(this auto&& self, T&& value) { return self.combine\_all(std::forward<T>(value)); }
    
    template <class T, class PREDICATE> auto any\_of(this auto&& self, T&& value, PREDICATE&& predicate) -> bool { const auto result = self.fold\_map( [&predicate](const auto& x) { return detail::Any{std::invoke(predicate, x)}; }, std::forward<T>(value));
    
    return result.d\_value; }
    
    template <class T, class PREDICATE> auto all\_of(this auto&& self, T&& value, PREDICATE&& predicate) -> bool { const auto result = self.fold\_map( [&predicate](const auto& x) { return detail::All{std::invoke(predicate, x)}; }, std::forward<T>(value));
    
    return result.d\_value; }
    
    template <class T> auto empty(this auto&& self, T&& value) -> bool { return !self.any\_of(std::forward<T>(value), [](const auto&) { return true; }); }
    
    template <class T> auto to\_vector(this auto&& self, T&& value) { return self.fold\_map( [](const auto& x) { using ValueType = remove\_cvref\_t<decltype(x)>; return std::vector<ValueType>{x}; }, std::forward<T>(value)); } // e3a1b1a2-6adf-4cb9-8c85-c0e39a7b98f2 end
    
    template <class T, class PREDICATE> auto find\_first(this auto&& self, T&& value, PREDICATE&& predicate) { const auto result = self.fold\_map( [&predicate](const auto& x) { using X = remove\_cvref\_t<decltype(x)>; if (std::invoke(predicate, x)) { return detail::First<X>{{x}}; } return detail::First<X>{{}}; }, std::forward<T>(value));
    
    return result.d\_value; }

};

template <class T> inline constexpr auto foldable\_typeclass = std::false\_type{};

} // close namespace smd

\#endif :lines 2- :src cpp :end "e3a1b1a2-6adf-4cb9-8c85-c0e39a7b98f2 end"


<a id="orgd10ff86"></a>

### Foldable proof: derived operations hold in tests

-   Derived operations agree operationally with the `fold_map` contract.
    
    // 4c8a5f77-8a62-4f1b-a9cf-95452c4b8ea4 using IntSequence = smd::typeclass::test::Sequence<int>; auto sequence = IntSequence{{1, 2, 3}}; const auto& int\_foldable = smd::foldable\_typeclass<IntSequence>;
    
    const auto as\_vector = int\_foldable.to\_vector(sequence); CHECK(as\_vector == (std::vector<int>{1, 2, 3}));
    
    using VectorSequence = smd::typeclass::test::Sequence<std::vector<int> >; auto vectors = VectorSequence{{{1, 2}, {3}}}; const auto& vector\_foldable = smd::foldable\_typeclass<VectorSequence>; const auto combined = vector\_foldable.combine\_all(vectors); CHECK(combined == (std::vector<int>{1, 2, 3}));
    
    const auto folded = vector\_foldable.fold(vectors); CHECK(folded == (std::vector<int>{1, 2, 3})); // 4c8a5f77-8a62-4f1b-a9cf-95452c4b8ea4 end

}

TEST\_CASE("FoldableTypeclassTest - AllOfAndFindFirstEdgeCases") { using Sequence = smd::typeclass::test::Sequence<int>; const auto& foldable = smd::foldable\_typeclass<Sequence>;

auto mixed = Sequence{{2, -1, 4}}; CHECK\_FALSE(foldable.all\_of(mixed, [](int x) { return x > 0; }));

auto found\_even = foldable.find\_first(mixed, [](int x) { return x % 2 `= 0; }); REQUIRE(found_even.has_value()); CHECK(*found_even =` 2);

auto found\_large = foldable.find\_first(mixed, [](int x) { return x > 100; }); CHECK\_FALSE(found\_large.has\_value()); } :lines 2- :src cpp :end "4c8a5f77-8a62-4f1b-a9cf-95452c4b8ea4 end"


<a id="org184fb34"></a>

### Applicative model: pure function over effectful arguments

-   Applicative captures independent effectful arguments without full monadic sequencing.
-   It is enough for lawful traversal in many practical cases.
-   In this talk, the convincing applicative instances are optional, ranges, and ZipList.
-   Tree applicative semantics are possible, but not the main motivation here.
-   Public API should read as ordinary invocation over effectful values.
-   Prefer `invoke` for C++ audiences; it aligns with the `std::invoke` mental model.
-   This follows McBride's "apply pure functions in effectful contexts" reading, with normalization to `pure` + `apply` under the covers.
-   `apply_pure(f, a, b, c)` is retained only as a teaching alias for audiences coming from FP notation like `[| f a b c |]`.
    
    */ 3f0c8d0e-9a6b-4a3e-9c2a-0c1e9c3d4f11 auto sum = applicative.invoke( [](int a, int b, int c) { return a + b + c; }, ax, ay, az); /* 3f0c8d0e-9a6b-4a3e-9c2a-0c1e9c3d4f11 end
    
    return sum;

}

} // close namespace smd::typeclass::examples :lines 2- :src cpp :end "3f0c8d0e-9a6b-4a3e-9c2a-0c1e9c3d4f11 end"


<a id="org0b168f1"></a>

### Applicative API: minimal core, user-facing invoke

-   Minimal implementation hook: `pure` and `apply`.
-   Future-facing user spelling is `invoke` over effectful arguments.
-   `apply_pure` is retained as a pedagogical alias.
    
    */ a11f7d8b-8f89-4f3e-9c92-f9f08ab7ef11 /* Teaching-friendly alias for "apply pure function to effectful arguments". // Prefer invoke as the primary C++ spelling (std::invoke model). template <class FUNCTION, class FIRST\_ARGUMENT, class&hellip; REST\_ARGUMENTS> auto apply\_pure(this auto&& self, FUNCTION&& function, FIRST\_ARGUMENT&& first\_argument, REST\_ARGUMENTS&&&hellip; rest\_arguments) { return self.invoke(std::forward<FUNCTION>(function), std::forward<FIRST\_ARGUMENT>(first\_argument), std::forward<REST\_ARGUMENTS>(rest\_arguments)&hellip;); }
    
    template <class FUNCTION, class FIRST\_ARGUMENT, class&hellip; REST\_ARGUMENTS> auto invoke(this auto&& self, FUNCTION&& function, FIRST\_ARGUMENT&& first\_argument, REST\_ARGUMENTS&&&hellip; rest\_arguments) { using SELF = std::remove\_reference\_t<decltype(self)>; using IMPL\_BASE = std::conditional\_t<std::is\_const\_v<SELF>, const Impl, Impl>;
    
    if constexpr (requires(IMPL\_BASE& impl) { impl.invoke(std::forward<FUNCTION>(function), std::forward<FIRST\_ARGUMENT>(first\_argument), std::forward<REST\_ARGUMENTS>(rest\_arguments)&hellip;); }) { return static\_cast<IMPL\_BASE&>(self).invoke( std::forward<FUNCTION>(function), std::forward<FIRST\_ARGUMENT>(first\_argument), std::forward<REST\_ARGUMENTS>(rest\_arguments)&hellip;); } else { auto lifted\_function = self.pure(detail::make\_terminating\_partial(std::forward<FUNCTION>(function))); return self.apply\_chain( self.ap(std::move(lifted\_function), std::forward<FIRST\_ARGUMENT>(first\_argument)), std::forward<REST\_ARGUMENTS>(rest\_arguments)&hellip;); } } // a11f7d8b-8f89-4f3e-9c92-f9f08ab7ef11 end
    
    private: template <class ACCUMULATED> auto apply\_chain(this auto&&, ACCUMULATED&& accumulated) { return std::forward<ACCUMULATED>(accumulated); }
    
    template <class ACCUMULATED, class NEXT\_ARGUMENT, class&hellip; REST\_ARGUMENTS> auto apply\_chain(this auto&& self, ACCUMULATED&& accumulated, NEXT\_ARGUMENT&& next\_argument, REST\_ARGUMENTS&&&hellip; rest\_arguments) { auto next = self.ap(std::forward<ACCUMULATED>(accumulated), std::forward<NEXT\_ARGUMENT>(next\_argument)); if constexpr (sizeof&hellip;(REST\_ARGUMENTS) == 0) { return next; } else { return self.apply\_chain(std::move(next), std::forward<REST\_ARGUMENTS>(rest\_arguments)&hellip;); } }
    
    public: template <class FUNCTION, class ARGUMENT> auto map(this auto&& self, FUNCTION&& function, ARGUMENT&& argument) { return self.invoke(std::forward<FUNCTION>(function), std::forward<ARGUMENT>(argument)); }
    
    template <class VALUE> auto lift(this auto&& self, VALUE&& value) { return self.pure(std::forward<VALUE>(value)); }
    
    template <class FUNCTION\_IN\_CONTEXT, class ARGUMENT\_IN\_CONTEXT> auto ap(this auto&& self, FUNCTION\_IN\_CONTEXT&& function, ARGUMENT\_IN\_CONTEXT&& argument) { return self.apply(std::forward<FUNCTION\_IN\_CONTEXT>(function), std::forward<ARGUMENT\_IN\_CONTEXT>(argument)); }
    
    template <class FUNCTION, class FIRST\_ARGUMENT, class SECOND\_ARGUMENT> auto zip\_with(this auto&& self, FUNCTION&& function, FIRST\_ARGUMENT&& first\_argument, SECOND\_ARGUMENT&& second\_argument) { return self.invoke(std::forward<FUNCTION>(function), std::forward<FIRST\_ARGUMENT>(first\_argument), std::forward<SECOND\_ARGUMENT>(second\_argument)); }
    
    template <class FIRST\_ARGUMENT, class SECOND\_ARGUMENT> auto discard\_first(this auto&& self, FIRST\_ARGUMENT&& first\_argument, SECOND\_ARGUMENT&& second\_argument) { return self.invoke( [](const auto&, auto&& rhs) { return std::forward<decltype(rhs)>(rhs); }, std::forward<FIRST\_ARGUMENT>(first\_argument), std::forward<SECOND\_ARGUMENT>(second\_argument)); }
    
    template <class FIRST\_ARGUMENT, class SECOND\_ARGUMENT> auto discard\_second(this auto&& self, FIRST\_ARGUMENT&& first\_argument, SECOND\_ARGUMENT&& second\_argument) { return self.invoke( [](auto&& lhs, const auto&) { return std::forward<decltype(lhs)>(lhs); }, std::forward<FIRST\_ARGUMENT>(first\_argument), std::forward<SECOND\_ARGUMENT>(second\_argument)); }
    
    template <class APPLICATIVE\_MAP, class FUNCTION, class FIRST\_ARGUMENT, class&hellip; REST\_ARGUMENTS> auto invoke\_with(this auto&&, const APPLICATIVE\_MAP& applicative\_map, FUNCTION&& function, FIRST\_ARGUMENT&& first\_argument, REST\_ARGUMENTS&&&hellip; rest\_arguments) { return applicative\_map.invoke(std::forward<FUNCTION>(function), std::forward<FIRST\_ARGUMENT>(first\_argument), std::forward<REST\_ARGUMENTS>(rest\_arguments)&hellip;); }
    
    template <class APPLICATIVE\_MAP, class FUNCTION, class FIRST\_ARGUMENT, class&hellip; REST\_ARGUMENTS> auto apply\_pure\_with(this auto&&, const APPLICATIVE\_MAP& applicative\_map, FUNCTION&& function, FIRST\_ARGUMENT&& first\_argument, REST\_ARGUMENTS&&&hellip; rest\_arguments) { return applicative\_map.invoke(std::forward<FUNCTION>(function), std::forward<FIRST\_ARGUMENT>(first\_argument), std::forward<REST\_ARGUMENTS>(rest\_arguments)&hellip;); }
    
    template <const auto& APPLICATIVE\_MAP, class FUNCTION, class FIRST\_ARGUMENT, class&hellip; REST\_ARGUMENTS> auto invoke\_with(this auto&&, FUNCTION&& function, FIRST\_ARGUMENT&& first\_argument, REST\_ARGUMENTS&&&hellip; rest\_arguments) { return APPLICATIVE\_MAP.invoke(std::forward<FUNCTION>(function), std::forward<FIRST\_ARGUMENT>(first\_argument), std::forward<REST\_ARGUMENTS>(rest\_arguments)&hellip;); }
    
    template <const auto& APPLICATIVE\_MAP, class FUNCTION, class FIRST\_ARGUMENT, class&hellip; REST\_ARGUMENTS> auto apply\_pure\_with(this auto&&, FUNCTION&& function, FIRST\_ARGUMENT&& first\_argument, REST\_ARGUMENTS&&&hellip; rest\_arguments) { return APPLICATIVE\_MAP.invoke(std::forward<FUNCTION>(function), std::forward<FIRST\_ARGUMENT>(first\_argument), std::forward<REST\_ARGUMENTS>(rest\_arguments)&hellip;); }

};

template <class T> inline constexpr auto applicative\_typeclass = std::false\_type{};

template <class VALUE\_TYPE> struct OptionalApplicativeImpl { template <class VALUE> auto pure(this auto&&, VALUE&& value) -> std::optional<remove\_cvref\_t<VALUE> > { return std::optional<remove\_cvref\_t<VALUE> >{std::forward<VALUE>(value)}; }

template <class FUNCTION\_IN\_CONTEXT, class ARGUMENT\_IN\_CONTEXT> auto apply(this auto&&, FUNCTION\_IN\_CONTEXT&& function, ARGUMENT\_IN\_CONTEXT&& argument) { using Result = std::invoke\_result\_t<decltype(\*function), decltype(\*argument)>;

if (!function || !argument) { return std::optional<remove\_cvref\_t<Result> >{}; }

return std::optional<remove\_cvref\_t<Result> >{ std::invoke(\*std::forward<FUNCTION\_IN\_CONTEXT>(function), \*std::forward<ARGUMENT\_IN\_CONTEXT>(argument))}; } };

template <class VALUE\_TYPE> struct OptionalApplicativeMap : Applicative<OptionalApplicativeImpl<VALUE\_TYPE> > { using OptionalApplicativeImpl<VALUE\_TYPE>::apply; using OptionalApplicativeImpl<VALUE\_TYPE>::pure; };

template <class VALUE\_TYPE> requires(!std::same\_as<beman::optional::optional<VALUE\_TYPE>, std::optional<VALUE\_TYPE> >) struct BemanOptionalApplicativeImpl { template <class VALUE> auto pure(this auto&&, VALUE&& value) -> beman::optional::optional<remove\_cvref\_t<VALUE> > { return beman::optional::optional<remove\_cvref\_t<VALUE> >{ std::forward<VALUE>(value)}; }

template <class FUNCTION\_IN\_CONTEXT, class ARGUMENT\_IN\_CONTEXT> auto apply(this auto&&, FUNCTION\_IN\_CONTEXT&& function, ARGUMENT\_IN\_CONTEXT&& argument) { using Result = std::invoke\_result\_t<decltype(\*function), decltype(\*argument)>;

if (!function || !argument) { return beman::optional::optional<remove\_cvref\_t<Result> >{}; }

return beman::optional::optional<remove\_cvref\_t<Result> >{ std::invoke(\*std::forward<FUNCTION\_IN\_CONTEXT>(function), \*std::forward<ARGUMENT\_IN\_CONTEXT>(argument))}; } };

template <class VALUE\_TYPE> requires(!std::same\_as<beman::optional::optional<VALUE\_TYPE>, std::optional<VALUE\_TYPE> >) struct BemanOptionalApplicativeMap

    Applicative<BemanOptionalApplicativeImpl<VALUE_TYPE> > {

using BemanOptionalApplicativeImpl<VALUE\_TYPE>::apply; using BemanOptionalApplicativeImpl<VALUE\_TYPE>::pure; };

template <class VALUE\_TYPE> inline constexpr auto applicative\_typeclass<std::optional<VALUE\_TYPE> > = OptionalApplicativeMap<VALUE\_TYPE>{};

template <class VALUE\_TYPE> requires(!std::same\_as<beman::optional::optional<VALUE\_TYPE>, std::optional<VALUE\_TYPE> >) inline constexpr auto applicative\_typeclass<beman::optional::optional<VALUE\_TYPE> > = BemanOptionalApplicativeMap<VALUE\_TYPE>{};

} // close namespace smd

\#endif :lines 2- :src cpp :end "a11f7d8b-8f89-4f3e-9c92-f9f08ab7ef11 end"


<a id="org835d2a8"></a>

### Applicative proof: n-ary use in tests

-   The same API handles arity > 2 without per-call-site plumbing.
    
    // 6e8bde7b-a9f1-4c98-8f1a-807d9ee0a93b std::optional<int> ax{2}; std::optional<int> ay{3}; std::optional<int> az{4}; const auto& applicative = smd::applicative\_typeclass<std::optional<int> >;
    
    auto result = applicative.apply\_pure( [](int a, int b, int c) { return a \* b + c; }, ax, ay, az); REQUIRE(result.has\_value()); CHECK(\*result == 10); // 6e8bde7b-a9f1-4c98-8f1a-807d9ee0a93b end

}

TEST\_CASE("ApplicativeTypeclassTest - MapOptional") { std::optional<int> value{21}; const auto& applicative = smd::applicative\_typeclass<std::optional<int> >;

auto result = applicative.map([](int x) { return x \* 2; }, value); REQUIRE(result.has\_value()); CHECK(\*result == 42); }

TEST\_CASE("ApplicativeTypeclassTest - InvokeWithExplicitMap") { std::optional<int> ax{10}; std::optional<int> ay{5}; const auto& default\_applicative = smd::applicative\_typeclass<std::optional<int> >; const auto& optional\_applicative = smd::applicative\_typeclass<std::optional<int> >;

auto result = default\_applicative.invoke\_with( optional\_applicative, [](int a, int b) { return a + b; }, ax, ay); REQUIRE(result.has\_value()); CHECK(\*result == 15); }

TEST\_CASE("ApplicativeTypeclassTest - OptionalEmptyPaths") { const auto& applicative = smd::applicative\_typeclass<std::optional<int> >;

std::optional<int (\*)(int)> no\_function{}; std::optional<int> argument{4}; auto no\_function\_result = applicative.apply(no\_function, argument); CHECK\_FALSE(no\_function\_result.has\_value());

std::optional<int (\*)(int)> function{+[](int x) { return x + 3; }}; std::optional<int> no\_argument{}; auto no\_argument\_result = applicative.apply(function, no\_argument); CHECK\_FALSE(no\_argument\_result.has\_value());

std::optional<int> ax{1}; std::optional<int> ay{}; auto invoke\_result = applicative.invoke([](int a, int b) { return a + b; }, ax, ay); CHECK\_FALSE(invoke\_result.has\_value()); }

TEST\_CASE("ApplicativeTypeclassTest - DerivedOperations") { const auto& applicative = smd::applicative\_typeclass<std::optional<int> >;

auto lifted = applicative.lift(9); REQUIRE(lifted.has\_value()); CHECK(\*lifted == 9);

std::optional<int (\*)(int)> function{+[](int x) { return x \* 3; }}; auto applied = applicative.ap(function, std::optional<int>{7}); REQUIRE(applied.has\_value()); CHECK(\*applied == 21);

auto zipped = applicative.zip\_with( [](int a, int b) { return a \* b; }, std::optional<int>{6}, std::optional<int>{5}); REQUIRE(zipped.has\_value()); CHECK(\*zipped == 30);

auto keep\_right = applicative.discard\_first(std::optional<int>{1}, std::optional<int>{2}); REQUIRE(keep\_right.has\_value()); CHECK(\*keep\_right == 2);

auto keep\_left = applicative.discard\_second(std::optional<int>{1}, std::optional<int>{2}); REQUIRE(keep\_left.has\_value()); CHECK(\*keep\_left == 1); }

TEST\_CASE("ApplicativeTypeclassTest - InvokeWithNttpMap") { const auto& default\_applicative = smd::applicative\_typeclass<std::optional<int> >;

auto result = default\_applicative.invoke\_with< smd::applicative\_typeclass<std::optional<int> >>( [](int a, int b, int c) { return a + b + c; }, std::optional<int>{1}, std::optional<int>{2}, std::optional<int>{3}); REQUIRE(result.has\_value()); CHECK(\*result == 6);

auto apply\_pure\_result = default\_applicative.apply\_pure\_with< smd::applicative\_typeclass<std::optional<int> >>( [](int a, int b) { return a - b; }, std::optional<int>{8}, std::optional<int>{5}); REQUIRE(apply\_pure\_result.has\_value()); CHECK(\*apply\_pure\_result == 3); }

TEST\_CASE("ApplicativeTypeclassTest - BemanOptional") { using BemanOptional = beman::optional::optional<int>; const auto& applicative = smd::applicative\_typeclass<BemanOptional>;

auto lifted = applicative.pure(11); REQUIRE(lifted.has\_value()); CHECK(\*lifted == 11);

beman::optional::optional<int (\*)(int)> function{+[](int x) { return x + 5; }}; BemanOptional argument{7}; auto applied = applicative.apply(function, argument); REQUIRE(applied.has\_value()); CHECK(\*applied == 12);

beman::optional::optional<int (\*)(int)> no\_function{}; auto no\_function\_applied = applicative.apply(no\_function, argument); CHECK\_FALSE(no\_function\_applied.has\_value());

BemanOptional no\_argument{}; auto no\_argument\_applied = applicative.apply(function, no\_argument); CHECK\_FALSE(no\_argument\_applied.has\_value());

auto invoked = applicative.invoke( [](int a, int b) { return a \* b; }, BemanOptional{3}, BemanOptional{4}); REQUIRE(invoked.has\_value()); CHECK(\*invoked == 12);

auto empty\_invoked = applicative.invoke( [](int a, int b) { return a \* b; }, BemanOptional{}, BemanOptional{4}); CHECK\_FALSE(empty\_invoked.has\_value()); }

TEST\_CASE("ApplicativeTypeclassTest - ApplyPureWithExplicitMap") { const auto& default\_applicative = smd::applicative\_typeclass<std::optional<int> >; const auto& optional\_applicative = smd::applicative\_typeclass<std::optional<int> >;

auto result = default\_applicative.apply\_pure\_with( optional\_applicative, [](int a, int b, int c) { return a + b + c; }, std::optional<int>{4}, std::optional<int>{5}, std::optional<int>{6}); REQUIRE(result.has\_value()); CHECK(\*result == 15); }

TEST\_CASE("ApplicativeTypeclassTest - TerminatingPartialExtendsAndInvokes") { auto partial = smd::detail::make\_terminating\_partial( [](int a, int b, int c) { return a \* 100 + b \* 10 + c; });

auto partial2 = partial(1); auto partial3 = partial2(2); CHECK(partial3(3) == 123);

const auto const\_partial = smd::detail::make\_terminating\_partial( [](int a, int b) { return a - b; }); auto const\_partial2 = const\_partial(9); const auto const\_partial3 = const\_partial2; CHECK(const\_partial3(4) == 5); }

TEST\_CASE("ApplicativeTypeclassTest - IdentityMapUsesDerivedInvokePath") { using Identity = smd::typeclass::test::Identity<int>; const auto& applicative = smd::applicative\_typeclass<Identity>;

auto binary = applicative.invoke( [](int a, int b) { return a + b; }, Identity{2}, Identity{3}); CHECK(binary.value == 5);

auto ternary = applicative.apply\_pure( [](int a, int b, int c) { return a \* 100 + b \* 10 + c; }, Identity{1}, Identity{2}, Identity{3}); CHECK(ternary.value == 123); }

TEST\_CASE("ApplicativeTypeclassTest - CustomInvokeDispatchPath") { const auto& default\_applicative = smd::applicative\_typeclass<std::optional<int> >;

auto result = default\_applicative.invoke\_with( direct\_invoke\_map, [](int a, int b, int c) { return a + b + c; }, smd::typeclass::test::Identity<int>{4}, smd::typeclass::test::Identity<int>{5}, smd::typeclass::test::Identity<int>{6}); CHECK(result.value == 15);

auto nttp\_result = default\_applicative.invoke\_with<direct\_invoke\_map>( [](int a, int b) { return a \* b; }, smd::typeclass::test::Identity<int>{7}, smd::typeclass::test::Identity<int>{8}); CHECK(nttp\_result.value == 56); }

TEST\_CASE("ApplicativeTypeclassTest - OptionalAndBemanVectorInstantiationPaths") { const auto& optional\_applicative = smd::applicative\_typeclass<std::optional<std::vector<int> > >;

auto lifted\_vector = optional\_applicative.pure(std::vector<int>{1, 2, 3}); REQUIRE(lifted\_vector.has\_value()); CHECK(lifted\_vector->size() == 3);

std::optional<std::vector<int> (\*)(std::vector<int>)> append\_value{ +[](std::vector<int> v) { v.push\_back(4); return v; }}; auto applied\_vector = optional\_applicative.apply(append\_value, lifted\_vector); REQUIRE(applied\_vector.has\_value()); CHECK(applied\_vector->size() == 4);

using BemanVectorOptional = beman::optional::optional<std::vector<int> >; const auto& beman\_applicative = smd::applicative\_typeclass<BemanVectorOptional>;

auto beman\_lifted = beman\_applicative.pure(std::vector<int>{8, 9}); REQUIRE(beman\_lifted.has\_value()); CHECK(beman\_lifted->size() == 2);

beman::optional::optional<std::vector<int> (\*)(std::vector<int>)> beman\_append{ +[](std::vector<int> v) { v.push\_back(10); return v; }}; auto beman\_applied = beman\_applicative.apply(beman\_append, beman\_lifted); REQUIRE(beman\_applied.has\_value()); CHECK(beman\_applied->size() == 3); }

TEST\_CASE("ApplicativeTypeclassTest - IdentityWrapperMethods") { using Identity = smd::typeclass::test::Identity<int>; const auto& applicative = smd::applicative\_typeclass<Identity>;

auto mapped = applicative.map([](int x) { return x + 1; }, Identity{9}); CHECK(mapped.value == 10);

auto zipped = applicative.zip\_with( [](int a, int b) { return a - b; }, Identity{20}, Identity{3}); CHECK(zipped.value == 17);

auto ap\_result = applicative.ap( smd::typeclass::test::Identity<int (\*)(int)>{+[](int x) { return x \* 5; }}, Identity{6}); CHECK(ap\_result.value == 30); }

TEST\_CASE("ApplicativeTypeclassTest - BareIdentityInvokeAndApplyChain") { using BareIdentity = smd::typeclass::test::BareIdentity<int>; const auto& applicative = smd::applicative\_typeclass<BareIdentity>;

auto unary = applicative.invoke([](int x) { return x + 1; }, BareIdentity{4}); CHECK(unary.value == 5);

auto ternary = applicative.invoke( [](int a, int b, int c) { return a \* b + c; }, BareIdentity{2}, BareIdentity{3}, BareIdentity{4}); CHECK(ternary.value == 10);

auto quaternary = applicative.apply\_pure( [](int a, int b, int c, int d) { return a + b + c + d; }, BareIdentity{1}, BareIdentity{2}, BareIdentity{3}, BareIdentity{4}); CHECK(quaternary.value == 10); }

TEST\_CASE("ApplicativeTypeclassTest - BareIdentityWrapperCoverage") { using BareIdentity = smd::typeclass::test::BareIdentity<int>; const auto& applicative = smd::applicative\_typeclass<BareIdentity>;

auto lifted = applicative.lift(33); CHECK(lifted.value == 33);

auto mapped = applicative.map([](int x) { return x \* 2; }, BareIdentity{11}); CHECK(mapped.value == 22);

auto applied = applicative.ap( smd::typeclass::test::BareIdentity<int (\*)(int)>{+[](int x) { return x - 2; }}, BareIdentity{9}); CHECK(applied.value == 7);

auto zipped = applicative.zip\_with( [](int a, int b) { return a - b; }, BareIdentity{40}, BareIdentity{8}); CHECK(zipped.value == 32);

auto keep\_right = applicative.discard\_first(BareIdentity{5}, BareIdentity{6}); CHECK(keep\_right.value == 6);

auto keep\_left = applicative.discard\_second(BareIdentity{5}, BareIdentity{6}); CHECK(keep\_left.value == 5); }

TEST\_CASE("ApplicativeTypeclassTest - BareIdentityInvokeWithMapCoverage") { using BareIdentity = smd::typeclass::test::BareIdentity<int>; const auto& default\_applicative = smd::applicative\_typeclass<std::optional<int> >; const auto& bare\_identity\_applicative = smd::applicative\_typeclass<BareIdentity>;

auto explicit\_map\_result = default\_applicative.invoke\_with( bare\_identity\_applicative, [](int a, int b, int c) { return a + b + c; }, BareIdentity{3}, BareIdentity{4}, BareIdentity{5}); CHECK(explicit\_map\_result.value == 12);

auto explicit\_apply\_pure\_result = default\_applicative.apply\_pure\_with( bare\_identity\_applicative, [](int a, int b) { return a \* b; }, BareIdentity{7}, BareIdentity{6}); CHECK(explicit\_apply\_pure\_result.value == 42);

auto nttp\_map\_result = default\_applicative.invoke\_with<bare\_identity\_applicative>( [](int a, int b) { return a - b; }, BareIdentity{20}, BareIdentity{9}); CHECK(nttp\_map\_result.value == 11);

auto nttp\_apply\_pure\_result = default\_applicative.apply\_pure\_with<bare\_identity\_applicative>( [](int a, int b, int c) { return a + b \* c; }, BareIdentity{2}, BareIdentity{3}, BareIdentity{4}); CHECK(nttp\_apply\_pure\_result.value == 14); }

TEST\_CASE("ApplicativeTypeclassTest - BareIdentityTypeMatrixCoverage") { run\_bare\_identity\_matrix\_case<int, short, unsigned>(3, 4, 5U); run\_bare\_identity\_matrix\_case<long, int, long long>(10L, 20, 30LL); run\_bare\_identity\_matrix\_case<float, double, int>(1.5F, 2.25, 3); }

TEST\_CASE("ApplicativeBehaviorTest - OptionalIdentityHomomorphismAndInvoke") { CHECK(smd::typeclass::test::check\_applicative\_identity\_law(std::optional<int>{8})); CHECK(smd::typeclass::test::check\_applicative\_homomorphism\_law<std::optional<int> >( +[](int x) { return x + 3; }, 5)); CHECK(smd::typeclass::test::check\_applicative\_invoke\_binary\_law( [](int a, int b) { return a \* 10 + b; }, std::optional<int>{2}, std::optional<int>{7})); }

TEST\_CASE("ApplicativeBehaviorTest - BareIdentityIdentityHomomorphismAndInvoke") { using BareIdentity = smd::typeclass::test::BareIdentity<int>; CHECK(smd::typeclass::test::check\_applicative\_identity\_law(BareIdentity{11})); CHECK(smd::typeclass::test::check\_applicative\_homomorphism\_law<BareIdentity>( +[](int x) { return x \* 4; }, 3)); CHECK(smd::typeclass::test::check\_applicative\_invoke\_binary\_law( [](int a, int b) { return a - b; }, BareIdentity{20}, BareIdentity{6})); }

TEST\_CASE("ApplicativeBehaviorTest - BemanIdentityHomomorphismAndInvoke") { using BemanOptional = beman::optional::optional<int>;

CHECK(smd::typeclass::test::check\_applicative\_identity\_law(BemanOptional{11})); CHECK(smd::typeclass::test::check\_applicative\_homomorphism\_law<BemanOptional>( +[](int x) { return x \* 4; }, 3)); CHECK(smd::typeclass::test::check\_applicative\_invoke\_binary\_law( [](int a, int b) { return a - b; }, BemanOptional{20}, BemanOptional{6})); }

TEST\_CASE("ApplicativeBehaviorTest - OptionalShortCircuit") { const auto& applicative = smd::applicative\_typeclass<std::optional<int> >;

std::optional<std::function<int(int)> > no\_function{}; auto no\_function\_result = applicative.ap(no\_function, std::optional<int>{4}); CHECK\_FALSE(no\_function\_result.has\_value());

std::optional<std::function<int(int)> > function{ [](int x) { return x + 1; }}; auto no\_argument\_result = applicative.ap(function, std::optional<int>{}); CHECK\_FALSE(no\_argument\_result.has\_value());

int calls = 0; auto invoke\_result = applicative.invoke( [&calls](int lhs, int rhs) { ++calls; return lhs + rhs; }, std::optional<int>{3}, std::optional<int>{}); CHECK\_FALSE(invoke\_result.has\_value()); CHECK(calls == 0); }

TEST\_CASE("ApplicativeBehaviorTest - BemanShortCircuit") { using BemanOptional = beman::optional::optional<int>; const auto& applicative = smd::applicative\_typeclass<BemanOptional>;

beman::optional::optional<std::function<int(int)> > no\_function{}; auto no\_function\_result = applicative.ap(no\_function, BemanOptional{5}); CHECK\_FALSE(no\_function\_result.has\_value());

beman::optional::optional<std::function<int(int)> > function{ [](int x) { return x \* 2; }}; auto no\_argument\_result = applicative.ap(function, BemanOptional{}); CHECK\_FALSE(no\_argument\_result.has\_value());

int calls = 0; auto invoke\_result = applicative.invoke( [&calls](int lhs, int rhs) { ++calls; return lhs - rhs; }, BemanOptional{9}, BemanOptional{}); CHECK\_FALSE(invoke\_result.has\_value()); CHECK(calls == 0); }

TEST\_CASE("ApplicativeBehaviorTest - InvokeDispatchThroughBaseAndDerivedPaths") { DirectInvokeIdentityApplicativeMap<int> custom\_map{}; auto& custom\_base = static\_cast<smd::Applicative<DirectInvokeIdentityApplicativeImpl<int> >&>( custom\_map);

auto custom\_dispatched = custom\_base.invoke( [](int a, int b, int c) { return a + b + c; }, smd::typeclass::test::Identity<int>{1}, smd::typeclass::test::Identity<int>{2}, smd::typeclass::test::Identity<int>{3}); CHECK(custom\_dispatched.value == 6);

smd::BareIdentityApplicativeMap<int> bare\_map{}; auto& bare\_base = static\_cast<smd::Applicative<smd::BareIdentityApplicativeImpl<int> >&>(bare\_map);

auto derived\_dispatched = bare\_base.invoke( [](int a, int b, int c) { return a \* 100 + b \* 10 + c; }, smd::typeclass::test::BareIdentity<int>{4}, smd::typeclass::test::BareIdentity<int>{5}, smd::typeclass::test::BareIdentity<int>{6}); CHECK(derived\_dispatched.value == 456); }

TEST\_CASE("ApplicativeBehaviorTest - BareIdentityConstAndNonConstInvokeApMap") { smd::BareIdentityApplicativeMap<int> mutable\_map{}; auto& mutable\_base = static\_cast<smd::Applicative<smd::BareIdentityApplicativeImpl<int> >&>(mutable\_map);

auto non\_const\_invoke = mutable\_base.invoke( [](int a, int b) { return a + b; }, smd::typeclass::test::BareIdentity<int>{10}, smd::typeclass::test::BareIdentity<int>{4}); CHECK(non\_const\_invoke.value == 14);

auto non\_const\_map = mutable\_base.map( [](int x) { return x \* 3; }, smd::typeclass::test::BareIdentity<int>{7}); CHECK(non\_const\_map.value == 21);

auto non\_const\_ap = mutable\_base.ap( smd::typeclass::test::BareIdentity<std::function<int(int)> >{ [](int x) { return x - 5; }}, smd::typeclass::test::BareIdentity<int>{12}); CHECK(non\_const\_ap.value == 7);

const smd::BareIdentityApplicativeMap<int> const\_map{}; const auto& const\_base = static\_cast<const smd::Applicative<smd::BareIdentityApplicativeImpl<int> >&>( const\_map);

auto const\_invoke = const\_base.invoke( [](int a, int b, int c) { return a \* b + c; }, smd::typeclass::test::BareIdentity<int>{3}, smd::typeclass::test::BareIdentity<int>{5}, smd::typeclass::test::BareIdentity<int>{2}); CHECK(const\_invoke.value == 17);

auto const\_map\_result = const\_base.map( [](int x) { return x + 8; }, smd::typeclass::test::BareIdentity<int>{1}); CHECK(const\_map\_result.value == 9);

auto const\_ap\_result = const\_base.ap( smd::typeclass::test::BareIdentity<std::function<int(int)> >{ [](int x) { return x \* x; }}, smd::typeclass::test::BareIdentity<int>{6}); CHECK(const\_ap\_result.value == 36); } :lines 2- :src cpp :end "6e8bde7b-a9f1-4c98-8f1a-807d9ee0a93b end"


<a id="org777d40d"></a>

### Traversable model: commute shape and effect

-   Traversal commutes shape and effect: from shape of effects to effect of shape.
-   This gives a generic path from many small checks to one checked result.
-   Use this to model validation, partial relabeling, and structured transformations.


<a id="org2b4d668"></a>

### Traversable API: one primitive, many derived operations

-   Minimal implementation hook: `traverse`.
-   `for_each`, `sequence`, and `sequence_with` are derived helper operations.
    
    // 8f1d5c4a-1a7e-4b9e-8cb4-908f4ab0ca11
    
    template <class T, class F> auto for\_each(this auto&& self, T&& value, F&& function) { return self.traverse(std::forward<F>(function), std::forward<T>(value)); }
    
    template <class T> auto sequence(this auto&& self, T&& value) { return self.traverse( [](auto&& x) { return std::forward<decltype(x)>(x); }, std::forward<T>(value)); }
    
    template <class TRAVERSABLE\_MAP, class T, class F> auto traverse\_with(this auto&&, const TRAVERSABLE\_MAP& traversable\_map, F&& function, T&& value) { return traversable\_map.traverse(std::forward<F>(function), std::forward<T>(value)); }
    
    template <class TRAVERSABLE\_MAP, class T> auto sequence\_with(this auto&& self, const TRAVERSABLE\_MAP& traversable\_map, T&& value) { return self.traverse\_with( traversable\_map, [](auto&& x) { return std::forward<decltype(x)>(x); }, std::forward<T>(value)); } // 8f1d5c4a-1a7e-4b9e-8cb4-908f4ab0ca11 end

};

template <class T> inline constexpr auto traversable\_typeclass = std::false\_type{};

} // close namespace smd

\#endif :lines 2- :src cpp :end "8f1d5c4a-1a7e-4b9e-8cb4-908f4ab0ca11 end"


<a id="orgace1d0d"></a>

### Traversable proof: derived operations hold in tests

-   Sequencing works both via implicit lookup and explicit object selection.
    
    // f1de12e0-2287-4568-98c7-75be4f6f7446 using IdentityOpt = smd::typeclass::test::Identity<std::optional<int> >; auto identity = IdentityOpt{std::optional<int>{1}}; const auto& traversable = smd::traversable\_typeclass<IdentityOpt>;
    
    auto sequenced = traversable.sequence(identity); REQUIRE(sequenced.has\_value()); CHECK(sequenced->value == 1);
    
    auto sequenced\_with = traversable.sequence\_with(traversable, identity); REQUIRE(sequenced\_with.has\_value()); CHECK(sequenced\_with->value == 1); // f1de12e0-2287-4568-98c7-75be4f6f7446 end

}

TEST\_CASE("TraversableTypeclassTest - ForEachMatchesTraverse") { using Identity = smd::typeclass::test::Identity<int>; auto identity = Identity{4}; const auto& traversable = smd::traversable\_typeclass<Identity>;

auto via\_traverse = traversable.traverse( [](int x) -> std::optional<int> { return std::optional<int>{x + 7}; }, identity); auto via\_for\_each = traversable.for\_each( identity, [](int x) -> std::optional<int> { return std::optional<int>{x + 7}; });

CHECK(via\_traverse == via\_for\_each); }

TEST\_CASE("TraversableTypeclassTest - SequenceMatchesTraverseIdentity") { using IdentityOpt = smd::typeclass::test::Identity<std::optional<int> >; auto identity = IdentityOpt{std::optional<int>{5}}; const auto& traversable = smd::traversable\_typeclass<IdentityOpt>;

auto via\_sequence = traversable.sequence(identity); auto via\_traverse\_identity = traversable.traverse( [](auto&& x) { return std::forward<decltype(x)>(x); }, identity);

CHECK(via\_sequence == via\_traverse\_identity); } :lines 2- :src cpp :end "f1de12e0-2287-4568-98c7-75be4f6f7446 end"


<a id="org7b5130b"></a>

### Traversable commute: Range and ZipList

-   Traversable commutes a range of ZipLists into a ZipList of ranges.
-   The inverse matrix view (ZipList of vectors to vector of ZipLists) is also tested.
    
    // 0e9a7d13-9082-4b9e-b93f-86ef0e0ba20a using Zip = smd::zip\_list<int>; auto values = smd::ranges::from\_vector(std::vector<Zip>{ Zip{{1, 2, 3}}, Zip{{10, 20}}, Zip{{100, 200, 300, 400}}});
    
    const auto& traversable = smd::traversable\_typeclass<decltype(values)>; auto sequenced = traversable.sequence(values);
    
    REQUIRE(sequenced.data.size() `= 2U); CHECK(collect(sequenced.data[0]) =` (std::vector<int>{1, 10, 100})); CHECK(collect(sequenced.data[1]) == (std::vector<int>{2, 20, 200})); // 0e9a7d13-9082-4b9e-b93f-86ef0e0ba20a end

}

TEST\_CASE("RangeTraversableTest - SequenceConvertsRangeOfZiplistsToZiplistOfRangesLengthFive") { using Zip = smd::zip\_list<int>; auto values = smd::ranges::from\_vector(std::vector<Zip>{ Zip{{1, 2, 3, 4, 5}}, Zip{{10, 20, 30, 40, 50}}, Zip{{100, 200, 300, 400, 500}}});

const auto& traversable = smd::traversable\_typeclass<decltype(values)>; auto sequenced = traversable.sequence(values);

REQUIRE(sequenced.data.size() `= 5U); CHECK(collect(sequenced.data[0]) =` (std::vector<int>{1, 10, 100})); CHECK(collect(sequenced.data[1]) `= (std::vector<int>{2, 20, 200})); CHECK(collect(sequenced.data[2]) =` (std::vector<int>{3, 30, 300})); CHECK(collect(sequenced.data[3]) `= (std::vector<int>{4, 40, 400})); CHECK(collect(sequenced.data[4]) =` (std::vector<int>{5, 50, 500})); }

TEST\_CASE("RangeTraversableTest - ConvertZiplistOfVectorsToVectorOfZiplists") { // 4be89584-35cc-4933-b3de-6d524d54371d smd::zip\_list<std::vector<int> > zip\_of\_vectors{ {{1, 10, 100}, {2, 20, 200}}};

auto as\_rows = to\_vector\_of\_ziplists(zip\_of\_vectors);

REQUIRE(as\_rows.size() `= 3U); CHECK(as_rows[0].data =` (std::vector<int>{1, 2})); CHECK(as\_rows[1].data `= (std::vector<int>{10, 20})); CHECK(as_rows[2].data =` (std::vector<int>{100, 200})); // 4be89584-35cc-4933-b3de-6d524d54371d end }

TEST\_CASE("RangeTraversableTest - ConvertZiplistOfVectorsToVectorOfZiplistsLengthFive") { smd::zip\_list<std::vector<int> > zip\_of\_vectors{ {{1, 10, 100, 1000, 10000}, {2, 20, 200, 2000, 20000}}};

auto as\_rows = to\_vector\_of\_ziplists(zip\_of\_vectors);

REQUIRE(as\_rows.size() `= 5U); CHECK(as_rows[0].data =` (std::vector<int>{1, 2})); CHECK(as\_rows[1].data `= (std::vector<int>{10, 20})); CHECK(as_rows[2].data =` (std::vector<int>{100, 200})); CHECK(as\_rows[3].data `= (std::vector<int>{1000, 2000})); CHECK(as_rows[4].data =` (std::vector<int>{10000, 20000})); } :lines 2- :src cpp :end "0e9a7d13-9082-4b9e-b93f-86ef0e0ba20a end"

// 4be89584-35cc-4933-b3de-6d524d54371d smd::zip\_list<std::vector<int> > zip\_of\_vectors{ {{1, 10, 100}, {2, 20, 200}}};

auto as\_rows = to\_vector\_of\_ziplists(zip\_of\_vectors);

REQUIRE(as\_rows.size() `= 3U); CHECK(as_rows[0].data =` (std::vector<int>{1, 2})); CHECK(as\_rows[1].data `= (std::vector<int>{10, 20})); CHECK(as_rows[2].data =` (std::vector<int>{100, 200})); // 4be89584-35cc-4933-b3de-6d524d54371d end }

TEST\_CASE("RangeTraversableTest - ConvertZiplistOfVectorsToVectorOfZiplistsLengthFive") { smd::zip\_list<std::vector<int> > zip\_of\_vectors{ {{1, 10, 100, 1000, 10000}, {2, 20, 200, 2000, 20000}}};

auto as\_rows = to\_vector\_of\_ziplists(zip\_of\_vectors);

REQUIRE(as\_rows.size() `= 5U); CHECK(as_rows[0].data =` (std::vector<int>{1, 2})); CHECK(as\_rows[1].data `= (std::vector<int>{10, 20})); CHECK(as_rows[2].data =` (std::vector<int>{100, 200})); CHECK(as\_rows[3].data `= (std::vector<int>{1000, 2000})); CHECK(as_rows[4].data =` (std::vector<int>{10000, 20000})); } :lines 2- :src cpp :end "4be89584-35cc-4933-b3de-6d524d54371d end"

<div class="notes" id="orga9a6bcd">
<p>
Key law intuition: preserve shape and evaluation order discipline.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="orgfea0044"></a>

### Laws that keep this honest

-   Target law set:
    -   Applicative: identity, composition, homomorphism, interchange.
    -   Traversable: identity, naturality, composition.
    -   Foldable: derived operations agree with `fold_map`.
-   Current automated checks in this repository:
    -   Applicative: identity, homomorphism, invoke/ap equivalence, plus ZipList interchange and composition.
    -   Traversable: traverse/for\_each/sequence coherence and ZipList-Range commute examples.
    -   Foldable: derived operations (`length`, `fold_left`, `fold_right`, `to_vector`, predicates) are exercised against `fold_map`-based behavior.

<div class="notes" id="org37be295">
<p>
If these fail, abstractions become accidental APIs rather than reliable interfaces.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="org8fc0a5b"></a>

### Tree Applicative as optional appendix

-   Treat tree applicative as a policy choice, not the core applicative story.
-   If presented, keep it brief and explicitly label semantics.
-   Mainline examples should stay with optional, ranges, and ZipList.

<div class="notes" id="org7ec0c4d">
<p>
This avoids spending scarce slide time on semantics debates.
The core teaching value of Applicative is already visible in optional/range/ZipList examples.
</p>

</div>


<a id="org7a29ac0"></a>

# Monoids and Measured Trees


<a id="orgecc1be4"></a>

### Associativity as algorithmic leverage

-   Associativity lets us regroup work without changing results.
-   Measured trees exploit this to maintain summaries incrementally.
-   This is the bridge from algebra to explicit performance contracts.

<div class="notes" id="orge891cfa">
<p>
If the measure is a monoid, split/search become compositional.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="orgd885838"></a>

### Annotations as summaries

-   Each node caches a measure of its subtree.
-   Measures are domain-specific: size, min priority, span, or cost.
-   Updating structure updates summaries locally.

<div class="notes" id="org213923f">
<p>
The data structure stays the same while behavior changes with the monoid.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="org54dc779"></a>

### Search and split driven by measures

-   Search is currently implemented by linear scan over the flattened sequence.
-   Split currently follows the first predicate flip in that linear scan.
-   This yields one structure with many interpretations.

<div class="notes" id="org949de3e">
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


<a id="org12e0a7d"></a>

# Finger Trees as a Case Study


<a id="org1f1424f"></a>

### Persistent concatenation and splitting

-   Current prototype gives cheap persistence-friendly concatenation.
-   Current split/search paths are correct with linear-time upper bounds.
-   The API is designed so split/search can be optimized later without changing call sites.
-   The API naturally composes with foldable/traversable abstractions.

<div class="notes" id="org4f56d85">
<p>
This is where abstractions meet implementation reality.
Paper-level target bounds remain the north star.
Current prototype contract is explicit linear split/search.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="orgb70ae5c"></a>

### One structure, many interpretations

-   Change the monoid, change the interpretation.
-   Same implementation can model sequence, priority queue, or rope.
-   Reuse is semantic, not just syntactic.

<div class="notes" id="orgb74a671">
<p>
This is the strongest argument for measured trees in a standard library context.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="orgc67ccc6"></a>

### Why this belongs in modern C++

-   Zero-cost abstractions and strong typing fit this design.
-   Multiple paradigms can coexist: value types, OO boundaries, generic algorithms.
-   This is not import Haskell; it is idiomatic modern C++ with better algebraic interfaces.

<div class="notes" id="orgc8a0d6d">
<p>
Pragmatic conclusion: values first, identity where required, and laws where possible.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="org9f37f89"></a>

# Designing APIs That Won’t Age Poorly


<a id="org844a0d1"></a>

### Library abstractions anticipating language features

-   Favor explicit, composable operations over magical overload sets.
-   Keep extension points separate from core type definitions.
-   Make future language support a simplification, not a rewrite.

<div class="notes" id="org21ee89d">
<p>
Pattern matching and richer generic facilities should refine this API, not replace it.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="org83b58f0"></a>

### Avoiding the `std::bind` vs lambda overlap

-   Avoid parallel abstractions that solve the same use case differently.
-   Choose one clear good path per concept.
-   For Applicative, that path is `invoke`; `apply_pure` remains a teaching aid.

<div class="notes" id="orge679c75">
<p>
The goal is reducing cognitive branching in generic code.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="orga9600fc"></a>

### Keeping the good path obvious

-   Make lawful defaults easy and alternate policies explicit.
-   Keep naming consistent across concepts.
-   Back claims with executable law tests.

<div class="notes" id="org221e605">
<p>
The best API docs in this space are tests that encode the laws.
Source: (Steve Downey, 2026).
</p>

</div>
