# Code Review: finger_tree5.hpp + finger_tree5_iterator.hpp — 2026-05-16

Design, implementation, style, and idiom review.
Preference: contemporary C++26 over "modern C++14/17" over C idioms.

---

## 1. Redundant `overloaded` deduction guide (C++20 supersedes)

**Lines 40–45** (`finger_tree5.hpp`):
```cpp
template <typename... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};
template <typename... Ts>
overloaded(Ts...) -> overloaded<Ts...>;
```

In C++20 and later, CTAD for aggregates makes the explicit deduction guide
unnecessary.  Since this codebase targets C++26, the deduction guide is
deadweight.  The struct itself could also be replaced by direct use of the
C++26 `std::overload` if/when it arrives, but for now the struct is fine —
just drop the guide.

**Suggested**: Remove the deduction guide (line 44–45).

---

## 2. `inline` on free function templates is redundant

**Lines 50, 55** (`finger_tree5.hpp`):
```cpp
template <typename Tag>
inline auto tag_id() -> Tag { ... }
```

Function templates have implicit `inline` linkage.  The `inline` keyword is
redundant noise.

**Suggested**: Remove `inline` from `tag_id`, `tag_op`, and the out-of-line
`begin`/`end` definitions in the iterator header (lines 461, 467).

---

## 3. `operator!=` is generated from `operator==` in C++20

**Lines 412–413** (`finger_tree5_iterator.hpp`):
```cpp
auto operator!=(const FingerTree5Iterator& other) const -> bool {
    return !(*this == other);
}
```

C++20 automatically synthesizes `!=` from `==`.  Defining it manually is
pre-C++20 idiom.

**Suggested**: Remove `operator!=` entirely.

---

## 4. `tag_id()` and `tag_op()` should be `constexpr`

**Lines 49–57**: These functions delegate to `monoid_v<Tag>` which is
`constexpr`.  The wrappers should propagate `constexpr`:

```cpp
template <typename Tag>
constexpr auto tag_id() -> Tag { ... }

template <typename Tag>
constexpr auto tag_op(const Tag &a, const Tag &b) -> Tag { ... }
```

**Suggested**: Add `constexpr` to both.

---

## 5. `make_leaf` takes `T value` by value — potential double-move

**Line 103**:
```cpp
auto make_leaf(MeasFn &&mf, T value) -> ElemPtr<T, Tag> {
    auto m = mf(value);   // <-- passes `value` by lvalue (measure sees the original)
```

`mf(value)` passes the already-moved-into parameter as an lvalue.  This is
intentional (measure needs the value before it moves into the Leaf).  However,
the function signature should document this: the parameter is taken by value
deliberately so the caller can move into it, and the function uses it twice
(once for measure, once for Leaf construction).  This is correct but subtly
non-obvious.

No code change needed — but a one-line comment would help:
```cpp
// value is taken by-value: measured first, then moved into the Leaf.
```

---

## 6. `nodes_from` takes `std::vector` by value but could take a span

**Line 233**:
```cpp
auto nodes_from(std::vector<ElemPtr<T, Tag>> elems) -> std::vector<...>
```

This takes ownership of the vector (by value) but only reads from indices and
moves out of them.  A `std::span` + move semantics would be more expressive
of intent and avoid the copy when the caller wants to keep the original.
However, all call sites pass rvalues (`std::move(combined)`), so in practice
this is fine.

**Suggested**: No change needed for correctness, but consider
`std::span<ElemPtr<T,Tag>>` with explicit moves in a future refactor.

---

## 7. `digit_to_vec` unnecessarily allocates — could return `std::span`

**Line 222**:
```cpp
auto digit_to_vec(const Digit<T, Tag> &d) -> std::vector<ElemPtr<T, Tag>> {
```

The `Digit` is an `inplace_vector<EP, 4>` which is already contiguous.
Returning a `std::span<const EP>` or even just using the `Digit` directly
would avoid the heap allocation.  The only consumer (`app3`) appends it into
another vector, so the allocation is just an intermediate copy.

**Suggested**: Replace with a span or iterate the Digit directly at the call
site in `app3`.  This eliminates a heap allocation per concat.

---

## 8. `app3` takes `middle` by value but immediately iterates it

**Line 711**:
```cpp
static auto app3(const FingerTree5 &left, std::vector<EP> middle,
                 const FingerTree5 &right)
```

The only call sites pass `{}` (empty vector) or `std::move(ns)`.  Taking by
value is correct for the move case.  The empty-vector case (`append` on line
956) allocates a zero-length vector on every call — `std::vector<EP>{}` is
not free (it may set up internal state).

**Suggested**: Consider an overload `app3(left, right)` with no middle
argument for the common `append` case, avoiding the empty-vector construction.

---

## 9. Raw `nullptr` used where `SpinePtr{}` would be more type-safe

**Lines 389, 396, 403, 502, etc.**:
```cpp
return make_deep(std::move(l), nullptr, std::move(r));
```

`nullptr` implicitly converts to `shared_ptr`, but `SpinePtr{}` is the
idiomatic null `shared_ptr`.  Using `SpinePtr{}` makes it explicit that this
is a typed null, not a raw C pointer being implicitly converted.

**Suggested**: Replace `nullptr` with `SpinePtr{}` at all `make_deep` call
sites.

---

## 10. `if (!spine || spine->is_empty())` duplicates a null check

**Lines 412, 426, 515, 553, etc.**:
```cpp
if (!spine || spine->is_empty())
```

`spine` is a `shared_ptr`.  The `!spine` check tests for null.  Then
`spine->is_empty()` tests the tree's content.  These are separate concerns
(null pointer vs empty tree) collapsed into one condition.  Consider encoding
"no spine" as always meaning "empty spine" (never null), which would allow a
single `spine->is_empty()` check.

**Suggested**: Establish an invariant: `SpinePtr` is either null (meaning
empty) OR points to a non-empty FingerTree5.  Then `!spine` is sufficient
without `spine->is_empty()`.  Document this invariant and remove the
`|| spine->is_empty()` clause.  OR always store an empty tree (never null) and
just check `spine->is_empty()`.

---

## 11. `view_l` / `view_r` copy the value out of the Leaf

**Line 900**:
```cpp
return View{ft5::leaf_value(iv->d_elem), std::move(iv->d_rest)};
```

`ft5::leaf_value` returns `const T&`, but `View::d_value` is `T` (by value).
This forces a copy of the element into the View.  For large `T` (e.g.,
`std::string`), this is a real cost.  The caller (`head()`) then *moves* out
of the View.

**Suggested**: Either make `View::d_value` a `const T&` (reference into the
shared_ptr-kept-alive tree), or document that `view_l`/`view_r` copy the
element.  A reference-based View would be a significant API change but would
avoid the copy for read-only access patterns.

---

## 12. `from_sequence` is O(N) via repeated `snoc` — could be O(N) bottom-up

**Line 946**:
```cpp
static auto from_sequence(std::vector<T> values) -> FingerTree5 {
    auto result = empty();
    for (auto &v : values)
        result = result.snoc(std::move(v));
    return result;
}
```

This is O(N) amortized but with high constant factor (each snoc may allocate
a new Deep/SpinePtr).  A bottom-up construction that builds Nodes directly
from chunks of the input, then assembles a balanced tree, would be O(N) with
fewer allocations and better cache behaviour.  Hinze & Paterson describe such
a construction.

**Suggested**: Keep the current implementation as a correct fallback, but add a
`from_range`-style construction that builds bottom-up for a future
optimization.

---

## 13. Iterator: `make_begin` copies the tree into a `shared_ptr`

**Line 293** (iterator):
```cpp
it.d_root_keepalive = std::make_shared<const FT>(tree);
```

This copies the entire FingerTree5 value (which is just a `Repr` variant —
either an `Empty{}`, a `Single{EP}`, or a `DeepPtr`) into a heap-allocated
shared_ptr.  The copy itself is cheap (shared_ptr refcounts increment), but
the `make_shared<const FT>` allocation is avoidable if the tree already lived
on the heap.

**Suggested**: If the user already holds a `shared_ptr<const FT>`, provide an
overload that accepts it directly and avoids the extra allocation.
Alternatively, since the tree is a small value (~32 bytes), the allocation
overhead is minimal — document this as intentional.

---

## 14. Missing `[[nodiscard]]` on pure-functional operations

Many operations (`cons`, `snoc`, `append`, `split`, `reversed`, etc.) return
new values and have no side effects.  Forgetting to use the return value is
always a bug.

**Suggested**: Add `[[nodiscard]]` to: `cons`, `snoc`, `view_l`, `view_r`,
`head`, `tail`, `last`, `init`, `flatten`, `append`, `concat`, `split`,
`split_at`, `split_at_measure`, `search`, `reversed`, `from_sequence`, `leaf`,
`empty`, `measure`, `spine_depth`.

---

## 15. `SpineFrame::Section` enum should be an `enum class`

**Line 51** (iterator):
```cpp
enum Section { LEFT = 0, SPINE = 1, RIGHT = 2 } section;
```

Unscoped enums leak their enumerators into the enclosing scope.  C++ idiom
since C++11 is `enum class` for scoped enumerations.

**Suggested**: `enum class Section : std::uint8_t { LEFT = 0, SPINE = 1, RIGHT = 2 };`

---

## 16. `assert(false && "...")` is not `[[noreturn]]`-friendly

**Lines 261, 274, 82** etc.:
```cpp
assert(false && "nodes_from: invalid count");
```

In release builds `assert` compiles to nothing, leaving the code path
fall-through.  The C++23 idiom is `std::unreachable()` after an assertion, or
use `std::unreachable()` alone in release and `assert(false)` only in debug.

**Suggested**: Replace `assert(false && "msg")` with
`assert(false && "msg"); std::unreachable();` (the `unreachable()` tells the
optimizer and silences warnings about missing returns).  This is already done
at line 407 in `digit_to_tree`.

---

## 17. Repetitive digit construction in `digit_to_tree`

**Lines 380–409**: Four nearly-identical cases construct digits by pushing
elements one at a time.  This is mechanical but verbose.  A helper
`Digit make_digit(std::initializer_list<EP>)` or a subrange view would reduce
the visual noise.

**Suggested**: Add a small helper:
```cpp
static auto make_digit(std::initializer_list<EP> elems) -> Digit {
    Digit d;
    for (auto& e : elems) d.push_back(e);
    return d;
}
```

---

## Summary priority

| # | Severity | Effort | Nature |
|---|----------|--------|--------|
| 14 | High | Low | `[[nodiscard]]` — catches real bugs |
| 3 | Medium | Trivial | Remove dead `operator!=` |
| 10 | Medium | Medium | Clarify spine null invariant |
| 16 | Medium | Low | `std::unreachable()` after assert(false) |
| 4 | Medium | Trivial | `constexpr` on tag helpers |
| 2 | Low | Trivial | Remove redundant `inline` |
| 1 | Low | Trivial | Remove deduction guide |
| 9 | Low | Low | `SpinePtr{}` over `nullptr` |
| 15 | Low | Trivial | `enum class Section` |
| 7, 8 | Low | Medium | Avoid intermediate vectors in concat path |
| 11 | Design | High | Reference-based View (breaking API change) |
| 12 | Design | High | Bottom-up construction (optimisation) |
| 13 | Design | Low | Accept shared_ptr in iterator construction |
