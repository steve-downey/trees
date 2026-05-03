# Overview
## Motivation
Other than the standard library's implementation of optional, optionals holding references are common. The desire for such a feature is well understood, and many optional types in commonly used libraries provide it, with the semantics proposed here.
One standard library implementation already provides an implementation of `std::optional<T&>` but disables its use, because the standard forbids it.

The research in JeanHeyd Meneide's _References for Standard Library Vocabulary Types - an optional case study._ P1683R0 shows conclusively that rebind semantics are the only safe semantic as assign through on engaged is too bug-prone. Implementations that attempt assign-through are abandoned. The standard library should follow existing practice and supply an `optional<T&>` that rebinds on assignment.

Additional background reading on `optional<T&>` can be found in JeanHeyd Meneide's article _To Bind and Loose a Reference_ REFBIND.

In freestanding environments or for safety-critical libraries, an optional type over references is important to implement containers, that otherwise as the standard library either would cause undefined behavior when accessing an non-available element, throw an exception, or silently create the element. Returning a plain pointer for such an optional reference, as the core guidelines suggest, is a non-type-safe solution and doesn't protect in any way from accessing an non-existing element by a `nullptr` de-reference. In addition, the monadic APIs of `std::optional` makes is especially attractive by streamlining client code receiving such an optional reference, in contrast to a pointer that requires an explicit nullptr check and de-reference.

There is a principled reason not to provide a partial specialization over `T&` as the semantics are in some ways subtly different than the primary template. Assignment may have side-effects not present in the primary, which has pure value semantics. However, I argue this is misleading, as reference semantics often has side-effects. The proposed semantic is similar to what an `optional<std::reference_wrapper<T>>` provides, with much greater usability.

There are well motivated suggestions that perhaps instead of an `optional<T&>` there should be an `optional_ref<T>` that is an independent primary template. This proposal rejects that, because we need a policy over all sum types as to how reference semantics should work, as optional is a variant over T and monostate. That the library sum type can not express the same range of types as the product type, tuple, is an increasing problem as we add more types logically equivalent to a variant. The template types `optional` and `expected` should behave as extensions of `variant<T, monostate>` and `variant<T, E>`, or we lose the ability to reason about generic types.

That we can't guarantee from `std::tuple<Args...>` (product type) that `std::variant<Args...>` (sum type) is valid, is a problem, and one that reflection can't solve. A language sum type could, but we need agreement on the semantics.

The semantics of a variant with a reference are as if it holds the address of the referent when referring to that referent. All other semantics are worse. Not being able to express a variant<T&> is inconsistent, hostile, and strictly worse than disallowing it.

Thus, we expect future papers to propose `std::expected<T&,E>` and `std::variant` with the ability to hold references.
The latter can be used as an iteration type over `std::tuple` elements.


## Design

The design is straightforward. The `optional<T&>` holds a pointer to the underlying object of type `T`, or `nullptr` if the optional is disengaged. The implementation is simple, especially with C++20 and up techniques, using concept constraints. As the held pointer is a primitive regular type with reference semantics, many operations can be defaulted and are `noexcept` by nature. See Downey_smd_optional_optional_T and rawgithu58:online. The `optional<T&>` implementation is less than 200 lines of code, much of it the monadic functions with identical textual implementations with different signatures and different overloads being called.

In place construction is not supported as it would just be a way of providing immediate life-time issues.

### Relational Operations

The definitions of the relational operators are the same as for the base template. Interoperable comparisons between T and optional<T&> work as expected. This is not true for the boost optional<T&>.

### make_optional
With further research, the existing uses of make_optional<X&> seem to be primarily test cases, and deliberate use seems to be exceedingly rare in the wild. Reflector review was much more positive about removing the misleading ability to create an `optional<X>` via `make_optional<X&>(x)`. In addition, the multiple argument forms can be used to attempt to construct a optional that contains a reference, but this becomes ill formed because of existing mandates at the type level. In order to preserve existing behavior, where make_optional is not well formed if it constructs a reference, changes to `make_optional` should be made.

Adding a non-type template parameter as the first template parameter to the single argument `make_optional` and mandating that the multi-argument version not request a reference type as the parameter, will diagnose mistaken use of `make_optional` and preserve the existing behavior.

Since construction of an object in order to make a reference to it to construct an optional containing a reference would always dangle, there do not seem to be any use cases for the multi-argument or initializer list forms of make_optional for reference types, and the constructor form seems to satisfy all cases for single argument construction of a optional containing a reference, there does not seem to be a need for a factory function for optional over reference.

There was also discussion of using `std::reference_wrapper` to indicate reference use, in analogy with std::tuple. Unfortunately there are existing uses of optional over reference_wrapper as a workaround for lack of reference specialization, and it would be a breaking change for such code.

### Trivial construction
Construction of `optional<T&>` should be trivial, because it is straightforward to implement, and `optional<T>` is trivial. Boost is not.

### Value Category Affects value()
For several implementations there are distinct overloads for functions depending on value category, with the same implementation. However, this makes it very easy to accidentally steal from the underlying referred to object. Value category should be shallow. Thanks to many people for pointing this out. If ``Deducing `this`'' had been used, the problem would have been much more subtle in code review.

### Shallow vs Deep const

There is some implementation divergence in optionals about deep const for `optional<T&>`. That is, can the referred to `int` be modified through a `const optional<int&>`. Does `operator->()` return an `int*` or a `const int*`, and does `operator*()` return an `int&` or a `const int&`. I believe it is overall more defensible if the `const` is shallow as it would be for a `struct ref {int * p;`` where the constness of the struct ref does not affect if the p pointer can be written through. This is consistent with the rebinding behavior being proposed.

Where deeper constness is desired, `optional<const T&>` would prevent non const access to the underlying object.

### Conditional Explicit
As in the base template, `explicit` is made conditional on the type used to construct the optional. `explicit(!std::is_convertible_v<U, T>)`. This is not present in boost::optional, leading to differences in construction between braced initialization and = that can be surprising.

### value_or

After extensive discussion, it seems there is no particularly wonderful solution for `value_or` that does not involve a time machine. Implementations of optionals that support reference semantics diverge over the return type, and the current one is arguably wrong, and should use something based on `common_reference_type`, which of course did not exist when `optional` was standardized.

The weak consensus is to return a `T` from `optional<T&>::value_or` as this is least likely to cause issues. There was at least one strong objection to this choice, but all other choices had more objections. The author intends to propose free functions `reference_or`, `value_or`, `or_invoke`, and `yield_if` over all types modeling optional-like, `concept std::maybe`, in the next revision of P1255R12. This would cover `optional`, `expected`, and pointer types.

Having `value_or` return by value also allows the common case of using a literal as the alternative to be expressed concisely.

### in_place_t construction
The reference specialization allows a limited form of in_place construction where the argument can be converted to the appropriate reference without creation of a temporary. As the reference specialization is non-owning, there is no ``place'' for a temporary to be constructed that will not dangle. For cases where the lifetime of the constructed object would match the lifetime of the optional, the temporary can be constructed explicitly, instead.

### Converting assignment
A similarly limited converting assignment operator is provided for cases where an `optional<U>` has a value or refers to a value which can be converted to a `T&` without construction of a temporary. In particular, converting an `optional<T&>` to an `optional<T const&>` is supported.


##  Principles for Reification of Design

Optional must never construct a temporary, or knowingly take the address of an temporary or part of an temporary.

It is always presumed safe to copy the pointer value from an optional, since by induction, it is not dangling.

Optional has no storage, so should never construct a T, it may convert a U to a T, so long as that conversion does not create a temporary.

Constructors that would convert from temporary are marked deleted. They should be sufficiently constrained that it was the correct choice and there is no more general, less constrained constructor that would not have created a dangling pointer.

Failure to compile either by ambiguity or no eligible constructors in the overload set is preferable to optional being responsible for use after free or dangling.

Assignment is always from an optional, which may have been an implicit construction. The assignment cannot throw, the construction/conversion may. The assignment may therefore need annotation converting the rhs if that constructor was explicit. This must not be necessary in the default case of creating an optional reference to an lvalue of the same type.

The model for the constraints and mandates for `optional<T&>` is taken from `std::tuple` over reference types. The type `std::tuple` takes the most care of types in the standard library in dealing with creation of temporaries.

As `optional` is designed to be converting, to create instances from arguments that can be used to create the underlying type, constructors should be explicit only where the operations used to create the pointer or the notional reference would be or are explicit.



### Construction from temporary

We disallow construction of `optional<T&>` from any type U in which:
- the constructor body will create a temporary and bind it to a reference.
- a const lvalue reference would be bound to rvalue.

An example of the first case would be construction `optional<std::string const&>` from `char const*`. These cases always dangle.

An example of the second case would be a construction `std::optional<std::string const&>` from temporary `std::string`.

Prohibiting the second case does prevent some safe uses of the optional as the function parameter.

Given:

`void process(std::optional<std::string const&> arg);`

This will make a `process(std::string("sdfd"))` invocation ill-formed, despite the arg being safe to use from within the function body.

This deviates from the design of the ``view'' parameters type, like `std::string_view` or `std::span`. However, we believe that this is the right choice due to the following:

-  Only a subset of cases would be working. As an illustration the very similar `process("text")} invocation is ill-formed, due to always being dangling.

-  Such design leads to the detection of reference to temporaries or local variables when `optional<T const&>` is used as the return type.


```c++
std::optional<std::string const&> getValue() {
  std::string localString;
  return localString; // Ill-formed.
  std::optional<std::string> localOptionalString;
  return localOptionalString; // ill-formed
}
```


One of the main motivational examples of `optional<T const&>` is return from a lookup function, and eliminating dangling in such cases outweighs parameter cases.

We are very grateful to Arthur O'Dwyer for his work on P2266R3  accepted in C++23, which makes it possible to implement this correctly.

-  We provide behavior consistent with `reference_wrapper<T const>`, that disallows binding to xvalues. We believe that `reference_wrapper<T>` is closer in spirit to `optional<T&>` than any view type. It certainly shares some of the features.

### Deleting dangling overloads

To achieve the dangling safety expressed before, the constructor is marked deleted if it would lead to binding of the reference to temporary or the xvalue.
However, deleted constructors are still considered to be candidates during overload resolution, leading to ambiguity in the following examples:

```c++
void process(std::optional<std::string const&>);
void process(std::optional<char const* const&>);

void test() {
  char const* cstr = "Text";
  std::string s = cstr;
  process(s); // Picks, optional<std::string const&> overload
  process(cstr); // Ambiguous, but only std::optional<char const* const&> is not dangling
}

```

During the reflector discussion, an option of an alternate design was presented, where the dangling overload would be constrained, and eliminated from the overload set.

We strongly oppose changing this behavior, as:
-  We think that it is impossible to detect temporary binding to xvalue in such a design.

- The behavior we propose is consistent with the behavior for optional for object types

```c++
void processVal(std::optional<std::string>);
void processVal(std::optional<char const*>);

void test() {
  char const* cstr = "Text";
  std::string s = cstr;
  processVal(s); // Picks std::string overload
  processVal(cstr); // Ambiguous
}

```

As language in general treats functions accepting by value and by const reference in the same manner during overload resolution, we believe achieving this consistency is a feature.

The design that was introduced by `std::tuple`, and `std::pair`, for references, is followed, where the detection of dangling does not affect the results of overload resolution and instead makes a call that would dangle be ill-formed and diagnosed.

### Assignment of optional<T&>

In the case of `optional<T&>`, any assignment operation is equivalent to assigning a pointer, and there is no observable difference between:
using converting assignment from `U&&` or `optional<U>`
constructing temporary `optional<T&>`, and then assigning it to it.

This observation allows us to provide only copy-assignment for `optional<T&>`, instead of a set of converting assignments, that would need to replicate the signatures of constructors and their constraints. Assignment from any other value is handled by first implicitly constructing `optional<T&>` and then using copy-assignment. Move-assignment is the same as copy-assignment, since only pointer copy is involved.

### Copy and Assignment of optional<U&>&& to optional<T>
Care must be take to prevent the assignment of a movable optional to disallow the copy or assignment of the underlying referred to value to be stolen. The `optional<T>::optional<U&> const&` assignment or copy constructor should be used instead, which also needs to check slightly different constraints for `converts-from-any-cvref` and for testing `is_assignable`. We thank Jan Kokemüller for uncovering this bug. The bug seems to be present in many optional implementations that support references.
