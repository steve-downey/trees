# Follow-up plan for FingerTree5 (handoff to a fresh Claude session)

## Orientation (read this first)

This is the `steve-downey/trees` repo — a C++26 codebase exploring functional data structures with a typeclass-object dispatch pattern.
Build is `make compile` (defaults to `gcc-16` + `-std=gnu++26` via `etc/gcc-16-toolchain.cmake`).
Tests are Catch2, all collected into the single `smd_tree_tests` executable.
Run specific tests with `uv run ctest --test-dir .build/build-gcc-16 -C Asan -R '<regex>' --output-on-failure`.
Project conventions are in `CLAUDE.md` (root) and `docs/CODING_RULES.md`.
Notable: classical include guards (no `#pragma once`), test files end in `.t.cpp` and `#include` the header under test twice to verify idempotency, headers must be self-contained (`make compile-headers` runs `verify_interface_header_sets`).

The FT5 work this plan builds on is the 7 commits `373d5695..5dfdd7c4` plus `e8b402af` (CLAUDE.md).
FingerTree5 is in `src/smd/tree/finger_tree5.hpp`, ~600 lines.
Its public API is intentionally identical to `FingerTree4` — anything new added by this follow-up plan must be **additive only**.
The detail namespace is `smd::tree::ft5::*`; the user-facing class is `smd::tree::FingerTree5<T, TAG_TYPE, MEASURE_POLICY>` with default `TAG_TYPE = std::size_t`, `MEASURE_POLICY = UnitMeasure5<T,Tag>`.

Three items to land, ordered by recommended sequencing.
Each can be a separate PR / commit series.

---

## Item 1 — Bidirectional iterator + `std::ranges::view_interface`

### Goal

A `bidirectional_iterator`-tagged cursor for FT5 with amortized O(1) `operator++`/`--`, O(log N) construction, and free `begin`/`end` so range-based-for and `<algorithm>`/`<ranges>` work.
**Refuse** the `random_access_iterator_tag` even though we could fake distance — tagging RA would lie to `std::sort` and friends (the doc this design came from explicitly warned about this; respect the warning).

### Files

```
src/smd/tree/finger_tree5_iterator.hpp     # iterator + free begin/end
src/smd/tree/finger_tree5_iterator.t.cpp   # tests
```

Then add to `src/smd/tree/finger_tree5.hpp` only:

- `auto begin() const`, `auto end() const`, `auto cbegin/cend` — these forward to the free functions in the iterator header.
  Add `#include <smd/tree/finger_tree5_iterator.hpp>` *after* the class definition, or use a fwd-decl trick.
  The cleanest path: put `begin`/`end` as free functions only, inheriting `std::ranges::view_interface<FingerTree5<...>>` is **not** worth the lifetime headaches (a FingerTree5 is a value, not a view; treat it as a *range* via free `begin`/`end` and leave `view_interface` alone).

Wire both files into `src/smd/tree/CMakeLists.txt` (FILE_SET + test sources).
Bump entries alphabetically as the existing pattern shows.

### Type & state

```cpp
namespace smd::tree {

template <typename T, typename Tag, typename MP>
class FingerTree5Iterator {
public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type        = T;
    using difference_type   = std::ptrdiff_t;
    using pointer           = const T*;
    using reference         = const T&;

    // ...
private:
    // Each spine frame says "I'm at section X of this Deep, digit position Y".
    struct SpineFrame {
        std::shared_ptr<const typename FingerTree5<T,Tag,MP>::Deep> deep;
        enum { LEFT = 0, SPINE = 1, RIGHT = 2 } section;
        std::size_t index;     // 0..3 within the section's digit
    };
    // Each elem frame says "I'm inside this Node2/Node3, descending into child K".
    struct ElemFrame {
        ft5::ElemPtr<T,Tag> elem;
        std::size_t child;     // 0/1 for Node2; 0/1/2 for Node3
    };
    std::vector<SpineFrame> d_spine_path;   // outermost..innermost
    std::vector<ElemFrame>  d_elem_path;    // outermost..innermost; bottom is parent of current leaf
    ft5::ElemPtr<T,Tag>     d_current_leaf; // null at end()
    std::size_t             d_absolute_index;
    std::size_t             d_tree_size;
    std::shared_ptr<const FingerTree5<T,Tag,MP>> d_root_keepalive;
};
```

### Algorithms

- **Constructor `begin`**: walk down the leftmost path.
  At each Deep, push `SpineFrame{deep, LEFT, 0}`; the current digit element is `deep->d_left[0]`.
  If that element is a Node, push an `ElemFrame{elem, 0}` and recurse into `elem.a` (Node2/Node3).
  Bottom out when current is a `Leaf` — store in `d_current_leaf`.
  `d_absolute_index = 0`.
  For an empty tree, `d_current_leaf = nullptr`; that's `end()`.

- **Constructor `end`**: `d_absolute_index = tree_size`, empty paths, null leaf.
  (Don't bother descending — that would be O(log N) for no payoff.)

- **`operator++`**: walk back up `d_elem_path` until we find an ancestor where there's a "right sibling" child to descend into; pop the consumed frame, push the sibling, descend leftmost to the next Leaf.
  If `d_elem_path` empties, advance the innermost `SpineFrame.index` and pick the next digit element; if the digit runs out, transition `LEFT → SPINE → RIGHT` at this level.
  If `SPINE` is selected and the spine is non-empty, push a fresh `SpineFrame` for the spine's root Deep and start over at its left.
  If `RIGHT` runs out at the innermost frame, pop it and try the same up one spine level.
  When `d_spine_path` empties entirely, we're at end.

- **`operator--`**: symmetric.
  Either descend rightmost from `end()` (O(log N), like reverse-`begin`), or step back from a non-end position by mirroring the forward logic.

- **`operator*`**: `return ft5::leaf_value(d_current_leaf);` — assert non-null first.

- **`operator==`**: compare `d_absolute_index` (assumes both iterators are from the same tree; document the precondition).

- Free `begin(t)` / `end(t)` constructed as above; member `t.begin()` / `t.end()` forward to them.

### Gotchas

1. **The two stacks are not independent.**
   When `operator++` exhausts the `d_elem_path` of the current digit element, advancing the digit position is straightforward; but when the digit exhausts and we move to the spine, the *spine itself is a `FingerTree5<T,Tag,MP>` of `ElemPtr<T,Tag>` whose elements are Nodes one level deeper*.
   So pushing a `SpineFrame` for the spine's root means the digit elements at *that* level are themselves Node2/Node3, and you'll need `d_elem_path` to dive further on the way down to a Leaf.
   Don't conflate spine depth with Elem depth — they're coupled (each spine level adds one to the Elem depth at that level), but the path stacks are conceptually separate.

2. **`d_root_keepalive` matters.**
   The path frames hold `shared_ptr<const Deep>` and `ElemPtr`, which are reference-counted.
   But moves of intermediate `FingerTree5` values (returned by `view_l`, etc.) could drop the spine pointer.
   Hold a `shared_ptr<const FingerTree5>` to the root for the iterator's lifetime, even though everything else in the path *transitively* keeps things alive.
   Belt and suspenders, but the cost is one extra ptr in the iterator.

3. **`tree_size` requires a measure that's a counter.**
   For the default `Tag = std::size_t` with `UnitMeasure5`, `tree.measure()` *is* the count.
   For arbitrary `Tag`, the iterator needs an absolute count threaded through some other path.
   Simplest: compute the size separately via `std::ranges::distance(t.flatten())` at construction time (O(N) — defeats O(log N) construction), or require the user to pass the size.
   Honest answer: this iterator is most useful when `Tag` is a count or has a `length()`-like projection.
   **Restrict construction so `tree_size` is computed from `tree.measure()` only when `Tag` is `std::size_t` and `MEASURE_POLICY` is `UnitMeasure5`**; otherwise require the user to pass `size` explicitly.
   Document this clearly.

   Alternative: deduce size via the existing `foldable_typeclass<FingerTree5<...>>::length`.
   That's O(N).
   Acceptable if you frame `iterator` as "use when you also want iteration"; the construction-time count is amortized away.

4. **`operator==` for end iterators of different trees**: undefined behavior in the standard; just say so in the docs and don't try to be cute.

5. **`view_interface` is the wrong abstraction.**
   FT5 is a *container*, not a *view*.
   Don't inherit from `view_interface`.
   Range-based for works via free `begin`/`end` regardless.

### Tests

`finger_tree5_iterator.t.cpp` — minimum:

- HeaderIsIdempotent.
- ForwardIterationMatchesFlatten — iterate FT5 manually, compare to `flatten()`.
- ReverseIterationMatchesReversedFlatten — `std::vector<int> v(t.rbegin(), t.rend());` or equivalent.
- BidirectionalWalkAndBack — `++++++ --- ---` returns to the same leaf.
- DistanceMatchesSize — `std::distance(begin, end) == 100` for a 100-element tree.
- EmptyTreeBeginEqualsEnd.
- SingleLeafIteration — one increment to end.
- RangeBasedFor — `for (auto x : tree)` accumulates correctly.
- RangesAlgorithms — `std::ranges::find(tree, 42)`, `std::ranges::any_of(tree, …)` compile and work.
- DigitOverflowBoundary — iterate a tree with ≥256 elements (forces nontrivial spine) and verify no skips/dupes.
- IteratorCategoryIsBidirectional — `static_assert(std::bidirectional_iterator<iterator>)` and `static_assert(!std::random_access_iterator<iterator>)`.

### Commit shape

One commit: "add bidirectional iterator + range integration for FingerTree5".

---

## Item 2 — `reversed()` + `DualMonoid<M>`

### Goal

Make right-to-left walks correct under non-commutative measures.
Introduce `DualMonoid<M>` as a reusable type.
Add `FingerTree5::reversed()` returning a tree that iterates in opposite order with the dual monoid semantics.

### Files

```
src/smd/typeclass/dual_monoid.hpp     # DualMonoid<M> + Monoid<DualMonoid<M>> spec
src/smd/typeclass/dual_monoid.t.cpp   # tests for the dual monoid in isolation
```

And extensions to:

```
src/smd/tree/finger_tree5.hpp         # reversed() member
src/smd/tree/finger_tree5.t.cpp       # tests for reversed() under commutative & non-commutative tags
```

Wire `dual_monoid.hpp` into `src/smd/typeclass/CMakeLists.txt` (existing typeclass library has its own file set).

### `DualMonoid<M>` design

```cpp
namespace smd::typeclass {

template <typename M>
struct DualMonoid {
    M value;
    friend bool operator==(const DualMonoid&, const DualMonoid&) = default;
};

template <typename M>
struct Monoid<DualMonoid<M>> {
    constexpr auto identity() const -> DualMonoid<M> {
        return DualMonoid<M>{monoid_v<M>.identity()};
    }
    constexpr auto combine(const DualMonoid<M>& lhs, const DualMonoid<M>& rhs) const
        -> DualMonoid<M> {
        return DualMonoid<M>{monoid_v<M>.combine(rhs.value, lhs.value)};
    }
};

} // namespace smd::typeclass
```

The combine flip is the entire mechanism.
Verify left-identity → right-identity via tests: `combine(identity, x) == x` AND `combine(x, identity) == x` (the dual swaps which is "left").

If `M` has `operator>=` (used by `split_at_measure`), forward it from `DualMonoid<M>` so `split_at_measure` still compiles on dualized trees.

### `reversed()` design — decision required

Two viable implementations, listed in order of recommendation.
Pick one *before writing code*; don't try to support both unless there's a concrete user.

**Option A (recommended) — eager rebuild:**

```cpp
auto reversed() const -> FingerTree5<T, DualMonoid<Tag>, ReversedMeasure5<MEASURE_POLICY>> {
    using DT = FingerTree5<T, DualMonoid<Tag>, ReversedMeasure5<MEASURE_POLICY>>;
    auto v = flatten();
    std::reverse(v.begin(), v.end());
    return DT::from_sequence(std::move(v));
}
```

Cost: O(N) time, O(N) space.
Simple, correct, always works.
`ReversedMeasure5` is just `MEASURE_POLICY` wrapped to return `DualMonoid<Tag>{policy(x)}` instead of `policy(x)`.

**Option B (deferred) — lazy structural reverse:**

Return a wrapper view that swaps left↔right digits at each Deep level on demand, with `DualMonoid<Tag>` as the cached measure type.
**The catch**: the cached measure values inside existing Deep/Elem nodes were computed under the *original* monoid's combine order.
They'd be wrong under the dual unless either (a) the monoid is commutative (cached values are accidentally correct), or (b) every Deep/Elem caches both `measure` and `dual_measure`.
Storing both doubles memory; doing it lazily breaks the O(1) measure invariant.

Option B is a footgun and not worth the complexity for the speculative non-commutative case.
Implement Option A.
If someone later proves a real need for O(log N) `reversed()` under non-commutative measures, revisit then.

### Gotchas

1. **For commutative monoids, `DualMonoid<Tag>` is gratuitous type noise.**
   Document that `tree.reversed()` returns a tree with a different *type* even when it's structurally equivalent.
   Provide a `reversed_unwrap()` or similar **only if a user complains** — don't pre-build.

2. **Involution:** `t.reversed().reversed()` returns a `FingerTree5<T, DualMonoid<DualMonoid<Tag>>, …>`.
   The double-dual is operationally identical to the original but not the same type.
   Test that `flatten()` matches; don't promise type identity.

3. **`split_at_measure` on the reversed tree** evaluates the predicate against accumulated `DualMonoid<Tag>` prefixes that *grow from the new left edge* (which is the old right edge).
   Users need to understand they're searching from the opposite end.
   Document with an example.

4. **`ReversedMeasure5`** is a meta-policy wrapping the original.
   Likely shape:

   ```cpp
   template <typename Policy>
   struct ReversedMeasure5 {
       Policy d_inner;
       template <typename T>
       auto operator()(const T& x) const {
           return smd::typeclass::DualMonoid{d_inner(x)};
       }
   };
   ```

### Tests

`dual_monoid.t.cpp`:

- IdentityIsIdentity — `combine(identity, x) == x`, `combine(x, identity) == x`.
- AssociativityHolds — `combine(combine(a,b), c) == combine(a, combine(b,c))` for a non-commutative `M` (use string concat as the test case).
- FlipsArguments — `monoid_v<DualMonoid<std::string>>.combine({"a"}, {"b"}).value == "ba"`.
- DoubleDualMatchesOriginal — `monoid_v<DualMonoid<DualMonoid<M>>>` behaves as `monoid_v<M>` for combine.

`finger_tree5.t.cpp` extensions:

- ReversedFlattenMatchesReverse — `t.reversed().flatten() == reverse(t.flatten())`.
- ReversedTwiceIsOriginal — `t.reversed().reversed().flatten() == t.flatten()`.
- ReversedMeasureOnCommutative — for default `Tag = std::size_t`, `t.reversed().measure().value == t.measure()`.
- ReversedMeasureOnNonCommutative — define a string-concat tag; verify that `t.reversed().measure().value` is the reversed concatenation of leaf values.
- ReversedSplitAtMeasure — split a reversed tree at a threshold and verify the resulting halves correspond to the *suffix*-then-prefix slicing of the original.

### Commit shape

Two commits:

1. "add DualMonoid<M> typeclass utility" (dual_monoid.hpp + .t.cpp + CMake wiring).
2. "add FingerTree5::reversed() built on DualMonoid" (FT5 header extension + tests).

---

## Item 3 — Parameterize wrappers on tree type

### Goal

The four wrappers in `src/smd/tree/` currently hard-code `FingerTree2`:

- `finger_tree_random_access.hpp`
- `finger_tree_rope.hpp`
- `finger_tree_priority_queue.hpp`
- `finger_tree_interval_index.hpp`

Parameterize them so a caller can pick `FingerTree2`, `FingerTree4`, or `FingerTree5` as the backing store, with `FingerTree2` remaining the default for backwards compatibility.

### Approach

Use a single type-parameter pattern — the **fully-specialized tree type** is the template argument, not a template-template:

```cpp
template <typename Tree = FingerTree2<std::string, std::size_t, RopeChunkMeasure>>
class FingerTreeRope { /* ... uses Tree directly ... */ };
```

Reason: a template-template parameter (`template <typename, typename, typename> class TreeT`) forces the wrapper to know the tree's exact template signature, which differs between FT2/FT3/FT4/FT5 (some have a `DEPTH` NTTP, others don't).
Taking the fully-specialized tree is what STL does (`std::stack<T, Container>`) and is robust to signature drift.

### Files to modify

For each wrapper, change the class to take `typename Tree = <existing FT2 specialization>`.
All internal uses of `FingerTree2<...>` become `Tree`.
Member typedefs like `using Tree = FingerTree2<...>;` become the template parameter.
Any place that constructs a tree (`Tree::empty()`, `Tree::leaf(...)`, `Tree::from_sequence(...)`) keeps working because all FT* variants share that surface.

For each wrapper, add a *second* test file that instantiates with FT5 and runs the same scenarios:

```
src/smd/tree/finger_tree_random_access_ft5.t.cpp
src/smd/tree/finger_tree_rope_ft5.t.cpp
src/smd/tree/finger_tree_priority_queue_ft5.t.cpp
src/smd/tree/finger_tree_interval_index_ft5.t.cpp
```

Each parallel test does `#include <smd/tree/finger_tree_X.hpp>` + `#include <smd/tree/finger_tree5.hpp>` and uses `using Wrapper = FingerTreeX<FingerTree5<...>>;`.
Copy the existing FT2 test cases and adjust types.

Wire all four new tests into `src/smd/tree/CMakeLists.txt`.

### Sequencing — do `random_access` first

It's the simplest wrapper (no special monoid setup beyond `UnitMeasure`).
Once that one works under FT5, the pattern is proven and the others fall in line.
Order:

1. `finger_tree_random_access.hpp` + `_ft5.t.cpp` — verify the parameterization approach.
2. `finger_tree_rope.hpp` + `_ft5.t.cpp` — custom `RopeChunkMeasure`, more interesting.
3. `finger_tree_priority_queue.hpp` + `_ft5.t.cpp` — custom `PriorityTag` monoid.
4. `finger_tree_interval_index.hpp` + `_ft5.t.cpp` — custom `IntervalMaxEndTag` monoid (most complex).

### Gotchas

1. **Existing FT2 tests must still pass unchanged.**
   The default template argument preserves the API for old call sites.
   Verify after each wrapper change.

2. **Foldable/Traversable adapters for the wrappers.**
   Check whether the wrappers register their own `*_typeclass` specializations.
   If they do, those specializations need to track the tree-type parameter too.
   (Likely they don't — adapters are mostly on FT2 directly — but verify.)

3. **`finger_tree_wrappers.hpp`** is an aggregator header that includes all four.
   It needs no change as long as the four headers stay self-contained.

4. **CI / build coverage.**
   Adding 4 new test files grows the test executable but doesn't change targets.
   No CMakeLists changes other than adding test sources.

5. **Don't try to abstract over `T`/`Tag`/`MeasurePolicy` separately.**
   The single `typename Tree` parameter is the right granularity.
   The wrapper's own `T = std::string` or `Tag = std::size_t` decisions are baked into the *default* tree type, not into the wrapper's own template parameters.

### Tests

For each wrapper's FT5 test file, port the same test cases that exist in the FT2 test file (look at `finger_tree_random_access.t.cpp` etc. for reference) and replace `FingerTreeX<>` with `FingerTreeX<FingerTree5<...>>`.
Add one cross-checking test per wrapper: build the same content in both an FT2-backed and FT5-backed wrapper, perform the same operations, assert the outputs match.
This catches subtle API divergence between FT2 and FT5.

### Commit shape

One commit per wrapper, four total.
Title each like "parameterize FingerTreeRandomAccess on tree type, validate under FT5".
Keep them small so reviewers can see the parameterization is mechanical.

---

## Conventions to honor throughout

- **Build/test cadence**: after each non-trivial edit, `make compile` and run the relevant `ctest -R` subset.
  The full suite (`ctest` with no `-R`) at the end of each phase.
  Tests must be 100% green at every commit.
- **Header self-containment**: `make compile-headers` after creating any new `.hpp`.
  If it fails, the header is missing an include or has a circular dep.
- **Commit message**: imperative-mood title (~70 char max), blank line, then a body explaining *why* in 2–4 short paragraphs.
  End with `Co-Authored-By: Claude Opus 4.7 (1M context) <noreply@anthropic.com>`.
  See the FT5 series commits (`git log 1322564a..5dfdd7c4`) for exemplars.
- **Don't run linters** (`make lint`) unless asked — they download tools on first run and are slow.
  Trust `clang-format` defaults; the project formats automatically via pre-commit if invoked.
- **Don't touch** `finger_tree2/3/4.hpp` — they're stable.
  Don't touch `CLAUDE.md` unless you're adding genuinely new project-level guidance (the file is documentation for *future* Claude sessions, not a scratchpad).
- **API parity rule**: any new method on FT5 must also work as additive on the existing public surface.
  Don't change existing method signatures; don't add a parameter to `split` etc.

---

## Suggested overall sequence

Phase A: **Iterator** (Item 1).
Smallest blast radius, gives FT5 a feature FT4 lacks, no API contracts at risk.

Phase B: **DualMonoid** standalone (first commit of Item 2), then **`reversed()`** (second commit of Item 2).
Two reviewable pieces.

Phase C: **Wrapper parameterization** (Item 3), one wrapper per commit.
Defer this if iterator/reversed are higher priority; the wrappers continue to work fine as-is.

Total: ~7–8 commits across three logical phases.
