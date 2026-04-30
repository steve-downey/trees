- [Algorithms For Trees](#orgb37fb63)
  - [Abstract](#orgf0e6afa)
  - [Foldable](#org6783486)
  - [Applicative](#orgddd31c5)
  - [Traversable](#org9cb506e)
  - [Not Monadic](#org303537a)
- [Ranges Flatten the World](#org9e44a78)
    - [Linearization as a design assumption](#org879df3d)
    - [Where structure carries meaning](#orgd0ce2e9)
    - [Trees that are not sequences](#orga23d43a)
- [Visitors, Pattern Matching, and the Missing Syntax](#orga46d17c)
    - [Visitor as manual recursion control](#org99a8a5d)
    - [Pattern matching as the intended interface](#orgc658c50)
    - [Designing today for tomorrow’s syntax](#org105033f)
- [Recursion Schemes You Can Actually Use](#orgfe5c058)
    - [F-algebras: how to collapse one layer](#org831dc82)
    - [Catamorphisms as principled fold](#orgcc6fcc1)
    - [Separating recursion from business logic](#orgda11076)
- [Preserving Shape: Traversable and Friends](#org7f71973)
    - [Foldable vs Traversable: sequence vs shape](#orge821431)
    - [Crisp contrast: flatten vs preserve shape](#org2428d9e)
    - [Typeclass object lookup in three calls](#orge4fc1c5)
    - [Typeclass object for implementors](#orgb48b097)
    - [How the implementation works: CRTP and deducing this](#org63693e4)
    - [Same algorithm, two tree representations](#org1f8de47)
    - [Foldable API: one primitive, many derived operations](#org91b0962)
    - [Foldable proof: derived operations hold in tests](#org6ac88b7)
    - [Applicative model: pure function over effectful arguments](#orgc87762a)
    - [Applicative API: minimal core, user-facing invoke](#org49f5349)
    - [Applicative proof: n-ary use in tests](#orgff838ec)
    - [Traversable model: commute shape and effect](#orgde11a93)
    - [Traversable API: one primitive, many derived operations](#orgd560936)
    - [Traversable proof: derived operations hold in tests](#org05e1f1c)
    - [Traversable commute: Range and ZipList](#org49ace6c)
    - [Laws that keep this honest](#org57e18f7)
    - [Tree Applicative as optional appendix](#org58058a6)
- [Monoids and Measured Trees](#org32e98a9)
    - [Associativity as algorithmic leverage](#org982cfc0)
    - [Annotations as summaries](#orgaf9e4d1)
    - [Search and split driven by measures](#org3fed4f6)
- [Finger Trees as a Case Study](#org7855831)
    - [Persistent concatenation and splitting](#org568d4e8)
    - [One structure, many interpretations](#org7aed6f9)
    - [Why this belongs in modern C++](#orgc953209)
- [Designing APIs That Won’t Age Poorly](#orgdbc468a)
    - [Library abstractions anticipating language features](#org9e70d01)
    - [Avoiding the `std::bind` vs lambda overlap](#org83cf7f9)
    - [Keeping the good path obvious](#orgabe34eb)



<a id="orgb37fb63"></a>

# Algorithms For Trees

-   Foldable
-   Applicative
-   Traversable


<a id="orgf0e6afa"></a>

## Abstract

The use of the functor and monad patterns in ranges, sender-receiver, optional, and expected has been broadly and widely successful. There are other type classes that C++ can profitably adopt for use in generic programming that have proven to be useful in other languages and ecosystems in the last decade.

In particular, I am interested in better support for algorithms over trees, and other data structures, where flattening into a sequence loses too much information. In this talk, I will focus on Foldable, Applicative, and Traversable type classes, as well as Monoid, as it provides capabilities for a number of tree algorithms.

The eventual goal of this work is to provide `fingertree` to the standard library, as well as support for application domain trees in use today, such as expression evaluators and syntax trees.


<a id="org6783486"></a>

## Foldable

-   **Foldables:** are types which can be made to look like a sequence of some sort, or a range, and support the basic `fold` operation which provides much of the power of std::ranges. Providing opt-in hooks for making a type Foldable rather than a Range is useful.


<a id="orgddd31c5"></a>

## Applicative

-   **Applicatives:** were introduced to provide the pattern of 'pure function applied to funny arguments', where a type "supports its own peculiar way of giving meaning to the usual [notion of function invocation]." The implementation details of partially applied functions in a container turn out to be a distraction from understanding. They turn out to be widely relevant in contexts such as data parallel operations, and with less overhead than monadic operations.


<a id="org9cb506e"></a>

## Traversable

-   **Traversables:** are generalizations of Foldables which allow preservation of the "shape" of a container, where a Foldable can only see the ordered sequence. A binary tree can be traversed and maintain the parent child relationships, where a fold can at most produce a range. Traversable also provides the ability to "commute" containers, generically, providing the ability to convert a range of tasks into a task producing a range.


<a id="org303537a"></a>

## Not Monadic

sorry


<a id="org9e44a78"></a>

# Ranges Flatten the World


<a id="org879df3d"></a>

### Linearization as a design assumption

-   Ranges are a great default when the structure is inherently sequential.
-   Many generic algorithms quietly assume that flattening first is semantically neutral.
-   For trees, flattening throws away parent/child relationships and subtree boundaries.

<div class="notes" id="orgcf847c7">
<p>
This is the setup: flattening is a design choice, not a law of nature.
The talk is about recovering algorithms that preserve structure when structure matters.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="orgd0ce2e9"></a>

### Where structure carries meaning

-   Search paths, balancing, and decomposition points are part of the meaning.
-   The same inorder sequence can come from many different trees.
-   If we flatten too early, we lose algorithmic leverage.

<div class="notes" id="org5410037">
<p>
The argument is practical: preserving shape enables better APIs for split/search/relabel.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="orga23d43a"></a>

### Trees that are not sequences

-   Expression trees: hierarchy controls precedence and rewrite legality.
-   Syntax trees: children have roles, not just positions.
-   Measured trees: internal summaries drive efficient split/search.
-   Measured trees: internal summaries define split/search interfaces and optimization direction.

<div class="notes" id="orgea6daa0">
<p>
A range view is still useful, but it should be derived, not the primary model.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="orga46d17c"></a>

# Visitors, Pattern Matching, and the Missing Syntax


<a id="org99a8a5d"></a>

### Visitor as manual recursion control

-   Visitor centralizes recursion, but at the cost of ceremony and indirection.
-   Every new operation requires another visitor type or lambda nest.
-   The control flow is explicit, but often noisy.

<div class="notes" id="org2be2f6a">
<p>
Visitor is not wrong; it is just too low-level for everyday algebraic operations.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="orgc658c50"></a>

### Pattern matching as the intended interface

-   Pattern matching expresses what cases exist directly.
-   C++ is moving in this direction, but we still need practical libraries now.
-   Typeclass-style APIs can encode the same intent with today's language.

<div class="notes" id="org634f6fb">
<p>
Design now so the API maps naturally to future language features.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="org105033f"></a>

### Designing today for tomorrow’s syntax

-   Keep recursion control in library algorithms, not business code.
-   Expose a small vocabulary: `fold_map`, `invoke`, `traverse`.
-   Make call sites read like intent, not machinery.

<div class="notes" id="orga3708d3">
<p>
The point is migration-friendly design, not speculative syntax tricks.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="orgfe5c058"></a>

# Recursion Schemes You Can Actually Use


<a id="org831dc82"></a>

### F-algebras: how to collapse one layer

-   Think of an algebra as consume one layer and summarize it.
-   The recursion pattern stays fixed while business logic changes.
-   This separation makes tree algorithms easier to reason about.

<div class="notes" id="org81f6ceb">
<p>
I only need the intuition here, not full categorical development.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="orgcc6fcc1"></a>

### Catamorphisms as principled fold

-   Catamorphism: apply the algebra recursively until the structure is collapsed.
-   In C++, this corresponds to a disciplined fold over a recursive representation.
-   You get reuse without hardcoding each algorithm into the node type.

<div class="notes" id="org7f6282d">
<p>
Foldable is the operational entry point for this in everyday code.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="orgda11076"></a>

### Separating recursion from business logic

-   Business logic should answer how to combine results, not how to recurse.
-   This yields smaller tests and more reusable algorithms.
-   It also creates a natural place to enforce laws.

<div class="notes" id="org2acefbb">
<p>
When recursion is abstracted, law tests become executable documentation.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="org7f71973"></a>

# Preserving Shape: Traversable and Friends


<a id="orge821431"></a>

### Foldable vs Traversable: sequence vs shape

-   Foldable consumes structure into a summary.
-   Traversable maps with effects while rebuilding the same outer shape.
-   For trees, this is the difference between count nodes and relabel nodes in place.


<a id="org2428d9e"></a>

### Crisp contrast: flatten vs preserve shape

-   Two differently shaped trees can flatten to the same sequence under Foldable.
-   Traversable can map values and keep the original branching shape.

1.  Foldable flattens and loses shape identity

    */ b1fd4b92-b060-4c47-8c08-97328ec02329 auto left<sub>flat</sub> = foldable.to<sub>vector</sub>(left<sub>heavy</sub>); auto right<sub>flat</sub> = foldable.to<sub>vector</sub>(right<sub>heavy</sub>); /* b1fd4b92-b060-4c47-8c08-97328ec02329 end
    
    return left<sub>flat</sub> == right<sub>flat</sub>; }
    
    } // close namespace smd::typeclass::examples :lines 2- :src cpp :end "b1fd4b92-b060-4c47-8c08-97328ec02329 end"

2.  Traversable maps while preserving shape

    */ d804ec63-77d1-4fa0-99a6-9effce6f741b auto mapped = traversable.traverse( [](int x) -> optional<int> { return optional<int>{x + 10}; }, tree); /* d804ec63-77d1-4fa0-99a6-9effce6f741b end
    
    if (!mapped || mapped->is<sub>leaf</sub>()) { return false; }
    
    return mapped->left().is<sub>leaf</sub>() && mapped->left().value() `= 11 && !mapped->right().is_leaf() && mapped->right().left().is_leaf() && mapped->right().left().value() =` 12 && mapped->right().right().is<sub>leaf</sub>() && mapped->right().right().value() == 13; }
    
    } // close namespace smd::typeclass::examples :lines 2- :src cpp :end "d804ec63-77d1-4fa0-99a6-9effce6f741b end"
    
    <div class="notes" id="org5f317af">
    <p>
    Use this as the one-slide intuition.
    Foldable can collapse two different shapes to the same flat view.
    Traversable keeps the tree skeleton and only transforms payloads.
    </p>
    
    </div>


<a id="orge4fc1c5"></a>

### Typeclass object lookup in three calls

-   User code calls the looked-up object, not a node method.
-   Lookup is a variable-template selection such as `foldable_typeclass<Tree>`.
-   The same pattern applies to `applicative_typeclass<Context>` and `traversable_typeclass<Tree>`.
-   You can use implicit lookup, explicit object arguments, or NTTP pinning for tests and policy control.

<div class="notes" id="orgbfa99f7">
<p>
This replaces a long historical detour with one operational model.
Call site intent stays stable while the representation changes.
</p>

</div>


<a id="orgb48b097"></a>

### Typeclass object for implementors

-   Implement one minimal hook per concept and inherit derived operations.
-   Foldable implements `fold_map` and gets `length`, `fold_left`, `fold_right`, and `to_vector`.
-   Applicative implements `pure` and `apply` and gets user-facing `invoke`.
-   Traversable implements `traverse` and gets `for_each` and `sequence` helpers.
-   Keep traversal order and shape-preservation choices explicit in instance tests.

<div class="notes" id="org127c6e3">
<p>
This is the key split.
Implementor surface is small, and user surface is rich.
</p>

</div>


<a id="org63693e4"></a>

### How the implementation works: CRTP and deducing this

-   Each concept wrapper is a CRTP base that exposes derived API in terms of minimal hooks.
-   `this auto&& self` preserves value category and constness through wrapper calls.
-   The wrapper can call either default derived behavior or an instance override when provided.
-   This keeps dispatch static and local while avoiding repetitive forwarding boilerplate.

<div class="notes" id="org52d03cc">
<p>
CRTP supplies structure.
Deducing this keeps wrappers generic without losing type information.
</p>

</div>


<a id="org1f8de47"></a>

### Same algorithm, two tree representations

-   Fixpoint tree and shared<sub>ptr</sub> binary tree can share the same Foldable call shape.
-   The representation changes; the typeclass API and algorithm intent stay the same.

1.  Fixpoint tree

    */ 9a1c4e2b-2c7e-4b1a-9f55-8b6a4d2e91aa auto n = foldable.length(tree); /* 9a1c4e2b-2c7e-4b1a-9f55-8b6a4d2e91aa end
    
    return n; }
    
    auto generic<sub>length</sub><sub>binary</sub><sub>tree</sub><sub>example</sub>() -> std::size<sub>t</sub> { using IntBinaryTree = smd::tree::BinaryTree<int>; auto tree = IntBinaryTree::from<sub>children</sub><sub>ptrs</sub>( 2, IntBinaryTree::make<sub>ptr</sub>(IntBinaryTree::leaf(1)), IntBinaryTree::make<sub>ptr</sub>(IntBinaryTree::from<sub>children</sub><sub>ptrs</sub>( 3, {}, IntBinaryTree::make<sub>ptr</sub>(IntBinaryTree::leaf(4)))));
    
    const auto& foldable = smd::foldable<sub>typeclass</sub><IntBinaryTree>;
    
    */ 53b9f5b4-3b3a-4e18-9b3c-07b7e2c980f4 auto n = foldable.length(tree); /* 53b9f5b4-3b3a-4e18-9b3c-07b7e2c980f4 end
    
    return n; }
    
    auto generic<sub>length</sub><sub>fringe</sub><sub>tree</sub><sub>example</sub>() -> std::size<sub>t</sub> { using Fringe = smd::tree::FringeTree<int>; auto tree = Fringe::branch( Fringe::branch(Fringe::leaf(1), Fringe::leaf(2)), Fringe::leaf(3));
    
    const auto& foldable = smd::foldable<sub>typeclass</sub><Fringe>;
    
    */ 7c2f11d9-ef09-45e2-80da-9229f3c8d82c auto n = foldable.length(tree); /* 7c2f11d9-ef09-45e2-80da-9229f3c8d82c end
    
    return n; }
    
    auto foldable<sub>flattens</sub><sub>shape</sub><sub>example</sub>() -> bool { using Tree = smd::tree::FixTree<int>; auto left<sub>heavy</sub> = Tree::branch( Tree::leaf(1), Tree::branch(Tree::leaf(2), Tree::leaf(3))); auto right<sub>heavy</sub> = Tree::branch( Tree::branch(Tree::leaf(1), Tree::leaf(2)), Tree::leaf(3));
    
    const auto& foldable = smd::foldable<sub>typeclass</sub><Tree>;
    
    */ b1fd4b92-b060-4c47-8c08-97328ec02329 auto left<sub>flat</sub> = foldable.to<sub>vector</sub>(left<sub>heavy</sub>); auto right<sub>flat</sub> = foldable.to<sub>vector</sub>(right<sub>heavy</sub>); /* b1fd4b92-b060-4c47-8c08-97328ec02329 end
    
    return left<sub>flat</sub> == right<sub>flat</sub>; }
    
    } // close namespace smd::typeclass::examples :lines 2- :src cpp :end "9a1c4e2b-2c7e-4b1a-9f55-8b6a4d2e91aa end"

2.  shared<sub>ptr</sub> binary tree

    */ 53b9f5b4-3b3a-4e18-9b3c-07b7e2c980f4 auto n = foldable.length(tree); /* 53b9f5b4-3b3a-4e18-9b3c-07b7e2c980f4 end
    
    return n; }
    
    auto generic<sub>length</sub><sub>fringe</sub><sub>tree</sub><sub>example</sub>() -> std::size<sub>t</sub> { using Fringe = smd::tree::FringeTree<int>; auto tree = Fringe::branch( Fringe::branch(Fringe::leaf(1), Fringe::leaf(2)), Fringe::leaf(3));
    
    const auto& foldable = smd::foldable<sub>typeclass</sub><Fringe>;
    
    */ 7c2f11d9-ef09-45e2-80da-9229f3c8d82c auto n = foldable.length(tree); /* 7c2f11d9-ef09-45e2-80da-9229f3c8d82c end
    
    return n; }
    
    auto foldable<sub>flattens</sub><sub>shape</sub><sub>example</sub>() -> bool { using Tree = smd::tree::FixTree<int>; auto left<sub>heavy</sub> = Tree::branch( Tree::leaf(1), Tree::branch(Tree::leaf(2), Tree::leaf(3))); auto right<sub>heavy</sub> = Tree::branch( Tree::branch(Tree::leaf(1), Tree::leaf(2)), Tree::leaf(3));
    
    const auto& foldable = smd::foldable<sub>typeclass</sub><Tree>;
    
    */ b1fd4b92-b060-4c47-8c08-97328ec02329 auto left<sub>flat</sub> = foldable.to<sub>vector</sub>(left<sub>heavy</sub>); auto right<sub>flat</sub> = foldable.to<sub>vector</sub>(right<sub>heavy</sub>); /* b1fd4b92-b060-4c47-8c08-97328ec02329 end
    
    return left<sub>flat</sub> == right<sub>flat</sub>; }
    
    } // close namespace smd::typeclass::examples :lines 2- :src cpp :end "53b9f5b4-3b3a-4e18-9b3c-07b7e2c980f4 end"

3.  fringetree (simplified fingertree)

    */ 7c2f11d9-ef09-45e2-80da-9229f3c8d82c auto n = foldable.length(tree); /* 7c2f11d9-ef09-45e2-80da-9229f3c8d82c end
    
    return n; }
    
    auto foldable<sub>flattens</sub><sub>shape</sub><sub>example</sub>() -> bool { using Tree = smd::tree::FixTree<int>; auto left<sub>heavy</sub> = Tree::branch( Tree::leaf(1), Tree::branch(Tree::leaf(2), Tree::leaf(3))); auto right<sub>heavy</sub> = Tree::branch( Tree::branch(Tree::leaf(1), Tree::leaf(2)), Tree::leaf(3));
    
    const auto& foldable = smd::foldable<sub>typeclass</sub><Tree>;
    
    */ b1fd4b92-b060-4c47-8c08-97328ec02329 auto left<sub>flat</sub> = foldable.to<sub>vector</sub>(left<sub>heavy</sub>); auto right<sub>flat</sub> = foldable.to<sub>vector</sub>(right<sub>heavy</sub>); /* b1fd4b92-b060-4c47-8c08-97328ec02329 end
    
    return left<sub>flat</sub> == right<sub>flat</sub>; }
    
    } // close namespace smd::typeclass::examples :lines 2- :src cpp :end "7c2f11d9-ef09-45e2-80da-9229f3c8d82c end"
    
    // 5c6b2d3e-7a44-4c8a-9c31-3d1e2a9b77c2 using beman::optional::optional;
    
    auto relabelled = traversable.traverse( [](int x) -> optional<int> { return x >= 0 ? optional<int>{x + 1} : optional<int>{}; }, tree); // 5c6b2d3e-7a44-4c8a-9c31-3d1e2a9b77c2 end
    
    if (!relabelled) { return {}; }
    
    const auto& foldable = smd::foldable<sub>typeclass</sub><IntTree>; return foldable.length(\*relabelled); }
    
    auto traversable<sub>preserves</sub><sub>shape</sub><sub>example</sub>() -> bool { using IntTree = smd::tree::FixTree<int>; using beman::optional::optional;
    
    auto tree = IntTree::branch( IntTree::leaf(1), IntTree::branch(IntTree::leaf(2), IntTree::leaf(3))); const auto& traversable = smd::traversable<sub>typeclass</sub><IntTree>;
    
    */ d804ec63-77d1-4fa0-99a6-9effce6f741b auto mapped = traversable.traverse( [](int x) -> optional<int> { return optional<int>{x + 10}; }, tree); /* d804ec63-77d1-4fa0-99a6-9effce6f741b end
    
    if (!mapped || mapped->is<sub>leaf</sub>()) { return false; }
    
    return mapped->left().is<sub>leaf</sub>() && mapped->left().value() `= 11 && !mapped->right().is_leaf() && mapped->right().left().is_leaf() && mapped->right().left().value() =` 12 && mapped->right().right().is<sub>leaf</sub>() && mapped->right().right().value() == 13; }
    
    } // close namespace smd::typeclass::examples :lines 2- :src cpp :end "5c6b2d3e-7a44-4c8a-9c31-3d1e2a9b77c2 end"


<a id="org91b0962"></a>

### Foldable API: one primitive, many derived operations

-   Minimal implementation hook: `fold_map`.
-   User-facing operations like `length`, `fold_left`, `fold_right`, `fold`, and `to_vector` are derived.
    
    // e3a1b1a2-6adf-4cb9-8c85-c0e39a7b98f2
    
    template <class T> auto length(this auto&& self, T&& value) -> std::size<sub>t</sub> { const auto count = self.fold<sub>map</sub>( [](const auto&) { return typeclass::Count{1}; }, std::forward<T>(value)); return count.d<sub>value</sub>; }
    
    template <class T, class STATE, class F> auto fold<sub>left</sub>(this auto&& self, T&& value, STATE initial<sub>state</sub>, F&& function) { using StateType = remove<sub>cvref</sub><sub>t</sub><STATE>; auto step = std::forward<F>(function);
    
    const auto program = self.fold<sub>map</sub>( [&step](const auto& x) { using ValueType = remove<sub>cvref</sub><sub>t</sub><decltype(x)>; return detail::LeftFoldProgram<StateType>{ [x<sub>copy</sub> = ValueType(x), &step](StateType s) { return std::invoke(step, std::move(s), x<sub>copy</sub>); }}; }, std::forward<T>(value));
    
    return program(StateType(std::move(initial<sub>state</sub>))); }
    
    template <class T, class STATE, class F> auto fold<sub>right</sub>(this auto&& self, T&& value, STATE initial<sub>state</sub>, F&& function) { using StateType = remove<sub>cvref</sub><sub>t</sub><STATE>; auto step = std::forward<F>(function);
    
    const auto program = self.fold<sub>map</sub>( [&step](const auto& x) { using ValueType = remove<sub>cvref</sub><sub>t</sub><decltype(x)>; return detail::RightFoldProgram<StateType>{ [x<sub>copy</sub> = ValueType(x), &step](StateType s) { return std::invoke(step, x<sub>copy</sub>, std::move(s)); }}; }, std::forward<T>(value));
    
    return program(StateType(std::move(initial<sub>state</sub>))); }
    
    template <class T> auto combine<sub>all</sub>(this auto&& self, T&& value) { return self.fold<sub>map</sub>([](const auto& x) { return x; }, std::forward<T>(value)); }
    
    template <class T> auto fold(this auto&& self, T&& value) { return self.combine<sub>all</sub>(std::forward<T>(value)); }
    
    template <class T, class PREDICATE> auto any<sub>of</sub>(this auto&& self, T&& value, PREDICATE&& predicate) -> bool { const auto result = self.fold<sub>map</sub>( [&predicate](const auto& x) { return detail::Any{std::invoke(predicate, x)}; }, std::forward<T>(value));
    
    return result.d<sub>value</sub>; }
    
    template <class T, class PREDICATE> auto all<sub>of</sub>(this auto&& self, T&& value, PREDICATE&& predicate) -> bool { const auto result = self.fold<sub>map</sub>( [&predicate](const auto& x) { return detail::All{std::invoke(predicate, x)}; }, std::forward<T>(value));
    
    return result.d<sub>value</sub>; }
    
    template <class T> auto empty(this auto&& self, T&& value) -> bool { return !self.any<sub>of</sub>(std::forward<T>(value), [](const auto&) { return true; }); }
    
    template <class T> auto to<sub>vector</sub>(this auto&& self, T&& value) { return self.fold<sub>map</sub>( [](const auto& x) { using ValueType = remove<sub>cvref</sub><sub>t</sub><decltype(x)>; return std::vector<ValueType>{x}; }, std::forward<T>(value)); } // e3a1b1a2-6adf-4cb9-8c85-c0e39a7b98f2 end
    
    template <class T, class PREDICATE> auto find<sub>first</sub>(this auto&& self, T&& value, PREDICATE&& predicate) { const auto result = self.fold<sub>map</sub>( [&predicate](const auto& x) { using X = remove<sub>cvref</sub><sub>t</sub><decltype(x)>; if (std::invoke(predicate, x)) { return detail::First<X>{{x}}; } return detail::First<X>{{}}; }, std::forward<T>(value));
    
    return result.d<sub>value</sub>; }

};

template <class T> inline constexpr auto foldable<sub>typeclass</sub> = std::false<sub>type</sub>{};

} // close namespace smd

\#endif :lines 2- :src cpp :end "e3a1b1a2-6adf-4cb9-8c85-c0e39a7b98f2 end"


<a id="org6ac88b7"></a>

### Foldable proof: derived operations hold in tests

-   Derived operations agree operationally with the `fold_map` contract.
    
    // 4c8a5f77-8a62-4f1b-a9cf-95452c4b8ea4 using IntSequence = smd::typeclass::test::Sequence<int>; auto sequence = IntSequence{{1, 2, 3}}; const auto& int<sub>foldable</sub> = smd::foldable<sub>typeclass</sub><IntSequence>;
    
    const auto as<sub>vector</sub> = int<sub>foldable.to</sub><sub>vector</sub>(sequence); CHECK(as<sub>vector</sub> == (std::vector<int>{1, 2, 3}));
    
    using VectorSequence = smd::typeclass::test::Sequence<std::vector<int> >; auto vectors = VectorSequence{{{1, 2}, {3}}}; const auto& vector<sub>foldable</sub> = smd::foldable<sub>typeclass</sub><VectorSequence>; const auto combined = vector<sub>foldable.combine</sub><sub>all</sub>(vectors); CHECK(combined == (std::vector<int>{1, 2, 3}));
    
    const auto folded = vector<sub>foldable.fold</sub>(vectors); CHECK(folded == (std::vector<int>{1, 2, 3})); // 4c8a5f77-8a62-4f1b-a9cf-95452c4b8ea4 end

}

TEST<sub>CASE</sub>("FoldableTypeclassTest - AllOfAndFindFirstEdgeCases") { using Sequence = smd::typeclass::test::Sequence<int>; const auto& foldable = smd::foldable<sub>typeclass</sub><Sequence>;

auto mixed = Sequence{{2, -1, 4}}; CHECK<sub>FALSE</sub>(foldable.all<sub>of</sub>(mixed, [](int x) { return x > 0; }));

auto found<sub>even</sub> = foldable.find<sub>first</sub>(mixed, [](int x) { return x % 2 `= 0; }); REQUIRE(found_even.has_value()); CHECK(*found_even =` 2);

auto found<sub>large</sub> = foldable.find<sub>first</sub>(mixed, [](int x) { return x > 100; }); CHECK<sub>FALSE</sub>(found<sub>large.has</sub><sub>value</sub>()); } :lines 2- :src cpp :end "4c8a5f77-8a62-4f1b-a9cf-95452c4b8ea4 end"


<a id="orgc87762a"></a>

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


<a id="org49f5349"></a>

### Applicative API: minimal core, user-facing invoke

-   Minimal implementation hook: `pure` and `apply`.
-   Future-facing user spelling is `invoke` over effectful arguments.
-   `apply_pure` is retained as a pedagogical alias.
    
    */ a11f7d8b-8f89-4f3e-9c92-f9f08ab7ef11 /* Teaching-friendly alias for "apply pure function to effectful arguments". // Prefer invoke as the primary C++ spelling (std::invoke model). template <class FUNCTION, class FIRST<sub>ARGUMENT</sub>, class&hellip; REST<sub>ARGUMENTS</sub>> auto apply<sub>pure</sub>(this auto&& self, FUNCTION&& function, FIRST<sub>ARGUMENT</sub>&& first<sub>argument</sub>, REST<sub>ARGUMENTS</sub>&&&hellip; rest<sub>arguments</sub>) { return self.invoke(std::forward<FUNCTION>(function), std::forward<FIRST<sub>ARGUMENT</sub>>(first<sub>argument</sub>), std::forward<REST<sub>ARGUMENTS</sub>>(rest<sub>arguments</sub>)&hellip;); }
    
    template <class FUNCTION, class FIRST<sub>ARGUMENT</sub>, class&hellip; REST<sub>ARGUMENTS</sub>> auto invoke(this auto&& self, FUNCTION&& function, FIRST<sub>ARGUMENT</sub>&& first<sub>argument</sub>, REST<sub>ARGUMENTS</sub>&&&hellip; rest<sub>arguments</sub>) { using SELF = std::remove<sub>reference</sub><sub>t</sub><decltype(self)>; using IMPL<sub>BASE</sub> = std::conditional<sub>t</sub><std::is<sub>const</sub><sub>v</sub><SELF>, const Impl, Impl>;
    
    if constexpr (requires(IMPL<sub>BASE</sub>& impl) { impl.invoke(std::forward<FUNCTION>(function), std::forward<FIRST<sub>ARGUMENT</sub>>(first<sub>argument</sub>), std::forward<REST<sub>ARGUMENTS</sub>>(rest<sub>arguments</sub>)&hellip;); }) { return static<sub>cast</sub><IMPL<sub>BASE</sub>&>(self).invoke( std::forward<FUNCTION>(function), std::forward<FIRST<sub>ARGUMENT</sub>>(first<sub>argument</sub>), std::forward<REST<sub>ARGUMENTS</sub>>(rest<sub>arguments</sub>)&hellip;); } else { auto lifted<sub>function</sub> = self.pure(detail::make<sub>terminating</sub><sub>partial</sub>(std::forward<FUNCTION>(function))); return self.apply<sub>chain</sub>( self.ap(std::move(lifted<sub>function</sub>), std::forward<FIRST<sub>ARGUMENT</sub>>(first<sub>argument</sub>)), std::forward<REST<sub>ARGUMENTS</sub>>(rest<sub>arguments</sub>)&hellip;); } } // a11f7d8b-8f89-4f3e-9c92-f9f08ab7ef11 end
    
    private: template <class ACCUMULATED> auto apply<sub>chain</sub>(this auto&&, ACCUMULATED&& accumulated) { return std::forward<ACCUMULATED>(accumulated); }
    
    template <class ACCUMULATED, class NEXT<sub>ARGUMENT</sub>, class&hellip; REST<sub>ARGUMENTS</sub>> auto apply<sub>chain</sub>(this auto&& self, ACCUMULATED&& accumulated, NEXT<sub>ARGUMENT</sub>&& next<sub>argument</sub>, REST<sub>ARGUMENTS</sub>&&&hellip; rest<sub>arguments</sub>) { auto next = self.ap(std::forward<ACCUMULATED>(accumulated), std::forward<NEXT<sub>ARGUMENT</sub>>(next<sub>argument</sub>)); if constexpr (sizeof&hellip;(REST<sub>ARGUMENTS</sub>) == 0) { return next; } else { return self.apply<sub>chain</sub>(std::move(next), std::forward<REST<sub>ARGUMENTS</sub>>(rest<sub>arguments</sub>)&hellip;); } }
    
    public: template <class FUNCTION, class ARGUMENT> auto map(this auto&& self, FUNCTION&& function, ARGUMENT&& argument) { return self.invoke(std::forward<FUNCTION>(function), std::forward<ARGUMENT>(argument)); }
    
    template <class VALUE> auto lift(this auto&& self, VALUE&& value) { return self.pure(std::forward<VALUE>(value)); }
    
    template <class FUNCTION<sub>IN</sub><sub>CONTEXT</sub>, class ARGUMENT<sub>IN</sub><sub>CONTEXT</sub>> auto ap(this auto&& self, FUNCTION<sub>IN</sub><sub>CONTEXT</sub>&& function, ARGUMENT<sub>IN</sub><sub>CONTEXT</sub>&& argument) { return self.apply(std::forward<FUNCTION<sub>IN</sub><sub>CONTEXT</sub>>(function), std::forward<ARGUMENT<sub>IN</sub><sub>CONTEXT</sub>>(argument)); }
    
    template <class FUNCTION, class FIRST<sub>ARGUMENT</sub>, class SECOND<sub>ARGUMENT</sub>> auto zip<sub>with</sub>(this auto&& self, FUNCTION&& function, FIRST<sub>ARGUMENT</sub>&& first<sub>argument</sub>, SECOND<sub>ARGUMENT</sub>&& second<sub>argument</sub>) { return self.invoke(std::forward<FUNCTION>(function), std::forward<FIRST<sub>ARGUMENT</sub>>(first<sub>argument</sub>), std::forward<SECOND<sub>ARGUMENT</sub>>(second<sub>argument</sub>)); }
    
    template <class FIRST<sub>ARGUMENT</sub>, class SECOND<sub>ARGUMENT</sub>> auto discard<sub>first</sub>(this auto&& self, FIRST<sub>ARGUMENT</sub>&& first<sub>argument</sub>, SECOND<sub>ARGUMENT</sub>&& second<sub>argument</sub>) { return self.invoke( [](const auto&, auto&& rhs) { return std::forward<decltype(rhs)>(rhs); }, std::forward<FIRST<sub>ARGUMENT</sub>>(first<sub>argument</sub>), std::forward<SECOND<sub>ARGUMENT</sub>>(second<sub>argument</sub>)); }
    
    template <class FIRST<sub>ARGUMENT</sub>, class SECOND<sub>ARGUMENT</sub>> auto discard<sub>second</sub>(this auto&& self, FIRST<sub>ARGUMENT</sub>&& first<sub>argument</sub>, SECOND<sub>ARGUMENT</sub>&& second<sub>argument</sub>) { return self.invoke( [](auto&& lhs, const auto&) { return std::forward<decltype(lhs)>(lhs); }, std::forward<FIRST<sub>ARGUMENT</sub>>(first<sub>argument</sub>), std::forward<SECOND<sub>ARGUMENT</sub>>(second<sub>argument</sub>)); }
    
    template <class APPLICATIVE<sub>MAP</sub>, class FUNCTION, class FIRST<sub>ARGUMENT</sub>, class&hellip; REST<sub>ARGUMENTS</sub>> auto invoke<sub>with</sub>(this auto&&, const APPLICATIVE<sub>MAP</sub>& applicative<sub>map</sub>, FUNCTION&& function, FIRST<sub>ARGUMENT</sub>&& first<sub>argument</sub>, REST<sub>ARGUMENTS</sub>&&&hellip; rest<sub>arguments</sub>) { return applicative<sub>map.invoke</sub>(std::forward<FUNCTION>(function), std::forward<FIRST<sub>ARGUMENT</sub>>(first<sub>argument</sub>), std::forward<REST<sub>ARGUMENTS</sub>>(rest<sub>arguments</sub>)&hellip;); }
    
    template <class APPLICATIVE<sub>MAP</sub>, class FUNCTION, class FIRST<sub>ARGUMENT</sub>, class&hellip; REST<sub>ARGUMENTS</sub>> auto apply<sub>pure</sub><sub>with</sub>(this auto&&, const APPLICATIVE<sub>MAP</sub>& applicative<sub>map</sub>, FUNCTION&& function, FIRST<sub>ARGUMENT</sub>&& first<sub>argument</sub>, REST<sub>ARGUMENTS</sub>&&&hellip; rest<sub>arguments</sub>) { return applicative<sub>map.invoke</sub>(std::forward<FUNCTION>(function), std::forward<FIRST<sub>ARGUMENT</sub>>(first<sub>argument</sub>), std::forward<REST<sub>ARGUMENTS</sub>>(rest<sub>arguments</sub>)&hellip;); }
    
    template <const auto& APPLICATIVE<sub>MAP</sub>, class FUNCTION, class FIRST<sub>ARGUMENT</sub>, class&hellip; REST<sub>ARGUMENTS</sub>> auto invoke<sub>with</sub>(this auto&&, FUNCTION&& function, FIRST<sub>ARGUMENT</sub>&& first<sub>argument</sub>, REST<sub>ARGUMENTS</sub>&&&hellip; rest<sub>arguments</sub>) { return APPLICATIVE<sub>MAP.invoke</sub>(std::forward<FUNCTION>(function), std::forward<FIRST<sub>ARGUMENT</sub>>(first<sub>argument</sub>), std::forward<REST<sub>ARGUMENTS</sub>>(rest<sub>arguments</sub>)&hellip;); }
    
    template <const auto& APPLICATIVE<sub>MAP</sub>, class FUNCTION, class FIRST<sub>ARGUMENT</sub>, class&hellip; REST<sub>ARGUMENTS</sub>> auto apply<sub>pure</sub><sub>with</sub>(this auto&&, FUNCTION&& function, FIRST<sub>ARGUMENT</sub>&& first<sub>argument</sub>, REST<sub>ARGUMENTS</sub>&&&hellip; rest<sub>arguments</sub>) { return APPLICATIVE<sub>MAP.invoke</sub>(std::forward<FUNCTION>(function), std::forward<FIRST<sub>ARGUMENT</sub>>(first<sub>argument</sub>), std::forward<REST<sub>ARGUMENTS</sub>>(rest<sub>arguments</sub>)&hellip;); }

};

template <class T> inline constexpr auto applicative<sub>typeclass</sub> = std::false<sub>type</sub>{};

template <class VALUE<sub>TYPE</sub>> struct OptionalApplicativeImpl { template <class VALUE> auto pure(this auto&&, VALUE&& value) -> std::optional<remove<sub>cvref</sub><sub>t</sub><VALUE> > { return std::optional<remove<sub>cvref</sub><sub>t</sub><VALUE> >{std::forward<VALUE>(value)}; }

template <class FUNCTION<sub>IN</sub><sub>CONTEXT</sub>, class ARGUMENT<sub>IN</sub><sub>CONTEXT</sub>> auto apply(this auto&&, FUNCTION<sub>IN</sub><sub>CONTEXT</sub>&& function, ARGUMENT<sub>IN</sub><sub>CONTEXT</sub>&& argument) { using Result = std::invoke<sub>result</sub><sub>t</sub><decltype(\*function), decltype(\*argument)>;

if (!function || !argument) { return std::optional<remove<sub>cvref</sub><sub>t</sub><Result> >{}; }

return std::optional<remove<sub>cvref</sub><sub>t</sub><Result> >{ std::invoke(\*std::forward<FUNCTION<sub>IN</sub><sub>CONTEXT</sub>>(function), \*std::forward<ARGUMENT<sub>IN</sub><sub>CONTEXT</sub>>(argument))}; } };

template <class VALUE<sub>TYPE</sub>> struct OptionalApplicativeMap : Applicative<OptionalApplicativeImpl<VALUE<sub>TYPE</sub>> > { using OptionalApplicativeImpl<VALUE<sub>TYPE</sub>>::apply; using OptionalApplicativeImpl<VALUE<sub>TYPE</sub>>::pure; };

template <class VALUE<sub>TYPE</sub>> requires(!std::same<sub>as</sub><beman::optional::optional<VALUE<sub>TYPE</sub>>, std::optional<VALUE<sub>TYPE</sub>> >) struct BemanOptionalApplicativeImpl { template <class VALUE> auto pure(this auto&&, VALUE&& value) -> beman::optional::optional<remove<sub>cvref</sub><sub>t</sub><VALUE> > { return beman::optional::optional<remove<sub>cvref</sub><sub>t</sub><VALUE> >{ std::forward<VALUE>(value)}; }

template <class FUNCTION<sub>IN</sub><sub>CONTEXT</sub>, class ARGUMENT<sub>IN</sub><sub>CONTEXT</sub>> auto apply(this auto&&, FUNCTION<sub>IN</sub><sub>CONTEXT</sub>&& function, ARGUMENT<sub>IN</sub><sub>CONTEXT</sub>&& argument) { using Result = std::invoke<sub>result</sub><sub>t</sub><decltype(\*function), decltype(\*argument)>;

if (!function || !argument) { return beman::optional::optional<remove<sub>cvref</sub><sub>t</sub><Result> >{}; }

return beman::optional::optional<remove<sub>cvref</sub><sub>t</sub><Result> >{ std::invoke(\*std::forward<FUNCTION<sub>IN</sub><sub>CONTEXT</sub>>(function), \*std::forward<ARGUMENT<sub>IN</sub><sub>CONTEXT</sub>>(argument))}; } };

template <class VALUE<sub>TYPE</sub>> requires(!std::same<sub>as</sub><beman::optional::optional<VALUE<sub>TYPE</sub>>, std::optional<VALUE<sub>TYPE</sub>> >) struct BemanOptionalApplicativeMap

    Applicative<BemanOptionalApplicativeImpl<VALUE_TYPE> > {

using BemanOptionalApplicativeImpl<VALUE<sub>TYPE</sub>>::apply; using BemanOptionalApplicativeImpl<VALUE<sub>TYPE</sub>>::pure; };

template <class VALUE<sub>TYPE</sub>> inline constexpr auto applicative<sub>typeclass</sub><std::optional<VALUE<sub>TYPE</sub>> > = OptionalApplicativeMap<VALUE<sub>TYPE</sub>>{};

template <class VALUE<sub>TYPE</sub>> requires(!std::same<sub>as</sub><beman::optional::optional<VALUE<sub>TYPE</sub>>, std::optional<VALUE<sub>TYPE</sub>> >) inline constexpr auto applicative<sub>typeclass</sub><beman::optional::optional<VALUE<sub>TYPE</sub>> > = BemanOptionalApplicativeMap<VALUE<sub>TYPE</sub>>{};

} // close namespace smd

\#endif :lines 2- :src cpp :end "a11f7d8b-8f89-4f3e-9c92-f9f08ab7ef11 end"


<a id="orgff838ec"></a>

### Applicative proof: n-ary use in tests

-   The same API handles arity > 2 without per-call-site plumbing.
    
    // 6e8bde7b-a9f1-4c98-8f1a-807d9ee0a93b std::optional<int> ax{2}; std::optional<int> ay{3}; std::optional<int> az{4}; const auto& applicative = smd::applicative<sub>typeclass</sub><std::optional<int> >;
    
    auto result = applicative.apply<sub>pure</sub>( [](int a, int b, int c) { return a \* b + c; }, ax, ay, az); REQUIRE(result.has<sub>value</sub>()); CHECK(\*result == 10); // 6e8bde7b-a9f1-4c98-8f1a-807d9ee0a93b end

}

TEST<sub>CASE</sub>("ApplicativeTypeclassTest - MapOptional") { std::optional<int> value{21}; const auto& applicative = smd::applicative<sub>typeclass</sub><std::optional<int> >;

auto result = applicative.map([](int x) { return x \* 2; }, value); REQUIRE(result.has<sub>value</sub>()); CHECK(\*result == 42); }

TEST<sub>CASE</sub>("ApplicativeTypeclassTest - InvokeWithExplicitMap") { std::optional<int> ax{10}; std::optional<int> ay{5}; const auto& default<sub>applicative</sub> = smd::applicative<sub>typeclass</sub><std::optional<int> >; const auto& optional<sub>applicative</sub> = smd::applicative<sub>typeclass</sub><std::optional<int> >;

auto result = default<sub>applicative.invoke</sub><sub>with</sub>( optional<sub>applicative</sub>, [](int a, int b) { return a + b; }, ax, ay); REQUIRE(result.has<sub>value</sub>()); CHECK(\*result == 15); }

TEST<sub>CASE</sub>("ApplicativeTypeclassTest - OptionalEmptyPaths") { const auto& applicative = smd::applicative<sub>typeclass</sub><std::optional<int> >;

std::optional<int (\*)(int)> no<sub>function</sub>{}; std::optional<int> argument{4}; auto no<sub>function</sub><sub>result</sub> = applicative.apply(no<sub>function</sub>, argument); CHECK<sub>FALSE</sub>(no<sub>function</sub><sub>result.has</sub><sub>value</sub>());

std::optional<int (\*)(int)> function{+[](int x) { return x + 3; }}; std::optional<int> no<sub>argument</sub>{}; auto no<sub>argument</sub><sub>result</sub> = applicative.apply(function, no<sub>argument</sub>); CHECK<sub>FALSE</sub>(no<sub>argument</sub><sub>result.has</sub><sub>value</sub>());

std::optional<int> ax{1}; std::optional<int> ay{}; auto invoke<sub>result</sub> = applicative.invoke([](int a, int b) { return a + b; }, ax, ay); CHECK<sub>FALSE</sub>(invoke<sub>result.has</sub><sub>value</sub>()); }

TEST<sub>CASE</sub>("ApplicativeTypeclassTest - DerivedOperations") { const auto& applicative = smd::applicative<sub>typeclass</sub><std::optional<int> >;

auto lifted = applicative.lift(9); REQUIRE(lifted.has<sub>value</sub>()); CHECK(\*lifted == 9);

std::optional<int (\*)(int)> function{+[](int x) { return x \* 3; }}; auto applied = applicative.ap(function, std::optional<int>{7}); REQUIRE(applied.has<sub>value</sub>()); CHECK(\*applied == 21);

auto zipped = applicative.zip<sub>with</sub>( [](int a, int b) { return a \* b; }, std::optional<int>{6}, std::optional<int>{5}); REQUIRE(zipped.has<sub>value</sub>()); CHECK(\*zipped == 30);

auto keep<sub>right</sub> = applicative.discard<sub>first</sub>(std::optional<int>{1}, std::optional<int>{2}); REQUIRE(keep<sub>right.has</sub><sub>value</sub>()); CHECK(\*keep<sub>right</sub> == 2);

auto keep<sub>left</sub> = applicative.discard<sub>second</sub>(std::optional<int>{1}, std::optional<int>{2}); REQUIRE(keep<sub>left.has</sub><sub>value</sub>()); CHECK(\*keep<sub>left</sub> == 1); }

TEST<sub>CASE</sub>("ApplicativeTypeclassTest - InvokeWithNttpMap") { const auto& default<sub>applicative</sub> = smd::applicative<sub>typeclass</sub><std::optional<int> >;

auto result = default<sub>applicative.invoke</sub><sub>with</sub>< smd::applicative<sub>typeclass</sub><std::optional<int> >>( [](int a, int b, int c) { return a + b + c; }, std::optional<int>{1}, std::optional<int>{2}, std::optional<int>{3}); REQUIRE(result.has<sub>value</sub>()); CHECK(\*result == 6);

auto apply<sub>pure</sub><sub>result</sub> = default<sub>applicative.apply</sub><sub>pure</sub><sub>with</sub>< smd::applicative<sub>typeclass</sub><std::optional<int> >>( [](int a, int b) { return a - b; }, std::optional<int>{8}, std::optional<int>{5}); REQUIRE(apply<sub>pure</sub><sub>result.has</sub><sub>value</sub>()); CHECK(\*apply<sub>pure</sub><sub>result</sub> == 3); }

TEST<sub>CASE</sub>("ApplicativeTypeclassTest - BemanOptional") { using BemanOptional = beman::optional::optional<int>; const auto& applicative = smd::applicative<sub>typeclass</sub><BemanOptional>;

auto lifted = applicative.pure(11); REQUIRE(lifted.has<sub>value</sub>()); CHECK(\*lifted == 11);

beman::optional::optional<int (\*)(int)> function{+[](int x) { return x + 5; }}; BemanOptional argument{7}; auto applied = applicative.apply(function, argument); REQUIRE(applied.has<sub>value</sub>()); CHECK(\*applied == 12);

beman::optional::optional<int (\*)(int)> no<sub>function</sub>{}; auto no<sub>function</sub><sub>applied</sub> = applicative.apply(no<sub>function</sub>, argument); CHECK<sub>FALSE</sub>(no<sub>function</sub><sub>applied.has</sub><sub>value</sub>());

BemanOptional no<sub>argument</sub>{}; auto no<sub>argument</sub><sub>applied</sub> = applicative.apply(function, no<sub>argument</sub>); CHECK<sub>FALSE</sub>(no<sub>argument</sub><sub>applied.has</sub><sub>value</sub>());

auto invoked = applicative.invoke( [](int a, int b) { return a \* b; }, BemanOptional{3}, BemanOptional{4}); REQUIRE(invoked.has<sub>value</sub>()); CHECK(\*invoked == 12);

auto empty<sub>invoked</sub> = applicative.invoke( [](int a, int b) { return a \* b; }, BemanOptional{}, BemanOptional{4}); CHECK<sub>FALSE</sub>(empty<sub>invoked.has</sub><sub>value</sub>()); }

TEST<sub>CASE</sub>("ApplicativeTypeclassTest - ApplyPureWithExplicitMap") { const auto& default<sub>applicative</sub> = smd::applicative<sub>typeclass</sub><std::optional<int> >; const auto& optional<sub>applicative</sub> = smd::applicative<sub>typeclass</sub><std::optional<int> >;

auto result = default<sub>applicative.apply</sub><sub>pure</sub><sub>with</sub>( optional<sub>applicative</sub>, [](int a, int b, int c) { return a + b + c; }, std::optional<int>{4}, std::optional<int>{5}, std::optional<int>{6}); REQUIRE(result.has<sub>value</sub>()); CHECK(\*result == 15); }

TEST<sub>CASE</sub>("ApplicativeTypeclassTest - TerminatingPartialExtendsAndInvokes") { auto partial = smd::detail::make<sub>terminating</sub><sub>partial</sub>( [](int a, int b, int c) { return a \* 100 + b \* 10 + c; });

auto partial2 = partial(1); auto partial3 = partial2(2); CHECK(partial3(3) == 123);

const auto const<sub>partial</sub> = smd::detail::make<sub>terminating</sub><sub>partial</sub>( [](int a, int b) { return a - b; }); auto const<sub>partial2</sub> = const<sub>partial</sub>(9); const auto const<sub>partial3</sub> = const<sub>partial2</sub>; CHECK(const<sub>partial3</sub>(4) == 5); }

TEST<sub>CASE</sub>("ApplicativeTypeclassTest - IdentityMapUsesDerivedInvokePath") { using Identity = smd::typeclass::test::Identity<int>; const auto& applicative = smd::applicative<sub>typeclass</sub><Identity>;

auto binary = applicative.invoke( [](int a, int b) { return a + b; }, Identity{2}, Identity{3}); CHECK(binary.value == 5);

auto ternary = applicative.apply<sub>pure</sub>( [](int a, int b, int c) { return a \* 100 + b \* 10 + c; }, Identity{1}, Identity{2}, Identity{3}); CHECK(ternary.value == 123); }

TEST<sub>CASE</sub>("ApplicativeTypeclassTest - CustomInvokeDispatchPath") { const auto& default<sub>applicative</sub> = smd::applicative<sub>typeclass</sub><std::optional<int> >;

auto result = default<sub>applicative.invoke</sub><sub>with</sub>( direct<sub>invoke</sub><sub>map</sub>, [](int a, int b, int c) { return a + b + c; }, smd::typeclass::test::Identity<int>{4}, smd::typeclass::test::Identity<int>{5}, smd::typeclass::test::Identity<int>{6}); CHECK(result.value == 15);

auto nttp<sub>result</sub> = default<sub>applicative.invoke</sub><sub>with</sub><direct<sub>invoke</sub><sub>map</sub>>( [](int a, int b) { return a \* b; }, smd::typeclass::test::Identity<int>{7}, smd::typeclass::test::Identity<int>{8}); CHECK(nttp<sub>result.value</sub> == 56); }

TEST<sub>CASE</sub>("ApplicativeTypeclassTest - OptionalAndBemanVectorInstantiationPaths") { const auto& optional<sub>applicative</sub> = smd::applicative<sub>typeclass</sub><std::optional<std::vector<int> > >;

auto lifted<sub>vector</sub> = optional<sub>applicative.pure</sub>(std::vector<int>{1, 2, 3}); REQUIRE(lifted<sub>vector.has</sub><sub>value</sub>()); CHECK(lifted<sub>vector</sub>->size() == 3);

std::optional<std::vector<int> (\*)(std::vector<int>)> append<sub>value</sub>{ +[](std::vector<int> v) { v.push<sub>back</sub>(4); return v; }}; auto applied<sub>vector</sub> = optional<sub>applicative.apply</sub>(append<sub>value</sub>, lifted<sub>vector</sub>); REQUIRE(applied<sub>vector.has</sub><sub>value</sub>()); CHECK(applied<sub>vector</sub>->size() == 4);

using BemanVectorOptional = beman::optional::optional<std::vector<int> >; const auto& beman<sub>applicative</sub> = smd::applicative<sub>typeclass</sub><BemanVectorOptional>;

auto beman<sub>lifted</sub> = beman<sub>applicative.pure</sub>(std::vector<int>{8, 9}); REQUIRE(beman<sub>lifted.has</sub><sub>value</sub>()); CHECK(beman<sub>lifted</sub>->size() == 2);

beman::optional::optional<std::vector<int> (\*)(std::vector<int>)> beman<sub>append</sub>{ +[](std::vector<int> v) { v.push<sub>back</sub>(10); return v; }}; auto beman<sub>applied</sub> = beman<sub>applicative.apply</sub>(beman<sub>append</sub>, beman<sub>lifted</sub>); REQUIRE(beman<sub>applied.has</sub><sub>value</sub>()); CHECK(beman<sub>applied</sub>->size() == 3); }

TEST<sub>CASE</sub>("ApplicativeTypeclassTest - IdentityWrapperMethods") { using Identity = smd::typeclass::test::Identity<int>; const auto& applicative = smd::applicative<sub>typeclass</sub><Identity>;

auto mapped = applicative.map([](int x) { return x + 1; }, Identity{9}); CHECK(mapped.value == 10);

auto zipped = applicative.zip<sub>with</sub>( [](int a, int b) { return a - b; }, Identity{20}, Identity{3}); CHECK(zipped.value == 17);

auto ap<sub>result</sub> = applicative.ap( smd::typeclass::test::Identity<int (\*)(int)>{+[](int x) { return x \* 5; }}, Identity{6}); CHECK(ap<sub>result.value</sub> == 30); }

TEST<sub>CASE</sub>("ApplicativeTypeclassTest - BareIdentityInvokeAndApplyChain") { using BareIdentity = smd::typeclass::test::BareIdentity<int>; const auto& applicative = smd::applicative<sub>typeclass</sub><BareIdentity>;

auto unary = applicative.invoke([](int x) { return x + 1; }, BareIdentity{4}); CHECK(unary.value == 5);

auto ternary = applicative.invoke( [](int a, int b, int c) { return a \* b + c; }, BareIdentity{2}, BareIdentity{3}, BareIdentity{4}); CHECK(ternary.value == 10);

auto quaternary = applicative.apply<sub>pure</sub>( [](int a, int b, int c, int d) { return a + b + c + d; }, BareIdentity{1}, BareIdentity{2}, BareIdentity{3}, BareIdentity{4}); CHECK(quaternary.value == 10); }

TEST<sub>CASE</sub>("ApplicativeTypeclassTest - BareIdentityWrapperCoverage") { using BareIdentity = smd::typeclass::test::BareIdentity<int>; const auto& applicative = smd::applicative<sub>typeclass</sub><BareIdentity>;

auto lifted = applicative.lift(33); CHECK(lifted.value == 33);

auto mapped = applicative.map([](int x) { return x \* 2; }, BareIdentity{11}); CHECK(mapped.value == 22);

auto applied = applicative.ap( smd::typeclass::test::BareIdentity<int (\*)(int)>{+[](int x) { return x - 2; }}, BareIdentity{9}); CHECK(applied.value == 7);

auto zipped = applicative.zip<sub>with</sub>( [](int a, int b) { return a - b; }, BareIdentity{40}, BareIdentity{8}); CHECK(zipped.value == 32);

auto keep<sub>right</sub> = applicative.discard<sub>first</sub>(BareIdentity{5}, BareIdentity{6}); CHECK(keep<sub>right.value</sub> == 6);

auto keep<sub>left</sub> = applicative.discard<sub>second</sub>(BareIdentity{5}, BareIdentity{6}); CHECK(keep<sub>left.value</sub> == 5); }

TEST<sub>CASE</sub>("ApplicativeTypeclassTest - BareIdentityInvokeWithMapCoverage") { using BareIdentity = smd::typeclass::test::BareIdentity<int>; const auto& default<sub>applicative</sub> = smd::applicative<sub>typeclass</sub><std::optional<int> >; const auto& bare<sub>identity</sub><sub>applicative</sub> = smd::applicative<sub>typeclass</sub><BareIdentity>;

auto explicit<sub>map</sub><sub>result</sub> = default<sub>applicative.invoke</sub><sub>with</sub>( bare<sub>identity</sub><sub>applicative</sub>, [](int a, int b, int c) { return a + b + c; }, BareIdentity{3}, BareIdentity{4}, BareIdentity{5}); CHECK(explicit<sub>map</sub><sub>result.value</sub> == 12);

auto explicit<sub>apply</sub><sub>pure</sub><sub>result</sub> = default<sub>applicative.apply</sub><sub>pure</sub><sub>with</sub>( bare<sub>identity</sub><sub>applicative</sub>, [](int a, int b) { return a \* b; }, BareIdentity{7}, BareIdentity{6}); CHECK(explicit<sub>apply</sub><sub>pure</sub><sub>result.value</sub> == 42);

auto nttp<sub>map</sub><sub>result</sub> = default<sub>applicative.invoke</sub><sub>with</sub><bare<sub>identity</sub><sub>applicative</sub>>( [](int a, int b) { return a - b; }, BareIdentity{20}, BareIdentity{9}); CHECK(nttp<sub>map</sub><sub>result.value</sub> == 11);

auto nttp<sub>apply</sub><sub>pure</sub><sub>result</sub> = default<sub>applicative.apply</sub><sub>pure</sub><sub>with</sub><bare<sub>identity</sub><sub>applicative</sub>>( [](int a, int b, int c) { return a + b \* c; }, BareIdentity{2}, BareIdentity{3}, BareIdentity{4}); CHECK(nttp<sub>apply</sub><sub>pure</sub><sub>result.value</sub> == 14); }

TEST<sub>CASE</sub>("ApplicativeTypeclassTest - BareIdentityTypeMatrixCoverage") { run<sub>bare</sub><sub>identity</sub><sub>matrix</sub><sub>case</sub><int, short, unsigned>(3, 4, 5U); run<sub>bare</sub><sub>identity</sub><sub>matrix</sub><sub>case</sub><long, int, long long>(10L, 20, 30LL); run<sub>bare</sub><sub>identity</sub><sub>matrix</sub><sub>case</sub><float, double, int>(1.5F, 2.25, 3); }

TEST<sub>CASE</sub>("ApplicativeBehaviorTest - OptionalIdentityHomomorphismAndInvoke") { CHECK(smd::typeclass::test::check<sub>applicative</sub><sub>identity</sub><sub>law</sub>(std::optional<int>{8})); CHECK(smd::typeclass::test::check<sub>applicative</sub><sub>homomorphism</sub><sub>law</sub><std::optional<int> >( +[](int x) { return x + 3; }, 5)); CHECK(smd::typeclass::test::check<sub>applicative</sub><sub>invoke</sub><sub>binary</sub><sub>law</sub>( [](int a, int b) { return a \* 10 + b; }, std::optional<int>{2}, std::optional<int>{7})); }

TEST<sub>CASE</sub>("ApplicativeBehaviorTest - BareIdentityIdentityHomomorphismAndInvoke") { using BareIdentity = smd::typeclass::test::BareIdentity<int>; CHECK(smd::typeclass::test::check<sub>applicative</sub><sub>identity</sub><sub>law</sub>(BareIdentity{11})); CHECK(smd::typeclass::test::check<sub>applicative</sub><sub>homomorphism</sub><sub>law</sub><BareIdentity>( +[](int x) { return x \* 4; }, 3)); CHECK(smd::typeclass::test::check<sub>applicative</sub><sub>invoke</sub><sub>binary</sub><sub>law</sub>( [](int a, int b) { return a - b; }, BareIdentity{20}, BareIdentity{6})); }

TEST<sub>CASE</sub>("ApplicativeBehaviorTest - BemanIdentityHomomorphismAndInvoke") { using BemanOptional = beman::optional::optional<int>;

CHECK(smd::typeclass::test::check<sub>applicative</sub><sub>identity</sub><sub>law</sub>(BemanOptional{11})); CHECK(smd::typeclass::test::check<sub>applicative</sub><sub>homomorphism</sub><sub>law</sub><BemanOptional>( +[](int x) { return x \* 4; }, 3)); CHECK(smd::typeclass::test::check<sub>applicative</sub><sub>invoke</sub><sub>binary</sub><sub>law</sub>( [](int a, int b) { return a - b; }, BemanOptional{20}, BemanOptional{6})); }

TEST<sub>CASE</sub>("ApplicativeBehaviorTest - OptionalShortCircuit") { const auto& applicative = smd::applicative<sub>typeclass</sub><std::optional<int> >;

std::optional<std::function<int(int)> > no<sub>function</sub>{}; auto no<sub>function</sub><sub>result</sub> = applicative.ap(no<sub>function</sub>, std::optional<int>{4}); CHECK<sub>FALSE</sub>(no<sub>function</sub><sub>result.has</sub><sub>value</sub>());

std::optional<std::function<int(int)> > function{ [](int x) { return x + 1; }}; auto no<sub>argument</sub><sub>result</sub> = applicative.ap(function, std::optional<int>{}); CHECK<sub>FALSE</sub>(no<sub>argument</sub><sub>result.has</sub><sub>value</sub>());

int calls = 0; auto invoke<sub>result</sub> = applicative.invoke( [&calls](int lhs, int rhs) { ++calls; return lhs + rhs; }, std::optional<int>{3}, std::optional<int>{}); CHECK<sub>FALSE</sub>(invoke<sub>result.has</sub><sub>value</sub>()); CHECK(calls == 0); }

TEST<sub>CASE</sub>("ApplicativeBehaviorTest - BemanShortCircuit") { using BemanOptional = beman::optional::optional<int>; const auto& applicative = smd::applicative<sub>typeclass</sub><BemanOptional>;

beman::optional::optional<std::function<int(int)> > no<sub>function</sub>{}; auto no<sub>function</sub><sub>result</sub> = applicative.ap(no<sub>function</sub>, BemanOptional{5}); CHECK<sub>FALSE</sub>(no<sub>function</sub><sub>result.has</sub><sub>value</sub>());

beman::optional::optional<std::function<int(int)> > function{ [](int x) { return x \* 2; }}; auto no<sub>argument</sub><sub>result</sub> = applicative.ap(function, BemanOptional{}); CHECK<sub>FALSE</sub>(no<sub>argument</sub><sub>result.has</sub><sub>value</sub>());

int calls = 0; auto invoke<sub>result</sub> = applicative.invoke( [&calls](int lhs, int rhs) { ++calls; return lhs - rhs; }, BemanOptional{9}, BemanOptional{}); CHECK<sub>FALSE</sub>(invoke<sub>result.has</sub><sub>value</sub>()); CHECK(calls == 0); }

TEST<sub>CASE</sub>("ApplicativeBehaviorTest - InvokeDispatchThroughBaseAndDerivedPaths") { DirectInvokeIdentityApplicativeMap<int> custom<sub>map</sub>{}; auto& custom<sub>base</sub> = static<sub>cast</sub><smd::Applicative<DirectInvokeIdentityApplicativeImpl<int> >&>( custom<sub>map</sub>);

auto custom<sub>dispatched</sub> = custom<sub>base.invoke</sub>( [](int a, int b, int c) { return a + b + c; }, smd::typeclass::test::Identity<int>{1}, smd::typeclass::test::Identity<int>{2}, smd::typeclass::test::Identity<int>{3}); CHECK(custom<sub>dispatched.value</sub> == 6);

smd::BareIdentityApplicativeMap<int> bare<sub>map</sub>{}; auto& bare<sub>base</sub> = static<sub>cast</sub><smd::Applicative<smd::BareIdentityApplicativeImpl<int> >&>(bare<sub>map</sub>);

auto derived<sub>dispatched</sub> = bare<sub>base.invoke</sub>( [](int a, int b, int c) { return a \* 100 + b \* 10 + c; }, smd::typeclass::test::BareIdentity<int>{4}, smd::typeclass::test::BareIdentity<int>{5}, smd::typeclass::test::BareIdentity<int>{6}); CHECK(derived<sub>dispatched.value</sub> == 456); }

TEST<sub>CASE</sub>("ApplicativeBehaviorTest - BareIdentityConstAndNonConstInvokeApMap") { smd::BareIdentityApplicativeMap<int> mutable<sub>map</sub>{}; auto& mutable<sub>base</sub> = static<sub>cast</sub><smd::Applicative<smd::BareIdentityApplicativeImpl<int> >&>(mutable<sub>map</sub>);

auto non<sub>const</sub><sub>invoke</sub> = mutable<sub>base.invoke</sub>( [](int a, int b) { return a + b; }, smd::typeclass::test::BareIdentity<int>{10}, smd::typeclass::test::BareIdentity<int>{4}); CHECK(non<sub>const</sub><sub>invoke.value</sub> == 14);

auto non<sub>const</sub><sub>map</sub> = mutable<sub>base.map</sub>( [](int x) { return x \* 3; }, smd::typeclass::test::BareIdentity<int>{7}); CHECK(non<sub>const</sub><sub>map.value</sub> == 21);

auto non<sub>const</sub><sub>ap</sub> = mutable<sub>base.ap</sub>( smd::typeclass::test::BareIdentity<std::function<int(int)> >{ [](int x) { return x - 5; }}, smd::typeclass::test::BareIdentity<int>{12}); CHECK(non<sub>const</sub><sub>ap.value</sub> == 7);

const smd::BareIdentityApplicativeMap<int> const<sub>map</sub>{}; const auto& const<sub>base</sub> = static<sub>cast</sub><const smd::Applicative<smd::BareIdentityApplicativeImpl<int> >&>( const<sub>map</sub>);

auto const<sub>invoke</sub> = const<sub>base.invoke</sub>( [](int a, int b, int c) { return a \* b + c; }, smd::typeclass::test::BareIdentity<int>{3}, smd::typeclass::test::BareIdentity<int>{5}, smd::typeclass::test::BareIdentity<int>{2}); CHECK(const<sub>invoke.value</sub> == 17);

auto const<sub>map</sub><sub>result</sub> = const<sub>base.map</sub>( [](int x) { return x + 8; }, smd::typeclass::test::BareIdentity<int>{1}); CHECK(const<sub>map</sub><sub>result.value</sub> == 9);

auto const<sub>ap</sub><sub>result</sub> = const<sub>base.ap</sub>( smd::typeclass::test::BareIdentity<std::function<int(int)> >{ [](int x) { return x \* x; }}, smd::typeclass::test::BareIdentity<int>{6}); CHECK(const<sub>ap</sub><sub>result.value</sub> == 36); } :lines 2- :src cpp :end "6e8bde7b-a9f1-4c98-8f1a-807d9ee0a93b end"


<a id="orgde11a93"></a>

### Traversable model: commute shape and effect

-   Traversal commutes shape and effect: from shape of effects to effect of shape.
-   This gives a generic path from many small checks to one checked result.
-   Use this to model validation, partial relabeling, and structured transformations.


<a id="orgd560936"></a>

### Traversable API: one primitive, many derived operations

-   Minimal implementation hook: `traverse`.
-   `for_each`, `sequence`, and `sequence_with` are derived helper operations.
    
    // 8f1d5c4a-1a7e-4b9e-8cb4-908f4ab0ca11
    
    template <class T, class F> auto for<sub>each</sub>(this auto&& self, T&& value, F&& function) { return self.traverse(std::forward<F>(function), std::forward<T>(value)); }
    
    template <class T> auto sequence(this auto&& self, T&& value) { return self.traverse( [](auto&& x) { return std::forward<decltype(x)>(x); }, std::forward<T>(value)); }
    
    template <class TRAVERSABLE<sub>MAP</sub>, class T, class F> auto traverse<sub>with</sub>(this auto&&, const TRAVERSABLE<sub>MAP</sub>& traversable<sub>map</sub>, F&& function, T&& value) { return traversable<sub>map.traverse</sub>(std::forward<F>(function), std::forward<T>(value)); }
    
    template <class TRAVERSABLE<sub>MAP</sub>, class T> auto sequence<sub>with</sub>(this auto&& self, const TRAVERSABLE<sub>MAP</sub>& traversable<sub>map</sub>, T&& value) { return self.traverse<sub>with</sub>( traversable<sub>map</sub>, [](auto&& x) { return std::forward<decltype(x)>(x); }, std::forward<T>(value)); } // 8f1d5c4a-1a7e-4b9e-8cb4-908f4ab0ca11 end

};

template <class T> inline constexpr auto traversable<sub>typeclass</sub> = std::false<sub>type</sub>{};

} // close namespace smd

\#endif :lines 2- :src cpp :end "8f1d5c4a-1a7e-4b9e-8cb4-908f4ab0ca11 end"


<a id="org05e1f1c"></a>

### Traversable proof: derived operations hold in tests

-   Sequencing works both via implicit lookup and explicit object selection.
    
    // f1de12e0-2287-4568-98c7-75be4f6f7446 using IdentityOpt = smd::typeclass::test::Identity<std::optional<int> >; auto identity = IdentityOpt{std::optional<int>{1}}; const auto& traversable = smd::traversable<sub>typeclass</sub><IdentityOpt>;
    
    auto sequenced = traversable.sequence(identity); REQUIRE(sequenced.has<sub>value</sub>()); CHECK(sequenced->value == 1);
    
    auto sequenced<sub>with</sub> = traversable.sequence<sub>with</sub>(traversable, identity); REQUIRE(sequenced<sub>with.has</sub><sub>value</sub>()); CHECK(sequenced<sub>with</sub>->value == 1); // f1de12e0-2287-4568-98c7-75be4f6f7446 end

}

TEST<sub>CASE</sub>("TraversableTypeclassTest - ForEachMatchesTraverse") { using Identity = smd::typeclass::test::Identity<int>; auto identity = Identity{4}; const auto& traversable = smd::traversable<sub>typeclass</sub><Identity>;

auto via<sub>traverse</sub> = traversable.traverse( [](int x) -> std::optional<int> { return std::optional<int>{x + 7}; }, identity); auto via<sub>for</sub><sub>each</sub> = traversable.for<sub>each</sub>( identity, [](int x) -> std::optional<int> { return std::optional<int>{x + 7}; });

CHECK(via<sub>traverse</sub> == via<sub>for</sub><sub>each</sub>); }

TEST<sub>CASE</sub>("TraversableTypeclassTest - SequenceMatchesTraverseIdentity") { using IdentityOpt = smd::typeclass::test::Identity<std::optional<int> >; auto identity = IdentityOpt{std::optional<int>{5}}; const auto& traversable = smd::traversable<sub>typeclass</sub><IdentityOpt>;

auto via<sub>sequence</sub> = traversable.sequence(identity); auto via<sub>traverse</sub><sub>identity</sub> = traversable.traverse( [](auto&& x) { return std::forward<decltype(x)>(x); }, identity);

CHECK(via<sub>sequence</sub> == via<sub>traverse</sub><sub>identity</sub>); } :lines 2- :src cpp :end "f1de12e0-2287-4568-98c7-75be4f6f7446 end"


<a id="org49ace6c"></a>

### Traversable commute: Range and ZipList

-   Traversable commutes a range of ZipLists into a ZipList of ranges.
-   The inverse matrix view (ZipList of vectors to vector of ZipLists) is also tested.
    
    // 0e9a7d13-9082-4b9e-b93f-86ef0e0ba20a using Zip = smd::zip<sub>list</sub><int>; auto values = smd::ranges::from<sub>vector</sub>(std::vector<Zip>{ Zip{{1, 2, 3}}, Zip{{10, 20}}, Zip{{100, 200, 300, 400}}});
    
    const auto& traversable = smd::traversable<sub>typeclass</sub><decltype(values)>; auto sequenced = traversable.sequence(values);
    
    REQUIRE(sequenced.data.size() `= 2U); CHECK(collect(sequenced.data[0]) =` (std::vector<int>{1, 10, 100})); CHECK(collect(sequenced.data[1]) == (std::vector<int>{2, 20, 200})); // 0e9a7d13-9082-4b9e-b93f-86ef0e0ba20a end

}

TEST<sub>CASE</sub>("RangeTraversableTest - SequenceConvertsRangeOfZiplistsToZiplistOfRangesLengthFive") { using Zip = smd::zip<sub>list</sub><int>; auto values = smd::ranges::from<sub>vector</sub>(std::vector<Zip>{ Zip{{1, 2, 3, 4, 5}}, Zip{{10, 20, 30, 40, 50}}, Zip{{100, 200, 300, 400, 500}}});

const auto& traversable = smd::traversable<sub>typeclass</sub><decltype(values)>; auto sequenced = traversable.sequence(values);

REQUIRE(sequenced.data.size() `= 5U); CHECK(collect(sequenced.data[0]) =` (std::vector<int>{1, 10, 100})); CHECK(collect(sequenced.data[1]) `= (std::vector<int>{2, 20, 200})); CHECK(collect(sequenced.data[2]) =` (std::vector<int>{3, 30, 300})); CHECK(collect(sequenced.data[3]) `= (std::vector<int>{4, 40, 400})); CHECK(collect(sequenced.data[4]) =` (std::vector<int>{5, 50, 500})); }

TEST<sub>CASE</sub>("RangeTraversableTest - ConvertZiplistOfVectorsToVectorOfZiplists") { // 4be89584-35cc-4933-b3de-6d524d54371d smd::zip<sub>list</sub><std::vector<int> > zip<sub>of</sub><sub>vectors</sub>{ {{1, 10, 100}, {2, 20, 200}}};

auto as<sub>rows</sub> = to<sub>vector</sub><sub>of</sub><sub>ziplists</sub>(zip<sub>of</sub><sub>vectors</sub>);

REQUIRE(as<sub>rows.size</sub>() `= 3U); CHECK(as_rows[0].data =` (std::vector<int>{1, 2})); CHECK(as<sub>rows</sub>[1].data `= (std::vector<int>{10, 20})); CHECK(as_rows[2].data =` (std::vector<int>{100, 200})); // 4be89584-35cc-4933-b3de-6d524d54371d end }

TEST<sub>CASE</sub>("RangeTraversableTest - ConvertZiplistOfVectorsToVectorOfZiplistsLengthFive") { smd::zip<sub>list</sub><std::vector<int> > zip<sub>of</sub><sub>vectors</sub>{ {{1, 10, 100, 1000, 10000}, {2, 20, 200, 2000, 20000}}};

auto as<sub>rows</sub> = to<sub>vector</sub><sub>of</sub><sub>ziplists</sub>(zip<sub>of</sub><sub>vectors</sub>);

REQUIRE(as<sub>rows.size</sub>() `= 5U); CHECK(as_rows[0].data =` (std::vector<int>{1, 2})); CHECK(as<sub>rows</sub>[1].data `= (std::vector<int>{10, 20})); CHECK(as_rows[2].data =` (std::vector<int>{100, 200})); CHECK(as<sub>rows</sub>[3].data `= (std::vector<int>{1000, 2000})); CHECK(as_rows[4].data =` (std::vector<int>{10000, 20000})); } :lines 2- :src cpp :end "0e9a7d13-9082-4b9e-b93f-86ef0e0ba20a end"

// 4be89584-35cc-4933-b3de-6d524d54371d smd::zip<sub>list</sub><std::vector<int> > zip<sub>of</sub><sub>vectors</sub>{ {{1, 10, 100}, {2, 20, 200}}};

auto as<sub>rows</sub> = to<sub>vector</sub><sub>of</sub><sub>ziplists</sub>(zip<sub>of</sub><sub>vectors</sub>);

REQUIRE(as<sub>rows.size</sub>() `= 3U); CHECK(as_rows[0].data =` (std::vector<int>{1, 2})); CHECK(as<sub>rows</sub>[1].data `= (std::vector<int>{10, 20})); CHECK(as_rows[2].data =` (std::vector<int>{100, 200})); // 4be89584-35cc-4933-b3de-6d524d54371d end }

TEST<sub>CASE</sub>("RangeTraversableTest - ConvertZiplistOfVectorsToVectorOfZiplistsLengthFive") { smd::zip<sub>list</sub><std::vector<int> > zip<sub>of</sub><sub>vectors</sub>{ {{1, 10, 100, 1000, 10000}, {2, 20, 200, 2000, 20000}}};

auto as<sub>rows</sub> = to<sub>vector</sub><sub>of</sub><sub>ziplists</sub>(zip<sub>of</sub><sub>vectors</sub>);

REQUIRE(as<sub>rows.size</sub>() `= 5U); CHECK(as_rows[0].data =` (std::vector<int>{1, 2})); CHECK(as<sub>rows</sub>[1].data `= (std::vector<int>{10, 20})); CHECK(as_rows[2].data =` (std::vector<int>{100, 200})); CHECK(as<sub>rows</sub>[3].data `= (std::vector<int>{1000, 2000})); CHECK(as_rows[4].data =` (std::vector<int>{10000, 20000})); } :lines 2- :src cpp :end "4be89584-35cc-4933-b3de-6d524d54371d end"

<div class="notes" id="orgf67951e">
<p>
Key law intuition: preserve shape and evaluation order discipline.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="org57e18f7"></a>

### Laws that keep this honest

-   Applicative: identity, composition, homomorphism, interchange.
-   Traversable: identity, naturality, composition.
-   Foldable: derived operations agree with `fold_map`.

<div class="notes" id="orgbced943">
<p>
If these fail, abstractions become accidental APIs rather than reliable interfaces.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="org58058a6"></a>

### Tree Applicative as optional appendix

-   Treat tree applicative as a policy choice, not the core applicative story.
-   If presented, keep it brief and explicitly label semantics.
-   Mainline examples should stay with optional, ranges, and ZipList.

<div class="notes" id="orge504239">
<p>
This avoids spending scarce slide time on semantics debates.
The core teaching value of Applicative is already visible in optional/range/ZipList examples.
</p>

</div>


<a id="org32e98a9"></a>

# Monoids and Measured Trees


<a id="org982cfc0"></a>

### Associativity as algorithmic leverage

-   Associativity lets us regroup work without changing results.
-   Measured trees exploit this to maintain summaries incrementally.
-   This is the bridge from algebra to explicit performance contracts.

<div class="notes" id="orge4df3d2">
<p>
If the measure is a monoid, split/search become compositional.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="orgaf9e4d1"></a>

### Annotations as summaries

-   Each node caches a measure of its subtree.
-   Measures are domain-specific: size, min priority, span, or cost.
-   Updating structure updates summaries locally.

<div class="notes" id="org5076b77">
<p>
The data structure stays the same while behavior changes with the monoid.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="org3fed4f6"></a>

### Search and split driven by measures

-   Search is currently implemented by linear scan over the flattened sequence.
-   Split currently follows the first predicate flip in that linear scan.
-   This yields one structure with many interpretations.

<div class="notes" id="org628999a">
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


<a id="org7855831"></a>

# Finger Trees as a Case Study


<a id="org568d4e8"></a>

### Persistent concatenation and splitting

-   Current prototype gives cheap persistence-friendly concatenation.
-   Current split/search paths are correct with linear-time upper bounds.
-   The API is designed so split/search can be optimized later without changing call sites.
-   The API naturally composes with foldable/traversable abstractions.

<div class="notes" id="org72fea50">
<p>
This is where abstractions meet implementation reality.
Paper-level target bounds remain the north star.
Current prototype contract is explicit linear split/search.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="org7aed6f9"></a>

### One structure, many interpretations

-   Change the monoid, change the interpretation.
-   Same implementation can model sequence, priority queue, or rope.
-   Reuse is semantic, not just syntactic.

<div class="notes" id="orgf403f73">
<p>
This is the strongest argument for measured trees in a standard library context.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="orgc953209"></a>

### Why this belongs in modern C++

-   Zero-cost abstractions and strong typing fit this design.
-   Multiple paradigms can coexist: value types, OO boundaries, generic algorithms.
-   This is not import Haskell; it is idiomatic modern C++ with better algebraic interfaces.

<div class="notes" id="org494fef3">
<p>
Pragmatic conclusion: values first, identity where required, and laws where possible.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="orgdbc468a"></a>

# Designing APIs That Won’t Age Poorly


<a id="org9e70d01"></a>

### Library abstractions anticipating language features

-   Favor explicit, composable operations over magical overload sets.
-   Keep extension points separate from core type definitions.
-   Make future language support a simplification, not a rewrite.

<div class="notes" id="orgf6f6c4d">
<p>
Pattern matching and richer generic facilities should refine this API, not replace it.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="org83cf7f9"></a>

### Avoiding the `std::bind` vs lambda overlap

-   Avoid parallel abstractions that solve the same use case differently.
-   Choose one clear good path per concept.
-   For Applicative, that path is `invoke`; `apply_pure` remains a teaching aid.

<div class="notes" id="org92c9f4e">
<p>
The goal is reducing cognitive branching in generic code.
Source: (Steve Downey, 2026).
</p>

</div>


<a id="orgabe34eb"></a>

### Keeping the good path obvious

-   Make lawful defaults easy and alternate policies explicit.
-   Keep naming consistent across concepts.
-   Back claims with executable law tests.

<div class="notes" id="org365aca6">
<p>
The best API docs in this space are tests that encode the laws.
Source: (Steve Downey, 2026).
</p>

</div>
