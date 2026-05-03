### Concise Directions for an Assistant: Implementing a C++ FixTree Adaptor

When tasked with generating a "fixtree" (an abstract syntax tree built on F-algebras) in modern C++, follow these precise architectural steps. *Note: As `std::indirect` (C++26) may be unavailable in the target compilation environment, this guide defaults to using `std::shared_ptr` to box recursive elements.*

**1. Define the Non-Recursive Functor using `std::variant`**
Do not allow the functor to reference itself directly. Parameterize the "holes" (recursive children) with a generic type `A`. To prevent C++ "incomplete type" compiler errors, alias a `Box` type to `std::shared_ptr` and a `make_box` helper mapped to `std::make_shared`. Use this `Box<A>` inside the struct definitions.
*Example:* `template <typename A> struct Add { Box<A> left; Box<A> right; };`
*Variant Definition:* `template <typename A> using ExprF = std::variant<Const<A>, Add<A>, Mul<A>>;`

**2. Define the `Fix` Combinator Struct**
Do *not* use inheritance (CRTP). Use a simple compositional `has-a` relationship.
*Implementation:* `template <template <typename> class F> struct Fix { F<Fix<F>> inner; };`

**3. Implement Phantom Operations (`wrap` and `unwrap`)**
Define free functions to transition between the isorecursive boundaries. Mark them `constexpr` so the compiler can elide them entirely.
*   `wrap`: Takes an `F<Fix<F>>` and returns a `Fix<F>`.
*   `unwrap`: Takes a `Fix<F>` and returns a reference to its `inner` member.

**4. Define `fmap` for the Functor**
Create a generic function that takes a mapping function `f` and a variant instance. Use `std::visit` with an overloaded lambda visitor to step into the variant state, dereference the child boxes (via `*a.left`), apply the function `f`, and repackage the results into new variant states using `make_box`.

**5. Define the Catamorphism (`cata`)**
Write the universal fold logic.
1. Call `unwrap` on the target tree to peel back a single layer.
2. Call `fmap` on the unwrapped layer, passing in a recursive call to `cata` as the transformation function.
3. Pass the fully evaluated result into the user-provided algebra lambda.

**6. Provide Smart Constructors**
Hide the `Fix` wrapper and the heap allocation details from the API consumer. Provide easy-to-use functions like `add_node` that take evaluated `Fix<F>` trees, move them into `make_box`, wrap them in the `variant`, and finally call `wrap` before returning to the user.

**7. Supply an Evaluation Algebra**
The domain logic (the algebra) should be entirely non-recursive. Define it using the overloaded `std::visit` pattern matching purely on evaluated types (e.g., `Add<int>`), computing the local result (e.g., `*a.left + *a.right`), and returning it. Pass this algebra into the `cata` function to collapse the structure.
