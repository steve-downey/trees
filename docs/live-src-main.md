---
title: Live Source Snapshot (main)
summary: Point-in-time fenced dump of live src C++ sources from main, excluding deadcode and smd/conceptmap.
source_of_truth: git HEAD on branch main
scope:
  include:
    - src/**/*.hpp
    - src/**/*.h
    - src/**/*.cpp
  exclude:
    - src/deadcode/**
    - src/smd/conceptmap/**
    - src/**/CMakeLists.txt
update_policy:
  when_to_update:
    - Any time live files under src are added, removed, renamed, or materially changed on main.
    - Before using this file as a review/reference baseline.
  how_to_update:
    - Regenerate from main using the command block in the "Regeneration" section below.
    - Replace this file atomically with regenerated output.
notes:
  - Section headers are canonical paths without the leading src/ prefix.
  - File contents are copied from git (HEAD), not the working tree.
---

# Live Source Snapshot (main)

Generated from main at commit 2bfab86c.

Includes files under src that are live in current targets and examples, excluding deadcode and smd/conceptmap.
Canonical names below omit the leading src/ prefix.

## examples/cpo_example.cpp

```cpp
#include <concepts>
#include <string>

namespace N::hidden {
template <typename T>
concept has_eq = requires(const T &v) {
    { eq(v, v) } -> std::same_as<bool>;
};

struct eq_fn {
    template <has_eq T>
    constexpr bool operator()(const T &x, const T &y) const {
        return eq(x, y);
    }
};

template <has_eq T>
constexpr bool ne(const T &x, const T &y) {
    return not eq(x, y);
}

template <typename T>
concept has_ne = requires(const T &v) {
    { ne(v, v) } -> std::same_as<bool>;
};

struct ne_fn {
    template <has_ne T>
    constexpr bool operator()(const T &x, const T &y) const {
        return ne(x, y);
    }
};
} // namespace N::hidden

namespace N {
inline namespace function_objects {
inline constexpr hidden::eq_fn eq{};
inline constexpr hidden::ne_fn ne{};
} // namespace function_objects

template <typename T>
concept equality_comparable = requires(const std::remove_reference_t<T> &t) {
    eq(t, t);
    ne(t, t);
};
} // namespace N

struct test {
    std::string id;

    friend bool eq(const test &t1, const test &t2) { return t1.id == t2.id; }
};

int main() {
    static_assert(N::equality_comparable<test>);

    test t1, t2;
    return N::ne(t1, t2);
}

```

## examples/fixpoint_tree_example.cpp

```cpp
// src/examples/fixpoint_tree_example.cpp                              -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <iostream>
#include <memory>
#include <variant>

// Box<A>: value-semantic wrapper for recursive positions.
// Stands in for std::indirect (C++26) until it ships.
template <typename A>
using Box = std::shared_ptr<A>;

template <typename A, typename... Args>
auto make_box(Args &&...args) -> Box<A> {
    return std::make_shared<A>(std::forward<Args>(args)...);
}

// 1. The flat, non-recursive functor.  No inheritance, no CRTP.
template <typename A>
struct Const {
    int val;
};
template <typename A>
struct Add {
    Box<A> left, right;
};
template <typename A>
struct Mul {
    Box<A> left, right;
};

template <typename A>
using ExprF = std::variant<Const<A>, Add<A>, Mul<A>>;

// 2. The type-level fixed-point combinator.  Pure composition (has-a).
template <template <typename> class F>
struct Fix {
    F<Fix<F>> inner;
};

// 3. Zero-cost phantom operations (isorecursive boundaries)
template <template <typename> class F>
constexpr Fix<F> wrap(F<Fix<F>> f) {
    return Fix<F>{std::move(f)};
}

template <template <typename> class F>
constexpr const F<Fix<F>> &unwrap(const Fix<F> &f) {
    return f.inner;
}

// 4. Visitor helper and fmap
template <class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};
template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

template <typename A, typename B, typename Func>
ExprF<B> fmap(Func &&f, const ExprF<A> &fa) {
    return std::visit(
        overloaded{
            [](const Const<A> &c) -> ExprF<B> { return Const<B>{c.val}; },
            [&f](const Add<A> &a) -> ExprF<B> {
                return Add<B>{make_box<B>(f(*a.left)),
                              make_box<B>(f(*a.right))};
            },
            [&f](const Mul<A> &m) -> ExprF<B> {
                return Mul<B>{make_box<B>(f(*m.left)),
                              make_box<B>(f(*m.right))};
            },
        },
        fa);
}

// 5. Catamorphism (cata): the universal fold for fixpoint types
template <template <typename> class F, typename Algebra>
auto cata(const Algebra &alg, const Fix<F> &tree)
    -> decltype(std::visit(alg, std::declval<F<int>>())) {
    using Carrier = decltype(std::visit(alg, std::declval<F<int>>()));
    return std::visit(alg, fmap<Fix<F>, Carrier>(
                               [&alg](const Fix<F> &child) -> Carrier {
                                   return cata<F>(alg, child);
                               },
                               unwrap(tree)));
}

// 6. Smart constructors
using ExprTree = Fix<ExprF>;

ExprTree const_node(int v) { return wrap<ExprF>(Const<ExprTree>{v}); }

ExprTree add_node(ExprTree l, ExprTree r) {
    return wrap<ExprF>(Add<ExprTree>{make_box<ExprTree>(std::move(l)),
                                     make_box<ExprTree>(std::move(r))});
}

ExprTree mul_node(ExprTree l, ExprTree r) {
    return wrap<ExprF>(Mul<ExprTree>{make_box<ExprTree>(std::move(l)),
                                     make_box<ExprTree>(std::move(r))});
}

// 7. Usage: building and evaluating
int main() {
    // Build the tree: (2 * 3) + 4
    ExprTree tree =
        add_node(mul_node(const_node(2), const_node(3)), const_node(4));

    // Evaluate using a non-recursive algebra
    auto eval_algebra = overloaded{
        [](const Const<int> &c) { return c.val; },
        [](const Add<int> &a) { return *a.left + *a.right; },
        [](const Mul<int> &m) { return *m.left * *m.right; },
    };

    std::cout << "Result: " << cata<ExprF>(eval_algebra, tree) << "\n";
    return 0;
}

```

## examples/main.cpp

```cpp
#include <cassert>
#include <iostream>
#include <smd/conceptmap/monoid.h>
#include <vector>

using namespace smd::conceptmap;

template <typename P, const auto &monoid = monoid_concept_map<P>>
void testP() {
    auto x = monoid.identity();
    assert(P{} == x);
    auto sum = monoid.op(x, P{1});
    assert(P{1} == sum);
    std::vector<P> v = {1, 2, 3, 4};
    auto k = monoid.concat(v);
    assert(k == 10);
}

template <typename P, const auto &monoid = monoid_concept_map<P>>
P testP2() {
    auto x = monoid.identity();
    auto op = monoid.op(x, P{2});
    assert(P{2} == op);
    std::vector<P> v = {1, 2, 3, 4};
    auto k = monoid.concat(v);
    return k;
}

void test() {
    std::cout << "\ntest int\n";
    testP<int>();
    std::cout << "\ntest long\n";
    testP<long>();
    std::cout << "\ntest char\n";
    testP<char>();

    std::cout << "\ntest int\n";
    int k1 = testP2<int>();
    assert(k1 == 10);

    std::cout << "\ntest int\n";
    int k2 = testP2<int, mult_map<int>>();
    assert(k2 == 24);

    std::cout << "\ntest string\n";
    auto d2 = monoid_concept_map<std::string>;
    auto x2 = d2.identity();
    assert(std::string{} == x2);
    auto sum2 = d2.op(x2, "1");
    assert(std::string{"1"} == sum2);
    std::vector<std::string> vs = {"1", "2", "3", "4"};
    auto k3 = d2.concat(vs);
    assert(k3 == std::string{"1234"});
}

int main() { test(); }

```

## examples/map_example.cpp

```cpp
#include <concepts>
#include <string>

namespace N::hidden {
template <typename T>
concept has_eq = requires(const T &v) {
    { operator==(v, v) } -> std::same_as<bool>;
};

inline constexpr struct partial_eq_default_t {
    constexpr bool eq(const has_eq auto &rhs, const has_eq auto &lhs) const {
        return (rhs == lhs);
    }
    constexpr bool ne(const has_eq auto &rhs, const has_eq auto &lhs) const {
        return (lhs != rhs);
    }
} partial_eq_default;

} // namespace N::hidden

namespace N {

template <class T>
inline constexpr auto partial_eq = hidden::partial_eq_default;

template <class T>
concept partial_equality = requires(const std::remove_reference_t<T> &t) {
    { partial_eq<T>.eq(t, t) } -> std::same_as<bool>;
    { partial_eq<T>.ne(t, t) } -> std::same_as<bool>;
};
} // namespace N

struct test {
    int id{};

    friend auto operator<=>(const test &, const test &) = default;
};

int main() {
    static_assert(N::partial_equality<test>);

    test t1, t2;
    return N::partial_eq<test>.ne(t1, t2);
}

// https://compiler-explorer.com/z/48E7osfxE

```

## smd/fixpoint/box.hpp

```cpp
// src/smd/fixpoint/box.hpp                                          -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_FIXPOINT_BOX
#define INCLUDED_SMD_FIXPOINT_BOX

#include <memory>

namespace smd::fixpoint {

/** Indirection type used inside F<Fix<F>> to break infinite template
 * instantiation. Box<A> = shared_ptr<A>; structural sharing is a side-effect.
 * @tparam A the pointed-to type (typically a recursive Fix instantiation)
 */
template <typename A>
using Box = std::shared_ptr<A>;

/** Construct a Box<A> in-place, forwarding @p args to A's constructor. */
template <typename A, typename... Args>
auto make_box(Args &&...args) -> Box<A> {
    return std::make_shared<A>(std::forward<Args>(args)...);
}

} // namespace smd::fixpoint

#endif

```

## smd/fixpoint/box.t.cpp

```cpp
// src/smd/fixpoint/box.t.cpp                                         -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <smd/fixpoint/box.hpp>
#include <smd/fixpoint/box.hpp> // Re-inclusion check

#include <catch2/catch_test_macros.hpp>

#include <concepts>
#include <string>

using smd::fixpoint::Box;
using smd::fixpoint::make_box;

static_assert(std::same_as<Box<int>, std::shared_ptr<int>>);

TEST_CASE("Box - MakeBoxInt") {
    auto b = make_box<int>(42);
    REQUIRE(b);
    CHECK(*b == 42);
}

TEST_CASE("Box - MakeBoxString") {
    auto b = make_box<std::string>("hello");
    REQUIRE(b);
    CHECK(*b == "hello");
}

TEST_CASE("Box - SharedOwnership") {
    auto b1 = make_box<int>(7);
    Box<int> b2 = b1;
    CHECK(b1.get() == b2.get());
    CHECK(*b1 == *b2);
}

```

## smd/fixpoint/cata.hpp

```cpp
// src/smd/fixpoint/cata.hpp                                          -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_FIXPOINT_CATA
#define INCLUDED_SMD_FIXPOINT_CATA

#include <smd/fixpoint/fix.hpp>

namespace smd::fixpoint {

/**
 * @brief Catamorphism: bottom-up fold over a Fix<F> tree.
 *
 * Recursively descends into the tree, applying @p algebra at each level.
 * At each node the @p fmap_fn lifts the recursive fold into the functor layer
 * so that @p algebra receives an F<Result> rather than an F<Fix<F>>.
 *
 * @tparam Result   the type produced at each level by @p algebra
 * @tparam F        the non-recursive functor whose fixed-point is being folded
 * @param algebra   function F<Result> -> Result (the catamorphism algebra)
 * @param fmap_fn   function (Fix<F>->Result, F<Fix<F>>) -> F<Result>
 *                  — lifts the fold function over one layer of F
 * @param tree      the fixed-point tree to fold
 * @return          the folded result at the root
 */
template <typename Result, template <typename> class F, typename Algebra,
          typename FMap>
auto cata(const Algebra &algebra, const FMap &fmap_fn, const Fix<F> &tree)
    -> Result {
    const auto &layer = unwrap(tree);
    auto evaluated = fmap_fn(
        [&](const Fix<F> &child) -> Result {
            return cata<Result>(algebra, fmap_fn, child);
        },
        layer);
    return algebra(evaluated);
}

} // namespace smd::fixpoint

#endif

```

## smd/fixpoint/cata.t.cpp

```cpp
// src/smd/fixpoint/cata.t.cpp                                        -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <smd/fixpoint/cata.hpp>
#include <smd/fixpoint/cata.hpp> // Re-inclusion check

#include <smd/fixpoint/box.hpp>
#include <smd/fixpoint/overloaded.hpp>

#include <catch2/catch_test_macros.hpp>

#include <functional>
#include <variant>

using smd::fixpoint::Box;
using smd::fixpoint::cata;
using smd::fixpoint::Fix;
using smd::fixpoint::make_box;
using smd::fixpoint::overloaded;
using smd::fixpoint::wrap;

namespace {

struct Zero {};

template <typename A>
struct Succ {
    Box<A> pred;
};

template <typename A>
using NatF = std::variant<Zero, Succ<A>>;

using Nat = Fix<NatF>;

auto make_zero() -> Nat { return wrap<NatF>(NatF<Nat>{Zero{}}); }

auto make_succ(Nat n) -> Nat {
    return wrap<NatF>(NatF<Nat>{Succ<Nat>{make_box<Nat>(std::move(n))}});
}

template <typename A, typename F>
auto fmap_nat(F &&f, const NatF<A> &nat) {
    using B = std::invoke_result_t<F, const A &>;
    return std::visit(overloaded{
                          [](const Zero &) -> NatF<B> { return Zero{}; },
                          [&f](const Succ<A> &s) -> NatF<B> {
                              return Succ<B>{
                                  make_box<B>(std::invoke(f, *s.pred))};
                          },
                      },
                      nat);
}

auto fmap_nat_fn = [](auto &&f, const auto &nat) {
    return fmap_nat(std::forward<decltype(f)>(f), nat);
};

auto count_algebra = [](const NatF<int> &n) -> int {
    return std::visit(overloaded{
                          [](const Zero &) { return 0; },
                          [](const Succ<int> &s) { return *s.pred + 1; },
                      },
                      n);
};

} // namespace

TEST_CASE("Cata - NatZero") {
    auto zero = make_zero();
    CHECK(cata<int>(count_algebra, fmap_nat_fn, zero) == 0);
}

TEST_CASE("Cata - NatTwo") {
    auto two = make_succ(make_succ(make_zero()));
    CHECK(cata<int>(count_algebra, fmap_nat_fn, two) == 2);
}

TEST_CASE("Cata - NatFive") {
    auto n = make_zero();
    for (int i = 0; i < 5; ++i) {
        n = make_succ(std::move(n));
    }
    CHECK(cata<int>(count_algebra, fmap_nat_fn, n) == 5);
}

TEST_CASE("Cata - NatCustomAlgebra") {
    auto three = make_succ(make_succ(make_succ(make_zero())));

    auto bool_algebra = [](const NatF<bool> &n) -> bool {
        return std::visit(overloaded{
                              [](const Zero &) { return true; },
                              [](const Succ<bool> &s) { return !*s.pred; },
                          },
                          n);
    };

    CHECK(cata<bool>(bool_algebra, fmap_nat_fn, three) == false);
}

```

## smd/fixpoint/fix.hpp

```cpp
// src/smd/fixpoint/fix.hpp                                           -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_FIXPOINT_FIX
#define INCLUDED_SMD_FIXPOINT_FIX

#include <utility>

namespace smd::fixpoint {

/** Fixed-point combinator that ties the recursive knot for a functor @p F.
 * 
 * Fix<F> is the iso-recursive type satisfying Fix<F> ≅ F<Fix<F>>.
 * The single data member @c inner holds one unwrapped layer; wrap/unwrap
 * are the iso-recursive isomorphism boundary.
 * Use Box<Fix<F>> inside F to avoid infinite template instantiation depth.
 * @tparam F unary template functor (takes the recursive position as its param)
 */
template <template <typename> class F>
struct Fix {
    F<Fix<F>> inner;
};

/** Wrap one layer of @p F into the fixed-point type. */
template <template <typename> class F>
constexpr auto wrap(F<Fix<F>> layer) -> Fix<F> {
    return Fix<F>{std::move(layer)};
}

/** Unwrap one layer from a fixed-point value, exposing F<Fix<F>>. */
template <template <typename> class F>
constexpr auto unwrap(const Fix<F> &fixed) -> const F<Fix<F>> & {
    return fixed.inner;
}

} // namespace smd::fixpoint

#endif

```

## smd/fixpoint/fix.t.cpp

```cpp
// src/smd/fixpoint/fix.t.cpp                                         -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <smd/fixpoint/fix.hpp>
#include <smd/fixpoint/fix.hpp> // Re-inclusion check

#include <smd/fixpoint/box.hpp>

#include <catch2/catch_test_macros.hpp>

#include <variant>

using smd::fixpoint::Box;
using smd::fixpoint::Fix;
using smd::fixpoint::make_box;
using smd::fixpoint::unwrap;
using smd::fixpoint::wrap;

namespace {

struct Zero {};

template <typename A>
struct Succ {
    Box<A> pred;
};

template <typename A>
using NatF = std::variant<Zero, Succ<A>>;

} // namespace

TEST_CASE("Fix - NatFZero") {
    using Nat = Fix<NatF>;
    auto zero = wrap<NatF>(NatF<Nat>{Zero{}});
    const auto &layer = unwrap(zero);
    CHECK(std::holds_alternative<Zero>(layer));
}

TEST_CASE("Fix - NatFSucc") {
    using Nat = Fix<NatF>;
    auto zero = wrap<NatF>(NatF<Nat>{Zero{}});
    auto one = wrap<NatF>(NatF<Nat>{Succ<Nat>{make_box<Nat>(zero)}});
    auto two = wrap<NatF>(NatF<Nat>{Succ<Nat>{make_box<Nat>(one)}});

    const auto &layer2 = unwrap(two);
    REQUIRE(std::holds_alternative<Succ<Nat>>(layer2));

    const auto &layer1 = unwrap(*std::get<Succ<Nat>>(layer2).pred);
    REQUIRE(std::holds_alternative<Succ<Nat>>(layer1));

    const auto &layer0 = unwrap(*std::get<Succ<Nat>>(layer1).pred);
    CHECK(std::holds_alternative<Zero>(layer0));
}

TEST_CASE("Fix - WrapUnwrapRoundTrip") {
    using Nat = Fix<NatF>;
    NatF<Nat> layer{Zero{}};
    auto fixed = wrap<NatF>(layer);
    const auto &recovered = unwrap(fixed);
    CHECK(std::holds_alternative<Zero>(recovered));
}

```

## smd/fixpoint/overloaded.hpp

```cpp
// src/smd/fixpoint/overloaded.hpp                                    -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_FIXPOINT_OVERLOADED
#define INCLUDED_SMD_FIXPOINT_OVERLOADED

namespace smd::fixpoint {

/** Aggregate that inherits operator() from each of @p Ts.
 * Used with std::visit to combine multiple lambdas into a single visitor
 * without writing a hand-rolled visitor struct.
 * Example: std::visit(overloaded{case1, case2, ...}, variant)
 */
template <typename... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};

/** Deduction guide so overloaded{...} works without explicit template args. */
template <typename... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

} // namespace smd::fixpoint

#endif

```

## smd/ranges/range_applicative.hpp

```cpp
// src/smd/ranges/range_applicative.hpp                               -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_RANGES_RANGE_APPLICATIVE
#define INCLUDED_SMD_RANGES_RANGE_APPLICATIVE

#include <smd/ranges/range_list.hpp>
#include <smd/typeclass/applicative.hpp>

#include <functional>
#include <ranges>
#include <type_traits>
#include <utility>
#include <vector>

namespace smd {

/** Applicative typeclass instance for list_range<VIEW> with Cartesian-product
 * (nondeterminism / list-monad) semantics.
 * 
 * pure(x) = {x} (singleton range).
 * apply(fs, xs) = {f(x) | f <- fs, x <- xs} — every function applied to
 * every argument, producing all combinations in fs-major order.
 * @tparam VIEW underlying view type of the list_range
 */
template <class VIEW>
struct ListRangeApplicativeImpl {
    /** Lift a value into a single-element range. */
    template <class VALUE>
    auto pure(this auto &&, VALUE &&value) {
        using Stored = remove_cvref_t<VALUE>;
        return smd::ranges::from_vector(
            std::vector<Stored>{std::forward<VALUE>(value)});
    }

    /**
     * @brief Apply every function in @p functions to every argument in
     *        @p arguments (Cartesian product).
     * @return range of size |functions| * |arguments|, in fs-major order
     */
    template <class FUNCTION_VIEW, class ARGUMENT_VIEW>
    auto apply(this auto &&,
               const smd::ranges::list_range<FUNCTION_VIEW> &functions,
               const smd::ranges::list_range<ARGUMENT_VIEW> &arguments) {
        using Function =
            std::ranges::range_value_t<smd::ranges::list_range<FUNCTION_VIEW>>;
        using Argument =
            std::ranges::range_value_t<smd::ranges::list_range<ARGUMENT_VIEW>>;
        using Result = std::invoke_result_t<const Function &, const Argument &>;

        auto function_values = smd::ranges::detail::materialize(functions);
        auto argument_values = smd::ranges::detail::materialize(arguments);
        std::vector<remove_cvref_t<Result>> output;
        output.reserve(function_values.size() * argument_values.size());

        for (const auto &function : function_values) {
            for (const auto &argument : argument_values) {
                output.push_back(std::invoke(function, argument));
            }
        }

        return smd::ranges::from_vector(std::move(output));
    }
};

/** Applicative map exposing pure and apply for list_range<VIEW>. */
template <class VIEW>
struct ListRangeApplicativeMap : Applicative<ListRangeApplicativeImpl<VIEW>> {
    using ListRangeApplicativeImpl<VIEW>::apply;
    using ListRangeApplicativeImpl<VIEW>::pure;
};

/** Registers ListRangeApplicativeMap as the Applicative instance for list_range<VIEW>. */
template <class VIEW>
inline constexpr auto applicative_typeclass<smd::ranges::list_range<VIEW>> =
    ListRangeApplicativeMap<VIEW>{};

} // namespace smd

#endif

```

## smd/ranges/range_applicative.t.cpp

```cpp
#include <smd/ranges/range_applicative.hpp>
#include <smd/ranges/range_applicative.hpp> // Re-inclusion check

#include <catch2/catch_test_macros.hpp>

#include <vector>

namespace {

template <std::ranges::input_range RANGE>
auto collect(RANGE &&range) {
    using Value = std::ranges::range_value_t<RANGE>;
    std::vector<Value> values;

    for (auto &&value : range) {
        values.emplace_back(value);
    }

    return values;
}

} // namespace

TEST_CASE("RangeApplicativeTest - PureCreatesSingletonRange") {
    const auto &applicative =
        smd::applicative_typeclass<decltype(smd::ranges::single(1))>;

    auto singleton = applicative.pure(7);

    CHECK(collect(singleton) == (std::vector<int>{7}));
}

TEST_CASE("RangeApplicativeTest - InvokeUsesListNondeterminism") {
    auto lhs = smd::ranges::from_vector(std::vector<int>{1, 2});
    auto rhs = smd::ranges::from_vector(std::vector<int>{10, 20});
    const auto &applicative = smd::applicative_typeclass<decltype(lhs)>;

    auto summed = applicative.invoke(
        [](int left, int right) { return left + right; }, lhs, rhs);

    CHECK(collect(summed) == (std::vector<int>{11, 21, 12, 22}));
}

```

## smd/ranges/range_foldable.hpp

```cpp
// src/smd/ranges/range_foldable.hpp                                  -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_RANGES_RANGE_FOLDABLE
#define INCLUDED_SMD_RANGES_RANGE_FOLDABLE

#include <smd/ranges/range_list.hpp>
#include <smd/typeclass/foldable.hpp>

#include <algorithm>
#include <functional>
#include <ranges>
#include <utility>

namespace smd {

/** Foldable typeclass instance for list_range<VIEW>.
 * Provides fold_map, fold_left, fold_right, and convenience query operations.
 * @tparam VIEW underlying view type
 */
template <class VIEW>
struct ListRangeFoldableImpl {
    /** Map @p function over all elements and combine results via their Monoid. */
    template <class FUNCTION>
    auto fold_map(this auto &&self, FUNCTION &&function,
                  const smd::ranges::list_range<VIEW> &values) {
        using Result = remove_cvref_t<std::invoke_result_t<
            FUNCTION,
            const typename smd::ranges::list_range<VIEW>::value_type &>>;

        return self.fold_left(values, smd::monoid_identity<Result>(),
                              [&function](Result acc, const auto &value) {
                                  return smd::monoid_combine(
                                      std::move(acc),
                                      std::invoke(function, value));
                              });
    }

    /** Return the number of elements in the range. */
    auto length(this auto &&, const smd::ranges::list_range<VIEW> &values)
        -> std::size_t {
        return static_cast<std::size_t>(std::ranges::distance(values));
    }

    /** Left fold: reduce the range to a single value, left-to-right. */
    template <class STATE, class FUNCTION>
    auto fold_left(this auto &&, const smd::ranges::list_range<VIEW> &values,
                   STATE initial_state, FUNCTION &&function) {
        return std::ranges::fold_left(values, std::move(initial_state),
                                      std::forward<FUNCTION>(function));
    }

    /** Right fold: reduce the range to a single value, right-to-left.
     * Materializes the range into a vector first to support reverse iteration.
     */
    template <class STATE, class FUNCTION>
    auto fold_right(this auto &&, const smd::ranges::list_range<VIEW> &values,
                    STATE initial_state, FUNCTION &&function) {
        return std::ranges::fold_right(smd::ranges::detail::materialize(values),
                                       std::move(initial_state),
                                       std::forward<FUNCTION>(function));
    }

    /** True if any element satisfies @p predicate. */
    template <class PREDICATE>
    auto any_of(this auto &&, const smd::ranges::list_range<VIEW> &values,
                PREDICATE &&predicate) -> bool {
        return std::ranges::any_of(values, std::forward<PREDICATE>(predicate));
    }

    /** True if all elements satisfy @p predicate. */
    template <class PREDICATE>
    auto all_of(this auto &&, const smd::ranges::list_range<VIEW> &values,
                PREDICATE &&predicate) -> bool {
        return std::ranges::all_of(values, std::forward<PREDICATE>(predicate));
    }

    /** True if the range contains no elements. */
    auto empty(this auto &&, const smd::ranges::list_range<VIEW> &values)
        -> bool {
        return std::ranges::empty(values);
    }

    /** Materialize the range into a vector. */
    auto to_vector(this auto &&, const smd::ranges::list_range<VIEW> &values) {
        return smd::ranges::detail::materialize(values);
    }

    /**
     * @brief Return the first element satisfying @p predicate, or nullopt.
     * @return optional containing the first matching value, or nullopt
     */
    template <class PREDICATE>
    auto find_first(this auto &&, const smd::ranges::list_range<VIEW> &values,
                    PREDICATE &&predicate) {
        auto it =
            std::ranges::find_if(values, std::forward<PREDICATE>(predicate));
        if (it == std::ranges::end(values)) {
            return std::optional<
                typename smd::ranges::list_range<VIEW>::value_type>{};
        }
        return std::optional<
            typename smd::ranges::list_range<VIEW>::value_type>{*it};
    }
};

/** Foldable map exposing all fold operations for list_range<VIEW>. */
template <class VIEW>
struct ListRangeFoldableMap : Foldable<ListRangeFoldableImpl<VIEW>> {
    using ListRangeFoldableImpl<VIEW>::all_of;
    using ListRangeFoldableImpl<VIEW>::any_of;
    using ListRangeFoldableImpl<VIEW>::empty;
    using ListRangeFoldableImpl<VIEW>::find_first;
    using ListRangeFoldableImpl<VIEW>::fold_map;
    using ListRangeFoldableImpl<VIEW>::fold_left;
    using ListRangeFoldableImpl<VIEW>::fold_right;
    using ListRangeFoldableImpl<VIEW>::length;
    using ListRangeFoldableImpl<VIEW>::to_vector;
};

/** Registers ListRangeFoldableMap as the Foldable instance for list_range<VIEW>. */
template <class VIEW>
inline constexpr auto foldable_typeclass<smd::ranges::list_range<VIEW>> =
    ListRangeFoldableMap<VIEW>{};

} // namespace smd

#endif

```

## smd/ranges/range_foldable.t.cpp

```cpp
#include <smd/ranges/range_foldable.hpp>
#include <smd/ranges/range_foldable.hpp> // Re-inclusion check

#include <catch2/catch_test_macros.hpp>

#include <ranges>
#include <vector>

TEST_CASE("RangeFoldableTest - LengthAndFoldLeftFollowRangeOrder") {
    auto values = smd::ranges::all(std::views::iota(1, 5));
    const auto &foldable = smd::foldable_typeclass<decltype(values)>;

    CHECK(foldable.length(values) == 4U);

    const auto folded = foldable.fold_left(
        values, 0, [](int acc, int value) { return acc * 10 + value; });
    CHECK(folded == 1234);
}

TEST_CASE("RangeFoldableTest - ToVectorMaterializesValues") {
    auto values = smd::ranges::from_vector(std::vector<int>{3, 1, 4});
    const auto &foldable = smd::foldable_typeclass<decltype(values)>;

    CHECK(foldable.to_vector(values) == (std::vector<int>{3, 1, 4}));
}

TEST_CASE("RangeFoldableTest - PredicatesAndFindUseRangeSemantics") {
    auto values = smd::ranges::all(std::views::iota(1, 6));
    const auto &foldable = smd::foldable_typeclass<decltype(values)>;

    CHECK(foldable.any_of(values, [](int value) { return value == 4; }));
    CHECK_FALSE(foldable.all_of(values, [](int value) { return value < 5; }));
    CHECK_FALSE(foldable.empty(values));

    auto found =
        foldable.find_first(values, [](int value) { return value > 3; });
    REQUIRE(found.has_value());
    CHECK(*found == 4);
}

```

## smd/ranges/range_functor.hpp

```cpp
// src/smd/ranges/range_functor.hpp                                   -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_RANGES_RANGE_FUNCTOR
#define INCLUDED_SMD_RANGES_RANGE_FUNCTOR

#include <smd/ranges/range_list.hpp>
#include <smd/typeclass/functor.hpp>

#include <ranges>
#include <utility>

namespace smd {

/** Functor typeclass instance for list_range<VIEW>.
 * fmap applies @p function lazily to each element via std::views::transform
 * and wraps the result in a new list_range.
 * @tparam VIEW underlying view type of the list_range being mapped
 */
template <class VIEW>
struct ListRangeFunctorImpl {
    template <class FUNCTION>
    auto fmap(this auto &&, FUNCTION &&function,
              const smd::ranges::list_range<VIEW> &values) {
        return smd::ranges::all(
            values | std::views::transform(std::forward<FUNCTION>(function)));
    }
};

/** Functor map that exposes fmap for list_range<VIEW>. */
template <class VIEW>
struct ListRangeFunctorMap : Functor<ListRangeFunctorImpl<VIEW>> {
    using ListRangeFunctorImpl<VIEW>::fmap;
};

/** Registers ListRangeFunctorMap as the Functor instance for list_range<VIEW>. */
template <class VIEW>
inline constexpr auto functor_typeclass<smd::ranges::list_range<VIEW>> =
    ListRangeFunctorMap<VIEW>{};

} // namespace smd

#endif

```

## smd/ranges/range_functor.t.cpp

```cpp
#include <smd/ranges/range_functor.hpp>
#include <smd/ranges/range_functor.hpp> // Re-inclusion check

#include <catch2/catch_test_macros.hpp>

#include <ranges>
#include <vector>

namespace {

template <std::ranges::input_range RANGE>
auto collect(RANGE &&range) {
    using Value = std::ranges::range_value_t<RANGE>;
    std::vector<Value> values;

    for (auto &&value : range) {
        values.emplace_back(value);
    }

    return values;
}

} // namespace

TEST_CASE("RangeFunctorTest - FmapUsesLazyRangeSemantics") {
    auto values = smd::ranges::all(std::views::iota(1, 5));
    const auto &functor = smd::functor_typeclass<decltype(values)>;

    auto mapped = functor.fmap([](int value) { return value * 10; }, values);

    CHECK(collect(mapped) == (std::vector<int>{10, 20, 30, 40}));
}

TEST_CASE("RangeFunctorTest - ReplaceKeepsRangeShape") {
    auto values = smd::ranges::all(std::views::iota(0, 3));
    const auto &functor = smd::functor_typeclass<decltype(values)>;

    auto replaced = functor.replace(values, 9);

    CHECK(collect(replaced) == (std::vector<int>{9, 9, 9}));
}

```

## smd/ranges/range_list.hpp

```cpp
// src/smd/ranges/range_list.hpp                                      -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_RANGES_RANGE_LIST
#define INCLUDED_SMD_RANGES_RANGE_LIST

#include <algorithm>
#include <iterator>
#include <ranges>
#include <type_traits>
#include <utility>
#include <vector>

namespace smd::ranges {

namespace detail {

template <std::ranges::input_range RANGE>
auto materialize(RANGE &&range) {
    using Value = std::ranges::range_value_t<RANGE>;
    std::vector<Value> values;

    if constexpr (std::ranges::sized_range<RANGE>) {
        values.reserve(std::ranges::size(range));
    }

    std::ranges::copy(range, std::back_inserter(values));

    return values;
}

} // namespace detail

/** Range adaptor that wraps any view and satisfies the Foldable, Applicative,
 * and Traversable interfaces. list_range is the canonical container type for
 * the range-based typeclass instances in smd::ranges.
 * @tparam VIEW underlying view type; must satisfy std::ranges::view and
 *              std::ranges::input_range
 */
template <class VIEW>
    requires(std::ranges::view<VIEW> && std::ranges::input_range<VIEW>)
class list_range : public std::ranges::view_interface<list_range<VIEW>> {
    VIEW d_view;

  public:
    using value_type = std::ranges::range_value_t<VIEW>;
    using view_type = VIEW;

    list_range()
        requires std::default_initializable<VIEW>
    = default;

    /** Construct a list_range wrapping @p view. */
    constexpr explicit list_range(VIEW view) : d_view(std::move(view)) {}

    constexpr auto begin() { return std::ranges::begin(d_view); }

    constexpr auto begin() const
        requires std::ranges::range<const VIEW>
    {
        return std::ranges::begin(d_view);
    }

    constexpr auto end() { return std::ranges::end(d_view); }

    constexpr auto end() const
        requires std::ranges::range<const VIEW>
    {
        return std::ranges::end(d_view);
    }

    /** Return a copy of the underlying view (lvalue overload). */
    constexpr auto base() const &
        requires std::copy_constructible<VIEW>
    {
        return d_view;
    }

    /** Return the underlying view by move (rvalue overload). */
    constexpr auto base() && { return std::move(d_view); }
};

/** Wrap any viewable range in a list_range using std::views::all. */
template <std::ranges::viewable_range RANGE>
auto all(RANGE &&range) {
    using View = std::views::all_t<RANGE>;
    return list_range<View>{std::views::all(std::forward<RANGE>(range))};
}

/** Create a single-element list_range holding @p value. */
template <class VALUE>
auto single(VALUE &&value) {
    using Stored = std::remove_cvref_t<VALUE>;
    return list_range<std::ranges::single_view<Stored>>{
        std::views::single(std::forward<VALUE>(value))};
}

/** Create a list_range from an owned vector. */
template <class VALUE>
auto from_vector(std::vector<VALUE> values) {
    return all(std::move(values));
}

} // namespace smd::ranges

#endif

```

## smd/ranges/range_list.t.cpp

```cpp
// src/smd/ranges/range_list.t.cpp                                    -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <smd/ranges/range_list.hpp>
#include <smd/ranges/range_list.hpp> // Re-inclusion check

#include <catch2/catch_test_macros.hpp>

#include <ranges>
#include <vector>

TEST_CASE("RangeListTest - FromVectorIterates") {
    auto rl = smd::ranges::from_vector(std::vector<int>{1, 2, 3});
    std::vector<int> got;
    for (auto x : rl) {
        got.push_back(x);
    }
    CHECK(got == (std::vector<int>{1, 2, 3}));
}

TEST_CASE("RangeListTest - SingleElementView") {
    auto rl = smd::ranges::single(42);
    CHECK(std::ranges::distance(rl) == 1);
    CHECK(*std::ranges::begin(rl) == 42);
}

TEST_CASE("RangeListTest - AllWrapsExistingVector") {
    std::vector<int> v{10, 20, 30};
    auto rl = smd::ranges::all(v);
    CHECK(std::ranges::distance(rl) == 3);
    CHECK(*std::ranges::begin(rl) == 10);
}

```

## smd/ranges/range_traversable.hpp

```cpp
// src/smd/ranges/range_traversable.hpp                               -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_RANGES_RANGE_TRAVERSABLE
#define INCLUDED_SMD_RANGES_RANGE_TRAVERSABLE

#include <smd/ranges/range_applicative.hpp>
#include <smd/ranges/range_list.hpp>
#include <smd/typeclass/traversable.hpp>

#include <functional>
#include <type_traits>
#include <utility>
#include <vector>

namespace smd {

/** Traversable typeclass instance for list_range<VIEW>.
 * traverse maps each element into an applicative context and collects the
 * results back into a list_range inside that context. Elements are processed
 * left-to-right; an empty range yields applicative.pure({}).
 * Requires a forward range so that the traversal can visit elements in order.
 * @tparam VIEW underlying view type; must satisfy std::ranges::forward_range
 */
template <class VIEW>
    requires std::ranges::forward_range<VIEW>
struct ListRangeTraversableImpl {
    using element_type = typename smd::ranges::list_range<VIEW>::value_type;

    template <class APPLICATIVE, class FUNCTION>
    auto traverse(this auto &&, const APPLICATIVE &applicative,
                  FUNCTION &&function,
                  const smd::ranges::list_range<VIEW> &values) {
        using Value = element_type;
        using Context =
            remove_cvref_t<std::invoke_result_t<FUNCTION, const Value &>>;
        using ResultValue = smd::applicative_value_t<Context>;

        auto current = std::ranges::begin(values);
        const auto last = std::ranges::end(values);

        if (current == last) {
            return applicative.map(
                [](auto &&materialized) {
                    return smd::ranges::from_vector(
                        std::forward<decltype(materialized)>(materialized));
                },
                applicative.pure(std::vector<ResultValue>{}));
        }

        auto collected = applicative.map(
            [](auto &&first_value) {
                using U = remove_cvref_t<decltype(first_value)>;
                return std::vector<U>{
                    std::forward<decltype(first_value)>(first_value)};
            },
            std::invoke(function, *current));
        ++current;

        for (; current != last; ++current) {
            auto lifted_value = std::invoke(function, *current);
            collected = applicative.invoke(
                [](std::vector<ResultValue> acc, auto &&next_value) {
                    acc.push_back(
                        std::forward<decltype(next_value)>(next_value));
                    return acc;
                },
                std::move(collected), std::move(lifted_value));
        }

        return applicative.map(
            [](auto &&materialized) {
                return smd::ranges::from_vector(
                    std::forward<decltype(materialized)>(materialized));
            },
            std::move(collected));
    }
};

/** Traversable map exposing traverse for list_range<VIEW>. */
template <class VIEW>
    requires std::ranges::forward_range<VIEW>
struct ListRangeTraversableMap : Traversable<ListRangeTraversableImpl<VIEW>> {
    using ListRangeTraversableImpl<VIEW>::traverse;
};

/** Registers ListRangeTraversableMap as the Traversable instance for list_range<VIEW>. */
template <class VIEW>
    requires std::ranges::forward_range<VIEW>
inline constexpr auto traversable_typeclass<smd::ranges::list_range<VIEW>> =
    ListRangeTraversableMap<VIEW>{};

} // namespace smd

#endif

```

## smd/ranges/range_traversable.t.cpp

```cpp
#include <smd/ranges/range_traversable.hpp>
#include <smd/ranges/range_traversable.hpp> // Re-inclusion check
#include <smd/ziplist/zip_list_applicative.hpp>

#include <catch2/catch_test_macros.hpp>

#include <optional>
#include <ranges>
#include <sstream>
#include <type_traits>
#include <vector>

#include <algorithm>

namespace {

template <std::ranges::input_range RANGE>
auto collect(RANGE &&range) {
    using Value = std::ranges::range_value_t<RANGE>;
    std::vector<Value> values;

    for (auto &&value : range) {
        values.emplace_back(value);
    }

    return values;
}

template <std::ranges::input_range OUTER_RANGE>
auto collect_nested(OUTER_RANGE &&outer_range) {
    std::vector<std::vector<
        std::ranges::range_value_t<std::ranges::range_value_t<OUTER_RANGE>>>>
        collected;

    for (auto &&inner_range : outer_range) {
        collected.push_back(collect(inner_range));
    }

    return collected;
}

auto to_vector_of_ziplists(
    const smd::zip_list<std::vector<int>> &zip_of_vectors)
    -> std::vector<smd::zip_list<int>> {
    std::vector<smd::zip_list<int>> rows;
    if (zip_of_vectors.data.empty()) {
        return rows;
    }

    std::size_t row_count = zip_of_vectors.data.front().size();
    for (const auto &column : zip_of_vectors.data) {
        row_count = std::min(row_count, column.size());
    }

    rows.assign(row_count, smd::zip_list<int>{});
    for (auto &row : rows) {
        row.data.reserve(zip_of_vectors.data.size());
    }

    for (std::size_t index = 0; index < row_count; ++index) {
        for (const auto &column : zip_of_vectors.data) {
            rows[index].data.push_back(column[index]);
        }
    }

    return rows;
}

} // namespace

TEST_CASE("RangeTraversableTest - TraverseOptionalSuccess") {
    // c7f3a1e8-2b5d-4f9c-a4e7-1b3d6c8a5f02
    auto values = smd::ranges::from_vector(std::vector<int>{1, 2, 3});

    auto traversed = smd::traverse(
        [](int value) -> std::optional<int> {
            return std::optional<int>{value + 1};
        },
        values);

    REQUIRE(traversed.has_value());
    CHECK(collect(*traversed) == (std::vector<int>{2, 3, 4}));
    // c7f3a1e8-2b5d-4f9c-a4e7-1b3d6c8a5f02 end
}

TEST_CASE("RangeTraversableTest - TraverseOptionalFailure") {
    // e9b1d4f2-7c3a-4e8b-f6c2-5d1a9e3b7f04
    auto values = smd::ranges::from_vector(std::vector<int>{1, -2, 3});

    auto traversed = smd::traverse(
        [](int value) -> std::optional<int> {
            return value >= 0 ? std::optional<int>{value + 1}
                              : std::optional<int>{};
        },
        values);

    CHECK_FALSE(traversed.has_value());
    // e9b1d4f2-7c3a-4e8b-f6c2-5d1a9e3b7f04 end
}

TEST_CASE(
    "RangeTraversableTest - TraverseWithRangeApplicativeEnumeratesChoices") {
    auto values = smd::ranges::from_vector(std::vector<int>{1, 2});

    auto traversed = smd::traverse(
        [](int value) {
            return smd::ranges::from_vector(
                std::vector<int>{value, value + 10});
        },
        values);

    CHECK(collect_nested(traversed) ==
          (std::vector<std::vector<int>>{{1, 2}, {1, 12}, {11, 2}, {11, 12}}));
}

TEST_CASE("RangeTraversableTest - TraversableIsNotDefinedForInputOnlyRanges") {
    using InputView = std::ranges::basic_istream_view<int, char>;
    using InputList = smd::ranges::list_range<InputView>;

    static_assert(
        std::is_same_v<decltype(smd::traversable_typeclass<InputList>),
                       const std::false_type>);
}

TEST_CASE(
    "RangeTraversableTest - SequenceConvertsRangeOfZiplistsToZiplistOfRanges") {
    // 0e9a7d13-9082-4b9e-b93f-86ef0e0ba20a
    using Zip = smd::zip_list<int>;
    auto values = smd::ranges::from_vector(std::vector<Zip>{
        Zip{{1, 2, 3}}, Zip{{10, 20}}, Zip{{100, 200, 300, 400}}});

    const auto &traversable = smd::traversable_typeclass<decltype(values)>;
    // d4f9b1e3-8c2a-4d7f-b6e1-3a5c9d2b7f48
    auto sequenced = traversable.sequence(values);

    REQUIRE(sequenced.data.size() == 2U);
    CHECK(collect(sequenced.data[0]) == (std::vector<int>{1, 10, 100}));
    CHECK(collect(sequenced.data[1]) == (std::vector<int>{2, 20, 200}));
    // d4f9b1e3-8c2a-4d7f-b6e1-3a5c9d2b7f48 end
    // 0e9a7d13-9082-4b9e-b93f-86ef0e0ba20a end
}

TEST_CASE("RangeTraversableTest - "
          "SequenceConvertsRangeOfZiplistsToZiplistOfRangesLengthFive") {
    using Zip = smd::zip_list<int>;
    auto values = smd::ranges::from_vector(
        std::vector<Zip>{Zip{{1, 2, 3, 4, 5}}, Zip{{10, 20, 30, 40, 50}},
                         Zip{{100, 200, 300, 400, 500}}});

    const auto &traversable = smd::traversable_typeclass<decltype(values)>;
    auto sequenced = traversable.sequence(values);

    REQUIRE(sequenced.data.size() == 5U);
    CHECK(collect(sequenced.data[0]) == (std::vector<int>{1, 10, 100}));
    CHECK(collect(sequenced.data[1]) == (std::vector<int>{2, 20, 200}));
    CHECK(collect(sequenced.data[2]) == (std::vector<int>{3, 30, 300}));
    CHECK(collect(sequenced.data[3]) == (std::vector<int>{4, 40, 400}));
    CHECK(collect(sequenced.data[4]) == (std::vector<int>{5, 50, 500}));
}

TEST_CASE("RangeTraversableTest - ConvertZiplistOfVectorsToVectorOfZiplists") {
    // 4be89584-35cc-4933-b3de-6d524d54371d
    smd::zip_list<std::vector<int>> zip_of_vectors{
        {{1, 10, 100}, {2, 20, 200}}};

    auto as_rows = to_vector_of_ziplists(zip_of_vectors);

    REQUIRE(as_rows.size() == 3U);
    CHECK(as_rows[0].data == (std::vector<int>{1, 2}));
    CHECK(as_rows[1].data == (std::vector<int>{10, 20}));
    CHECK(as_rows[2].data == (std::vector<int>{100, 200}));
    // 4be89584-35cc-4933-b3de-6d524d54371d end
}

TEST_CASE("RangeTraversableTest - "
          "ConvertZiplistOfVectorsToVectorOfZiplistsLengthFive") {
    smd::zip_list<std::vector<int>> zip_of_vectors{
        {{1, 10, 100, 1000, 10000}, {2, 20, 200, 2000, 20000}}};

    auto as_rows = to_vector_of_ziplists(zip_of_vectors);

    REQUIRE(as_rows.size() == 5U);
    CHECK(as_rows[0].data == (std::vector<int>{1, 2}));
    CHECK(as_rows[1].data == (std::vector<int>{10, 20}));
    CHECK(as_rows[2].data == (std::vector<int>{100, 200}));
    CHECK(as_rows[3].data == (std::vector<int>{1000, 2000}));
    CHECK(as_rows[4].data == (std::vector<int>{10000, 20000}));
}

```

## smd/thunk/delay.hpp

```cpp
// src/smd/thunk/delay.hpp                                            -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_THUNK_DELAY
#define INCLUDED_SMD_THUNK_DELAY

#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>

namespace smd::thunk {

/**
 * @brief Capture a callable and its arguments into a re-evaluating closure.
 *
 * The returned nullary callable stores @p c and @p args... by value; each
 * invocation re-evaluates callable(args...) from scratch. For call-once
 * (memoized) semantics use smd::thunk::memoize() instead.
 *
 * @param c       callable to defer
 * @param args    arguments forwarded into the closure by value
 * @return nullary callable that invokes c(args...) on every call
 */
template <typename Callable, typename... Args>
auto delay(Callable &&c, Args &&...args) {
    using CallableT = std::remove_cvref_t<Callable>;
    using ArgsTuple = std::tuple<std::remove_cvref_t<Args>...>;

    return [callable = CallableT(std::forward<Callable>(c)),
            arguments = ArgsTuple(std::forward<Args>(args)...)]() mutable {
        return std::apply(
            [&](auto &...unpacked) {
                return std::invoke(callable, unpacked...);
            },
            arguments);
    };
}

} // namespace smd::thunk

#endif

```

## smd/thunk/delay.t.cpp

```cpp
// src/smd/thunk/delay.t.cpp                                          -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#include <smd/thunk/delay.hpp>
#include <smd/thunk/delay.hpp> // Re-inclusion check

#include <catch2/catch_test_macros.hpp>

TEST_CASE("delay - invokes callable with captured arguments") {
    auto d = smd::thunk::delay([](int x, int y) { return x + y; }, 3, 4);
    CHECK(d() == 7);
}

TEST_CASE("delay - re-evaluates on each call") {
    int count = 0;
    auto d = smd::thunk::delay([&count]() { return ++count; });
    CHECK(d() == 1);
    CHECK(d() == 2);
    CHECK(d() == 3);
}

TEST_CASE("delay - captures args by value") {
    int x = 10;
    auto d = smd::thunk::delay([](int v) { return v * 2; }, x);
    x = 99;
    CHECK(d() == 20);
}

```

## smd/thunk/memoize.hpp

```cpp
// src/smd/thunk/memoize.hpp                                          -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_THUNK_MEMOIZE
#define INCLUDED_SMD_THUNK_MEMOIZE

// Thread-safety note (reviewer-three.org, 2026-05-01):
// memoize() uses a mutable std::variant<> behind a std::shared_ptr.  Copies
// of the returned callable share the same State object, so concurrent first
// calls from multiple threads constitute a data race.  While std::shared_ptr's
// reference counting is thread-safe, mutating the variant is not.  If
// thread-safe memoization is required, replace the mutable-variant approach
// with std::call_once + std::once_flag inside SharedState.

#include <smd/thunk/delay.hpp>

#include <cassert>
#include <functional>
#include <memory>
#include <type_traits>
#include <utility>
#include <variant>

namespace smd::thunk {

/** Type-erased, copyable wrapper for any callable returning @p Result.
 * All copies of an erased_thunk share the same underlying callable via
 * shared_ptr, so invoking any copy calls the same object.
 * Virtual dispatch is used so that the concrete callable type need not be
 * known at the point of storage (e.g. in a heterogeneous container).
 */
template <typename Result>
class erased_thunk {
    struct ThunkBase {
        virtual ~ThunkBase() = default;
        virtual auto invoke() -> const Result & = 0;
    };

    template <typename Callable>
    struct ThunkModel final : ThunkBase {
        Callable d_callable;

        explicit ThunkModel(Callable callable)
            : d_callable(std::move(callable)) {}

        auto invoke() -> const Result & override { return d_callable(); }
    };

    std::shared_ptr<ThunkBase> d_impl;

  public:
    erased_thunk() = default;

    /** Construct from any compatible callable (excluded: self-type). */
    template <
        typename Callable, typename CallableT = std::remove_cvref_t<Callable>,
        std::enable_if_t<!std::is_same_v<CallableT, erased_thunk>, int> = 0>
    erased_thunk(Callable &&callable)
        : d_impl(std::make_shared<ThunkModel<CallableT>>(
              std::forward<Callable>(callable))) {}

    /** Invoke the wrapped callable and return a reference to its result. */
    [[nodiscard]] auto operator()() const -> const Result & {
        assert(d_impl != nullptr);
        return d_impl->invoke();
    }
};

/**
 * @brief Return a callable that evaluates delay(callable, args...) exactly
 *        once and caches the result; all copies share the cached value.
 *
 * Not thread-safe: concurrent first calls race on the internal variant
 * mutation. See the thread-safety note at the top of this file.
 *
 * @param c     callable to defer and memoize
 * @param args  arguments forwarded into the underlying delay closure
 * @return nullary callable returning const Result& on every call after the
 *         first evaluation
 */
template <typename Callable, typename... Args>
auto memoize(Callable &&c, Args &&...args) {
    using Closure =
        decltype(delay(std::forward<Callable>(c), std::forward<Args>(args)...));
    using Result = std::invoke_result_t<Closure &>;
    using State = std::variant<std::monostate, Closure, Result>;

    auto state = std::make_shared<State>(
        std::in_place_index<1>,
        delay(std::forward<Callable>(c), std::forward<Args>(args)...));

    return [state = std::move(state)]() mutable -> const Result & {
        if (state->index() == 1U) {
            state->template emplace<2>(std::get<1>(*state)());
        }
        return std::get<2>(*state);
    };
}

/**
 * @brief Like memoize(), but attaches a pre-computed @p measure alongside the
 *        deferred result.
 *
 * Returns a struct with cached_measure() (returns the measure without forcing
 * the thunk) and force() (evaluates or returns the cached result).
 *
 * @param measure  pre-computed annotation value stored eagerly
 * @param c        callable to memoize
 * @param args     arguments forwarded to the underlying memoize closure
 */
template <typename Measure, typename Callable, typename... Args>
auto measured_memoize(Measure measure, Callable &&c, Args &&...args) {
    auto deferred =
        memoize(std::forward<Callable>(c), std::forward<Args>(args)...);

    return [measure = std::move(measure),
            deferred = std::move(deferred)]() mutable {
        struct MeasuredAccess {
            Measure d_measure;
            mutable decltype(deferred) d_force;

            /** Return the pre-computed measure without evaluating the thunk. */
            [[nodiscard]] auto cached_measure() const -> const Measure & {
                return d_measure;
            }
            /** Force evaluation (or return cached value) of the deferred callable. */
            [[nodiscard]] auto force() const -> decltype(auto) {
                return d_force();
            }
        };
        return MeasuredAccess{std::move(measure), std::move(deferred)};
    }();
}

} // namespace smd::thunk

#endif

```

## smd/thunk/memoize.t.cpp

```cpp
// src/smd/thunk/memoize.t.cpp                                        -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#include <smd/thunk/memoize.hpp>
#include <smd/thunk/memoize.hpp> // Re-inclusion check

#include <catch2/catch_test_macros.hpp>

#include <string>

TEST_CASE("memoize - evaluates callable exactly once") {
    int call_count = 0;
    auto m = smd::thunk::memoize([&call_count]() {
        ++call_count;
        return 42;
    });
    CHECK(m() == 42);
    CHECK(m() == 42);
    CHECK(m() == 42);
    CHECK(call_count == 1);
}

TEST_CASE("memoize - captures arguments") {
    auto m = smd::thunk::memoize([](int x, int y) { return x * y; }, 6, 7);
    CHECK(m() == 42);
    CHECK(m() == 42);
}

TEST_CASE("memoize - copies share cached value") {
    int call_count = 0;
    auto m1 = smd::thunk::memoize([&call_count]() {
        ++call_count;
        return std::string("hello");
    });
    auto m2 = m1;
    CHECK(m1() == "hello");
    CHECK(m2() == "hello");
    CHECK(call_count == 1);
}

TEST_CASE("erased_thunk - wraps and invokes callable") {
    int call_count = 0;
    auto inner = smd::thunk::memoize([&call_count]() {
        ++call_count;
        return 99;
    });
    smd::thunk::erased_thunk<int> e{std::move(inner)};
    CHECK(e() == 99);
    CHECK(e() == 99);
    CHECK(call_count == 1);
}

TEST_CASE("measured_memoize - provides measure and deferred value") {
    auto m = smd::thunk::measured_memoize(std::string("my-measure"),
                                          []() { return 123; });
    CHECK(m.cached_measure() == "my-measure");
    CHECK(m.force() == 123);
    CHECK(m.force() == 123);
}

```

## smd/tree/binary_tree_applicative.hpp

```cpp
// src/smd/tree/binary_tree_applicative.hpp                           -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_BINARY_TREE_APPLICATIVE
#define INCLUDED_SMD_TREE_BINARY_TREE_APPLICATIVE

#include <smd/tree/binary_tree.hpp>
#include <smd/typeclass/applicative.hpp>

#include <memory>
#include <type_traits>
#include <utility>

namespace smd {

/** Applicative typeclass instance for BinaryTree<T> with shape-aware semantics.
 * 
 * pure(v) produces a single leaf. apply recurses pairwise over matching tree
 * structure: a leaf function distributes over the argument's shape; a leaf
 * argument distributes over the function's shape; when both have children,
 * only positions where both trees have a child are combined (pairwise).
 * These are monad-derived (not zip) applicative semantics.
 * @tparam T element type of the function tree (F is the function type stored)
 */
template <class T>
struct BinaryTreeApplicativeImpl {
    /** Lift a plain value into a single-leaf tree. */
    template <class VALUE>
    auto pure(this auto &&, VALUE &&value) {
        using U = remove_cvref_t<VALUE>;
        return smd::tree::BinaryTree<U>::leaf(std::forward<VALUE>(value));
    }

    /**
     * @brief Apply a tree of functions to a tree of arguments, shape-aware.
     * @param functions tree whose nodes contain callables
     * @param arguments tree whose nodes contain arguments
     * @return tree of results; shape determined by pairwise recursion rules
     */
    template <class F, class A>
    auto apply(this auto &&self, const smd::tree::BinaryTree<F> &functions,
               const smd::tree::BinaryTree<A> &arguments)
        -> smd::tree::BinaryTree<std::invoke_result_t<const F &, const A &>> {
        using R = std::invoke_result_t<const F &, const A &>;

        std::shared_ptr<smd::tree::BinaryTree<R>> left{};
        std::shared_ptr<smd::tree::BinaryTree<R>> right{};

        const auto function_is_leaf =
            !functions.has_left() && !functions.has_right();
        const auto argument_is_leaf =
            !arguments.has_left() && !arguments.has_right();

        if (function_is_leaf) {
            // pure(f) should distribute f over the argument shape.
            if (arguments.has_left()) {
                left = smd::tree::BinaryTree<R>::make_ptr(
                    self.apply(functions, arguments.left()));
            }
            if (arguments.has_right()) {
                right = smd::tree::BinaryTree<R>::make_ptr(
                    self.apply(functions, arguments.right()));
            }
        } else if (argument_is_leaf) {
            // A non-leaf function tree can be applied pointwise to a single
            // argument.
            if (functions.has_left()) {
                left = smd::tree::BinaryTree<R>::make_ptr(
                    self.apply(functions.left(), arguments));
            }
            if (functions.has_right()) {
                right = smd::tree::BinaryTree<R>::make_ptr(
                    self.apply(functions.right(), arguments));
            }
        } else {
            // If both have shape, recurse pairwise where both children exist.
            if (functions.has_left() && arguments.has_left()) {
                left = smd::tree::BinaryTree<R>::make_ptr(
                    self.apply(functions.left(), arguments.left()));
            }

            if (functions.has_right() && arguments.has_right()) {
                right = smd::tree::BinaryTree<R>::make_ptr(
                    self.apply(functions.right(), arguments.right()));
            }
        }

        return smd::tree::BinaryTree<R>::from_children_ptrs(
            functions.value()(arguments.value()), std::move(left),
            std::move(right));
    }
};

/** Applicative map exposing pure and apply for BinaryTree<T>. */
template <class T>
struct BinaryTreeApplicativeMap : Applicative<BinaryTreeApplicativeImpl<T>> {
    using BinaryTreeApplicativeImpl<T>::apply;
    using BinaryTreeApplicativeImpl<T>::pure;
};

/** Registers BinaryTreeApplicativeMap as the Applicative instance for BinaryTree<T>. */
template <class T>
inline constexpr auto applicative_typeclass<smd::tree::BinaryTree<T>> =
    BinaryTreeApplicativeMap<T>{};

} // namespace smd

#endif

```

## smd/tree/binary_tree_applicative.t.cpp

```cpp
#include <smd/tree/binary_tree.hpp>
#include <smd/tree/binary_tree.hpp> // Re-inclusion check
#include <smd/tree/binary_tree_applicative.hpp>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("BinaryTreeApplicativeTest - InvokeAndApply") {
    using Tree = smd::tree::BinaryTree<int>;
    auto lhs = Tree::from_children_ptrs(10, Tree::make_ptr(Tree::leaf(1)),
                                        Tree::make_ptr(Tree::leaf(2)));
    auto rhs = Tree::from_children_ptrs(3, Tree::make_ptr(Tree::leaf(4)),
                                        Tree::make_ptr(Tree::leaf(5)));

    const auto &applicative = smd::applicative_typeclass<Tree>;
    auto summed =
        applicative.invoke([](int a, int b) { return a + b; }, lhs, rhs);

    CHECK(summed.value() == 13);
    REQUIRE(summed.has_left());
    REQUIRE(summed.has_right());
    CHECK(summed.left().value() == 5);
    CHECK(summed.right().value() == 7);

    auto fs = smd::tree::BinaryTree<int (*)(int)>::from_children_ptrs(
        +[](int x) { return x * 2; },
        smd::tree::BinaryTree<int (*)(int)>::make_ptr(
            smd::tree::BinaryTree<int (*)(int)>::leaf(
                +[](int x) { return x + 1; })),
        {});
    auto applied = applicative.apply(fs, lhs);
    CHECK(applied.value() == 20);
    REQUIRE(applied.has_left());
    CHECK(applied.left().value() == 2);
    CHECK_FALSE(applied.has_right());
}

TEST_CASE(
    "BinaryTreeApplicativeTest - PureFunctionDistributesOverArgumentShape") {
    using Tree = smd::tree::BinaryTree<int>;
    const auto &applicative = smd::applicative_typeclass<Tree>;

    auto fs = smd::tree::BinaryTree<int (*)(int)>::leaf(
        +[](int x) { return x + 10; });
    auto xs = Tree::from_children_ptrs(1, Tree::make_ptr(Tree::leaf(2)),
                                       Tree::make_ptr(Tree::leaf(3)));

    auto applied = applicative.apply(fs, xs);
    CHECK(applied.value() == 11);
    REQUIRE(applied.has_left());
    REQUIRE(applied.has_right());
    CHECK(applied.left().value() == 12);
    CHECK(applied.right().value() == 13);
}

TEST_CASE(
    "BinaryTreeApplicativeTest - FunctionTreeAppliesPointwiseToLeafArgument") {
    using Tree = smd::tree::BinaryTree<int>;
    const auto &applicative = smd::applicative_typeclass<Tree>;

    auto fs = smd::tree::BinaryTree<int (*)(int)>::from_children_ptrs(
        +[](int x) { return x * 2; },
        smd::tree::BinaryTree<int (*)(int)>::make_ptr(
            smd::tree::BinaryTree<int (*)(int)>::leaf(
                +[](int x) { return x + 1; })),
        smd::tree::BinaryTree<int (*)(int)>::make_ptr(
            smd::tree::BinaryTree<int (*)(int)>::leaf(
                +[](int x) { return x - 1; })));

    auto applied = applicative.apply(fs, Tree::leaf(10));
    CHECK(applied.value() == 20);
    REQUIRE(applied.has_left());
    REQUIRE(applied.has_right());
    CHECK(applied.left().value() == 11);
    CHECK(applied.right().value() == 9);
}

TEST_CASE("BinaryTreeApplicativeTest - PairwiseApplyRequiresMatchingChildren") {
    using Tree = smd::tree::BinaryTree<int>;
    const auto &applicative = smd::applicative_typeclass<Tree>;

    auto fs = smd::tree::BinaryTree<int (*)(int)>::from_children_ptrs(
        +[](int x) { return x + 100; },
        smd::tree::BinaryTree<int (*)(int)>::make_ptr(
            smd::tree::BinaryTree<int (*)(int)>::leaf(
                +[](int x) { return x + 1; })),
        {});

    auto xs = Tree::from_children_ptrs(1, {}, Tree::make_ptr(Tree::leaf(2)));

    auto applied = applicative.apply(fs, xs);
    CHECK(applied.value() == 101);
    CHECK_FALSE(applied.has_left());
    CHECK_FALSE(applied.has_right());
}

```

## smd/tree/binary_tree_foldable.hpp

```cpp
// src/smd/tree/binary_tree_foldable.hpp                              -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_BINARY_TREE_FOLDABLE
#define INCLUDED_SMD_TREE_BINARY_TREE_FOLDABLE

#include <smd/tree/binary_tree.hpp>
#include <smd/typeclass/foldable.hpp>

#include <functional>
#include <type_traits>
#include <utility>

namespace smd {

/** Foldable typeclass instance for BinaryTree<T>.
 * fold_map applies @p function to every node value (in-order: left, root,
 * right) and combines the results using the Monoid for the return type.
 * @tparam T element type of the tree being folded
 */
template <class T>
struct BinaryTreeFoldableImpl {
    template <class F>
    auto fold_map(this auto &&self, F &&function,
                  const smd::tree::BinaryTree<T> &tree)
        -> remove_cvref_t<decltype(std::invoke(function, tree.value()))> {
        auto value_result = std::invoke(function, tree.value());
        using Result = remove_cvref_t<decltype(value_result)>;

        Result acc = tree.has_left() ? smd::typeclass::monoid_v<Result>.combine(
                                           self.fold_map(function, tree.left()),
                                           std::move(value_result))
                                     : std::move(value_result);

        if (tree.has_right()) {
            acc = smd::typeclass::monoid_v<Result>.combine(
                std::move(acc), self.fold_map(function, tree.right()));
        }

        return acc;
    }
};

/** Foldable map that exposes the fold_map operation for BinaryTree<T>. */
template <class T>
struct BinaryTreeFoldableMap : Foldable<BinaryTreeFoldableImpl<T>> {
    using BinaryTreeFoldableImpl<T>::fold_map;
};

/** Registers BinaryTreeFoldableMap as the Foldable instance for BinaryTree<T>. */
template <class T>
inline constexpr auto foldable_typeclass<smd::tree::BinaryTree<T>> =
    BinaryTreeFoldableMap<T>{};

} // namespace smd

#endif

```

## smd/tree/binary_tree_foldable.t.cpp

```cpp
#include <smd/tree/binary_tree.hpp>
#include <smd/tree/binary_tree.hpp> // Re-inclusion check
#include <smd/tree/binary_tree_foldable.hpp>
#include <smd/typeclass/foldable.hpp>

#include <catch2/catch_test_macros.hpp>

#include <vector>

TEST_CASE("BinaryTreeFoldableTest - InorderFoldAndLength") {
    using Tree = smd::tree::BinaryTree<int>;
    auto tree =
        Tree::from_children_ptrs(2, Tree::make_ptr(Tree::leaf(1)),
                                 Tree::make_ptr(Tree::from_children_ptrs(
                                     3, {}, Tree::make_ptr(Tree::leaf(4)))));

    const auto &foldable = smd::foldable_typeclass<Tree>;
    CHECK(foldable.length(tree) == 4U);

    const auto as_vector = foldable.to_vector(tree);
    CHECK(as_vector == (std::vector<int>{1, 2, 3, 4}));

    const auto left = foldable.fold_left(
        tree, 0, [](int acc, int x) { return acc * 10 + x; });
    CHECK(left == 1234);
}

```

## smd/tree/binary_tree.hpp

```cpp
// src/smd/tree/binary_tree.hpp                                       -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_BINARY_TREE
#define INCLUDED_SMD_TREE_BINARY_TREE

#include <cassert>
#include <memory>
#include <utility>

namespace smd::tree {

/** Persistent binary tree where every node carries a value.
 * Nodes are either leaves (no children) or internal nodes with left and right
 * subtrees. Sharing is structural: subtrees are held by shared_ptr, so copies
 * are cheap and the tree is immutable once built.
 * @tparam T element type stored at every node
 */
template <class T>
class BinaryTree {
    T d_value;
    std::shared_ptr<BinaryTree> d_left;
    std::shared_ptr<BinaryTree> d_right;

  public:
    using value_type = T;

    /** Construct a leaf node holding @p value (no children). */
    static auto leaf(T value) -> BinaryTree {
        return BinaryTree(std::move(value), {}, {});
    }

    /** Construct an internal node with @p value and two children. */
    static auto node(T value, BinaryTree left, BinaryTree right) -> BinaryTree {
        return BinaryTree(std::move(value),
                          std::make_shared<BinaryTree>(std::move(left)),
                          std::make_shared<BinaryTree>(std::move(right)));
    }

    /** Alias for node(); prefer node() in new code. */
    static auto branch(T value, BinaryTree left, BinaryTree right)
        -> BinaryTree {
        return node(std::move(value), std::move(left), std::move(right));
    }

    /** Low-level constructor accepting pre-built child shared_ptrs.
     * Null pointers represent absent children.
     */
    static auto from_children_ptrs(T value, std::shared_ptr<BinaryTree> left,
                                   std::shared_ptr<BinaryTree> right)
        -> BinaryTree {
        return BinaryTree(std::move(value), std::move(left), std::move(right));
    }

    /** Heap-allocate a copy of @p tree and return the owning pointer. */
    static auto make_ptr(BinaryTree tree) -> std::shared_ptr<BinaryTree> {
        return std::make_shared<BinaryTree>(std::move(tree));
    }

    /** Return the value stored at this node. */
    auto value() const -> const T & { return d_value; }

    /** True when this node has a left child. */
    auto has_left() const -> bool { return static_cast<bool>(d_left); }
    /** True when this node has a right child. */
    auto has_right() const -> bool { return static_cast<bool>(d_right); }

    /** Return the left child; precondition: has_left(). */
    auto left() const -> const BinaryTree & {
        assert(d_left);
        return *d_left;
    }

    /** Return the right child; precondition: has_right(). */
    auto right() const -> const BinaryTree & {
        assert(d_right);
        return *d_right;
    }

    /** Shared pointer to the left child; may be null. */
    auto left_ptr() const -> const std::shared_ptr<BinaryTree> & {
        return d_left;
    }
    /** Shared pointer to the right child; may be null. */
    auto right_ptr() const -> const std::shared_ptr<BinaryTree> & {
        return d_right;
    }

  private:
    BinaryTree(T value, std::shared_ptr<BinaryTree> left,
               std::shared_ptr<BinaryTree> right)
        : d_value(std::move(value)), d_left(std::move(left)),
          d_right(std::move(right)) {}
};

} // namespace smd::tree

#endif

```

## smd/tree/binary_tree.t.cpp

```cpp
// src/smd/tree/binary_tree.t.cpp                                     -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <smd/tree/binary_tree.hpp>
#include <smd/tree/binary_tree.hpp> // Re-inclusion check

#include <catch2/catch_test_macros.hpp>

TEST_CASE("BinaryTreeTest - LeafConstruction") {
    auto t = smd::tree::BinaryTree<int>::leaf(42);
    CHECK(t.value() == 42);
    CHECK_FALSE(t.has_left());
    CHECK_FALSE(t.has_right());
}

TEST_CASE("BinaryTreeTest - NodeConstruction") {
    using Tree = smd::tree::BinaryTree<int>;
    auto t = Tree::node(1, Tree::leaf(2), Tree::leaf(3));
    CHECK(t.value() == 1);
    CHECK(t.has_left());
    CHECK(t.has_right());
    CHECK(t.left().value() == 2);
    CHECK(t.right().value() == 3);
}

TEST_CASE("BinaryTreeTest - DeepTree") {
    using Tree = smd::tree::BinaryTree<int>;
    auto t = Tree::node(1, Tree::node(2, Tree::leaf(4), Tree::leaf(5)),
                        Tree::leaf(3));
    CHECK(t.value() == 1);
    CHECK(t.left().value() == 2);
    CHECK(t.left().left().value() == 4);
    CHECK(t.left().right().value() == 5);
    CHECK_FALSE(t.right().has_left());
    CHECK_FALSE(t.right().has_right());
}

```

## smd/tree/binary_tree_traversable.hpp

```cpp
// src/smd/tree/binary_tree_traversable.hpp                           -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_BINARY_TREE_TRAVERSABLE
#define INCLUDED_SMD_TREE_BINARY_TREE_TRAVERSABLE

#include <smd/tree/binary_tree.hpp>
#include <smd/tree/binary_tree_applicative.hpp>
#include <smd/typeclass/traversable.hpp>

#include <functional>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>

namespace smd {

/** Traversable typeclass instance for BinaryTree<T>.
 * traverse maps each node value into an applicative context and rebuilds a
 * BinaryTree inside that context, preserving the original tree's shape.
 * @tparam T element type of the tree being traversed
 */
template <class T>
struct BinaryTreeTraversableImpl {
    using element_type = T;

    template <class APPLICATIVE, class F>
    auto traverse(this auto &&self, const APPLICATIVE &applicative,
                  F &&function, const smd::tree::BinaryTree<T> &tree) {
        auto value_context =
            std::invoke(std::forward<F>(function), tree.value());
        using Context = remove_cvref_t<decltype(value_context)>;
        using U = smd::applicative_value_t<Context>;
        using TreeContext = decltype(applicative.invoke(
            [](auto &&value) {
                using V = remove_cvref_t<decltype(value)>;
                return smd::tree::BinaryTree<V>::leaf(
                    std::forward<decltype(value)>(value));
            },
            value_context));

        if (!tree.has_left() && !tree.has_right()) {
            return applicative.invoke(
                [](auto &&value) {
                    using V = remove_cvref_t<decltype(value)>;
                    return smd::tree::BinaryTree<V>::leaf(
                        std::forward<decltype(value)>(value));
                },
                value_context);
        }

        std::optional<TreeContext> left_tree_context;
        if (tree.has_left()) {
            left_tree_context.emplace(
                self.traverse(applicative, function, tree.left()));
        }

        std::optional<TreeContext> right_tree_context;
        if (tree.has_right()) {
            right_tree_context.emplace(
                self.traverse(applicative, function, tree.right()));
        }

        auto to_child_ptr = [&](const auto &child_tree_context) {
            return applicative.invoke(
                [](auto &&subtree) {
                    using SubTree = remove_cvref_t<decltype(subtree)>;
                    return std::make_shared<SubTree>(
                        std::forward<decltype(subtree)>(subtree));
                },
                child_tree_context);
        };

        auto empty_child_like = [&](const auto &child_tree_context) {
            return applicative.invoke(
                [](const auto &) {
                    return std::shared_ptr<smd::tree::BinaryTree<U>>{};
                },
                child_tree_context);
        };

        auto left_context = [&]() {
            if (left_tree_context.has_value()) {
                return to_child_ptr(*left_tree_context);
            }

            return empty_child_like(*right_tree_context);
        }();

        auto right_context = [&]() {
            if (right_tree_context.has_value()) {
                return to_child_ptr(*right_tree_context);
            }

            return empty_child_like(*left_tree_context);
        }();

        return applicative.invoke(
            [](auto &&value, auto &&left, auto &&right) {
                using U = remove_cvref_t<decltype(value)>;
                return smd::tree::BinaryTree<U>::from_children_ptrs(
                    std::forward<decltype(value)>(value),
                    std::forward<decltype(left)>(left),
                    std::forward<decltype(right)>(right));
            },
            value_context, left_context, right_context);
    }
};

/** Traversable map that exposes traverse for BinaryTree<T>. */
template <class T>
struct BinaryTreeTraversableMap : Traversable<BinaryTreeTraversableImpl<T>> {
    using BinaryTreeTraversableImpl<T>::traverse;
};

/** Registers BinaryTreeTraversableMap as the Traversable instance for BinaryTree<T>. */
template <class T>
inline constexpr auto traversable_typeclass<smd::tree::BinaryTree<T>> =
    BinaryTreeTraversableMap<T>{};

} // namespace smd

#endif

```

## smd/tree/binary_tree_traversable.t.cpp

```cpp
#include <smd/tree/binary_tree.hpp>
#include <smd/tree/binary_tree.hpp> // Re-inclusion check
#include <smd/tree/binary_tree_traversable.hpp>

#include <catch2/catch_test_macros.hpp>

#include <optional>

namespace {

struct PositiveTimesTen {
    auto operator()(int x) const -> std::optional<int> {
        return x > 0 ? std::optional<int>{x * 10} : std::optional<int>{};
    }
};

struct TimesTen {
    auto operator()(int x) const -> std::optional<int> {
        return std::optional<int>{x * 10};
    }
};

struct PlusOne {
    auto operator()(int x) const -> std::optional<int> {
        return std::optional<int>{x + 1};
    }
};

struct NonNegativeIdentity {
    auto operator()(int x) const -> std::optional<int> {
        return x >= 0 ? std::optional<int>{x} : std::optional<int>{};
    }
};

} // namespace

TEST_CASE("BinaryTreeTraversableTest - TraverseOptionalPreservesShape") {
    using Tree = smd::tree::BinaryTree<int>;
    auto tree =
        Tree::from_children_ptrs(2, Tree::make_ptr(Tree::leaf(1)),
                                 Tree::make_ptr(Tree::from_children_ptrs(
                                     3, {}, Tree::make_ptr(Tree::leaf(4)))));

    auto traversed = smd::traverse(PositiveTimesTen{}, tree);

    REQUIRE(traversed.has_value());
    CHECK(traversed->value() == 20);
    REQUIRE(traversed->has_left());
    CHECK(traversed->left().value() == 10);
    REQUIRE(traversed->has_right());
    CHECK(traversed->right().value() == 30);
    CHECK_FALSE(traversed->right().has_left());
    REQUIRE(traversed->right().has_right());
    CHECK(traversed->right().right().value() == 40);
}

TEST_CASE(
    "BinaryTreeTraversableTest - TraverseOptionalDoesNotDuplicateRootEffect") {
    using Tree = smd::tree::BinaryTree<int>;
    auto tree = Tree::from_children_ptrs(2, {}, Tree::make_ptr(Tree::leaf(5)));

    int invocations = 0;
    auto traversed = smd::traverse(
        [&](int x) -> std::optional<int> {
            ++invocations;
            return TimesTen{}(x);
        },
        tree);

    REQUIRE(traversed.has_value());
    CHECK(invocations == 2);
    CHECK(traversed->value() == 20);
    CHECK_FALSE(traversed->has_left());
    REQUIRE(traversed->has_right());
    CHECK(traversed->right().value() == 50);
}

TEST_CASE("BinaryTreeTraversableTest - TraverseOptionalLeaf") {
    using Tree = smd::tree::BinaryTree<int>;
    auto tree = Tree::leaf(9);

    auto traversed = smd::traverse(PlusOne{}, tree);

    REQUIRE(traversed.has_value());
    CHECK(traversed->value() == 10);
    CHECK_FALSE(traversed->has_left());
    CHECK_FALSE(traversed->has_right());
}

TEST_CASE("BinaryTreeTraversableTest - TraverseOptionalLeftOnly") {
    using Tree = smd::tree::BinaryTree<int>;
    auto tree = Tree::from_children_ptrs(2, Tree::make_ptr(Tree::leaf(3)), {});

    auto traversed = smd::traverse(TimesTen{}, tree);

    REQUIRE(traversed.has_value());
    CHECK(traversed->value() == 20);
    REQUIRE(traversed->has_left());
    CHECK_FALSE(traversed->has_right());
    CHECK(traversed->left().value() == 30);
}

TEST_CASE("BinaryTreeTraversableTest - TraverseOptionalFailure") {
    using Tree = smd::tree::BinaryTree<int>;
    auto tree = Tree::from_children_ptrs(2, Tree::make_ptr(Tree::leaf(-1)), {});

    auto traversed = smd::traverse(NonNegativeIdentity{}, tree);

    CHECK_FALSE(traversed.has_value());
}

```

## smd/tree/deadcode/fix_tree_applicative.hpp

```cpp
// src/smd/tree/fix_tree_applicative.hpp                              -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_FIX_TREE_APPLICATIVE
#define INCLUDED_SMD_TREE_FIX_TREE_APPLICATIVE

#include <smd/tree/fix_tree.hpp>
#include <smd/typeclass/applicative.hpp>

#include <functional>
#include <type_traits>
#include <utility>

namespace smd {

template <class T>
struct FixTreeApplicativeImpl {
    template <class VALUE>
    auto pure(this auto &&, VALUE &&x) {
        using U = std::remove_cvref_t<VALUE>;
        return smd::tree::FixTree<U>::leaf(std::forward<VALUE>(x));
    }

    template <class F, class A>
    auto apply(this auto &&self, const smd::tree::FixTree<F> &fs,
               const smd::tree::FixTree<A> &xs) {
        using R = decltype(fs.value()(xs.value()));
        if (fs.is_leaf()) {
            auto f = fs.value();
            if (xs.is_leaf()) {
                return smd::tree::FixTree<R>::leaf(f(xs.value()));
            }
            return smd::tree::FixTree<R>::node(self.apply(fs, xs.left()),
                                               self.apply(fs, xs.right()));
        }
        return smd::tree::FixTree<R>::node(self.apply(fs.left(), xs),
                                           self.apply(fs.right(), xs));
    }
};

template <class T>
struct FixTreeApplicativeMap : Applicative<FixTreeApplicativeImpl<T>> {
    using FixTreeApplicativeImpl<T>::apply;
    using FixTreeApplicativeImpl<T>::pure;
};

template <class T>
inline constexpr auto applicative_typeclass<smd::tree::FixTree<T>> =
    FixTreeApplicativeMap<T>{};

} // namespace smd

#endif

```

## smd/tree/deadcode/fix_tree_applicative.t.cpp

```cpp
#include <smd/tree/fix_tree.hpp>
#include <smd/tree/fix_tree.hpp> // Re-inclusion check
#include <smd/tree/fix_tree_applicative.hpp>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("FixTreeApplicativeTest - InvokeDistributesLeafOverShape") {
    using Tree = smd::tree::FixTree<int>;
    auto scalar = Tree::leaf(10);
    auto shaped = Tree::node(Tree::leaf(1), Tree::leaf(2));

    const auto &applicative = smd::applicative_typeclass<Tree>;
    auto summed = applicative.invoke([](int lhs, int rhs) { return lhs + rhs; },
                                     scalar, shaped);

    REQUIRE_FALSE(summed.is_leaf());
    CHECK(summed.left().value() == 11);
    CHECK(summed.right().value() == 12);
}

```

## smd/tree/deadcode/fix_tree_foldable.hpp

```cpp
// src/smd/tree/fix_tree_foldable.hpp                                 -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_FIX_TREE_FOLDABLE
#define INCLUDED_SMD_TREE_FIX_TREE_FOLDABLE

#include <smd/tree/fix_tree.hpp>
#include <smd/typeclass/foldable.hpp>

#include <functional>
#include <type_traits>

namespace smd {

template <class T>
struct FixTreeFoldableImpl {

    // a3f7b2e1-9c4d-4f8a-b6e3-2d5c8a1f4b07
    template <class F>
    auto fold_map(this auto &&self, F &&f, const smd::tree::FixTree<T> &t) {
        if (t.is_leaf()) {
            return std::invoke(f, t.value());
        }

        auto lhs = self.fold_map(f, t.left());
        auto rhs = self.fold_map(f, t.right());

        using Result = std::remove_cvref_t<decltype(lhs)>;
        return smd::typeclass::monoid_v<Result>.combine(lhs, rhs);
    }
    // a3f7b2e1-9c4d-4f8a-b6e3-2d5c8a1f4b07 end
};

template <class T>
struct FixTreeFoldableMap : Foldable<FixTreeFoldableImpl<T>> {
    using FixTreeFoldableImpl<T>::fold_map;
};

// d6e2b9f4-1a7c-4b3e-8f5d-3c9a2e7b6f08
template <class T>
inline constexpr auto foldable_typeclass<smd::tree::FixTree<T>> =
    FixTreeFoldableMap<T>{};
// d6e2b9f4-1a7c-4b3e-8f5d-3c9a2e7b6f08 end

} // namespace smd

#endif

```

## smd/tree/deadcode/fix_tree_foldable.t.cpp

```cpp
#include <smd/tree/fix_tree.hpp>
#include <smd/tree/fix_tree.hpp> // Re-inclusion check
#include <smd/tree/fix_tree_foldable.hpp>
#include <smd/typeclass/foldable.hpp>

#include <catch2/catch_test_macros.hpp>

#include <vector>

namespace {

template <class TREE, const auto &FOLDABLE = smd::foldable_typeclass<TREE>>
auto sum_with_nttp_lookup(const TREE &tree) {
    return FOLDABLE.fold_map([](int x) { return x; }, tree);
}

template <class TREE, const auto &FOLDABLE = smd::foldable_typeclass<TREE>>
auto fold_left_with_nttp_lookup(const TREE &tree) {
    return FOLDABLE.fold_left(tree, 0,
                              [](int acc, int x) { return acc * 10 + x; });
}

template <class TREE, const auto &FOLDABLE = smd::foldable_typeclass<TREE>>
auto fold_right_with_nttp_lookup(const TREE &tree) {
    return FOLDABLE.fold_right(tree, 0,
                               [](int x, int acc) { return x * 10 + acc; });
}

} // namespace

TEST_CASE("FixTreeFoldableTest - Length") {
    using Tree = smd::tree::FixTree<int>;
    auto tree =
        Tree::branch(Tree::leaf(1), Tree::branch(Tree::leaf(2), Tree::leaf(3)));

    const auto &foldable = smd::foldable_typeclass<Tree>;
    CHECK(foldable.length(tree) == 3U);
}

TEST_CASE("FixTreeFoldableTest - FoldMapSum") {
    using Tree = smd::tree::FixTree<int>;
    auto tree =
        Tree::branch(Tree::leaf(1), Tree::branch(Tree::leaf(2), Tree::leaf(3)));

    const auto &foldable = smd::foldable_typeclass<Tree>;
    const auto sum = foldable.fold_map([](int x) { return x; }, tree);
    CHECK(sum == 6);
}

TEST_CASE("FixTreeFoldableTest - FoldMapWithExplicitObject") {
    using Tree = smd::tree::FixTree<int>;
    auto tree =
        Tree::branch(Tree::leaf(1), Tree::branch(Tree::leaf(2), Tree::leaf(3)));

    const auto &foldable = smd::foldable_typeclass<Tree>;
    const auto sum = foldable.fold_map([](int x) { return x; }, tree);
    CHECK(sum == 6);
}

TEST_CASE("FixTreeFoldableTest - FoldMapWithNttpLookup") {
    using Tree = smd::tree::FixTree<int>;
    auto tree =
        Tree::branch(Tree::leaf(1), Tree::branch(Tree::leaf(2), Tree::leaf(3)));

    CHECK(sum_with_nttp_lookup(tree) == 6);
}

TEST_CASE("FixTreeFoldableTest - FoldLeftAndRight") {
    using Tree = smd::tree::FixTree<int>;
    auto tree =
        Tree::branch(Tree::leaf(1), Tree::branch(Tree::leaf(2), Tree::leaf(3)));
    const auto &foldable = smd::foldable_typeclass<Tree>;

    const auto left = foldable.fold_left(
        tree, 0, [](int acc, int x) { return acc * 10 + x; });
    const auto right = foldable.fold_right(
        tree, 0, [](int x, int acc) { return x * 10 + acc; });

    CHECK(left == 123);
    CHECK(right == 60);
}

TEST_CASE("FixTreeFoldableTest - FoldLeftRightWithExplicitObject") {
    using Tree = smd::tree::FixTree<int>;
    auto tree =
        Tree::branch(Tree::leaf(1), Tree::branch(Tree::leaf(2), Tree::leaf(3)));

    const auto &foldable = smd::foldable_typeclass<Tree>;
    const auto left = foldable.fold_left(
        tree, 0, [](int acc, int x) { return acc * 10 + x; });
    const auto right = foldable.fold_right(
        tree, 0, [](int x, int acc) { return x * 10 + acc; });

    CHECK(left == 123);
    CHECK(right == 60);
}

TEST_CASE("FixTreeFoldableTest - FoldLeftRightWithNttpLookup") {
    using Tree = smd::tree::FixTree<int>;
    auto tree =
        Tree::branch(Tree::leaf(1), Tree::branch(Tree::leaf(2), Tree::leaf(3)));

    CHECK(fold_left_with_nttp_lookup(tree) == 123);
    CHECK(fold_right_with_nttp_lookup(tree) == 60);
}

TEST_CASE("FixTreeFoldableTest - PredicatesAndFind") {
    using Tree = smd::tree::FixTree<int>;
    auto tree =
        Tree::branch(Tree::leaf(1), Tree::branch(Tree::leaf(2), Tree::leaf(3)));
    const auto &foldable = smd::foldable_typeclass<Tree>;

    CHECK(foldable.any_of(tree, [](int x) { return x == 2; }));
    CHECK(foldable.all_of(tree, [](int x) { return x > 0; }));
    CHECK_FALSE(foldable.empty(tree));

    auto found = foldable.find_first(tree, [](int x) { return x > 1; });
    REQUIRE(found.has_value());
    CHECK(*found == 2);
}

TEST_CASE("FixTreeTest - CoreConstructionAndAccess") {
    using Tree = smd::tree::FixTree<int>;

    auto l = Tree::leaf(4);
    CHECK(l.is_leaf());
    CHECK(l.value() == 4);

    auto via_node = Tree::node(Tree::leaf(1), Tree::leaf(2));
    CHECK_FALSE(via_node.is_leaf());
    CHECK(via_node.left().is_leaf());
    CHECK(via_node.right().is_leaf());
    CHECK(via_node.left().value() == 1);
    CHECK(via_node.right().value() == 2);

    auto via_branch = Tree::branch(Tree::leaf(7), Tree::leaf(8));
    CHECK_FALSE(via_branch.is_leaf());
    CHECK(via_branch.left().value() == 7);
    CHECK(via_branch.right().value() == 8);
}

```

## smd/tree/deadcode/fix_tree.hpp

```cpp
// src/smd/tree/fix_tree.hpp                                          -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_FIX_TREE
#define INCLUDED_SMD_TREE_FIX_TREE
#include <memory>
#include <variant>
namespace smd::tree {
template <class T>
class FixTree {
    struct Leaf {
        T v;
    };
    struct Node {
        std::shared_ptr<FixTree> l, r;
    };
    std::variant<Leaf, Node> d;

  public:
    using value_type = T;

    static FixTree leaf(T v) { return FixTree(Leaf{v}); }
    static FixTree node(FixTree a, FixTree b) {
        return FixTree(
            Node{std::make_shared<FixTree>(a), std::make_shared<FixTree>(b)});
    }
    static FixTree branch(FixTree a, FixTree b) {
        return node(std::move(a), std::move(b));
    }
    bool is_leaf() const { return std::holds_alternative<Leaf>(d); }
    const T &value() const { return std::get<Leaf>(d).v; }
    const FixTree &left() const { return *std::get<Node>(d).l; }
    const FixTree &right() const { return *std::get<Node>(d).r; }

  private:
    FixTree(Leaf l) : d(l) {}
    FixTree(Node n) : d(n) {}
};
} // namespace smd::tree
#endif

```

## smd/tree/deadcode/fix_tree.t.cpp

```cpp
// src/smd/tree/fix_tree.t.cpp                                        -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <smd/tree/fix_tree.hpp>
#include <smd/tree/fix_tree.hpp> // Re-inclusion check

#include <catch2/catch_test_macros.hpp>

TEST_CASE("FixTreeTest - LeafConstruction") {
    auto t = smd::tree::FixTree<int>::leaf(7);
    CHECK(t.is_leaf());
    CHECK(t.value() == 7);
}

TEST_CASE("FixTreeTest - NodeConstruction") {
    using Tree = smd::tree::FixTree<int>;
    auto t = Tree::node(Tree::leaf(1), Tree::leaf(2));
    CHECK_FALSE(t.is_leaf());
    CHECK(t.left().is_leaf());
    CHECK(t.left().value() == 1);
    CHECK(t.right().is_leaf());
    CHECK(t.right().value() == 2);
}

TEST_CASE("FixTreeTest - NestedNodes") {
    using Tree = smd::tree::FixTree<int>;
    auto t =
        Tree::node(Tree::node(Tree::leaf(1), Tree::leaf(2)), Tree::leaf(3));
    CHECK_FALSE(t.is_leaf());
    CHECK_FALSE(t.left().is_leaf());
    CHECK(t.left().left().value() == 1);
    CHECK(t.right().value() == 3);
}

```

## smd/tree/deadcode/fix_tree_traversable.hpp

```cpp
// src/smd/tree/fix_tree_traversable.hpp                              -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDED_SMD_TREE_FIX_TREE_TRAVERSABLE
#define INCLUDED_SMD_TREE_FIX_TREE_TRAVERSABLE

#include <smd/tree/fix_tree.hpp>
#include <smd/tree/fix_tree_applicative.hpp>
#include <smd/typeclass/traversable.hpp>

#include <functional>
#include <type_traits>
#include <utility>

namespace smd {

template <class T>
struct FixTreeTraversableImpl {
    using element_type = T;

    template <class APPLICATIVE, class F>
    auto traverse(this auto &&self, const APPLICATIVE &applicative, F &&f,
                  const smd::tree::FixTree<T> &t) {
        if (t.is_leaf()) {
            return applicative.invoke(
                [](auto &&value) {
                    using U = std::remove_cvref_t<decltype(value)>;
                    return smd::tree::FixTree<U>::leaf(
                        std::forward<decltype(value)>(value));
                },
                std::invoke(std::forward<F>(f), t.value()));
        }

        auto left = self.traverse(applicative, f, t.left());
        auto right = self.traverse(applicative, f, t.right());

        return applicative.invoke(
            [](auto &&l, auto &&r) {
                using U = std::remove_cvref_t<decltype(l.value())>;
                return smd::tree::FixTree<U>::node(
                    std::forward<decltype(l)>(l), std::forward<decltype(r)>(r));
            },
            left, right);
    }
};

template <class T>
struct FixTreeTraversableMap : Traversable<FixTreeTraversableImpl<T>> {
    using FixTreeTraversableImpl<T>::traverse;
};

template <class T>
inline constexpr auto traversable_typeclass<smd::tree::FixTree<T>> =
    FixTreeTraversableMap<T>{};

} // namespace smd

#endif

```

## smd/tree/deadcode/fix_tree_traversable.t.cpp

```cpp
#include <smd/tree/fix_tree.hpp>
#include <smd/tree/fix_tree.hpp> // Re-inclusion check
#include <smd/tree/fix_tree_traversable.hpp>

#include <catch2/catch_test_macros.hpp>

#include <beman/optional/optional.hpp>

#include <optional>

namespace {

struct NonNegativePlusOne {
    auto operator()(int x) const -> std::optional<int> {
        return x >= 0 ? std::optional<int>{x + 1} : std::optional<int>{};
    }
};

struct TimesTwo {
    auto operator()(int x) const -> std::optional<int> {
        return std::optional<int>{x * 2};
    }
};

struct MinusTwo {
    auto operator()(int x) const -> std::optional<int> {
        return std::optional<int>{x - 2};
    }
};

struct TimesFiveBeman {
    auto operator()(int x) const -> beman::optional::optional<int> {
        return beman::optional::optional<int>{x * 5};
    }
};

} // namespace

TEST_CASE("FixTreeTraversableTest - TraverseOptionalSuccess") {
    using Tree = smd::tree::FixTree<int>;
    auto tree = Tree::branch(Tree::leaf(1), Tree::leaf(2));

    auto traversed = smd::traverse(NonNegativePlusOne{}, tree);

    REQUIRE(traversed.has_value());
    CHECK(traversed->left().value() == 2);
    CHECK(traversed->right().value() == 3);
}

TEST_CASE("FixTreeTraversableTest - TraverseOptionalFailure") {
    using Tree = smd::tree::FixTree<int>;
    auto tree = Tree::branch(Tree::leaf(1), Tree::leaf(-2));

    auto traversed = smd::traverse(NonNegativePlusOne{}, tree);

    CHECK_FALSE(traversed.has_value());
}

TEST_CASE("FixTreeTraversableTest - ForEachOptionalSuccess") {
    using Tree = smd::tree::FixTree<int>;
    auto tree = Tree::branch(Tree::leaf(3), Tree::leaf(4));
    const auto &traversable = smd::traversable_typeclass<Tree>;

    auto traversed = traversable.for_each(tree, TimesTwo{});

    REQUIRE(traversed.has_value());
    CHECK(traversed->left().value() == 6);
    CHECK(traversed->right().value() == 8);
}

TEST_CASE("FixTreeTraversableTest - TraverseLeaf") {
    using Tree = smd::tree::FixTree<int>;
    auto tree = Tree::leaf(9);

    auto traversed = smd::traverse(MinusTwo{}, tree);

    REQUIRE(traversed.has_value());
    REQUIRE(traversed->is_leaf());
    CHECK(traversed->value() == 7);
}

TEST_CASE("FixTreeTraversableTest - TraverseBemanOptional") {
    using Tree = smd::tree::FixTree<int>;
    auto tree = Tree::branch(Tree::leaf(2), Tree::leaf(3));

    auto traversed = smd::traverse(TimesFiveBeman{}, tree);

    REQUIRE(traversed.has_value());
    CHECK(traversed->left().value() == 10);
    CHECK(traversed->right().value() == 15);
}

```

## smd/tree/finger_tree_foldable.hpp

```cpp
// src/smd/tree/finger_tree_foldable.hpp                              -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_FINGER_TREE_FOLDABLE
#define INCLUDED_SMD_TREE_FINGER_TREE_FOLDABLE

#include <smd/tree/finger_tree.hpp>
#include <smd/typeclass/foldable.hpp>

#include <functional>
#include <type_traits>
#include <utility>

namespace smd {

/** Foldable typeclass implementation for FingerTree; uses for_each to avoid
 * heap allocation during traversal.
 */
template <class T, class TAG_TYPE, class MEASURE_POLICY>
struct FingerTreeFoldableImpl {
    template <class F>
    auto
    fold_map(this auto &&, F &&function,
             const smd::tree::FingerTree<T, TAG_TYPE, MEASURE_POLICY> &tree)
        -> remove_cvref_t<std::invoke_result_t<F, const T &>> {
        using Result = remove_cvref_t<std::invoke_result_t<F, const T &>>;

        Result acc = smd::typeclass::monoid_v<Result>.identity();
        tree.for_each([&](const T &value) {
            acc = smd::typeclass::monoid_v<Result>.combine(
                std::move(acc), std::invoke(function, value));
        });
        return acc;
    }
};

/** Foldable typeclass map entry for FingerTree. */
template <class T, class TAG_TYPE, class MEASURE_POLICY>
struct FingerTreeFoldableMap
    : Foldable<FingerTreeFoldableImpl<T, TAG_TYPE, MEASURE_POLICY>> {
    using FingerTreeFoldableImpl<T, TAG_TYPE, MEASURE_POLICY>::fold_map;
};

/** Registers FingerTree as a Foldable for all tag and measure combinations. */
template <class T, class TAG_TYPE, class MEASURE_POLICY>
inline constexpr auto
    foldable_typeclass<smd::tree::FingerTree<T, TAG_TYPE, MEASURE_POLICY>> =
        FingerTreeFoldableMap<T, TAG_TYPE, MEASURE_POLICY>{};

} // namespace smd

#endif

```

## smd/tree/finger_tree_foldable.t.cpp

```cpp
// src/smd/tree/finger_tree_foldable.t.cpp                            -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <smd/tree/finger_tree_foldable.hpp>
#include <smd/tree/finger_tree_foldable.hpp> // Re-inclusion check

#include <smd/typeclass/foldable.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <vector>

TEST_CASE("FingerTreeFoldableTest - LengthAndToVector") {
    using Tree = smd::tree::FingerTree<int>;
    const auto &foldable = smd::foldable_typeclass<Tree>;
    auto t = Tree::from_sequence({1, 2, 3, 4, 5});
    CHECK(foldable.length(t) == 5);
    CHECK(foldable.to_vector(t) == (std::vector<int>{1, 2, 3, 4, 5}));
}

TEST_CASE("FingerTreeFoldableTest - FoldMapAccumulates") {
    using Tree = smd::tree::FingerTree<int>;
    const auto &foldable = smd::foldable_typeclass<Tree>;
    auto t = Tree::from_sequence({1, 2, 3});
    auto sum = foldable.fold_map([](int x) { return x; }, t);
    CHECK(sum == 6);
}

TEST_CASE("FingerTreeFoldableTest - EmptyTreeHasLengthZero") {
    using Tree = smd::tree::FingerTree<int>;
    const auto &foldable = smd::foldable_typeclass<Tree>;
    auto t = Tree::empty();
    CHECK(foldable.length(t) == 0);
    CHECK(foldable.empty(t));
    CHECK(foldable.to_vector(t) == (std::vector<int>{}));
}

```

## smd/tree/finger_tree.hpp

```cpp
// src/smd/tree/finger_tree.hpp                                       -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_FINGER_TREE
#define INCLUDED_SMD_TREE_FINGER_TREE

#include <smd/typeclass/monoid.hpp>

#include <cassert>
#include <concepts>
#include <cstddef>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace smd::tree {

/** Heap-allocated, shared, immutable value — used for tree nodes and spine. */
template <typename T>
using Boxed = std::shared_ptr<const T>;

namespace detail {

// Digit types: FingerTree ends hold 1–4 elements in a structural variant.
// Not part of the public API — do not depend on these types directly.
template <typename T>
struct One {
    T a;
};

template <typename T>
struct Two {
    T a;
    T b;
};

template <typename T>
struct Three {
    T a;
    T b;
    T c;
};

template <typename T>
struct Four {
    T a;
    T b;
    T c;
    T d;
};

template <typename T>
using Digit = std::variant<One<T>, Two<T>, Three<T>, Four<T>>;

// Node types: FingerTree spine holds cached-measure nodes of 2–3 children.
// Not part of the public API — do not depend on these types directly.
template <typename T, typename TAG_TYPE>
struct Node2 {
    TAG_TYPE d_measure;
    std::size_t d_leaf_count;
    Boxed<T> a;
    Boxed<T> b;
};

template <typename T, typename TAG_TYPE>
struct Node3 {
    TAG_TYPE d_measure;
    std::size_t d_leaf_count;
    Boxed<T> a;
    Boxed<T> b;
    Boxed<T> c;
};

template <typename T, typename TAG_TYPE>
using Node = std::variant<Node2<T, TAG_TYPE>, Node3<T, TAG_TYPE>>;

// Helper for multi-overload lambdas passed to std::visit.
template <typename... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};

template <typename... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

} // namespace detail

/** Measure policy that counts each element as 1; TAG_TYPE must be an additive
 * monoid (e.g. std::size_t).
 */
template <typename VALUE_TYPE, typename TAG_TYPE>
struct UnitMeasure {
    auto operator()(const VALUE_TYPE &) const -> TAG_TYPE {
        return TAG_TYPE{1};
    }
};

/** Measure policy that lifts the cached measure stored inside a Node variant. */
template <typename NODE_T, typename TAG_TYPE>
struct NodeMeasure {
    auto operator()(const NODE_T &node) const -> TAG_TYPE {
        return std::visit([](const auto &n) -> TAG_TYPE { return n.d_measure; },
                          node);
    }
};

/** @brief Persistent Hinze-Paterson 2-3 finger tree.
 *
 * @tparam T              Element type stored at the leaves.
 * @tparam TAG_TYPE       Monoid-valued measure annotation cached at every node.
 * @tparam MEASURE_POLICY Callable that maps a @p T value to a @p TAG_TYPE
 *                        measure; must be default-constructible.
 * @tparam DEPTH          NTTP used to break polymorphic recursion; the spine is
 *                        a @c FingerTree<Node<T>, TAG_TYPE, NodeMeasure, DEPTH+1>.
 *
 * Complexity:
 * - cons/snoc:        O(1) amortized
 * - append/concat:    O(log min(n, m))
 * - split/search:     O(log n)
 * - flatten/for_each: O(n)
 * - view_l/view_r:    O(1) amortized
 */
template <typename T, typename TAG_TYPE = std::size_t,
          typename MEASURE_POLICY = UnitMeasure<T, TAG_TYPE>, int DEPTH = 0>
class FingerTree {
    template <typename, typename, typename, int>
    friend class FingerTree;

    static_assert(std::is_default_constructible_v<MEASURE_POLICY>,
                  "FingerTree measure policy must be default-constructible");

    static constexpr int kMaxDepth = 10;

    // Shadow internal detail types into class scope so the implementation body
    // below does not need detail:: prefixes throughout.
    template <typename U>
    using One = detail::One<U>;
    template <typename U>
    using Two = detail::Two<U>;
    template <typename U>
    using Three = detail::Three<U>;
    template <typename U>
    using Four = detail::Four<U>;
    template <typename U>
    using Digit = detail::Digit<U>;
    template <typename U, typename V>
    using Node2 = detail::Node2<U, V>;
    template <typename U, typename V>
    using Node3 = detail::Node3<U, V>;
    template <typename U, typename V>
    using Node = detail::Node<U, V>;

    using Tag = TAG_TYPE;
    using MeasurePolicy = MEASURE_POLICY;
    using NodeT = Node<T, Tag>;
    using SpineMeasure = NodeMeasure<NodeT, Tag>;
    struct SpineTerminal {};
    using SpineTree =
        std::conditional_t<(DEPTH < kMaxDepth),
                           FingerTree<NodeT, Tag, SpineMeasure, DEPTH + 1>,
                           SpineTerminal>;
    using SpinePtr = std::shared_ptr<const SpineTree>;

    // -- Tag operations --

    static auto tag_identity() -> Tag {
        return smd::typeclass::monoid_v<Tag>.identity();
    }

    static auto tag_combine(const Tag &lhs, const Tag &rhs) -> Tag {
        return smd::typeclass::monoid_v<Tag>.combine(lhs, rhs);
    }

    static auto tag_value(const T &value) -> Tag {
        return MeasurePolicy{}(value);
    }

    static constexpr auto elem_leaf_count([[maybe_unused]] const T &elem)
        -> std::size_t {
        if constexpr (DEPTH == 0) {
            return 1U;
        } else {
            return std::visit([](const auto &n) { return n.d_leaf_count; },
                              elem);
        }
    }

    // -- Node construction --

    static auto make_node2(T a, T b) -> NodeT {
        auto m = tag_combine(tag_value(a), tag_value(b));
        auto lc = elem_leaf_count(a) + elem_leaf_count(b);
        return Node2<T, Tag>{std::move(m), lc,
                             std::make_shared<const T>(std::move(a)),
                             std::make_shared<const T>(std::move(b))};
    }

    static auto make_node3(T a, T b, T c) -> NodeT {
        auto m =
            tag_combine(tag_combine(tag_value(a), tag_value(b)), tag_value(c));
        auto lc = elem_leaf_count(a) + elem_leaf_count(b) + elem_leaf_count(c);
        return Node3<T, Tag>{std::move(m), lc,
                             std::make_shared<const T>(std::move(a)),
                             std::make_shared<const T>(std::move(b)),
                             std::make_shared<const T>(std::move(c))};
    }

    static auto node_measure(const NodeT &node) -> Tag {
        return std::visit([](const auto &n) -> Tag { return n.d_measure; },
                          node);
    }

    static void node_flatten_into(const NodeT &node, std::vector<T> &out) {
        std::visit(detail::overloaded{[&](const Node2<T, Tag> &n) {
                                          out.push_back(*n.a);
                                          out.push_back(*n.b);
                                      },
                                      [&](const Node3<T, Tag> &n) {
                                          out.push_back(*n.a);
                                          out.push_back(*n.b);
                                          out.push_back(*n.c);
                                      }},
                   node);
    }

    template <typename F>
    static void node_for_each(const NodeT &node, const F &callback) {
        std::visit(detail::overloaded{[&](const Node2<T, Tag> &n) {
                                          callback(*n.a);
                                          callback(*n.b);
                                      },
                                      [&](const Node3<T, Tag> &n) {
                                          callback(*n.a);
                                          callback(*n.b);
                                          callback(*n.c);
                                      }},
                   node);
    }

    static auto node_to_digit(const NodeT &node) -> Digit<T> {
        return std::visit(
            detail::overloaded{[](const Node2<T, Tag> &n) -> Digit<T> {
                                   return Two<T>{*n.a, *n.b};
                               },
                               [](const Node3<T, Tag> &n) -> Digit<T> {
                                   return Three<T>{*n.a, *n.b, *n.c};
                               }},
            node);
    }

    static auto node_to_list(const NodeT &node) -> std::vector<T> {
        std::vector<T> result;
        node_flatten_into(node, result);
        return result;
    }

    // -- Digit helpers --

    static auto digit_measure(const Digit<T> &d) -> Tag {
        return std::visit(
            detail::overloaded{
                [](const One<T> &x) { return tag_value(x.a); },
                [](const Two<T> &x) {
                    return tag_combine(tag_value(x.a), tag_value(x.b));
                },
                [](const Three<T> &x) {
                    return tag_combine(
                        tag_combine(tag_value(x.a), tag_value(x.b)),
                        tag_value(x.c));
                },
                [](const Four<T> &x) {
                    return tag_combine(
                        tag_combine(tag_combine(tag_value(x.a), tag_value(x.b)),
                                    tag_value(x.c)),
                        tag_value(x.d));
                }},
            d);
    }

    static auto digit_leaf_count(const Digit<T> &d) -> std::size_t {
        return std::visit(
            detail::overloaded{
                [](const One<T> &x) { return elem_leaf_count(x.a); },
                [](const Two<T> &x) {
                    return elem_leaf_count(x.a) + elem_leaf_count(x.b);
                },
                [](const Three<T> &x) {
                    return elem_leaf_count(x.a) + elem_leaf_count(x.b) +
                           elem_leaf_count(x.c);
                },
                [](const Four<T> &x) {
                    return elem_leaf_count(x.a) + elem_leaf_count(x.b) +
                           elem_leaf_count(x.c) + elem_leaf_count(x.d);
                }},
            d);
    }

    static void digit_flatten_into(const Digit<T> &d, std::vector<T> &out) {
        std::visit(
            detail::overloaded{[&](const One<T> &x) { out.push_back(x.a); },
                               [&](const Two<T> &x) {
                                   out.push_back(x.a);
                                   out.push_back(x.b);
                               },
                               [&](const Three<T> &x) {
                                   out.push_back(x.a);
                                   out.push_back(x.b);
                                   out.push_back(x.c);
                               },
                               [&](const Four<T> &x) {
                                   out.push_back(x.a);
                                   out.push_back(x.b);
                                   out.push_back(x.c);
                                   out.push_back(x.d);
                               }},
            d);
    }

    template <typename F>
    static void digit_for_each(const Digit<T> &d, const F &callback) {
        std::visit(detail::overloaded{[&](const One<T> &x) { callback(x.a); },
                                      [&](const Two<T> &x) {
                                          callback(x.a);
                                          callback(x.b);
                                      },
                                      [&](const Three<T> &x) {
                                          callback(x.a);
                                          callback(x.b);
                                          callback(x.c);
                                      },
                                      [&](const Four<T> &x) {
                                          callback(x.a);
                                          callback(x.b);
                                          callback(x.c);
                                          callback(x.d);
                                      }},
                   d);
    }

    static auto digit_to_list(const Digit<T> &d) -> std::vector<T> {
        std::vector<T> result;
        digit_flatten_into(d, result);
        return result;
    }

    static auto digit_head(const Digit<T> &d) -> const T & {
        return std::visit([](const auto &x) -> const T & { return x.a; }, d);
    }

    static auto digit_last(const Digit<T> &d) -> const T & {
        return std::visit(
            detail::overloaded{
                [](const One<T> &x) -> const T & { return x.a; },
                [](const Two<T> &x) -> const T & { return x.b; },
                [](const Three<T> &x) -> const T & { return x.c; },
                [](const Four<T> &x) -> const T & { return x.d; }},
            d);
    }

    static auto digit_tail(const Digit<T> &d) -> std::optional<Digit<T>> {
        return std::visit(detail::overloaded{
                              [](const One<T> &) -> std::optional<Digit<T>> {
                                  return std::nullopt;
                              },
                              [](const Two<T> &x) -> std::optional<Digit<T>> {
                                  return One<T>{x.b};
                              },
                              [](const Three<T> &x) -> std::optional<Digit<T>> {
                                  return Two<T>{x.b, x.c};
                              },
                              [](const Four<T> &x) -> std::optional<Digit<T>> {
                                  return Three<T>{x.b, x.c, x.d};
                              }},
                          d);
    }

    static auto digit_init(const Digit<T> &d) -> std::optional<Digit<T>> {
        return std::visit(detail::overloaded{
                              [](const One<T> &) -> std::optional<Digit<T>> {
                                  return std::nullopt;
                              },
                              [](const Two<T> &x) -> std::optional<Digit<T>> {
                                  return One<T>{x.a};
                              },
                              [](const Three<T> &x) -> std::optional<Digit<T>> {
                                  return Two<T>{x.a, x.b};
                              },
                              [](const Four<T> &x) -> std::optional<Digit<T>> {
                                  return Three<T>{x.a, x.b, x.c};
                              }},
                          d);
    }

    // -- Spine helpers --

    static auto spine_measure(const SpinePtr &sp) -> Tag {
        if constexpr (DEPTH < kMaxDepth) {
            if (!sp)
                return tag_identity();
            return sp->measure();
        } else {
            (void)sp;
            return tag_identity();
        }
    }

    static auto spine_breadth(const SpinePtr &sp) -> std::size_t {
        if constexpr (DEPTH < kMaxDepth) {
            if (!sp)
                return 0U;
            return sp->breadth();
        } else {
            (void)sp;
            return 0U;
        }
    }

    static auto spine_depth(const SpinePtr &sp) -> std::size_t {
        if constexpr (DEPTH < kMaxDepth) {
            if (!sp)
                return 0U;
            return sp->depth();
        } else {
            (void)sp;
            return 0U;
        }
    }

    static auto spine_is_empty(const SpinePtr &sp) -> bool {
        if constexpr (DEPTH < kMaxDepth) {
            return !sp || sp->is_empty();
        } else {
            (void)sp;
            return true;
        }
    }

    static auto spine_cons(const SpinePtr &spine, NodeT node) -> SpinePtr {
        if constexpr (DEPTH < kMaxDepth) {
            if (spine_is_empty(spine)) {
                return std::make_shared<const SpineTree>(
                    SpineTree::leaf(std::move(node)));
            }
            return std::make_shared<const SpineTree>(
                spine->cons(std::move(node)));
        } else {
            (void)node;
            return spine;
        }
    }

    static auto spine_snoc(const SpinePtr &spine, NodeT node) -> SpinePtr {
        if constexpr (DEPTH < kMaxDepth) {
            if (spine_is_empty(spine)) {
                return std::make_shared<const SpineTree>(
                    SpineTree::leaf(std::move(node)));
            }
            return std::make_shared<const SpineTree>(
                spine->snoc(std::move(node)));
        } else {
            (void)node;
            return spine;
        }
    }

    // -- Internal representation --

    struct Empty {};

    struct Single {
        Tag d_measure;
        T d_value;
    };

    struct Deep {
        Tag d_measure;
        std::size_t d_breadth;
        std::size_t d_depth;
        Digit<T> d_left;
        SpinePtr d_spine;
        Digit<T> d_right;
    };

    using DeepPtr = std::shared_ptr<const Deep>;
    using Repr = std::variant<Empty, Single, DeepPtr>;

    Repr d_repr;

    explicit FingerTree(Repr repr) : d_repr(std::move(repr)) {}

    // -- Smart constructors --

    static auto make_empty() -> FingerTree { return FingerTree(Repr{Empty{}}); }

    static auto make_single(T value) -> FingerTree {
        auto m = tag_value(value);
        return FingerTree(Repr{Single{std::move(m), std::move(value)}});
    }

    static auto make_deep(Digit<T> left, SpinePtr spine, Digit<T> right)
        -> FingerTree {
        auto m =
            tag_combine(tag_combine(digit_measure(left), spine_measure(spine)),
                        digit_measure(right));
        auto b = digit_leaf_count(left) + spine_breadth(spine) +
                 digit_leaf_count(right);
        auto d = std::size_t{1} + spine_depth(spine);
        return FingerTree(Repr{std::make_shared<const Deep>(
            Deep{std::move(m), b, d, std::move(left), std::move(spine),
                 std::move(right)})});
    }

    static auto digit_to_tree(const Digit<T> &d) -> FingerTree {
        return std::visit(
            detail::overloaded{
                [](const One<T> &x) { return make_single(x.a); },
                [](const Two<T> &x) {
                    return make_deep(One<T>{x.a}, nullptr, One<T>{x.b});
                },
                [](const Three<T> &x) {
                    return make_deep(Two<T>{x.a, x.b}, nullptr, One<T>{x.c});
                },
                [](const Four<T> &x) {
                    return make_deep(Two<T>{x.a, x.b}, nullptr,
                                     Two<T>{x.c, x.d});
                }},
            d);
    }

    static auto deep_l(SpinePtr spine, Digit<T> right) -> FingerTree {
        if constexpr (DEPTH < kMaxDepth) {
            if (spine_is_empty(spine)) {
                return digit_to_tree(right);
            }
            auto vl = spine->view_l();
            assert(vl.has_value());
            auto new_left = node_to_digit(vl->d_value);
            SpinePtr new_spine;
            if (!vl->d_rest.is_empty()) {
                new_spine =
                    std::make_shared<const SpineTree>(std::move(vl->d_rest));
            }
            return make_deep(std::move(new_left), std::move(new_spine),
                             std::move(right));
        } else {
            return digit_to_tree(right);
        }
    }

    static auto deep_r(Digit<T> left, SpinePtr spine) -> FingerTree {
        if constexpr (DEPTH < kMaxDepth) {
            if (spine_is_empty(spine)) {
                return digit_to_tree(left);
            }
            auto vr = spine->view_r();
            assert(vr.has_value());
            auto new_right = node_to_digit(vr->d_value);
            SpinePtr new_spine;
            if (!vr->d_rest.is_empty()) {
                new_spine =
                    std::make_shared<const SpineTree>(std::move(vr->d_rest));
            }
            return make_deep(std::move(left), std::move(new_spine),
                             std::move(new_right));
        } else {
            return digit_to_tree(left);
        }
    }

    // -- nodes: pack elements into Node2/Node3 sequence --

    static auto nodes_from(std::vector<T> elems) -> std::vector<NodeT> {
        std::vector<NodeT> result;
        auto n = elems.size();
        std::size_t i = 0;
        while (n - i > 4) {
            result.push_back(make_node3(std::move(elems[i]),
                                        std::move(elems[i + 1]),
                                        std::move(elems[i + 2])));
            i += 3;
        }
        switch (n - i) {
        case 2:
            result.push_back(
                make_node2(std::move(elems[i]), std::move(elems[i + 1])));
            break;
        case 3:
            result.push_back(make_node3(std::move(elems[i]),
                                        std::move(elems[i + 1]),
                                        std::move(elems[i + 2])));
            break;
        case 4:
            result.push_back(
                make_node2(std::move(elems[i]), std::move(elems[i + 1])));
            result.push_back(
                make_node2(std::move(elems[i + 2]), std::move(elems[i + 3])));
            break;
        default:
            assert(false && "nodes_from: invalid element count");
        }
        return result;
    }

    // -- app3: Hinze-Paterson recursive concatenation --

    static auto app3(const FingerTree &left, std::vector<T> middle,
                     const FingerTree &right) -> FingerTree {
        if (left.is_empty()) {
            auto result = right;
            for (auto it = middle.rbegin(); it != middle.rend(); ++it) {
                result = result.cons(std::move(*it));
            }
            return result;
        }
        if (right.is_empty()) {
            auto result = left;
            for (auto &elem : middle) {
                result = result.snoc(std::move(elem));
            }
            return result;
        }
        if (left.is_leaf()) {
            auto result = right;
            for (auto it = middle.rbegin(); it != middle.rend(); ++it) {
                result = result.cons(std::move(*it));
            }
            return result.cons(std::get<Single>(left.d_repr).d_value);
        }
        if (right.is_leaf()) {
            auto result = left;
            for (auto &elem : middle) {
                result = result.snoc(std::move(elem));
            }
            return result.snoc(std::get<Single>(right.d_repr).d_value);
        }

        if constexpr (DEPTH < kMaxDepth) {
            const auto &ld = *std::get<DeepPtr>(left.d_repr);
            const auto &rd = *std::get<DeepPtr>(right.d_repr);

            auto combined = digit_to_list(ld.d_right);
            combined.insert(combined.end(),
                            std::make_move_iterator(middle.begin()),
                            std::make_move_iterator(middle.end()));
            {
                auto right_left = digit_to_list(rd.d_left);
                combined.insert(combined.end(), right_left.begin(),
                                right_left.end());
            }

            auto ns = nodes_from(std::move(combined));

            auto left_spine = ld.d_spine ? *ld.d_spine : SpineTree::empty();
            auto right_spine = rd.d_spine ? *rd.d_spine : SpineTree::empty();

            auto new_spine =
                SpineTree::app3(left_spine, std::move(ns), right_spine);
            SpinePtr sp;
            if (!new_spine.is_empty()) {
                sp = std::make_shared<const SpineTree>(std::move(new_spine));
            }

            return make_deep(ld.d_left, std::move(sp), rd.d_right);
        } else {
            auto result = left.flatten();
            result.insert(result.end(), std::make_move_iterator(middle.begin()),
                          std::make_move_iterator(middle.end()));
            auto rv = right.flatten();
            result.insert(result.end(), rv.begin(), rv.end());
            return from_sequence(std::move(result));
        }
    }

    // -- Split helpers --

    struct DigitSplit {
        std::optional<Digit<T>> d_left;
        T d_pivot;
        std::optional<Digit<T>> d_right;
    };

    template <typename PREDICATE>
    static auto split_digit(const PREDICATE &predicate, Tag prefix,
                            const Digit<T> &d) -> std::optional<DigitSplit> {
        auto elems = digit_to_list(d);
        auto running = prefix;
        for (std::size_t i = 0; i < elems.size(); ++i) {
            running = tag_combine(running, tag_value(elems[i]));
            if (predicate(running)) {
                std::optional<Digit<T>> left_d;
                std::optional<Digit<T>> right_d;
                if (i > 0) {
                    std::vector<T> lv(elems.begin(),
                                      elems.begin() +
                                          static_cast<std::ptrdiff_t>(i));
                    left_d = list_to_digit(std::move(lv));
                }
                auto remaining = elems.size() - i - 1;
                if (remaining > 0) {
                    std::vector<T> rv(elems.begin() +
                                          static_cast<std::ptrdiff_t>(i + 1),
                                      elems.end());
                    right_d = list_to_digit(std::move(rv));
                }
                return DigitSplit{std::move(left_d), std::move(elems[i]),
                                  std::move(right_d)};
            }
        }
        return std::nullopt;
    }

    static auto list_to_digit(std::vector<T> elems) -> std::optional<Digit<T>> {
        switch (elems.size()) {
        case 0:
            return std::nullopt;
        case 1:
            return One<T>{std::move(elems[0])};
        case 2:
            return Two<T>{std::move(elems[0]), std::move(elems[1])};
        case 3:
            return Three<T>{std::move(elems[0]), std::move(elems[1]),
                            std::move(elems[2])};
        case 4:
            return Four<T>{std::move(elems[0]), std::move(elems[1]),
                           std::move(elems[2]), std::move(elems[3])};
        default:
            assert(false);
            return std::nullopt;
        }
    }

    // Assemble tree from optional digit + spine + digit
    static auto assemble_left(std::optional<Digit<T>> left_d, SpinePtr spine,
                              Digit<T> right) -> FingerTree {
        if (left_d.has_value()) {
            return make_deep(std::move(*left_d), std::move(spine),
                             std::move(right));
        }
        return deep_l(std::move(spine), std::move(right));
    }

    static auto assemble_right(Digit<T> left, SpinePtr spine,
                               std::optional<Digit<T>> right_d) -> FingerTree {
        if (right_d.has_value()) {
            return make_deep(std::move(left), std::move(spine),
                             std::move(*right_d));
        }
        return deep_r(std::move(left), std::move(spine));
    }

    // Flatten spine nodes into elements
    static void spine_flatten_into(const SpinePtr &sp, std::vector<T> &out) {
        if constexpr (DEPTH < kMaxDepth) {
            if (spine_is_empty(sp))
                return;
            auto spine_elems = sp->flatten();
            for (auto &node : spine_elems) {
                node_flatten_into(node, out);
            }
        }
    }

    template <typename F>
    static void spine_for_each(const SpinePtr &sp, const F &callback) {
        if constexpr (DEPTH < kMaxDepth) {
            if (spine_is_empty(sp))
                return;
            sp->for_each(
                [&](const NodeT &node) { node_for_each(node, callback); });
        }
    }

  public:
    /** The leaf element type. */
    using value_type = T;
    /** The monoid measure type annotating each node. */
    using tag_type = Tag;

    /** Result of view_l() / view_r(): the exposed element and the remaining tree. */
    struct View {
        T d_value;
        FingerTree d_rest;
    };

    /** Result of split(): the pivot element extracted between two subtrees. */
    struct Split {
        FingerTree d_left;
        T d_pivot;
        FingerTree d_right;
    };

    /** Result of split_at(): the tree partitioned into two halves. */
    struct SplitAt {
        FingerTree d_left;
        FingerTree d_right;
    };

  private:
    // -- split_impl: measure-guided split --

    template <typename PREDICATE>
    auto split_impl(const PREDICATE &predicate, Tag prefix) const
        -> std::optional<Split> {
        if (is_empty())
            return std::nullopt;

        if (is_leaf()) {
            const auto &s = std::get<Single>(d_repr);
            auto p = tag_combine(prefix, s.d_measure);
            if (predicate(p)) {
                return Split{make_empty(), s.d_value, make_empty()};
            }
            return std::nullopt;
        }

        if constexpr (DEPTH < kMaxDepth) {
            const auto &d = *std::get<DeepPtr>(d_repr);

            // Check left digit
            auto vl = tag_combine(prefix, digit_measure(d.d_left));
            if (predicate(vl)) {
                auto ds = split_digit(predicate, prefix, d.d_left);
                if (!ds.has_value())
                    return std::nullopt;
                auto left_tree = ds->d_left.has_value()
                                     ? digit_to_tree(*ds->d_left)
                                     : make_empty();
                auto right_tree =
                    assemble_left(std::move(ds->d_right), d.d_spine, d.d_right);
                return Split{std::move(left_tree), std::move(ds->d_pivot),
                             std::move(right_tree)};
            }

            // Check spine
            auto vm = tag_combine(vl, spine_measure(d.d_spine));
            if (predicate(vm)) {
                if (spine_is_empty(d.d_spine))
                    return std::nullopt;

                auto spine_split = d.d_spine->split_impl(
                    [&](const Tag &t) { return predicate(tag_combine(vl, t)); },
                    tag_identity());
                if (!spine_split.has_value())
                    return std::nullopt;

                auto node_prefix =
                    tag_combine(vl, spine_split->d_left.measure());

                auto node_digit = node_to_digit(spine_split->d_pivot);
                auto node_split =
                    split_digit(predicate, node_prefix, node_digit);
                if (!node_split.has_value())
                    return std::nullopt;

                SpinePtr sl;
                if (!spine_split->d_left.is_empty()) {
                    sl = std::make_shared<const SpineTree>(
                        std::move(spine_split->d_left));
                }
                SpinePtr sr;
                if (!spine_split->d_right.is_empty()) {
                    sr = std::make_shared<const SpineTree>(
                        std::move(spine_split->d_right));
                }

                auto left_tree = assemble_right(d.d_left, std::move(sl),
                                                std::move(node_split->d_left));
                auto right_tree = assemble_left(std::move(node_split->d_right),
                                                std::move(sr), d.d_right);
                return Split{std::move(left_tree),
                             std::move(node_split->d_pivot),
                             std::move(right_tree)};
            }

            // Check right digit
            auto ds = split_digit(predicate, vm, d.d_right);
            if (!ds.has_value())
                return std::nullopt;
            auto left_tree =
                assemble_right(d.d_left, d.d_spine, std::move(ds->d_left));
            auto right_tree = ds->d_right.has_value()
                                  ? digit_to_tree(*ds->d_right)
                                  : make_empty();
            return Split{std::move(left_tree), std::move(ds->d_pivot),
                         std::move(right_tree)};
        } else {
            auto vec = flatten();
            auto running = prefix;
            for (std::size_t i = 0; i < vec.size(); ++i) {
                running = tag_combine(running, tag_value(vec[i]));
                if (predicate(running)) {
                    std::vector<T> lv(vec.begin(),
                                      vec.begin() +
                                          static_cast<std::ptrdiff_t>(i));
                    std::vector<T> rv(vec.begin() +
                                          static_cast<std::ptrdiff_t>(i + 1),
                                      vec.end());
                    return Split{from_sequence(std::move(lv)),
                                 std::move(vec[i]),
                                 from_sequence(std::move(rv))};
                }
            }
            return std::nullopt;
        }
    }

  public:
    /** Constructs an empty tree. */
    FingerTree() : d_repr(Empty{}) {}

    /** Returns an empty tree. */
    static auto empty() -> FingerTree { return make_empty(); }

    /** Returns a single-element tree containing @p value. */
    static auto leaf(T value) -> FingerTree {
        return make_single(std::move(value));
    }

    /** Returns true if the tree contains no elements. */
    auto is_empty() const -> bool {
        return std::holds_alternative<Empty>(d_repr);
    }

    /** Returns true if the tree contains exactly one element. */
    auto is_leaf() const -> bool {
        return std::holds_alternative<Single>(d_repr);
    }

    /** Returns true if the tree has a deep (two-digit + spine) structure. */
    auto is_branch() const -> bool {
        return std::holds_alternative<DeepPtr>(d_repr);
    }

    /** Returns the cached monoid measure over all elements in the tree. */
    auto measure() const -> Tag {
        return std::visit(
            detail::overloaded{
                [](const Empty &) { return tag_identity(); },
                [](const Single &s) -> Tag { return s.d_measure; },
                [](const DeepPtr &d) -> Tag { return d->d_measure; }},
            d_repr);
    }

    /** Returns the number of leaf elements (the element count). */
    auto breadth() const -> std::size_t {
        return std::visit(
            detail::overloaded{
                [](const Empty &) -> std::size_t { return 0U; },
                [](const Single &s) -> std::size_t {
                    return elem_leaf_count(s.d_value);
                },
                [](const DeepPtr &d) -> std::size_t { return d->d_breadth; }},
            d_repr);
    }

    /** Returns the structural depth of the spine (number of nested levels). */
    auto depth() const -> std::size_t {
        return std::visit(
            detail::overloaded{
                [](const Empty &) -> std::size_t { return 0U; },
                [](const Single &) -> std::size_t { return 1U; },
                [](const DeepPtr &d) -> std::size_t { return d->d_depth; }},
            d_repr);
    }

    /** Returns the single element stored in a leaf tree; asserts if not a leaf. */
    auto value() const -> const T & {
        assert(is_leaf());
        return std::get<Single>(d_repr).d_value;
    }

    /** Prepends @p x to the front of the tree; O(1) amortized. */
    auto cons(T x) const -> FingerTree {
        return std::visit(
            detail::overloaded{
                [&](const Empty &) { return make_single(std::move(x)); },
                [&](const Single &s) {
                    return make_deep(One<T>{std::move(x)}, nullptr,
                                     One<T>{s.d_value});
                },
                [&](const DeepPtr &d) -> FingerTree {
                    return std::visit(
                        detail::overloaded{
                            [&](const One<T> &dig) {
                                return make_deep(Two<T>{std::move(x), dig.a},
                                                 d->d_spine, d->d_right);
                            },
                            [&](const Two<T> &dig) {
                                return make_deep(
                                    Three<T>{std::move(x), dig.a, dig.b},
                                    d->d_spine, d->d_right);
                            },
                            [&](const Three<T> &dig) {
                                return make_deep(
                                    Four<T>{std::move(x), dig.a, dig.b, dig.c},
                                    d->d_spine, d->d_right);
                            },
                            [&](const Four<T> &dig) -> FingerTree {
                                auto node = make_node3(dig.b, dig.c, dig.d);
                                auto new_spine =
                                    spine_cons(d->d_spine, std::move(node));
                                return make_deep(Two<T>{std::move(x), dig.a},
                                                 std::move(new_spine),
                                                 d->d_right);
                            }},
                        d->d_left);
                }},
            d_repr);
    }

    /** Appends @p x to the back of the tree; O(1) amortized. */
    auto snoc(T x) const -> FingerTree {
        return std::visit(
            detail::overloaded{
                [&](const Empty &) { return make_single(std::move(x)); },
                [&](const Single &s) {
                    return make_deep(One<T>{s.d_value}, nullptr,
                                     One<T>{std::move(x)});
                },
                [&](const DeepPtr &d) -> FingerTree {
                    return std::visit(
                        detail::overloaded{
                            [&](const One<T> &dig) {
                                return make_deep(d->d_left, d->d_spine,
                                                 Two<T>{dig.a, std::move(x)});
                            },
                            [&](const Two<T> &dig) {
                                return make_deep(
                                    d->d_left, d->d_spine,
                                    Three<T>{dig.a, dig.b, std::move(x)});
                            },
                            [&](const Three<T> &dig) {
                                return make_deep(
                                    d->d_left, d->d_spine,
                                    Four<T>{dig.a, dig.b, dig.c, std::move(x)});
                            },
                            [&](const Four<T> &dig) -> FingerTree {
                                auto node = make_node3(dig.a, dig.b, dig.c);
                                auto new_spine =
                                    spine_snoc(d->d_spine, std::move(node));
                                return make_deep(d->d_left,
                                                 std::move(new_spine),
                                                 Two<T>{dig.d, std::move(x)});
                            }},
                        d->d_right);
                }},
            d_repr);
    }

    /** Concatenates @p right onto this tree; O(log min(n, m)). */
    auto append(const FingerTree &right) const -> FingerTree {
        return app3(*this, {}, right);
    }

    /** Static alias: concatenates @p left and @p right. */
    static auto branch(const FingerTree &left, const FingerTree &right)
        -> FingerTree {
        return left.append(right);
    }

    /** Static alias: cons — prepends @p value to @p tree. */
    static auto prepend(T value, const FingerTree &tree) -> FingerTree {
        return tree.cons(std::move(value));
    }

    /** Static alias: snoc — appends @p value to @p tree. */
    static auto append(const FingerTree &tree, T value) -> FingerTree {
        return tree.snoc(std::move(value));
    }

    /** Static alias for append(); concatenates @p left and @p right. */
    static auto concat(const FingerTree &left, const FingerTree &right)
        -> FingerTree {
        return left.append(right);
    }

    /** Materialises all elements into a vector in sequence order; O(n). */
    auto flatten() const -> std::vector<T> {
        return std::visit(
            detail::overloaded{
                [](const Empty &) -> std::vector<T> { return {}; },
                [](const Single &s) -> std::vector<T> { return {s.d_value}; },
                [](const DeepPtr &d) -> std::vector<T> {
                    std::vector<T> out;
                    out.reserve(d->d_breadth);
                    digit_flatten_into(d->d_left, out);
                    spine_flatten_into(d->d_spine, out);
                    digit_flatten_into(d->d_right, out);
                    return out;
                }},
            d_repr);
    }

    /** Calls @p callback for each element in sequence order without allocating;
     * prefer over flatten() when the result does not need to outlive the loop.
     */
    template <typename F>
    void for_each(F &&callback) const {
        std::visit(detail::overloaded{[](const Empty &) {},
                                      [&](const Single &s) {
                                          std::invoke(callback, s.d_value);
                                      },
                                      [&](const DeepPtr &d) {
                                          digit_for_each(d->d_left, callback);
                                          spine_for_each(d->d_spine, callback);
                                          digit_for_each(d->d_right, callback);
                                      }},
                   d_repr);
    }

    /** Decomposes into (head, tail); returns nullopt on an empty tree; O(1) amortized. */
    auto view_l() const -> std::optional<View> {
        return std::visit(
            detail::overloaded{
                [](const Empty &) -> std::optional<View> {
                    return std::nullopt;
                },
                [](const Single &s) -> std::optional<View> {
                    return View{s.d_value, make_empty()};
                },
                [](const DeepPtr &d) -> std::optional<View> {
                    auto h = digit_head(d->d_left);
                    auto t = digit_tail(d->d_left);
                    if (t.has_value()) {
                        return View{h, make_deep(std::move(*t), d->d_spine,
                                                 d->d_right)};
                    }
                    return View{h, deep_l(d->d_spine, d->d_right)};
                }},
            d_repr);
    }

    /** Decomposes into (init, last); returns nullopt on an empty tree; O(1) amortized. */
    auto view_r() const -> std::optional<View> {
        return std::visit(detail::overloaded{
                              [](const Empty &) -> std::optional<View> {
                                  return std::nullopt;
                              },
                              [](const Single &s) -> std::optional<View> {
                                  return View{s.d_value, make_empty()};
                              },
                              [](const DeepPtr &d) -> std::optional<View> {
                                  auto l = digit_last(d->d_right);
                                  auto i = digit_init(d->d_right);
                                  if (i.has_value()) {
                                      return View{l, make_deep(d->d_left,
                                                               d->d_spine,
                                                               std::move(*i))};
                                  }
                                  return View{l, deep_r(d->d_left, d->d_spine)};
                              }},
                          d_repr);
    }

    /** Returns the first element; asserts if empty. */
    auto head() const -> T {
        auto v = view_l();
        assert(v.has_value());
        return std::move(v->d_value);
    }

    /** Returns the tree without its first element; returns empty if already empty. */
    auto tail() const -> FingerTree {
        auto v = view_l();
        return v.has_value() ? std::move(v->d_rest) : empty();
    }

    /** Returns the last element; asserts if empty. */
    auto last() const -> T {
        auto v = view_r();
        assert(v.has_value());
        return std::move(v->d_value);
    }

    /** Returns the tree without its last element; returns empty if already empty. */
    auto init() const -> FingerTree {
        auto v = view_r();
        return v.has_value() ? std::move(v->d_rest) : empty();
    }

    /** Returns the pivot value from split(@p predicate), or nullopt; O(log n). */
    template <typename PREDICATE>
    auto search(PREDICATE &&predicate) const -> std::optional<T> {
        auto sp = split(std::forward<PREDICATE>(predicate));
        if (!sp.has_value())
            return std::nullopt;
        return std::move(sp->d_pivot);
    }

    /** @brief Splits the tree at the first element where the accumulated prefix
     *         measure satisfies @p predicate; O(log n).
     *
     * @return nullopt if no element satisfies the predicate.
     *         Otherwise a Split whose @c d_pivot is that element, @c d_left
     *         contains all prior elements, and @c d_right contains all
     *         subsequent elements.
     */
    template <typename PREDICATE>
    auto split(PREDICATE &&predicate) const -> std::optional<Split> {
        return split_impl(predicate, tag_identity());
    }

    /** @brief Splits into (left, right) where @p predicate first becomes true
     *         on the accumulated prefix; the triggering element is placed at
     *         the front of @c d_right; O(log n).
     */
    template <typename PREDICATE>
    auto split_at(PREDICATE &&predicate) const -> SplitAt {
        auto sp = split(std::forward<PREDICATE>(predicate));
        if (!sp.has_value()) {
            return SplitAt{*this, empty()};
        }
        return SplitAt{std::move(sp->d_left),
                       sp->d_right.cons(std::move(sp->d_pivot))};
    }

    /** @brief Splits after the first @p index elements; O(log n) when
     *         TAG_TYPE is std::size_t with UnitMeasure, O(n) otherwise.
     *
     * @param index Number of elements to place in @c d_left.
     *              Clamped to [0, breadth()].
     */
    auto split_at_index(std::size_t index) const -> SplitAt {
        if (index == 0U) {
            return SplitAt{empty(), *this};
        }
        if (index >= breadth()) {
            return SplitAt{*this, empty()};
        }
        if constexpr (std::is_same_v<Tag, std::size_t> &&
                      std::is_same_v<MeasurePolicy, UnitMeasure<T, Tag>>) {
            // Prefix measure is exactly running element count; navigate
            // structurally, no flatten.
            return split_at(
                [index](std::size_t prefix) { return prefix > index; });
        } else {
            // For non-count measures (including weighted size_t),
            // split_at_index must preserve index semantics, so fall back to
            // flatten-and-rebuild.
            auto vec = flatten();
            auto clamped = index > vec.size() ? vec.size() : index;
            std::vector<T> lv(vec.begin(),
                              vec.begin() +
                                  static_cast<std::ptrdiff_t>(clamped));
            std::vector<T> rv(
                vec.begin() + static_cast<std::ptrdiff_t>(clamped), vec.end());
            return SplitAt{from_sequence(std::move(lv)),
                           from_sequence(std::move(rv))};
        }
    }

    /** @brief Splits when the accumulated prefix measure first reaches
     *         @p threshold; requires TAG_TYPE to support @c >=; O(log n).
     */
    auto split_at_measure(const Tag &threshold) const -> SplitAt
        requires requires(const Tag &lhs, const Tag &rhs) {
            { lhs >= rhs } -> std::convertible_to<bool>;
        }
    {
        return split_at(
            [&threshold](const Tag &prefix) { return prefix >= threshold; });
    }

    /** Builds a tree from a vector by snocing each element in order; O(n). */
    static auto from_sequence(std::vector<T> values) -> FingerTree {
        auto result = empty();
        for (auto &v : values) {
            result = result.snoc(std::move(v));
        }
        return result;
    }
};

} // namespace smd::tree

#endif

```

## smd/tree/finger_tree_interval_index.hpp

```cpp
// src/smd/tree/finger_tree_interval_index.hpp                        -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_FINGER_TREE_INTERVAL_INDEX
#define INCLUDED_SMD_TREE_FINGER_TREE_INTERVAL_INDEX

#include <smd/tree/finger_tree.hpp>
#include <smd/typeclass/applicative.hpp>
#include <smd/typeclass/foldable.hpp>
#include <smd/typeclass/monoid.hpp>
#include <smd/typeclass/traversable.hpp>

#include <cstddef>
#include <functional>
#include <type_traits>
#include <utility>
#include <vector>

namespace smd::tree {

/** A half-open interval [d_start, d_end) annotated with an arbitrary payload. */
template <typename PAYLOAD_TYPE>
struct Interval {
    std::size_t d_start;
    std::size_t d_end;
    PAYLOAD_TYPE d_payload;
};

/** Measure tag caching the maximum endpoint of all intervals in a subtree.
 * Used to prune interval stabbing and overlap queries.
 */
template <typename PAYLOAD_TYPE>
struct IntervalMaxEndTag {
    std::size_t d_max_end;

    friend bool operator==(const IntervalMaxEndTag &,
                           const IntervalMaxEndTag &) = default;
};

/** Measure policy that maps an Interval to its IntervalMaxEndTag. */
template <typename PAYLOAD_TYPE>
struct IntervalMeasure {
    auto operator()(const Interval<PAYLOAD_TYPE> &interval) const
        -> IntervalMaxEndTag<PAYLOAD_TYPE> {
        return IntervalMaxEndTag<PAYLOAD_TYPE>{interval.d_end};
    }
};

/** @brief Persistent interval index supporting stabbing and overlap queries.
 *
 * @tparam PAYLOAD_TYPE Arbitrary data associated with each interval.
 *
 * Stores half-open intervals [start, end).  The IntervalMaxEndTag measure
 * allows measure-guided pruning: subtrees whose maximum endpoint is at or
 * below the query point cannot contain any matching interval.
 *
 * Complexity:
 * - insert:        O(1) amortized (snoc)
 * - query_point:   O(log n + k) where k is the number of results
 * - query_overlap: O(log n + k) where k is the number of results
 * - entries:       O(n)
 */
template <typename PAYLOAD_TYPE>
class FingerTreeIntervalIndex {
    using Entry = Interval<PAYLOAD_TYPE>;
    using Tree = FingerTree<Entry, IntervalMaxEndTag<PAYLOAD_TYPE>,
                            IntervalMeasure<PAYLOAD_TYPE>>;

    Tree d_tree;

  public:
    /** Constructs an empty interval index. */
    FingerTreeIntervalIndex() : d_tree(Tree::empty()) {}

    /** Builds an interval index from a vector of intervals; O(n). */
    static auto from_intervals(std::vector<Entry> entries)
        -> FingerTreeIntervalIndex {
        return FingerTreeIntervalIndex{Tree::from_sequence(std::move(entries))};
    }

    /** Returns a new index with @p entry appended; O(1) amortized. */
    auto insert(Entry entry) const -> FingerTreeIntervalIndex {
        return FingerTreeIntervalIndex{d_tree.snoc(std::move(entry))};
    }

    /** @brief Returns payloads of all intervals containing @p point.
     *
     * Uses measure-guided pruning to skip subtrees that cannot contain a match.
     */
    auto query_point(std::size_t point) const -> std::vector<PAYLOAD_TYPE> {
        std::vector<PAYLOAD_TYPE> out;

        // Prune: skip all intervals whose subtree max_end <= point.
        auto parts = d_tree.split_at(
            [point](const IntervalMaxEndTag<PAYLOAD_TYPE> &prefix) {
                return prefix.d_max_end > point;
            });

        // Fold the candidate subtree directly — no intermediate vector.
        parts.d_right.for_each([&](const Entry &e) {
            if (e.d_start <= point && point < e.d_end) {
                out.push_back(e.d_payload);
            }
        });

        return out;
    }

    /** @brief Returns payloads of all intervals overlapping [start, end).
     *
     * Two half-open intervals overlap when neither is entirely before the
     * other.  Uses measure-guided pruning to skip non-overlapping subtrees.
     */
    auto query_overlap(std::size_t start, std::size_t end) const
        -> std::vector<PAYLOAD_TYPE> {
        std::vector<PAYLOAD_TYPE> out;

        // Prune: skip all intervals whose subtree max_end <= start.
        auto parts = d_tree.split_at(
            [start](const IntervalMaxEndTag<PAYLOAD_TYPE> &prefix) {
                return prefix.d_max_end > start;
            });

        // Fold the candidate subtree directly — no intermediate vector.
        parts.d_right.for_each([&](const Entry &e) {
            if (e.d_start < end && start < e.d_end) {
                out.push_back(e.d_payload);
            }
        });

        return out;
    }

    /** Materialises all stored intervals in insertion order; O(n). */
    auto entries() const -> std::vector<Entry> { return d_tree.flatten(); }

  private:
    explicit FingerTreeIntervalIndex(Tree tree) : d_tree(std::move(tree)) {}
};

} // namespace smd::tree

namespace smd::typeclass {

/** Monoid instance for IntervalMaxEndTag: identity is 0; combine takes the max. */
template <typename PAYLOAD_TYPE>
struct Monoid<smd::tree::IntervalMaxEndTag<PAYLOAD_TYPE>> {
    auto identity() const -> smd::tree::IntervalMaxEndTag<PAYLOAD_TYPE> {
        return {0U};
    }

    auto combine(const smd::tree::IntervalMaxEndTag<PAYLOAD_TYPE> &lhs,
                 const smd::tree::IntervalMaxEndTag<PAYLOAD_TYPE> &rhs) const
        -> smd::tree::IntervalMaxEndTag<PAYLOAD_TYPE> {
        return {std::max(lhs.d_max_end, rhs.d_max_end)};
    }
};

} // namespace smd::typeclass

namespace smd {

/** Foldable typeclass implementation for FingerTreeIntervalIndex; folds over payloads. */
template <class PAYLOAD_TYPE>
struct FingerTreeIntervalIndexFoldableImpl {
    template <class F>
    auto fold_map(this auto &&, F &&function,
                  const smd::tree::FingerTreeIntervalIndex<PAYLOAD_TYPE> &index)
        -> remove_cvref_t<std::invoke_result_t<F, const PAYLOAD_TYPE &>> {
        using Result =
            remove_cvref_t<std::invoke_result_t<F, const PAYLOAD_TYPE &>>;
        return std::ranges::fold_left(
            index.entries(), smd::typeclass::monoid_v<Result>.identity(),
            [&](Result acc, const auto &entry) {
                return smd::typeclass::monoid_v<Result>.combine(
                    std::move(acc), std::invoke(function, entry.d_payload));
            });
    }
};

/** Foldable typeclass map entry for FingerTreeIntervalIndex. */
template <class PAYLOAD_TYPE>
struct FingerTreeIntervalIndexFoldableMap
    : Foldable<FingerTreeIntervalIndexFoldableImpl<PAYLOAD_TYPE>> {
    using FingerTreeIntervalIndexFoldableImpl<PAYLOAD_TYPE>::fold_map;
};

/** Registers FingerTreeIntervalIndex as a Foldable. */
template <class PAYLOAD_TYPE>
inline constexpr auto
    foldable_typeclass<smd::tree::FingerTreeIntervalIndex<PAYLOAD_TYPE>> =
        FingerTreeIntervalIndexFoldableMap<PAYLOAD_TYPE>{};

/** Traversable typeclass implementation for FingerTreeIntervalIndex; maps over
 * payloads while preserving interval geometry.
 */
template <class PAYLOAD_TYPE>
struct FingerTreeIntervalIndexTraversableImpl {
    using element_type = PAYLOAD_TYPE;

    template <class APPLICATIVE, class F>
    auto
    traverse(this auto &&, const APPLICATIVE &applicative, F &&function,
             const smd::tree::FingerTreeIntervalIndex<PAYLOAD_TYPE> &index) {
        using Context =
            remove_cvref_t<std::invoke_result_t<F, const PAYLOAD_TYPE &>>;
        using U = smd::applicative_value_t<Context>;

        auto accumulated =
            applicative.pure(std::vector<smd::tree::Interval<U>>{});

        for (const auto &entry : index.entries()) {
            auto lifted = std::invoke(function, entry.d_payload);
            accumulated = applicative.invoke(
                [start = entry.d_start, end = entry.d_end](
                    std::vector<smd::tree::Interval<U>> values, U payload) {
                    values.push_back(
                        smd::tree::Interval<U>{start, end, std::move(payload)});
                    return values;
                },
                std::move(accumulated), std::move(lifted));
        }

        return applicative.invoke(
            [](std::vector<smd::tree::Interval<U>> values) {
                return smd::tree::FingerTreeIntervalIndex<U>::from_intervals(
                    std::move(values));
            },
            std::move(accumulated));
    }
};

/** Traversable typeclass map entry for FingerTreeIntervalIndex. */
template <class PAYLOAD_TYPE>
struct FingerTreeIntervalIndexTraversableMap
    : Traversable<FingerTreeIntervalIndexTraversableImpl<PAYLOAD_TYPE>> {
    using FingerTreeIntervalIndexTraversableImpl<PAYLOAD_TYPE>::traverse;
};

/** Registers FingerTreeIntervalIndex as a Traversable. */
template <class PAYLOAD_TYPE>
inline constexpr auto
    traversable_typeclass<smd::tree::FingerTreeIntervalIndex<PAYLOAD_TYPE>> =
        FingerTreeIntervalIndexTraversableMap<PAYLOAD_TYPE>{};

} // namespace smd

#endif

```

## smd/tree/finger_tree_interval_index.t.cpp

```cpp
#include <smd/tree/finger_tree_interval_index.hpp>
#include <smd/tree/finger_tree_interval_index.hpp> // Re-inclusion check

#include <catch2/catch_test_macros.hpp>

#include <optional>
#include <string>
#include <vector>

TEST_CASE("FingerTreeIntervalIndexTest - WrapperOperations") {
    using Index = smd::tree::FingerTreeIntervalIndex<std::string>;
    using Entry = smd::tree::Interval<std::string>;

    auto idx = Index::from_intervals(
        {Entry{0U, 5U, "A"}, Entry{3U, 10U, "B"}, Entry{8U, 12U, "C"}});

    CHECK(idx.query_point(2U) == (std::vector<std::string>{"A"}));
    CHECK(idx.query_point(4U) == (std::vector<std::string>{"A", "B"}));
    CHECK(idx.query_overlap(9U, 11U) == (std::vector<std::string>{"B", "C"}));
}

TEST_CASE("FingerTreeIntervalIndexTest - FoldableTypeclass") {
    using Index = smd::tree::FingerTreeIntervalIndex<std::string>;
    using Entry = smd::tree::Interval<std::string>;

    auto idx = Index::from_intervals(
        {Entry{0U, 5U, "A"}, Entry{3U, 10U, "B"}, Entry{8U, 12U, "C"}});
    const auto &foldable = smd::foldable_typeclass<Index>;

    CHECK(foldable.fold_map([](const std::string &payload) { return payload; },
                            idx) == "ABC");
    CHECK(foldable.length(idx) == 3U);
}

TEST_CASE("FingerTreeIntervalIndexTest - TraversableTypeclass") {
    using Index = smd::tree::FingerTreeIntervalIndex<std::string>;
    using Entry = smd::tree::Interval<std::string>;

    auto idx = Index::from_intervals(
        {Entry{0U, 5U, "A"}, Entry{3U, 10U, "B"}, Entry{8U, 12U, "C"}});

    auto success = smd::traverse(
        [](const std::string &payload) -> std::optional<std::string> {
            return payload + "!";
        },
        idx);
    REQUIRE(success.has_value());
    CHECK(success->query_point(4U) == (std::vector<std::string>{"A!", "B!"}));
    CHECK(success->query_overlap(9U, 11U) ==
          (std::vector<std::string>{"B!", "C!"}));

    auto failure = smd::traverse(
        [](const std::string &payload) -> std::optional<std::string> {
            if (payload == "B") {
                return std::nullopt;
            }
            return payload;
        },
        idx);
    CHECK_FALSE(failure.has_value());
}

```

## smd/tree/finger_tree_priority_queue.hpp

```cpp
// src/smd/tree/finger_tree_priority_queue.hpp                        -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_FINGER_TREE_PRIORITY_QUEUE
#define INCLUDED_SMD_TREE_FINGER_TREE_PRIORITY_QUEUE

#include <smd/tree/finger_tree.hpp>
#include <smd/typeclass/applicative.hpp>
#include <smd/typeclass/foldable.hpp>
#include <smd/typeclass/monoid.hpp>
#include <smd/typeclass/traversable.hpp>

#include <algorithm>
#include <cstddef>
#include <functional>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

namespace smd::tree {

/** Measure tag tracking the minimum element in a subtree; empty subtree is
 * represented by nullopt.
 */
template <typename T>
struct MinTag {
    std::optional<T> d_value;

    friend bool operator==(const MinTag &, const MinTag &) = default;
};

/** Measure tag tracking the maximum element in a subtree; empty subtree is
 * represented by nullopt.
 */
template <typename T>
struct MaxTag {
    std::optional<T> d_value;

    friend bool operator==(const MaxTag &, const MaxTag &) = default;
};

/** Combined measure tracking both min and max in a single tree pass. */
template <typename T>
struct PriorityTag {
    MinTag<T> d_min;
    MaxTag<T> d_max;

    friend bool operator==(const PriorityTag &, const PriorityTag &) = default;
};

/** Measure policy that lifts a single element into a PriorityTag. */
template <typename T>
struct PriorityMeasure {
    auto operator()(const T &value) const -> PriorityTag<T> {
        return PriorityTag<T>{MinTag<T>{value}, MaxTag<T>{value}};
    }
};

/** @brief Persistent double-ended priority queue backed by a finger tree.
 *
 * @tparam T Element type; must be totally ordered.
 *
 * pop_min and pop_max are O(log n) via measure-guided split.
 * push is O(1) amortized.
 * min/max queries are O(1) via the cached PriorityTag.
 */
template <typename T>
class FingerTreePriorityQueue {
    using Tree = FingerTree<T, PriorityTag<T>, PriorityMeasure<T>>;

    Tree d_tree;

    explicit FingerTreePriorityQueue(Tree tree) : d_tree(std::move(tree)) {}

  public:
    /** Constructs an empty priority queue. */
    FingerTreePriorityQueue() : d_tree(Tree::empty()) {}

    /** Builds a priority queue from a vector of values; O(n). */
    static auto from_values(std::vector<T> values) -> FingerTreePriorityQueue {
        return FingerTreePriorityQueue{Tree::from_sequence(std::move(values))};
    }

    /** Returns true if the queue contains no elements. */
    auto empty() const -> bool { return d_tree.is_empty(); }

    /** Returns the number of elements. */
    auto size() const -> std::size_t { return d_tree.breadth(); }

    /** Returns the minimum element, or nullopt if empty; O(1). */
    auto min() const -> std::optional<T> {
        auto m = d_tree.measure().d_min.d_value;
        return m.has_value() ? std::optional<T>{*m} : std::nullopt;
    }

    /** Returns the maximum element, or nullopt if empty; O(1). */
    auto max() const -> std::optional<T> {
        auto m = d_tree.measure().d_max.d_value;
        return m.has_value() ? std::optional<T>{*m} : std::nullopt;
    }

    /** Returns a new queue with @p value inserted; O(1) amortized. */
    auto push(T value) const -> FingerTreePriorityQueue {
        return FingerTreePriorityQueue{d_tree.snoc(std::move(value))};
    }

    // O(log n): prefix min is non-increasing; predicate flips true at the first
    // element whose value equals the global min and stays true thereafter.
    /** @brief Removes and returns the minimum element; O(log n).
     *
     * @return nullopt if empty; otherwise a pair of the minimum value and the
     *         remaining queue.
     */
    auto pop_min() const
        -> std::optional<std::pair<T, FingerTreePriorityQueue>> {
        auto tag = d_tree.measure();
        if (!tag.d_min.d_value.has_value()) {
            return std::nullopt;
        }
        T global_min = *tag.d_min.d_value;
        auto sp = d_tree.split([global_min](const PriorityTag<T> &p) {
            return p.d_min.d_value.has_value() &&
                   *p.d_min.d_value <= global_min;
        });
        if (!sp.has_value()) {
            return std::nullopt;
        }
        return std::pair<T, FingerTreePriorityQueue>{
            sp->d_pivot,
            FingerTreePriorityQueue{Tree::concat(sp->d_left, sp->d_right)}};
    }

    // O(log n): prefix max is non-decreasing; predicate flips true at the first
    // element whose value equals the global max and stays true thereafter.
    /** @brief Removes and returns the maximum element; O(log n).
     *
     * @return nullopt if empty; otherwise a pair of the maximum value and the
     *         remaining queue.
     */
    auto pop_max() const
        -> std::optional<std::pair<T, FingerTreePriorityQueue>> {
        auto tag = d_tree.measure();
        if (!tag.d_max.d_value.has_value()) {
            return std::nullopt;
        }
        T global_max = *tag.d_max.d_value;
        auto sp = d_tree.split([global_max](const PriorityTag<T> &p) {
            return p.d_max.d_value.has_value() &&
                   *p.d_max.d_value >= global_max;
        });
        if (!sp.has_value()) {
            return std::nullopt;
        }
        return std::pair<T, FingerTreePriorityQueue>{
            sp->d_pivot,
            FingerTreePriorityQueue{Tree::concat(sp->d_left, sp->d_right)}};
    }

    /** Materialises all elements into a vector in insertion order; O(n). */
    auto to_vector() const -> std::vector<T> { return d_tree.flatten(); }
};

} // namespace smd::tree

namespace smd::typeclass {

/** Monoid instance for MinTag: identity is nullopt; combine keeps the lesser value. */
template <typename T>
struct Monoid<smd::tree::MinTag<T>> {
    auto identity() const -> smd::tree::MinTag<T> { return {std::nullopt}; }

    auto combine(const smd::tree::MinTag<T> &lhs,
                 const smd::tree::MinTag<T> &rhs) const
        -> smd::tree::MinTag<T> {
        if (!lhs.d_value.has_value()) {
            return rhs;
        }
        if (!rhs.d_value.has_value()) {
            return lhs;
        }

        return lhs.d_value.value() <= rhs.d_value.value() ? lhs : rhs;
    }
};

/** Monoid instance for MaxTag: identity is nullopt; combine keeps the greater value. */
template <typename T>
struct Monoid<smd::tree::MaxTag<T>> {
    auto identity() const -> smd::tree::MaxTag<T> { return {std::nullopt}; }

    auto combine(const smd::tree::MaxTag<T> &lhs,
                 const smd::tree::MaxTag<T> &rhs) const
        -> smd::tree::MaxTag<T> {
        if (!lhs.d_value.has_value()) {
            return rhs;
        }
        if (!rhs.d_value.has_value()) {
            return lhs;
        }

        return lhs.d_value.value() >= rhs.d_value.value() ? lhs : rhs;
    }
};

/** Monoid instance for PriorityTag: combines MinTag and MaxTag independently. */
template <typename T>
struct Monoid<smd::tree::PriorityTag<T>> {
    auto identity() const -> smd::tree::PriorityTag<T> {
        return {Monoid<smd::tree::MinTag<T>>{}.identity(),
                Monoid<smd::tree::MaxTag<T>>{}.identity()};
    }

    auto combine(const smd::tree::PriorityTag<T> &lhs,
                 const smd::tree::PriorityTag<T> &rhs) const
        -> smd::tree::PriorityTag<T> {
        return {Monoid<smd::tree::MinTag<T>>{}.combine(lhs.d_min, rhs.d_min),
                Monoid<smd::tree::MaxTag<T>>{}.combine(lhs.d_max, rhs.d_max)};
    }
};

} // namespace smd::typeclass

namespace smd {

/** Foldable typeclass implementation for FingerTreePriorityQueue. */
template <class T>
struct FingerTreePriorityQueueFoldableImpl {
    template <class F>
    auto fold_map(this auto &&, F &&function,
                  const smd::tree::FingerTreePriorityQueue<T> &queue)
        -> remove_cvref_t<std::invoke_result_t<F, const T &>> {
        using Result = remove_cvref_t<std::invoke_result_t<F, const T &>>;
        return std::ranges::fold_left(
            queue.to_vector(), smd::typeclass::monoid_v<Result>.identity(),
            [&](Result acc, const auto &value) {
                return smd::typeclass::monoid_v<Result>.combine(
                    std::move(acc), std::invoke(function, value));
            });
    }
};

/** Foldable typeclass map entry for FingerTreePriorityQueue. */
template <class T>
struct FingerTreePriorityQueueFoldableMap
    : Foldable<FingerTreePriorityQueueFoldableImpl<T>> {
    using FingerTreePriorityQueueFoldableImpl<T>::fold_map;
};

/** Registers FingerTreePriorityQueue as a Foldable. */
template <class T>
inline constexpr auto
    foldable_typeclass<smd::tree::FingerTreePriorityQueue<T>> =
        FingerTreePriorityQueueFoldableMap<T>{};

/** Traversable typeclass implementation for FingerTreePriorityQueue. */
template <class T>
struct FingerTreePriorityQueueTraversableImpl {
    using element_type = T;

    template <class APPLICATIVE, class F>
    auto traverse(this auto &&, const APPLICATIVE &applicative, F &&function,
                  const smd::tree::FingerTreePriorityQueue<T> &queue) {
        using Context = remove_cvref_t<std::invoke_result_t<F, const T &>>;
        using U = smd::applicative_value_t<Context>;

        auto accumulated = applicative.pure(std::vector<U>{});

        for (const auto &value : queue.to_vector()) {
            auto lifted = std::invoke(function, value);
            accumulated = applicative.invoke(
                [](std::vector<U> values, U element) {
                    values.push_back(std::move(element));
                    return values;
                },
                std::move(accumulated), std::move(lifted));
        }

        return applicative.invoke(
            [](std::vector<U> values) {
                return smd::tree::FingerTreePriorityQueue<U>::from_values(
                    std::move(values));
            },
            std::move(accumulated));
    }
};

/** Traversable typeclass map entry for FingerTreePriorityQueue. */
template <class T>
struct FingerTreePriorityQueueTraversableMap
    : Traversable<FingerTreePriorityQueueTraversableImpl<T>> {
    using FingerTreePriorityQueueTraversableImpl<T>::traverse;
};

/** Registers FingerTreePriorityQueue as a Traversable. */
template <class T>
inline constexpr auto
    traversable_typeclass<smd::tree::FingerTreePriorityQueue<T>> =
        FingerTreePriorityQueueTraversableMap<T>{};

} // namespace smd

#endif

```

## smd/tree/finger_tree_priority_queue.t.cpp

```cpp
#include <smd/tree/finger_tree_priority_queue.hpp>
#include <smd/tree/finger_tree_priority_queue.hpp> // Re-inclusion check

#include <catch2/catch_test_macros.hpp>

#include <optional>
#include <set>
#include <vector>

TEST_CASE("FingerTreePriorityQueueTest - WrapperOperations") {
    using Queue = smd::tree::FingerTreePriorityQueue<int>;

    // a4d7f1c6-3b9e-4a2d-f8c4-6e5b1d9c3a07
    auto q = Queue::from_values({5, 2, 8, 2, 7});
    REQUIRE(q.min().has_value());
    REQUIRE(q.max().has_value());
    CHECK(*q.min() == 2);
    CHECK(*q.max() == 8);
    // a4d7f1c6-3b9e-4a2d-f8c4-6e5b1d9c3a07 end

    auto min_pop = q.pop_min();
    REQUIRE(min_pop.has_value());
    CHECK(min_pop->first == 2);
    REQUIRE(min_pop->second.min().has_value());
    CHECK(*min_pop->second.min() == 2);

    auto max_pop = min_pop->second.pop_max();
    REQUIRE(max_pop.has_value());
    CHECK(max_pop->first == 8);
    REQUIRE(max_pop->second.max().has_value());
    CHECK(*max_pop->second.max() == 7);
}

TEST_CASE("FingerTreePriorityQueueTest - FoldableTypeclass") {
    using Queue = smd::tree::FingerTreePriorityQueue<int>;

    auto q = Queue::from_values({5, 2, 8, 2, 7});
    const auto &foldable = smd::foldable_typeclass<Queue>;

    CHECK(foldable.fold_map([](int value) { return value; }, q) == 24);
    CHECK(foldable.length(q) == 5U);
}

TEST_CASE("FingerTreePriorityQueueTest - TraversableTypeclass") {
    using Queue = smd::tree::FingerTreePriorityQueue<int>;

    auto q = Queue::from_values({5, 2, 8});

    auto success = smd::traverse(
        [](int value) -> std::optional<int> { return value * 10; }, q);
    REQUIRE(success.has_value());
    CHECK(success->to_vector() == (std::vector<int>{50, 20, 80}));
    REQUIRE(success->min().has_value());
    REQUIRE(success->max().has_value());
    CHECK(*success->min() == 20);
    CHECK(*success->max() == 80);

    auto failure = smd::traverse(
        [](int value) -> std::optional<int> {
            if (value == 8) {
                return std::nullopt;
            }
            return value;
        },
        q);
    CHECK_FALSE(failure.has_value());
}

TEST_CASE("FingerTreePriorityQueueTest - RepeatedPushPopMatchesMultiset") {
    using Queue = smd::tree::FingerTreePriorityQueue<int>;

    auto q = Queue::from_values({5, 2, 8, 2, 7, 1, 9, 1});
    std::multiset<int> expected{5, 2, 8, 2, 7, 1, 9, 1};

    for (int i = 0; i < 250; ++i) {
        auto value = (i * 7) % 11;
        q = q.push(value);
        expected.insert(value);

        if ((i % 2) == 0) {
            auto popped = q.pop_min();
            REQUIRE(popped.has_value());
            REQUIRE_FALSE(expected.empty());
            CHECK(popped->first == *expected.begin());
            expected.erase(expected.begin());
            q = std::move(popped->second);
        } else {
            auto popped = q.pop_max();
            REQUIRE(popped.has_value());
            REQUIRE_FALSE(expected.empty());
            auto it = std::prev(expected.end());
            CHECK(popped->first == *it);
            expected.erase(it);
            q = std::move(popped->second);
        }

        CHECK(q.size() == expected.size());
        CHECK(q.size() == q.to_vector().size());

        if (!expected.empty()) {
            REQUIRE(q.min().has_value());
            REQUIRE(q.max().has_value());
            CHECK(*q.min() == *expected.begin());
            CHECK(*q.max() == *std::prev(expected.end()));
        } else {
            CHECK_FALSE(q.min().has_value());
            CHECK_FALSE(q.max().has_value());
        }
    }

    auto values = q.to_vector();
    std::multiset<int> actual(values.begin(), values.end());
    CHECK(actual == expected);
}

TEST_CASE("FingerTreePriorityQueueTest - PopMinWithDuplicates") {
    using Queue = smd::tree::FingerTreePriorityQueue<int>;

    auto q = Queue::from_values({5, 2, 8, 2, 7});
    REQUIRE(q.min().has_value());
    CHECK(*q.min() == 2);

    auto pop1 = q.pop_min();
    REQUIRE(pop1.has_value());
    CHECK(pop1->first == 2);

    auto q2 = pop1->second;
    REQUIRE(q2.min().has_value());
    REQUIRE(q2.max().has_value());
    CHECK(*q2.min() == 2);
    CHECK(*q2.max() == 8);

    auto pop2 = q2.pop_min();
    REQUIRE(pop2.has_value());
    CHECK(pop2->first == 2);

    auto q3 = pop2->second;
    REQUIRE(q3.min().has_value());
    REQUIRE(q3.max().has_value());
    CHECK(*q3.min() == 5);
    CHECK(*q3.max() == 8);
}

TEST_CASE("FingerTreePriorityQueueTest - PopMaxWithDuplicates") {
    using Queue = smd::tree::FingerTreePriorityQueue<int>;

    auto q = Queue::from_values({5, 8, 2, 8, 7});
    REQUIRE(q.max().has_value());
    CHECK(*q.max() == 8);

    auto pop1 = q.pop_max();
    REQUIRE(pop1.has_value());
    CHECK(pop1->first == 8);

    auto q2 = pop1->second;
    REQUIRE(q2.min().has_value());
    REQUIRE(q2.max().has_value());
    CHECK(*q2.max() == 8);
    CHECK(*q2.min() == 2);

    auto pop2 = q2.pop_max();
    REQUIRE(pop2.has_value());
    CHECK(pop2->first == 8);

    auto q3 = pop2->second;
    REQUIRE(q3.min().has_value());
    REQUIRE(q3.max().has_value());
    CHECK(*q3.max() == 7);
    CHECK(*q3.min() == 2);
}

```

## smd/tree/finger_tree_random_access.hpp

```cpp
// src/smd/tree/finger_tree_random_access.hpp                         -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_FINGER_TREE_RANDOM_ACCESS
#define INCLUDED_SMD_TREE_FINGER_TREE_RANDOM_ACCESS

#include <smd/tree/finger_tree.hpp>
#include <smd/typeclass/applicative.hpp>
#include <smd/typeclass/foldable.hpp>
#include <smd/typeclass/traversable.hpp>

#include <algorithm>
#include <cstddef>
#include <functional>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

namespace smd::tree {

/** @brief Persistent random-access sequence backed by a finger tree.
 *
 * @tparam T Element type.
 *
 * Uses UnitMeasure (element count) as the finger tree measure, enabling O(log
 * n) index navigation without materialisation.
 *
 * Complexity:
 * - at / update / insert / erase: O(log n)
 * - push_front / push_back:       O(1) amortized
 * - size / empty:                 O(1)
 * - to_vector:                    O(n)
 */
template <typename T>
class FingerTreeRandomAccess {
    FingerTree<T> d_tree;

  public:
    /** Constructs an empty sequence. */
    FingerTreeRandomAccess() : d_tree(FingerTree<T>::empty()) {}

    /** Constructs from an existing finger tree. */
    explicit FingerTreeRandomAccess(FingerTree<T> tree)
        : d_tree(std::move(tree)) {}

    /** Builds a sequence from a vector in order; O(n). */
    static auto from_sequence(std::vector<T> values) -> FingerTreeRandomAccess {
        return FingerTreeRandomAccess(
            FingerTree<T>::from_sequence(std::move(values)));
    }

    /** Returns the number of elements. */
    auto size() const -> std::size_t { return d_tree.breadth(); }

    /** Returns true if the sequence contains no elements. */
    auto empty() const -> bool { return d_tree.is_empty(); }

    /** Returns the element at @p index, or nullopt if out of range; O(log n). */
    auto at(std::size_t index) const -> std::optional<T> {
        if (index >= size()) {
            return std::nullopt;
        }
        // split() with count predicate: pivot is the element at position index.
        // O(log n).
        auto sp = d_tree.split(
            [index](std::size_t prefix) { return prefix > index; });
        if (!sp.has_value()) {
            return std::nullopt;
        }
        return sp->d_pivot;
    }

    /** Returns a new sequence with @p value appended at the back; O(1) amortized. */
    auto push_back(T value) const -> FingerTreeRandomAccess {
        return FingerTreeRandomAccess(d_tree.snoc(std::move(value)));
    }

    /** Returns a new sequence with @p value prepended at the front; O(1) amortized. */
    auto push_front(T value) const -> FingerTreeRandomAccess {
        return FingerTreeRandomAccess(d_tree.cons(std::move(value)));
    }

    /** Returns a new sequence with @p value inserted before position @p index; O(log n). */
    auto insert(std::size_t index, T value) const -> FingerTreeRandomAccess {
        // split_at with count predicate puts [0,index) left, [index,n) right.
        // O(log n).
        auto parts = d_tree.split_at(
            [index](std::size_t prefix) { return prefix > index; });
        return FingerTreeRandomAccess(FingerTree<T>::concat(
            FingerTree<T>::concat(parts.d_left,
                                  FingerTree<T>::leaf(std::move(value))),
            parts.d_right));
    }

    /** Returns a new sequence with the element at @p index removed; O(log n).
     * Returns @c *this unchanged if @p index is out of range.
     */
    auto erase(std::size_t index) const -> FingerTreeRandomAccess {
        if (index >= size()) {
            return *this;
        }
        // split() finds the element at index as pivot; drop it by concat(left,
        // right). O(log n).
        auto sp = d_tree.split(
            [index](std::size_t prefix) { return prefix > index; });
        if (!sp.has_value()) {
            return *this;
        }
        return FingerTreeRandomAccess(
            FingerTree<T>::concat(sp->d_left, sp->d_right));
    }

    /** Returns a new sequence with position @p index replaced by @p value; O(log n).
     * Returns @c *this unchanged if @p index is out of range.
     */
    auto update(std::size_t index, T value) const -> FingerTreeRandomAccess {
        if (index >= size()) {
            return *this;
        }
        // Single split+replace: find element at index as pivot, swap in the new
        // value. O(log n).
        auto sp = d_tree.split(
            [index](std::size_t prefix) { return prefix > index; });
        if (!sp.has_value()) {
            return *this;
        }
        return FingerTreeRandomAccess(FingerTree<T>::concat(
            sp->d_left.snoc(std::move(value)), sp->d_right));
    }

    /** Materialises all elements into a vector in sequence order; O(n). */
    auto to_vector() const -> std::vector<T> { return d_tree.flatten(); }
};

} // namespace smd::tree

namespace smd {

/** Foldable typeclass implementation for FingerTreeRandomAccess. */
template <class T>
struct FingerTreeRandomAccessFoldableImpl {
    template <class F>
    auto fold_map(this auto &&, F &&function,
                  const smd::tree::FingerTreeRandomAccess<T> &sequence)
        -> remove_cvref_t<std::invoke_result_t<F, const T &>> {
        using Result = remove_cvref_t<std::invoke_result_t<F, const T &>>;
        return std::ranges::fold_left(
            sequence.to_vector(), smd::typeclass::monoid_v<Result>.identity(),
            [&](Result acc, const auto &value) {
                return smd::typeclass::monoid_v<Result>.combine(
                    std::move(acc), std::invoke(function, value));
            });
    }
};

/** Foldable typeclass map entry for FingerTreeRandomAccess. */
template <class T>
struct FingerTreeRandomAccessFoldableMap
    : Foldable<FingerTreeRandomAccessFoldableImpl<T>> {
    using FingerTreeRandomAccessFoldableImpl<T>::fold_map;
};

/** Registers FingerTreeRandomAccess as a Foldable. */
template <class T>
inline constexpr auto foldable_typeclass<smd::tree::FingerTreeRandomAccess<T>> =
    FingerTreeRandomAccessFoldableMap<T>{};

/** Traversable typeclass implementation for FingerTreeRandomAccess. */
template <class T>
struct FingerTreeRandomAccessTraversableImpl {
    using element_type = T;

    template <class APPLICATIVE, class F>
    auto traverse(this auto &&, const APPLICATIVE &applicative, F &&function,
                  const smd::tree::FingerTreeRandomAccess<T> &sequence) {
        using Context = remove_cvref_t<std::invoke_result_t<F, const T &>>;
        using U = smd::applicative_value_t<Context>;

        auto accumulated = applicative.pure(std::vector<U>{});

        for (const auto &value : sequence.to_vector()) {
            auto lifted = std::invoke(function, value);
            accumulated = applicative.invoke(
                [](std::vector<U> values, U element) {
                    values.push_back(std::move(element));
                    return values;
                },
                std::move(accumulated), std::move(lifted));
        }

        return applicative.invoke(
            [](std::vector<U> values) {
                return smd::tree::FingerTreeRandomAccess<U>::from_sequence(
                    std::move(values));
            },
            std::move(accumulated));
    }
};

/** Traversable typeclass map entry for FingerTreeRandomAccess. */
template <class T>
struct FingerTreeRandomAccessTraversableMap
    : Traversable<FingerTreeRandomAccessTraversableImpl<T>> {
    using FingerTreeRandomAccessTraversableImpl<T>::traverse;
};

/** Registers FingerTreeRandomAccess as a Traversable. */
template <class T>
inline constexpr auto
    traversable_typeclass<smd::tree::FingerTreeRandomAccess<T>> =
        FingerTreeRandomAccessTraversableMap<T>{};

} // namespace smd

#endif

```

## smd/tree/finger_tree_random_access.t.cpp

```cpp
#include <smd/tree/finger_tree_random_access.hpp>
#include <smd/tree/finger_tree_random_access.hpp> // Re-inclusion check

#include <catch2/catch_test_macros.hpp>

#include <optional>
#include <vector>

TEST_CASE("FingerTreeRandomAccessTest - WrapperOperations") {
    // f2a6c9b3-8d1e-4f7a-c5b9-4e2d7a3c6f01
    using Seq = smd::tree::FingerTreeRandomAccess<int>;

    auto seq = Seq::from_sequence({1, 2, 3});
    REQUIRE(seq.at(0).has_value());
    CHECK(*seq.at(0) == 1);
    CHECK_FALSE(seq.at(99).has_value());

    auto edited =
        seq.push_back(4).push_front(0).insert(2, 9).update(3, 7).erase(1);
    CHECK(edited.to_vector() == (std::vector<int>{0, 9, 7, 3, 4}));
    // f2a6c9b3-8d1e-4f7a-c5b9-4e2d7a3c6f01 end
}

TEST_CASE("FingerTreeRandomAccessTest - FoldableTypeclass") {
    using Seq = smd::tree::FingerTreeRandomAccess<int>;

    auto seq = Seq::from_sequence({1, 2, 3, 4});
    const auto &foldable = smd::foldable_typeclass<Seq>;

    CHECK(foldable.fold_map([](int value) { return value; }, seq) == 10);
    CHECK(foldable.length(seq) == 4U);
}

TEST_CASE("FingerTreeRandomAccessTest - TraversableTypeclass") {
    using Seq = smd::tree::FingerTreeRandomAccess<int>;

    auto seq = Seq::from_sequence({1, 2, 3});

    auto success = smd::traverse(
        [](int value) -> std::optional<int> { return value * 10; }, seq);
    REQUIRE(success.has_value());
    CHECK(success->to_vector() == (std::vector<int>{10, 20, 30}));

    auto failure = smd::traverse(
        [](int value) -> std::optional<int> {
            if (value == 2) {
                return std::nullopt;
            }
            return value;
        },
        seq);
    CHECK_FALSE(failure.has_value());
}

```

## smd/tree/finger_tree_rope.hpp

```cpp
// src/smd/tree/finger_tree_rope.hpp                                  -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_FINGER_TREE_ROPE
#define INCLUDED_SMD_TREE_FINGER_TREE_ROPE

#include <smd/tree/finger_tree.hpp>
#include <smd/typeclass/applicative.hpp>
#include <smd/typeclass/foldable.hpp>
#include <smd/typeclass/traversable.hpp>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <functional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace smd::tree {

/** Measure policy that maps a string chunk to its byte length. */
struct RopeChunkMeasure {
    auto operator()(const std::string &value) const -> std::size_t {
        return value.size();
    }
};

/** @brief Persistent text buffer implemented as a rope over string chunks.
 *
 * The underlying finger tree uses RopeChunkMeasure so the accumulated measure
 * at any node is the total byte count of all chunks to the left.  This enables
 * O(log n) positional split and, consequently, O(log n) insert/erase/replace.
 *
 * Chunk boundaries are not exposed in the public API; callers work with byte
 * positions and string_view values.
 *
 * Complexity:
 * - from_text:    O(n / chunk_size)
 * - size_bytes:   O(1)
 * - to_string:    O(n)
 * - insert/erase/replace: O(log n)
 * - chunks:       O(n)
 */
class FingerTreeRope {
    using Tree = FingerTree<std::string, std::size_t, RopeChunkMeasure>;

    Tree d_tree;

    /** Splits the rope at byte position @p pos; may split a chunk in two. */
    auto split_chars(std::size_t pos) const
        -> std::pair<FingerTreeRope, FingerTreeRope> {
        if (pos == 0) {
            return {FingerTreeRope{}, *this};
        }

        if (pos >= size_bytes()) {
            return {*this, FingerTreeRope{}};
        }

        auto split =
            d_tree.split([pos](std::size_t prefix) { return prefix > pos; });

        if (!split.has_value()) {
            return {*this, FingerTreeRope{}};
        }

        auto left_prefix_bytes = split->d_left.measure();
        auto local = pos - left_prefix_bytes;

        const auto &pivot = split->d_pivot;
        assert(local < pivot.size());

        auto left = split->d_left;
        if (local > 0) {
            left = left.snoc(pivot.substr(0, local));
        }

        auto right = split->d_right;
        if (local < pivot.size()) {
            right = right.cons(pivot.substr(local));
        }

        return {FingerTreeRope{std::move(left)},
                FingerTreeRope{std::move(right)}};
    }

  public:
    /** Constructs an empty rope. */
    FingerTreeRope() : d_tree(Tree::empty()) {}

    /** Builds a rope from a vector of pre-formed string chunks; O(n chunks). */
    static auto from_chunks(std::vector<std::string> chunks) -> FingerTreeRope {
        return FingerTreeRope{Tree::from_sequence(std::move(chunks))};
    }

    /** @brief Builds a rope from a text string, splitting into chunks of at
     *         most @p chunk_size bytes.
     */
    static auto from_text(std::string_view text, std::size_t chunk_size = 16)
        -> FingerTreeRope {
        std::vector<std::string> chunks;
        chunks.reserve((text.size() / chunk_size) + 1);

        for (std::size_t i = 0; i < text.size(); i += chunk_size) {
            const auto n = std::min(chunk_size, text.size() - i);
            chunks.emplace_back(text.substr(i, n));
        }

        return from_chunks(std::move(chunks));
    }

    /** Returns the total length of the text in bytes; O(1). */
    auto size_bytes() const -> std::size_t { return d_tree.measure(); }

    /** Concatenates all chunks into a single string; O(n). */
    auto to_string() const -> std::string {
        std::string out;
        out.reserve(size_bytes());
        std::ranges::for_each(
            d_tree.flatten(),
            [&out](const std::string &chunk) { out += chunk; });
        return out;
    }

    /** Returns a new rope with @p text inserted at byte position @p pos; O(log n). */
    auto insert(std::size_t pos, std::string_view text) const
        -> FingerTreeRope {
        auto [left, right] = split_chars(pos);
        auto middle = from_text(text);
        return FingerTreeRope{Tree::concat(
            Tree::concat(left.d_tree, middle.d_tree), right.d_tree)};
    }

    /** Returns a new rope with @p count bytes removed starting at @p pos; O(log n). */
    auto erase(std::size_t pos, std::size_t count) const -> FingerTreeRope {
        auto [left, rest] = split_chars(pos);
        auto [drop, right] = rest.split_chars(count);
        static_cast<void>(drop);
        return FingerTreeRope{Tree::concat(left.d_tree, right.d_tree)};
    }

    /** Returns a new rope with @p count bytes at @p pos replaced by @p text; O(log n). */
    auto replace(std::size_t pos, std::size_t count,
                 std::string_view text) const -> FingerTreeRope {
        return erase(pos, count).insert(pos, text);
    }

    /** Returns a snapshot of the internal chunk vector in sequence order; O(n). */
    auto chunks() const -> std::vector<std::string> { return d_tree.flatten(); }

  private:
    explicit FingerTreeRope(Tree tree) : d_tree(std::move(tree)) {}
};

} // namespace smd::tree

namespace smd {

/** Foldable typeclass implementation for FingerTreeRope; folds over string chunks. */
struct FingerTreeRopeFoldableImpl {
    template <class F>
    auto fold_map(this auto &&, F &&function,
                  const smd::tree::FingerTreeRope &rope)
        -> remove_cvref_t<std::invoke_result_t<F, const std::string &>> {
        using Result =
            remove_cvref_t<std::invoke_result_t<F, const std::string &>>;
        return std::ranges::fold_left(
            rope.chunks(), smd::typeclass::monoid_v<Result>.identity(),
            [&](Result acc, const auto &chunk) {
                return smd::typeclass::monoid_v<Result>.combine(
                    std::move(acc), std::invoke(function, chunk));
            });
    }
};

/** Foldable typeclass map entry for FingerTreeRope. */
struct FingerTreeRopeFoldableMap : Foldable<FingerTreeRopeFoldableImpl> {
    using FingerTreeRopeFoldableImpl::fold_map;
};

/** Registers FingerTreeRope as a Foldable. */
template <>
inline constexpr auto foldable_typeclass<smd::tree::FingerTreeRope> =
    FingerTreeRopeFoldableMap{};

/** Traversable typeclass implementation for FingerTreeRope; maps over chunks. */
struct FingerTreeRopeTraversableImpl {
    using element_type = std::string;

    template <class APPLICATIVE, class F>
    auto traverse(this auto &&, const APPLICATIVE &applicative, F &&function,
                  const smd::tree::FingerTreeRope &rope) {
        using Context =
            remove_cvref_t<std::invoke_result_t<F, const std::string &>>;
        using U = smd::applicative_value_t<Context>;

        auto accumulated = applicative.pure(std::vector<U>{});

        for (const auto &chunk : rope.chunks()) {
            auto lifted = std::invoke(function, chunk);
            accumulated = applicative.invoke(
                [](std::vector<U> values, U element) {
                    values.push_back(std::move(element));
                    return values;
                },
                std::move(accumulated), std::move(lifted));
        }

        return applicative.invoke(
            [](std::vector<U> values) {
                return smd::tree::FingerTreeRope::from_chunks(
                    std::move(values));
            },
            std::move(accumulated));
    }
};

/** Traversable typeclass map entry for FingerTreeRope. */
struct FingerTreeRopeTraversableMap
    : Traversable<FingerTreeRopeTraversableImpl> {
    using FingerTreeRopeTraversableImpl::traverse;
};

/** Registers FingerTreeRope as a Traversable. */
template <>
inline constexpr auto traversable_typeclass<smd::tree::FingerTreeRope> =
    FingerTreeRopeTraversableMap{};

} // namespace smd

#endif

```

## smd/tree/finger_tree_rope.t.cpp

```cpp
#include <smd/tree/finger_tree_rope.hpp>
#include <smd/tree/finger_tree_rope.hpp> // Re-inclusion check

#include <catch2/catch_test_macros.hpp>

#include <optional>
#include <string>

TEST_CASE("FingerTreeRopeTest - WrapperOperations") {
    using Rope = smd::tree::FingerTreeRope;

    // c8e3b5f9-2a7d-4c1e-b9f3-5a4d2b8c7e06
    auto rope = Rope::from_text("abCDxy", 2)
                    .insert(2, "--")
                    .erase(5, 2)
                    .replace(0, 2, "AB");

    CHECK(rope.to_string() == "AB--Cy");
    CHECK(rope.size_bytes() == 6U);
    // c8e3b5f9-2a7d-4c1e-b9f3-5a4d2b8c7e06 end
}

TEST_CASE("FingerTreeRopeTest - FoldableTypeclass") {
    using Rope = smd::tree::FingerTreeRope;

    auto rope = Rope::from_text("abcdefgh", 2);
    const auto &foldable = smd::foldable_typeclass<Rope>;

    CHECK(
        foldable.fold_map([](const std::string &chunk) { return chunk.size(); },
                          rope) == 8U);
    CHECK(foldable.length(rope) == 4U);
}

TEST_CASE("FingerTreeRopeTest - TraversableTypeclass") {
    using Rope = smd::tree::FingerTreeRope;

    auto rope = Rope::from_text("abcd", 2);

    auto success = smd::traverse(
        [](const std::string &chunk) -> std::optional<std::string> {
            return chunk + "!";
        },
        rope);
    REQUIRE(success.has_value());
    CHECK(success->to_string() == "ab!cd!");

    auto failure = smd::traverse(
        [](const std::string &chunk) -> std::optional<std::string> {
            if (chunk == "cd") {
                return std::nullopt;
            }
            return chunk;
        },
        rope);
    CHECK_FALSE(failure.has_value());
}

```

## smd/tree/finger_tree.t.cpp

```cpp
#include <smd/thunk/memoize.hpp>
#include <smd/tree/finger_tree.hpp>
#include <smd/tree/finger_tree.hpp> // Re-inclusion check
#include <smd/tree/finger_tree_foldable.hpp>
#include <smd/tree/finger_tree_traversable.hpp>

#include <catch2/catch_test_macros.hpp>

#include <atomic>
#include <cstddef>
#include <optional>
#include <vector>

namespace {

auto ceil_log2(std::size_t n) -> std::size_t {
    if (n <= 1U) {
        return 0U;
    }

    std::size_t value = 1U;
    std::size_t bits = 0U;
    while (value < n) {
        value <<= 1U;
        ++bits;
    }
    return bits;
}

struct Weighted {
    std::size_t d_total;

    friend bool operator==(const Weighted &, const Weighted &) = default;
    friend bool operator>=(const Weighted &lhs, const Weighted &rhs) {
        return lhs.d_total >= rhs.d_total;
    }
};

struct WeightedMeasure {
    auto operator()(int value) const -> Weighted {
        return Weighted{static_cast<std::size_t>(value * 10)};
    }
};

} // namespace

namespace smd::typeclass {

template <>
struct Monoid<Weighted> {
    constexpr auto identity() const -> Weighted { return Weighted{0U}; }

    constexpr auto combine(const Weighted &lhs, const Weighted &rhs) const
        -> Weighted {
        return Weighted{lhs.d_total + rhs.d_total};
    }
};

} // namespace smd::typeclass

TEST_CASE("FingerTreeTest - EmptyLeafAndPredicates") {
    using Tree = smd::tree::FingerTree<int>;

    auto empty = Tree::empty();
    CHECK(empty.is_empty());
    CHECK_FALSE(empty.is_leaf());
    CHECK_FALSE(empty.is_branch());
    CHECK(empty.measure() == 0U);
    CHECK(empty.breadth() == 0U);
    CHECK(empty.depth() == 0U);
    CHECK(empty.flatten() == (std::vector<int>{}));
    CHECK_FALSE(empty.view_l().has_value());
    CHECK_FALSE(empty.view_r().has_value());

    auto single = Tree::leaf(42);
    CHECK_FALSE(single.is_empty());
    CHECK(single.is_leaf());
    CHECK_FALSE(single.is_branch());
    CHECK(single.measure() == 1U);
    CHECK(single.value() == 42);
    CHECK(single.flatten() == (std::vector<int>{42}));
}

TEST_CASE("FingerTreeStrictnessTest - MemoizedThunkForcesOnce") {
    std::atomic<int> evaluations{0};
    auto delayed = smd::thunk::memoize([&evaluations]() {
        evaluations.fetch_add(1, std::memory_order_relaxed);
        return 42;
    });

    CHECK(delayed() == 42);
    CHECK(delayed() == 42);
    CHECK(evaluations.load(std::memory_order_relaxed) == 1);
}

TEST_CASE("FingerTreeStrictnessTest - MemoizedThunkSharesAcrossCopies") {
    std::atomic<int> evaluations{0};
    auto delayed = smd::thunk::memoize([&evaluations]() {
        evaluations.fetch_add(1, std::memory_order_relaxed);
        return 7;
    });
    auto alias = delayed;

    CHECK(delayed() == 7);
    CHECK(alias() == 7);
    CHECK(evaluations.load(std::memory_order_relaxed) == 1);
}

TEST_CASE("FingerTreeStrictnessTest - "
          "MeasuredThunkExposesCachedMeasureWithoutForce") {
    std::atomic<int> evaluations{0};
    auto delayed =
        smd::thunk::measured_memoize(std::size_t{99}, [&evaluations]() {
            evaluations.fetch_add(1, std::memory_order_relaxed);
            return 123;
        });

    CHECK(delayed.cached_measure() == 99U);
    CHECK(evaluations.load(std::memory_order_relaxed) == 0);
    CHECK(delayed.force() == 123);
    CHECK(delayed.cached_measure() == 99U);
    CHECK(evaluations.load(std::memory_order_relaxed) == 1);
}

TEST_CASE("FingerTreeTest - FromSequenceConsSnocAndMemberAppend") {
    using Tree = smd::tree::FingerTree<int>;

    auto from = Tree::from_sequence({1, 2, 3});
    CHECK(from.flatten() == (std::vector<int>{1, 2, 3}));

    auto with_cons = from.cons(0);
    CHECK(with_cons.flatten() == (std::vector<int>{0, 1, 2, 3}));

    auto with_snoc = with_cons.snoc(4);
    CHECK(with_snoc.flatten() == (std::vector<int>{0, 1, 2, 3, 4}));

    auto appended_member = from.append(Tree::from_sequence({4, 5}));
    CHECK(appended_member.flatten() == (std::vector<int>{1, 2, 3, 4, 5}));
}

TEST_CASE("FingerTreeTest - SingletonViewsAndEmptyTailInit") {
    using Tree = smd::tree::FingerTree<int>;

    auto single = Tree::leaf(7);
    auto left = single.view_l();
    REQUIRE(left.has_value());
    CHECK(left->d_value == 7);
    CHECK(left->d_rest.is_empty());

    auto right = single.view_r();
    REQUIRE(right.has_value());
    CHECK(right->d_value == 7);
    CHECK(right->d_rest.is_empty());

    auto empty = Tree::empty();
    CHECK(empty.tail().is_empty());
    CHECK(empty.init().is_empty());
}

TEST_CASE("FingerTreeTest - BasicMeasureDepthFlatten") {
    using Tree = smd::tree::FingerTree<int>;

    auto tree =
        Tree::branch(Tree::branch(Tree::leaf(1), Tree::leaf(2)), Tree::leaf(3));

    CHECK(tree.measure() == 3U);
    CHECK(tree.breadth() == 3U);
    CHECK(tree.depth() >= 1U);
    CHECK(tree.flatten() == (std::vector<int>{1, 2, 3}));
}

TEST_CASE("FingerTreeTest - ViewsAndListOps") {
    using Tree = smd::tree::FingerTree<int>;

    auto tree =
        Tree::branch(Tree::branch(Tree::leaf(1), Tree::leaf(2)), Tree::leaf(3));

    auto left_view = tree.view_l();
    REQUIRE(left_view.has_value());
    CHECK(left_view->d_value == 1);
    CHECK(left_view->d_rest.flatten() == (std::vector<int>{2, 3}));

    auto right_view = tree.view_r();
    REQUIRE(right_view.has_value());
    CHECK(right_view->d_value == 3);
    CHECK(right_view->d_rest.flatten() == (std::vector<int>{1, 2}));

    CHECK(tree.head() == 1);
    CHECK(tree.last() == 3);
    CHECK(tree.tail().flatten() == (std::vector<int>{2, 3}));
    CHECK(tree.init().flatten() == (std::vector<int>{1, 2}));
}

TEST_CASE("FingerTreeTest - PrependAppendConcat") {
    using Tree = smd::tree::FingerTree<int>;

    auto tree = Tree::branch(Tree::leaf(1), Tree::leaf(2));

    auto prepended = Tree::prepend(0, tree);
    CHECK(prepended.flatten() == (std::vector<int>{0, 1, 2}));

    auto appended = Tree::append(tree, 3);
    CHECK(appended.flatten() == (std::vector<int>{1, 2, 3}));

    auto concatenated = Tree::concat(tree, tree);
    CHECK(concatenated.flatten() == (std::vector<int>{1, 2, 1, 2}));
}

TEST_CASE("FingerTreeTest - MonoidTaggedMeasure") {
    using Tree = smd::tree::FingerTree<int, Weighted, WeightedMeasure>;

    auto tree = Tree::from_sequence({1, 2, 3});
    CHECK(tree.measure() == Weighted{60U});

    auto prepended = Tree::prepend(4, tree);
    CHECK(prepended.measure() == Weighted{100U});

    auto concatenated = Tree::concat(tree, Tree::leaf(5));
    CHECK(concatenated.measure() == Weighted{110U});
}

TEST_CASE("FingerTreeTest - MeasureGuidedSearchAndSplit") {
    using Tree = smd::tree::FingerTree<int>;

    auto tree = Tree::from_sequence({1, 2, 3, 4, 5});

    auto found = tree.search([](std::size_t prefix) { return prefix >= 3U; });
    REQUIRE(found.has_value());
    CHECK(*found == 3);

    auto split = tree.split([](std::size_t prefix) { return prefix >= 3U; });
    REQUIRE(split.has_value());
    CHECK(split->d_left.flatten() == (std::vector<int>{1, 2}));
    CHECK(split->d_pivot == 3);
    CHECK(split->d_right.flatten() == (std::vector<int>{4, 5}));

    CHECK_FALSE(tree.search([](std::size_t prefix) { return prefix >= 6U; })
                    .has_value());
    CHECK_FALSE(tree.split([](std::size_t prefix) { return prefix >= 6U; })
                    .has_value());
}

TEST_CASE("FingerTreeTest - MeasureGuidedSearchAndSplitWithCustomTag") {
    using Tree = smd::tree::FingerTree<int, Weighted, WeightedMeasure>;

    auto tree = Tree::from_sequence({1, 2, 3, 4});

    auto found =
        tree.search([](Weighted prefix) { return prefix.d_total >= 35U; });
    REQUIRE(found.has_value());
    CHECK(*found == 3);

    auto split =
        tree.split([](Weighted prefix) { return prefix.d_total >= 35U; });
    REQUIRE(split.has_value());
    CHECK(split->d_left.flatten() == (std::vector<int>{1, 2}));
    CHECK(split->d_left.measure() == Weighted{30U});
    CHECK(split->d_pivot == 3);
    CHECK(split->d_right.flatten() == (std::vector<int>{4}));
    CHECK(split->d_right.measure() == Weighted{40U});
}

TEST_CASE("FingerTreeTest - SplitAtCountBoundary") {
    using Tree = smd::tree::FingerTree<int>;

    auto tree = Tree::from_sequence({1, 2, 3, 4, 5});

    auto at_three =
        tree.split_at([](std::size_t prefix) { return prefix >= 3U; });
    CHECK(at_three.d_left.flatten() == (std::vector<int>{1, 2}));
    CHECK(at_three.d_right.flatten() == (std::vector<int>{3, 4, 5}));

    auto at_one =
        tree.split_at([](std::size_t prefix) { return prefix >= 1U; });
    CHECK(at_one.d_left.is_empty());
    CHECK(at_one.d_right.flatten() == (std::vector<int>{1, 2, 3, 4, 5}));

    auto none = tree.split_at([](std::size_t prefix) { return prefix >= 6U; });
    CHECK(none.d_left.flatten() == (std::vector<int>{1, 2, 3, 4, 5}));
    CHECK(none.d_right.is_empty());
}

TEST_CASE("FingerTreeTest - SplitAtWeightedBoundary") {
    using Tree = smd::tree::FingerTree<int, Weighted, WeightedMeasure>;

    auto tree = Tree::from_sequence({1, 2, 3, 4});

    auto split =
        tree.split_at([](Weighted prefix) { return prefix.d_total >= 35U; });
    CHECK(split.d_left.flatten() == (std::vector<int>{1, 2}));
    CHECK(split.d_left.measure() == Weighted{30U});
    CHECK(split.d_right.flatten() == (std::vector<int>{3, 4}));
    CHECK(split.d_right.measure() == Weighted{70U});
}

TEST_CASE("FingerTreeTest - SplitAtIndexConvenience") {
    using Tree = smd::tree::FingerTree<int>;

    auto tree = Tree::from_sequence({1, 2, 3, 4, 5});

    auto at_zero = tree.split_at_index(0U);
    CHECK(at_zero.d_left.is_empty());
    CHECK(at_zero.d_right.flatten() == (std::vector<int>{1, 2, 3, 4, 5}));

    auto at_three = tree.split_at_index(3U);
    CHECK(at_three.d_left.flatten() == (std::vector<int>{1, 2, 3}));
    CHECK(at_three.d_right.flatten() == (std::vector<int>{4, 5}));

    auto beyond = tree.split_at_index(99U);
    CHECK(beyond.d_left.flatten() == (std::vector<int>{1, 2, 3, 4, 5}));
    CHECK(beyond.d_right.is_empty());
}

TEST_CASE(
    "FingerTreeTest - SplitAtIndexUsesElementIndexForWeightedSizeTMeasure") {
    // WeightedSizeMeasure: Tag = std::size_t but NOT UnitMeasure.
    // split_at_index must still split by element position, not accumulated
    // weight.
    struct WeightedSizeMeasure {
        auto operator()(int value) const -> std::size_t {
            return static_cast<std::size_t>(value * 10);
        }
    };

    using WeightedTree =
        smd::tree::FingerTree<int, std::size_t, WeightedSizeMeasure>;

    auto tree = WeightedTree::from_sequence({1, 2, 3, 4});

    auto split = tree.split_at_index(2U);
    CHECK(split.d_left.flatten() == (std::vector<int>{1, 2}));
    CHECK(split.d_right.flatten() == (std::vector<int>{3, 4}));
}

TEST_CASE("FingerTreeTest - SplitAtMeasureConvenience") {
    using CountTree = smd::tree::FingerTree<int>;

    auto count_tree = CountTree::from_sequence({1, 2, 3, 4, 5});
    auto count_split = count_tree.split_at_measure(3U);
    CHECK(count_split.d_left.flatten() == (std::vector<int>{1, 2}));
    CHECK(count_split.d_right.flatten() == (std::vector<int>{3, 4, 5}));

    using WeightedTree = smd::tree::FingerTree<int, Weighted, WeightedMeasure>;

    auto weighted_tree = WeightedTree::from_sequence({1, 2, 3, 4});
    auto weighted_split = weighted_tree.split_at_measure(Weighted{35U});
    CHECK(weighted_split.d_left.flatten() == (std::vector<int>{1, 2}));
    CHECK(weighted_split.d_right.flatten() == (std::vector<int>{3, 4}));
}

TEST_CASE("FingerTreePersistenceTest - SharedVersionsSurviveAppendAndPops") {
    using Tree = smd::tree::FingerTree<int>;

    auto base = Tree::from_sequence({1, 2, 3, 4});
    auto appended = base.append(Tree::from_sequence({5, 6}));
    auto left_popped = appended.tail();
    auto right_popped = appended.init();

    CHECK(base.flatten() == (std::vector<int>{1, 2, 3, 4}));
    CHECK(base.head() == 1);
    CHECK(base.last() == 4);

    CHECK(appended.flatten() == (std::vector<int>{1, 2, 3, 4, 5, 6}));
    CHECK(left_popped.flatten() == (std::vector<int>{2, 3, 4, 5, 6}));
    CHECK(right_popped.flatten() == (std::vector<int>{1, 2, 3, 4, 5}));

    CHECK(base.flatten() == (std::vector<int>{1, 2, 3, 4}));
}

TEST_CASE("FingerTreePersistenceTest - SharedVersionsSurviveSearchAndSplit") {
    using Tree = smd::tree::FingerTree<int>;

    auto base = Tree::from_sequence({1, 2, 3, 4, 5, 6});
    auto appended = base.append(Tree::from_sequence({7, 8, 9}));
    auto split =
        appended.split([](std::size_t prefix) { return prefix >= 7U; });
    REQUIRE(split.has_value());

    auto count_split = appended.split_at_index(4U);
    auto found =
        appended.search([](std::size_t prefix) { return prefix >= 8U; });

    REQUIRE(found.has_value());
    CHECK(*found == 8);
    CHECK(split->d_left.flatten() == (std::vector<int>{1, 2, 3, 4, 5, 6}));
    CHECK(split->d_pivot == 7);
    CHECK(split->d_right.flatten() == (std::vector<int>{8, 9}));
    CHECK(count_split.d_left.flatten() == (std::vector<int>{1, 2, 3, 4}));
    CHECK(count_split.d_right.flatten() == (std::vector<int>{5, 6, 7, 8, 9}));

    CHECK(base.flatten() == (std::vector<int>{1, 2, 3, 4, 5, 6}));
    CHECK(base.search([](std::size_t prefix) { return prefix >= 4U; }) ==
          std::optional<int>{4});
}

TEST_CASE("FingerTreePersistenceTest - WeightedSharedVersionsKeepMeasures") {
    using Tree = smd::tree::FingerTree<int, Weighted, WeightedMeasure>;

    auto base = Tree::from_sequence({1, 2, 3});
    auto appended = base.append(Tree::from_sequence({4, 5}));
    auto split = appended.split_at_measure(Weighted{60U});

    CHECK(base.measure() == Weighted{60U});
    CHECK(base.flatten() == (std::vector<int>{1, 2, 3}));

    CHECK(appended.measure() == Weighted{150U});
    CHECK(appended.flatten() == (std::vector<int>{1, 2, 3, 4, 5}));
    CHECK(split.d_left.measure() == Weighted{30U});
    CHECK(split.d_right.measure() == Weighted{120U});

    CHECK(base.measure() == Weighted{60U});
    CHECK(base.flatten() == (std::vector<int>{1, 2, 3}));
}

TEST_CASE("FingerTreePersistenceTest - RepeatedSplitPopAcrossSharedVersions") {
    using Tree = smd::tree::FingerTree<int>;

    auto base = Tree::from_sequence({1, 2, 3, 4, 5, 6, 7, 8});
    auto shared = base.append(Tree::from_sequence({9, 10, 11, 12}));

    const auto base_snapshot = base.flatten();
    const auto shared_snapshot = shared.flatten();

    auto current = shared;
    for (int round = 0; round < 4; ++round) {
        const auto flat = current.flatten();
        REQUIRE(flat.size() >= 4U);

        auto split = current.split_at_index(flat.size() / 2U);
        auto left_flat = split.d_left.flatten();
        auto right_flat = split.d_right.flatten();

        REQUIRE_FALSE(left_flat.empty());
        REQUIRE_FALSE(right_flat.empty());

        auto left_tail = split.d_left.tail();
        auto right_init = split.d_right.init();
        auto recombined = left_tail.append(right_init);

        auto expected = flat;
        expected.erase(expected.begin());
        expected.pop_back();
        CHECK(recombined.flatten() == expected);

        auto rebuilt = split.d_left.append(split.d_right);
        CHECK(rebuilt.flatten() == flat);

        CHECK(base.flatten() == base_snapshot);
        CHECK(shared.flatten() == shared_snapshot);

        current = rebuilt.append(Tree::leaf(100 + round)).tail();
    }
}

TEST_CASE("FingerTreeTest - DepthRemainsLogarithmic") {
    using Tree = smd::tree::FingerTree<int>;
    constexpr std::size_t kSize = 1024U;

    auto by_snoc = Tree::empty();
    for (std::size_t i = 0; i < kSize; ++i) {
        by_snoc = by_snoc.snoc(static_cast<int>(i));
    }

    auto bound = 2U * ceil_log2(kSize + 1U) + 1U;
    CHECK(by_snoc.depth() <= bound);

    auto by_append = Tree::empty();
    for (std::size_t i = 0; i < kSize; ++i) {
        by_append = by_append.append(Tree::leaf(static_cast<int>(i)));
    }
    CHECK(by_append.depth() <= bound);
}

TEST_CASE("FingerTreeFoldableTest - FoldMapAndDerivedOperations") {
    using Tree = smd::tree::FingerTree<int>;

    auto tree = Tree::from_sequence({1, 2, 3, 4});
    const auto &foldable = smd::foldable_typeclass<Tree>;

    CHECK(foldable.length(tree) == 4U);
    CHECK(foldable.fold_map([](int x) { return x; }, tree) == 10);
    CHECK(foldable.to_vector(tree) == (std::vector<int>{1, 2, 3, 4}));

    const auto left = foldable.fold_left(
        tree, 0, [](int acc, int x) { return acc * 10 + x; });
    CHECK(left == 1234);
}

TEST_CASE("FingerTreeTraversableTest - TraverseOptionalSuccess") {
    using Tree = smd::tree::FingerTree<int>;

    auto tree = Tree::from_sequence({1, 2, 3});

    auto traversed = smd::traverse(
        [](int x) -> std::optional<int> {
            return x > 0 ? std::optional<int>{x * 10} : std::optional<int>{};
        },
        tree);

    REQUIRE(traversed.has_value());
    CHECK(traversed->flatten() == (std::vector<int>{10, 20, 30}));
}

TEST_CASE("FingerTreeTraversableTest - TraverseOptionalFailure") {
    using Tree = smd::tree::FingerTree<int>;

    auto tree = Tree::from_sequence({1, -2, 3});

    auto traversed = smd::traverse(
        [](int x) -> std::optional<int> {
            return x > 0 ? std::optional<int>{x * 10} : std::optional<int>{};
        },
        tree);

    CHECK_FALSE(traversed.has_value());
}

TEST_CASE("FingerTreeTest - FourDigitOverflowOnCons") {
    using Tree = smd::tree::FingerTree<int>;

    auto tree = Tree::from_sequence({1, 2, 3, 4, 5});
    auto t6 = tree.cons(0);
    CHECK(t6.flatten() == (std::vector<int>{0, 1, 2, 3, 4, 5}));
    CHECK(t6.breadth() == 6U);
    CHECK(t6.measure() == 6U);
    CHECK(t6.head() == 0);
    CHECK(t6.last() == 5);

    auto t7 = t6.cons(-1);
    CHECK(t7.flatten() == (std::vector<int>{-1, 0, 1, 2, 3, 4, 5}));
    CHECK(t7.breadth() == 7U);

    auto t8 = t7.cons(-2);
    CHECK(t8.flatten() == (std::vector<int>{-2, -1, 0, 1, 2, 3, 4, 5}));
    CHECK(t8.breadth() == 8U);

    auto t9 = t8.cons(-3);
    CHECK(t9.flatten() == (std::vector<int>{-3, -2, -1, 0, 1, 2, 3, 4, 5}));
    CHECK(t9.breadth() == 9U);
    CHECK(t9.depth() >= 2U);
}

TEST_CASE("FingerTreeTest - FourDigitOverflowOnSnoc") {
    using Tree = smd::tree::FingerTree<int>;

    auto tree = Tree::from_sequence({1, 2, 3, 4, 5});
    auto t6 = tree.snoc(6);
    auto t7 = t6.snoc(7);
    auto t8 = t7.snoc(8);
    auto t9 = t8.snoc(9);
    auto t10 = t9.snoc(10);

    CHECK(t10.flatten() == (std::vector<int>{1, 2, 3, 4, 5, 6, 7, 8, 9, 10}));
    CHECK(t10.breadth() == 10U);
    CHECK(t10.measure() == 10U);
    CHECK(t10.head() == 1);
    CHECK(t10.last() == 10);
}

TEST_CASE("FingerTreeTest - NodeMeasureCachingWithWeightedSplit") {
    using Tree = smd::tree::FingerTree<int, Weighted, WeightedMeasure>;

    auto tree = Tree::empty();
    for (int i = 1; i <= 20; ++i) {
        tree = tree.snoc(i);
    }

    CHECK(tree.measure() == Weighted{2100U});
    CHECK(tree.breadth() == 20U);

    auto found = tree.search([](Weighted p) { return p.d_total >= 550U; });
    REQUIRE(found.has_value());
    CHECK(*found == 10);

    auto split = tree.split([](Weighted p) { return p.d_total >= 550U; });
    REQUIRE(split.has_value());
    CHECK(split->d_left.measure() == Weighted{450U});
    CHECK(split->d_pivot == 10);
    CHECK(split->d_right.measure() == Weighted{1550U});
    CHECK(split->d_left.breadth() + 1U + split->d_right.breadth() == 20U);
}

TEST_CASE("FingerTreeTest - NodesFromPackingViaConcat") {
    using Tree = smd::tree::FingerTree<int>;

    auto left = Tree::from_sequence({1, 2, 3, 4});
    auto right = Tree::from_sequence({5, 6, 7, 8});
    auto cat = Tree::concat(left, right);
    CHECK(cat.flatten() == (std::vector<int>{1, 2, 3, 4, 5, 6, 7, 8}));
    CHECK(cat.breadth() == 8U);
    CHECK(cat.measure() == 8U);

    auto cat3 = Tree::concat(cat, Tree::from_sequence({9, 10, 11}));
    CHECK(cat3.flatten() ==
          (std::vector<int>{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}));
    CHECK(cat3.breadth() == 11U);

    auto split = cat3.split([](std::size_t p) { return p >= 6U; });
    REQUIRE(split.has_value());
    CHECK(split->d_left.flatten() == (std::vector<int>{1, 2, 3, 4, 5}));
    CHECK(split->d_pivot == 6);
    CHECK(split->d_right.flatten() == (std::vector<int>{7, 8, 9, 10, 11}));
}

TEST_CASE("FingerTreeTest - SpineBorrowingViewL") {
    using Tree = smd::tree::FingerTree<int>;

    auto tree = Tree::from_sequence({1, 2, 3, 4, 5, 6, 7, 8, 9, 10});
    auto current = tree;
    std::vector<int> collected;
    while (!current.is_empty()) {
        auto vl = current.view_l();
        REQUIRE(vl.has_value());
        collected.push_back(vl->d_value);
        current = std::move(vl->d_rest);
    }
    CHECK(collected == (std::vector<int>{1, 2, 3, 4, 5, 6, 7, 8, 9, 10}));
}

TEST_CASE("FingerTreeTest - SpineBorrowingViewR") {
    using Tree = smd::tree::FingerTree<int>;

    auto tree = Tree::from_sequence({1, 2, 3, 4, 5, 6, 7, 8, 9, 10});
    auto current = tree;
    std::vector<int> collected;
    while (!current.is_empty()) {
        auto vr = current.view_r();
        REQUIRE(vr.has_value());
        collected.push_back(vr->d_value);
        current = std::move(vr->d_rest);
    }
    CHECK(collected == (std::vector<int>{10, 9, 8, 7, 6, 5, 4, 3, 2, 1}));
}

TEST_CASE("FingerTreeTest - LargeTreeSplitAndConcat") {
    using Tree = smd::tree::FingerTree<int>;
    constexpr std::size_t kN = 256U;

    auto tree = Tree::empty();
    for (std::size_t i = 0; i < kN; ++i) {
        tree = tree.snoc(static_cast<int>(i));
    }
    CHECK(tree.breadth() == kN);
    CHECK(tree.measure() == kN);
    CHECK(tree.head() == 0);
    CHECK(tree.last() == 255);

    auto mid = tree.split_at_index(kN / 2U);
    CHECK(mid.d_left.breadth() == kN / 2U);
    CHECK(mid.d_right.breadth() == kN / 2U);
    auto rebuilt = Tree::concat(mid.d_left, mid.d_right);
    CHECK(rebuilt.flatten() == tree.flatten());

    auto other = Tree::empty();
    for (std::size_t i = kN; i < 2U * kN; ++i) {
        other = other.snoc(static_cast<int>(i));
    }
    auto big = Tree::concat(tree, other);
    CHECK(big.breadth() == 2U * kN);
    CHECK(big.head() == 0);
    CHECK(big.last() == 511);

    auto found = big.search([](std::size_t p) { return p >= 300U; });
    REQUIRE(found.has_value());
    CHECK(*found == 299);
}

TEST_CASE("FingerTreeTest - ConcatEdgeCases") {
    using Tree = smd::tree::FingerTree<int>;

    auto empty = Tree::empty();
    auto single = Tree::leaf(42);
    auto multi = Tree::from_sequence({1, 2, 3});

    CHECK(Tree::concat(empty, empty).is_empty());
    CHECK(Tree::concat(empty, single).flatten() == (std::vector<int>{42}));
    CHECK(Tree::concat(single, empty).flatten() == (std::vector<int>{42}));
    CHECK(Tree::concat(single, single).flatten() == (std::vector<int>{42, 42}));
    CHECK(Tree::concat(single, multi).flatten() ==
          (std::vector<int>{42, 1, 2, 3}));
    CHECK(Tree::concat(multi, single).flatten() ==
          (std::vector<int>{1, 2, 3, 42}));
    CHECK(Tree::concat(multi, multi).flatten() ==
          (std::vector<int>{1, 2, 3, 1, 2, 3}));
}

TEST_CASE("FingerTreeTest - RepeatedTailDrainsTree") {
    using Tree = smd::tree::FingerTree<int>;

    auto tree = Tree::from_sequence({1, 2, 3, 4, 5, 6, 7, 8});
    auto expected = tree.flatten();

    std::vector<int> collected;
    auto current = tree;
    while (!current.is_empty()) {
        collected.push_back(current.head());
        current = current.tail();
    }
    CHECK(collected == expected);
    CHECK(current.is_empty());
    CHECK(current.breadth() == 0U);
    CHECK(current.tail().is_empty());
}

```

## smd/tree/finger_tree_traversable.hpp

```cpp
// src/smd/tree/finger_tree_traversable.hpp                           -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_FINGER_TREE_TRAVERSABLE
#define INCLUDED_SMD_TREE_FINGER_TREE_TRAVERSABLE

// Traversable constraint for FingerTree core:
// - Materialization: traverse materializes the tree via flatten() into a
// vector.
// - Preservation: monoid measure semantics are preserved through the traversal.
// - Reconstruction: results are rebuilt via FingerTree<U>::from_sequence() with
// same measure policy.
// - Applicative semantics: all traversals follow left-to-right order
// independent of tree structure.
//
// Rationale: FingerTree provides efficient structural operations (cons, snoc,
// split); traversal is not a primary performance path, so O(n) materialization
// is acceptable. Wrapper types override with specialized traversal that
// preserves wrapper invariants.

#include <smd/tree/finger_tree.hpp>
#include <smd/typeclass/applicative.hpp>
#include <smd/typeclass/traversable.hpp>

#include <functional>
#include <type_traits>
#include <utility>
#include <vector>

namespace smd {

/** Traversable typeclass implementation for FingerTree; materialises via
 * flatten() then reconstructs with from_sequence(); O(n).
 */
template <class T, class TAG_TYPE, class MEASURE_POLICY>
struct FingerTreeTraversableImpl {
    using element_type = T;

    template <class APPLICATIVE, class F>
    auto
    traverse(this auto &&, const APPLICATIVE &applicative, F &&function,
             const smd::tree::FingerTree<T, TAG_TYPE, MEASURE_POLICY> &tree) {
        using Context = remove_cvref_t<std::invoke_result_t<F, const T &>>;
        using U = smd::applicative_value_t<Context>;

        auto accumulated = applicative.pure(std::vector<U>{});

        for (const auto &value : tree.flatten()) {
            auto lifted = std::invoke(function, value);
            accumulated = applicative.invoke(
                [](std::vector<U> values, U element) {
                    values.push_back(std::move(element));
                    return values;
                },
                std::move(accumulated), std::move(lifted));
        }

        return applicative.invoke(
            [](std::vector<U> values) {
                return smd::tree::FingerTree<U>::from_sequence(
                    std::move(values));
            },
            std::move(accumulated));
    }
};

/** Traversable typeclass map entry for FingerTree. */
template <class T, class TAG_TYPE, class MEASURE_POLICY>
struct FingerTreeTraversableMap
    : Traversable<FingerTreeTraversableImpl<T, TAG_TYPE, MEASURE_POLICY>> {
    using FingerTreeTraversableImpl<T, TAG_TYPE, MEASURE_POLICY>::traverse;
};

/** Registers FingerTree as a Traversable for all tag and measure combinations. */
template <class T, class TAG_TYPE, class MEASURE_POLICY>
inline constexpr auto
    traversable_typeclass<smd::tree::FingerTree<T, TAG_TYPE, MEASURE_POLICY>> =
        FingerTreeTraversableMap<T, TAG_TYPE, MEASURE_POLICY>{};

} // namespace smd

#endif

```

## smd/tree/finger_tree_traversable.t.cpp

```cpp
// src/smd/tree/finger_tree_traversable.t.cpp                         -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <smd/tree/finger_tree_traversable.hpp>
#include <smd/tree/finger_tree_traversable.hpp> // Re-inclusion check

#include <smd/typeclass/traversable.hpp>

#include <catch2/catch_test_macros.hpp>

#include <optional>
#include <vector>

TEST_CASE("FingerTreeTraversableTest - TraverseOptionalSucceeds") {
    using Tree = smd::tree::FingerTree<int>;
    auto t = Tree::from_sequence({1, 2, 3});
    auto result = smd::traverse([](int x) { return std::optional{x * 10}; }, t);
    REQUIRE(result.has_value());
    CHECK(result->flatten() == (std::vector<int>{10, 20, 30}));
}

TEST_CASE("FingerTreeTraversableTest - TraverseOptionalFailsOnNullopt") {
    using Tree = smd::tree::FingerTree<int>;
    auto t = Tree::from_sequence({1, 2, 3});
    auto result = smd::traverse(
        [](int x) -> std::optional<int> {
            return x == 2 ? std::nullopt : std::optional{x};
        },
        t);
    CHECK_FALSE(result.has_value());
}

TEST_CASE("FingerTreeTraversableTest - TraversePreservesShape") {
    using Tree = smd::tree::FingerTree<int>;
    auto t = Tree::from_sequence({10, 20, 30, 40});
    auto result = smd::traverse([](int x) { return std::optional{x + 1}; }, t);
    REQUIRE(result.has_value());
    CHECK(result->flatten() == (std::vector<int>{11, 21, 31, 41}));
}

```

## smd/tree/finger_tree_wrappers.hpp

```cpp
// src/smd/tree/finger_tree_wrappers.hpp                              -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_FINGER_TREE_WRAPPERS
#define INCLUDED_SMD_TREE_FINGER_TREE_WRAPPERS

/** Convenience header that includes all four finger tree wrapper types:
 * - FingerTreeIntervalIndex  — interval stabbing/overlap queries
 * - FingerTreePriorityQueue  — persistent double-ended priority queue
 * - FingerTreeRandomAccess   — persistent random-access sequence
 * - FingerTreeRope            — persistent text rope
 */

#include <smd/tree/finger_tree_interval_index.hpp>
#include <smd/tree/finger_tree_priority_queue.hpp>
#include <smd/tree/finger_tree_random_access.hpp>
#include <smd/tree/finger_tree_rope.hpp>

#endif

```

## smd/tree/finger_tree_wrappers.t.cpp

```cpp
// src/smd/tree/finger_tree_wrappers.t.cpp                            -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <smd/tree/finger_tree_wrappers.hpp>
#include <smd/tree/finger_tree_wrappers.hpp> // Re-inclusion check

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <vector>

TEST_CASE("FingerTreeWrappersTest - RandomAccessBasicOps") {
    auto ra = smd::tree::FingerTreeRandomAccess<int>{};
    ra = ra.push_back(1).push_back(2).push_back(3);
    CHECK(ra.size() == 3);
    CHECK(ra.at(0) == std::optional{1});
    CHECK(ra.at(2) == std::optional{3});
    CHECK(ra.to_vector() == (std::vector<int>{1, 2, 3}));
}

TEST_CASE("FingerTreeWrappersTest - PriorityQueueMinMax") {
    auto pq = smd::tree::FingerTreePriorityQueue<int>{};
    pq = pq.push(3).push(1).push(2);
    CHECK(pq.size() == 3);
    CHECK(pq.min() == 1);
    CHECK(pq.max() == 3);
}

TEST_CASE("FingerTreeWrappersTest - RopeConcat") {
    auto r = smd::tree::FingerTreeRope::from_chunks({"hello", " ", "world"});
    CHECK(r.to_string() == "hello world");
    CHECK(r.size_bytes() == 11);
}

TEST_CASE("FingerTreeWrappersTest - IntervalIndexQuery") {
    using Idx = smd::tree::FingerTreeIntervalIndex<int>;
    using Entry = smd::tree::Interval<int>;
    auto idx = Idx{};
    idx = idx.insert(Entry{0, 10, 42}).insert(Entry{5, 15, 99});
    auto hits = idx.query_point(7);
    CHECK(hits.size() == 2);
}

```

## smd/tree/fixpoint_tree_foldable.hpp

```cpp
// src/smd/tree/fixpoint_tree_foldable.hpp                             -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_FIXPOINT_TREE_FOLDABLE
#define INCLUDED_SMD_TREE_FIXPOINT_TREE_FOLDABLE

#include <smd/tree/fixpoint_tree.hpp>
#include <smd/typeclass/foldable.hpp>

#include <functional>
#include <type_traits>
#include <variant>

namespace smd {

/** Foldable typeclass instance for Fix<ExprF> (the fixpoint expression tree).
 * fold_map applies @p f to each ExprConst leaf value and combines results
 * using the Monoid for the return type. Add and Mul nodes combine their
 * children's results left-then-right.
 */
struct FixpointTreeFoldableImpl {

    // b2c8e4f1-3a6d-4f1b-9e7c-5d2b8a4f3c91
    template <class F>
    auto fold_map(this auto &&self, F &&f,
                  const smd::fixpoint::Fix<smd::tree::ExprF> &t) {
        using smd::fixpoint::unwrap;
        using smd::tree::ExprAdd;
        using smd::tree::ExprConst;
        using smd::tree::ExprMul;
        using Expr = smd::tree::Expr;

        const auto &layer = unwrap(t);

        if (std::holds_alternative<ExprConst<Expr>>(layer)) {
            return std::invoke(f, std::get<ExprConst<Expr>>(layer).value);
        }

        const auto fold_children = [&](const auto &left, const auto &right) {
            auto lhs = self.fold_map(f, *left);
            auto rhs = self.fold_map(f, *right);
            using Result = std::remove_cvref_t<decltype(lhs)>;
            return smd::typeclass::monoid_v<Result>.combine(lhs, rhs);
        };

        if (std::holds_alternative<ExprAdd<Expr>>(layer)) {
            const auto &a = std::get<ExprAdd<Expr>>(layer);
            return fold_children(a.left, a.right);
        }

        const auto &m = std::get<ExprMul<Expr>>(layer);
        return fold_children(m.left, m.right);
    }
    // b2c8e4f1-3a6d-4f1b-9e7c-5d2b8a4f3c91 end
};

/** Foldable map that exposes fold_map for Fix<ExprF>. */
struct FixpointTreeFoldableMap : Foldable<FixpointTreeFoldableImpl> {
    using FixpointTreeFoldableImpl::fold_map;
};

// c4d9f2a7-6b1e-4c3f-8a5d-2e7b9c1f4a83
/** Registers FixpointTreeFoldableMap as the Foldable instance for Fix<ExprF>. */
template <>
inline constexpr auto foldable_typeclass<smd::fixpoint::Fix<smd::tree::ExprF>> =
    FixpointTreeFoldableMap{};
// c4d9f2a7-6b1e-4c3f-8a5d-2e7b9c1f4a83 end

} // namespace smd

#endif

```

## smd/tree/fixpoint_tree_foldable.t.cpp

```cpp
// src/smd/tree/fixpoint_tree_foldable.t.cpp                           -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <smd/tree/fixpoint_tree_foldable.hpp>
#include <smd/tree/fixpoint_tree_foldable.hpp> // Re-inclusion check

#include <catch2/catch_test_macros.hpp>

#include <vector>

using smd::tree::add_expr;
using smd::tree::const_expr;
using smd::tree::eval;
using smd::tree::Expr;
using smd::tree::mul_expr;

namespace {

template <const auto &FOLDABLE = smd::foldable_typeclass<Expr>>
auto sum_with_nttp_lookup(const Expr &tree) {
    return FOLDABLE.fold_left(tree, 0.0,
                              [](double acc, double x) { return acc + x; });
}

} // namespace

TEST_CASE("FixpointTreeFoldableTest - Length") {
    auto tree = add_expr(const_expr(1.0), const_expr(2.0));
    const auto &foldable = smd::foldable_typeclass<Expr>;
    CHECK(foldable.length(tree) == 2U);
}

TEST_CASE("FixpointTreeFoldableTest - LengthComplex") {
    // (1 + 2) * (3 + 4) has 4 constants
    auto tree = mul_expr(add_expr(const_expr(1.0), const_expr(2.0)),
                         add_expr(const_expr(3.0), const_expr(4.0)));
    const auto &foldable = smd::foldable_typeclass<Expr>;
    CHECK(foldable.length(tree) == 4U);
}

TEST_CASE("FixpointTreeFoldableTest - FoldMapSum") {
    auto tree =
        mul_expr(add_expr(const_expr(1.0), const_expr(2.0)), const_expr(4.0));
    const auto &foldable = smd::foldable_typeclass<Expr>;
    auto sum =
        foldable.fold_map([](double x) { return static_cast<int>(x); }, tree);
    CHECK(sum == 7);
}

TEST_CASE("FixpointTreeFoldableTest - FoldLeft") {
    // 1 + 2: fold_left accumulates left-to-right
    auto tree = add_expr(const_expr(1.0), const_expr(2.0));
    const auto &foldable = smd::foldable_typeclass<Expr>;
    auto result = foldable.fold_left(
        tree, 0.0, [](double acc, double x) { return acc * 10.0 + x; });
    CHECK(result == 12.0);
}

TEST_CASE("FixpointTreeFoldableTest - FoldRight") {
    auto tree = add_expr(const_expr(1.0), const_expr(2.0));
    const auto &foldable = smd::foldable_typeclass<Expr>;
    auto result = foldable.fold_right(
        tree, 0.0, [](double x, double acc) { return x * 10.0 + acc; });
    CHECK(result == 30.0);
}

TEST_CASE("FixpointTreeFoldableTest - FoldLeftComplex") {
    // (1 + 2) * 3: constants are 1, 2, 3
    auto tree =
        mul_expr(add_expr(const_expr(1.0), const_expr(2.0)), const_expr(3.0));
    const auto &foldable = smd::foldable_typeclass<Expr>;
    auto sum = foldable.fold_left(tree, 0.0,
                                  [](double acc, double x) { return acc + x; });
    CHECK(sum == 6.0);
    CHECK(eval(tree) == 9.0);
}

TEST_CASE("FixpointTreeFoldableTest - ToVector") {
    auto tree =
        mul_expr(add_expr(const_expr(1.0), const_expr(2.0)), const_expr(3.0));
    const auto &foldable = smd::foldable_typeclass<Expr>;
    auto values = foldable.to_vector(tree);
    CHECK(values == std::vector<double>{1.0, 2.0, 3.0});
}

TEST_CASE("FixpointTreeFoldableTest - PredicatesAndFind") {
    auto tree =
        mul_expr(add_expr(const_expr(1.0), const_expr(5.0)), const_expr(3.0));
    const auto &foldable = smd::foldable_typeclass<Expr>;

    CHECK(foldable.any_of(tree, [](double x) { return x == 5.0; }));
    CHECK_FALSE(foldable.any_of(tree, [](double x) { return x == 99.0; }));
    CHECK(foldable.all_of(tree, [](double x) { return x > 0.0; }));
    CHECK_FALSE(foldable.all_of(tree, [](double x) { return x > 2.0; }));
    CHECK_FALSE(foldable.empty(tree));

    auto found = foldable.find_first(tree, [](double x) { return x > 2.0; });
    REQUIRE(found.has_value());
    CHECK(*found == 5.0);
}

TEST_CASE("FixpointTreeFoldableTest - NttpLookup") {
    auto tree = add_expr(const_expr(10.0), const_expr(20.0));
    CHECK(sum_with_nttp_lookup(tree) == 30.0);
}

TEST_CASE("FixpointTreeFoldableTest - SingleConst") {
    auto tree = const_expr(42.0);
    const auto &foldable = smd::foldable_typeclass<Expr>;
    CHECK(foldable.length(tree) == 1U);

    auto values = foldable.to_vector(tree);
    CHECK(values == std::vector<double>{42.0});
}

TEST_CASE("FixpointTreeFoldableTest - FoldSumMatchesEval") {
    // For a pure-addition tree, fold_left with + equals eval
    auto tree = add_expr(add_expr(const_expr(1.0), const_expr(2.0)),
                         add_expr(const_expr(3.0), const_expr(4.0)));
    const auto &foldable = smd::foldable_typeclass<Expr>;
    auto sum = foldable.fold_left(tree, 0.0,
                                  [](double acc, double x) { return acc + x; });
    CHECK(sum == eval(tree));
}

```

## smd/tree/fixpoint_tree.hpp

```cpp
// src/smd/tree/fixpoint_tree.hpp                                      -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_FIXPOINT_TREE
#define INCLUDED_SMD_TREE_FIXPOINT_TREE

#include <smd/fixpoint/box.hpp>
#include <smd/fixpoint/cata.hpp>
#include <smd/fixpoint/fix.hpp>
#include <smd/fixpoint/overloaded.hpp>

#include <functional>
#include <utility>
#include <variant>

namespace smd::tree {

using smd::fixpoint::Box;
using smd::fixpoint::make_box;

/** Constant-leaf functor for the expression tree; carries a double literal.
 * @tparam A recursive position placeholder (not yet fixed)
 */
template <typename A>
struct ExprConst {
    double value;
};

/** Addition node functor; holds boxed left and right sub-expressions.
 * @tparam A recursive position placeholder
 */
template <typename A>
struct ExprAdd {
    Box<A> left;
    Box<A> right;
};

/** Multiplication node functor; holds boxed left and right sub-expressions.
 * @tparam A recursive position placeholder
 */
template <typename A>
struct ExprMul {
    Box<A> left;
    Box<A> right;
};

/** Non-recursive expression functor: variant of Const, Add, Mul over @p A.
 * Expr = Fix<ExprF> gives the recursive fixed-point tree type.
 */
template <typename A>
using ExprF = std::variant<ExprConst<A>, ExprAdd<A>, ExprMul<A>>;

/**
 * @brief Lift a function over one layer of ExprF (the fmap for ExprF).
 * @param f function to apply at each recursive position
 * @param expr the ExprF layer to map over
 * @return ExprF<B> with f applied to every A inside the layer
 */
template <typename F, typename A>
auto fmap_expr(F &&f, const ExprF<A> &expr) {
    using B = std::invoke_result_t<F, const A &>;
    return std::visit(
        smd::fixpoint::overloaded{
            [](const ExprConst<A> &c) -> ExprF<B> {
                return ExprConst<B>{c.value};
            },
            [&f](const ExprAdd<A> &a) -> ExprF<B> {
                return ExprAdd<B>{make_box<B>(std::invoke(f, *a.left)),
                                  make_box<B>(std::invoke(f, *a.right))};
            },
            [&f](const ExprMul<A> &m) -> ExprF<B> {
                return ExprMul<B>{make_box<B>(std::invoke(f, *m.left)),
                                  make_box<B>(std::invoke(f, *m.right))};
            },
        },
        expr);
}

/** Callable wrapper around fmap_expr for use as an fmap_fn argument to cata. */
inline constexpr auto fmap_expr_fn = [](auto &&f, const auto &expr) {
    return fmap_expr(std::forward<decltype(f)>(f), expr);
};

/** The fixed-point expression tree type: Fix<ExprF>. */
using Expr = smd::fixpoint::Fix<ExprF>;

/** Build a constant-leaf expression node. */
inline auto const_expr(double value) -> Expr {
    return smd::fixpoint::wrap<ExprF>(ExprF<Expr>{ExprConst<Expr>{value}});
}

/** Build an addition node from two sub-expressions. */
inline auto add_expr(Expr left, Expr right) -> Expr {
    return smd::fixpoint::wrap<ExprF>(ExprF<Expr>{ExprAdd<Expr>{
        make_box<Expr>(std::move(left)), make_box<Expr>(std::move(right))}});
}

/** Build a multiplication node from two sub-expressions. */
inline auto mul_expr(Expr left, Expr right) -> Expr {
    return smd::fixpoint::wrap<ExprF>(ExprF<Expr>{ExprMul<Expr>{
        make_box<Expr>(std::move(left)), make_box<Expr>(std::move(right))}});
}

// e3a7f1c2-9b4d-4e2a-8f6c-1d5b3a9e7c04
/** Catamorphism algebra that reduces ExprF<double> to a double.
 * Used by eval() to perform numeric evaluation in a single bottom-up pass.
 */
inline auto eval_algebra(const ExprF<double> &expr) -> double {
    return std::visit(
        smd::fixpoint::overloaded{
            [](const ExprConst<double> &c) { return c.value; },
            [](const ExprAdd<double> &a) { return *a.left + *a.right; },
            [](const ExprMul<double> &m) { return *m.left * *m.right; },
        },
        expr);
}

/** Evaluate an expression tree to a double via catamorphism. */
inline auto eval(const Expr &expr) -> double {
    return smd::fixpoint::cata<double>(eval_algebra, fmap_expr_fn, expr);
}
// e3a7f1c2-9b4d-4e2a-8f6c-1d5b3a9e7c04 end

} // namespace smd::tree

#endif

```

## smd/tree/fixpoint_tree.t.cpp

```cpp
// src/smd/tree/fixpoint_tree.t.cpp                                    -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <smd/tree/fixpoint_tree.hpp>
#include <smd/tree/fixpoint_tree.hpp> // Re-inclusion check

#include <smd/fixpoint/box.hpp>
#include <smd/fixpoint/cata.hpp>
#include <smd/fixpoint/overloaded.hpp>

#include <catch2/catch_test_macros.hpp>

#include <iomanip>
#include <sstream>
#include <string>

using smd::tree::add_expr;
using smd::tree::const_expr;
using smd::tree::eval;
using smd::tree::eval_algebra;
using smd::tree::Expr;
using smd::tree::ExprAdd;
using smd::tree::ExprConst;
using smd::tree::ExprF;
using smd::tree::ExprMul;
using smd::tree::fmap_expr;
using smd::tree::fmap_expr_fn;
using smd::tree::mul_expr;

TEST_CASE("FixpointTree - ConstExprConstruction") {
    auto c = const_expr(42.0);
    const auto &layer = smd::fixpoint::unwrap(c);
    CHECK(std::holds_alternative<ExprConst<Expr>>(layer));
}

TEST_CASE("FixpointTree - AddExprConstruction") {
    auto e = add_expr(const_expr(1.0), const_expr(2.0));
    const auto &layer = smd::fixpoint::unwrap(e);
    CHECK(std::holds_alternative<ExprAdd<Expr>>(layer));
}

TEST_CASE("FixpointTree - MulExprConstruction") {
    auto e = mul_expr(const_expr(3.0), const_expr(4.0));
    const auto &layer = smd::fixpoint::unwrap(e);
    CHECK(std::holds_alternative<ExprMul<Expr>>(layer));
}

TEST_CASE("FixpointTree - EvalConst") { CHECK(eval(const_expr(42.0)) == 42.0); }

TEST_CASE("FixpointTree - EvalAdd") {
    CHECK(eval(add_expr(const_expr(1.0), const_expr(2.0))) == 3.0);
}

TEST_CASE("FixpointTree - EvalMul") {
    CHECK(eval(mul_expr(const_expr(3.0), const_expr(4.0))) == 12.0);
}

TEST_CASE("FixpointTree - EvalComplex") {
    // (1 + 2) * 4 == 12
    auto e =
        mul_expr(add_expr(const_expr(1.0), const_expr(2.0)), const_expr(4.0));
    CHECK(eval(e) == 12.0);
}

TEST_CASE("FixpointTree - EvalNested") {
    // (1 + 2) * (3 + 4) == 21
    auto e = mul_expr(add_expr(const_expr(1.0), const_expr(2.0)),
                      add_expr(const_expr(3.0), const_expr(4.0)));
    CHECK(eval(e) == 21.0);
}

TEST_CASE("FixpointTree - EvalDeeplyNested") {
    // ((1 + 2) + (3 + 4)) * 2 == 20
    auto left = add_expr(add_expr(const_expr(1.0), const_expr(2.0)),
                         add_expr(const_expr(3.0), const_expr(4.0)));
    auto e = mul_expr(left, const_expr(2.0));
    CHECK(eval(e) == 20.0);
}

TEST_CASE("FixpointTree - CustomPrettyPrintAlgebra") {
    using smd::fixpoint::cata;
    using smd::fixpoint::overloaded;

    auto format_constant = [](double value) -> std::string {
        std::ostringstream stream;
        stream << std::fixed << std::setprecision(1) << value;
        return stream.str();
    };

    auto print_algebra =
        [format_constant](const ExprF<std::string> &expr) -> std::string {
        return std::visit(
            overloaded{
                [format_constant](const ExprConst<std::string> &c) {
                    return format_constant(c.value);
                },
                [](const ExprAdd<std::string> &a) {
                    return "(" + *a.left + " + " + *a.right + ")";
                },
                [](const ExprMul<std::string> &m) {
                    return "(" + *m.left + " * " + *m.right + ")";
                },
            },
            expr);
    };

    auto e =
        mul_expr(add_expr(const_expr(1.0), const_expr(2.0)), const_expr(4.0));
    auto result = cata<std::string>(print_algebra, fmap_expr_fn, e);
    CHECK(result == "((1.0 + 2.0) * 4.0)");
}

TEST_CASE("FixpointTree - FmapExprDirect") {
    using smd::fixpoint::make_box;

    ExprF<int> add_layer = ExprAdd<int>{make_box<int>(10), make_box<int>(20)};
    auto doubled = fmap_expr([](int x) { return x * 2; }, add_layer);

    REQUIRE(std::holds_alternative<ExprAdd<int>>(doubled));
    const auto &a = std::get<ExprAdd<int>>(doubled);
    CHECK(*a.left == 20);
    CHECK(*a.right == 40);
}

TEST_CASE("FixpointTree - FmapExprConst") {
    ExprF<int> const_layer = ExprConst<int>{3.14};
    auto mapped =
        fmap_expr([](int x) { return std::to_string(x); }, const_layer);

    REQUIRE(std::holds_alternative<ExprConst<std::string>>(mapped));
    CHECK(std::get<ExprConst<std::string>>(mapped).value == 3.14);
}

TEST_CASE("FixpointTree - EvalAlgebraDirect") {
    using smd::fixpoint::make_box;

    ExprF<double> layer =
        ExprAdd<double>{make_box<double>(10.0), make_box<double>(20.0)};
    CHECK(eval_algebra(layer) == 30.0);
}

```

## smd/tree/fixpoint_tree_traversable.hpp

```cpp
// src/smd/tree/fixpoint_tree_traversable.hpp                          -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_FIXPOINT_TREE_TRAVERSABLE
#define INCLUDED_SMD_TREE_FIXPOINT_TREE_TRAVERSABLE

#include <smd/tree/fixpoint_tree.hpp>
#include <smd/typeclass/traversable.hpp>

#include <functional>
#include <utility>
#include <variant>

namespace smd {

/** Traversable typeclass instance for Fix<ExprF> (the fixpoint expression tree).
 * Treats ExprConst leaf doubles as the traversal element type. traverse maps
 * each constant value through the applicative and rebuilds an Expr in that
 * context; Add and Mul nodes combine their traversed children with the
 * appropriate builder (add_expr / mul_expr) inside the applicative.
 */
struct FixpointTreeTraversableImpl {
    using element_type = double;

    template <class APPLICATIVE, class F>
    auto traverse(this auto &&self, const APPLICATIVE &applicative, F &&f,
                  const smd::fixpoint::Fix<smd::tree::ExprF> &t) {
        using smd::fixpoint::unwrap;
        using smd::tree::ExprAdd;
        using smd::tree::ExprConst;
        using smd::tree::ExprMul;
        using Expr = smd::tree::Expr;

        const auto &layer = unwrap(t);

        if (std::holds_alternative<ExprConst<Expr>>(layer)) {
            return applicative.invoke(
                [](double value) { return smd::tree::const_expr(value); },
                std::invoke(std::forward<F>(f),
                            std::get<ExprConst<Expr>>(layer).value));
        }

        const auto traverse_pair = [&](const auto &left, const auto &right,
                                       auto builder) {
            auto l = self.traverse(applicative, f, *left);
            auto r = self.traverse(applicative, f, *right);
            return applicative.invoke(std::move(builder), l, r);
        };

        if (std::holds_alternative<ExprAdd<Expr>>(layer)) {
            const auto &a = std::get<ExprAdd<Expr>>(layer);
            return traverse_pair(a.left, a.right, [](auto &&l, auto &&r) {
                return smd::tree::add_expr(std::forward<decltype(l)>(l),
                                           std::forward<decltype(r)>(r));
            });
        }

        const auto &m = std::get<ExprMul<Expr>>(layer);
        return traverse_pair(m.left, m.right, [](auto &&l, auto &&r) {
            return smd::tree::mul_expr(std::forward<decltype(l)>(l),
                                       std::forward<decltype(r)>(r));
        });
    }
};

/** Traversable map that exposes traverse for Fix<ExprF>. */
struct FixpointTreeTraversableMap : Traversable<FixpointTreeTraversableImpl> {
    using FixpointTreeTraversableImpl::traverse;
};

/** Registers FixpointTreeTraversableMap as the Traversable instance for Fix<ExprF>. */
template <>
inline constexpr auto
    traversable_typeclass<smd::fixpoint::Fix<smd::tree::ExprF>> =
        FixpointTreeTraversableMap{};

} // namespace smd

#endif

```

## smd/tree/fixpoint_tree_traversable.t.cpp

```cpp
// src/smd/tree/fixpoint_tree_traversable.t.cpp                        -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <smd/tree/fixpoint_tree_traversable.hpp>
#include <smd/tree/fixpoint_tree_traversable.hpp> // Re-inclusion check

#include <catch2/catch_test_macros.hpp>

#include <optional>

using smd::tree::add_expr;
using smd::tree::const_expr;
using smd::tree::eval;
using smd::tree::Expr;
using smd::tree::mul_expr;

namespace {

struct ValidatePositive {
    auto operator()(double x) const -> std::optional<double> {
        return x > 0.0 ? std::optional<double>{x} : std::nullopt;
    }
};

struct DoubleValue {
    auto operator()(double x) const -> std::optional<double> {
        return std::optional<double>{x * 2.0};
    }
};

struct IncrementValue {
    auto operator()(double x) const -> std::optional<double> {
        return std::optional<double>{x + 1.0};
    }
};

} // namespace

TEST_CASE("FixpointTreeTraversableTest - TraverseOptionalSuccess") {
    // (1 + 2) * 3 = 9, all positive
    auto tree =
        mul_expr(add_expr(const_expr(1.0), const_expr(2.0)), const_expr(3.0));

    auto traversed = smd::traverse(ValidatePositive{}, tree);

    REQUIRE(traversed.has_value());
    CHECK(eval(*traversed) == 9.0);
}

TEST_CASE("FixpointTreeTraversableTest - TraverseOptionalFailure") {
    // (-1 + 2) * 3, -1 is not positive
    auto tree =
        mul_expr(add_expr(const_expr(-1.0), const_expr(2.0)), const_expr(3.0));

    auto traversed = smd::traverse(ValidatePositive{}, tree);

    CHECK_FALSE(traversed.has_value());
}

TEST_CASE("FixpointTreeTraversableTest - TraverseDoublesConstants") {
    // (1 + 2) * 3 = 9. After doubling: (2 + 4) * 6 = 36
    auto tree =
        mul_expr(add_expr(const_expr(1.0), const_expr(2.0)), const_expr(3.0));

    auto traversed = smd::traverse(DoubleValue{}, tree);

    REQUIRE(traversed.has_value());
    CHECK(eval(*traversed) == 36.0);
}

TEST_CASE("FixpointTreeTraversableTest - TraverseIncrementsConstants") {
    // (1 + 2) * 3 = 9. After +1: (2 + 3) * 4 = 20
    auto tree =
        mul_expr(add_expr(const_expr(1.0), const_expr(2.0)), const_expr(3.0));

    auto traversed = smd::traverse(IncrementValue{}, tree);

    REQUIRE(traversed.has_value());
    CHECK(eval(*traversed) == 20.0);
}

TEST_CASE("FixpointTreeTraversableTest - TraverseLeaf") {
    auto tree = const_expr(7.0);

    auto traversed = smd::traverse(DoubleValue{}, tree);

    REQUIRE(traversed.has_value());
    CHECK(eval(*traversed) == 14.0);
}

TEST_CASE("FixpointTreeTraversableTest - ForEachOptionalSuccess") {
    auto tree = add_expr(const_expr(10.0), const_expr(20.0));
    const auto &traversable = smd::traversable_typeclass<Expr>;

    auto traversed = traversable.for_each(tree, DoubleValue{});

    REQUIRE(traversed.has_value());
    CHECK(eval(*traversed) == 60.0);
}

TEST_CASE("FixpointTreeTraversableTest - TraversePreservesStructure") {
    using smd::fixpoint::unwrap;
    using smd::tree::ExprAdd;

    auto tree = add_expr(const_expr(1.0), const_expr(2.0));
    auto traversed = smd::traverse(IncrementValue{}, tree);

    REQUIRE(traversed.has_value());
    const auto &layer = unwrap(*traversed);
    CHECK(std::holds_alternative<ExprAdd<Expr>>(layer));
}

TEST_CASE("FixpointTreeTraversableTest - ExplicitObjectLookup") {
    auto tree = mul_expr(const_expr(5.0), const_expr(6.0));
    const auto &traversable = smd::traversable_typeclass<Expr>;

    const auto &applicative = smd::applicative_typeclass<std::optional<double>>;
    auto traversed = traversable.traverse(applicative, DoubleValue{}, tree);

    REQUIRE(traversed.has_value());
    CHECK(eval(*traversed) == 120.0);
}

```

## smd/tree/fringe_tree_applicative.hpp

```cpp
// src/smd/tree/fringe_tree_applicative.hpp                           -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_FRINGE_TREE_APPLICATIVE
#define INCLUDED_SMD_TREE_FRINGE_TREE_APPLICATIVE

#include <smd/tree/fringe_tree.hpp>
#include <smd/typeclass/applicative.hpp>

#include <type_traits>
#include <utility>

namespace smd {

/** Applicative typeclass instance for FringeTree<T> with shape-aware semantics.
 * 
 * pure(v) produces a single leaf. apply recurses pairwise: a leaf function
 * distributes over the argument's shape, a leaf argument distributes over the
 * function's shape, and two branches recurse on matching sides. Empty operands
 * yield an empty result. These are monad-derived (not zip) applicative
 * semantics; the structure mirrors the sequence monad over the fringe.
 * @tparam T element type of the function tree
 */
template <class T>
struct FringeTreeApplicativeImpl {
    /** Lift a plain value into a single-leaf tree. */
    template <class VALUE>
    auto pure(this auto &&, VALUE &&value) {
        using U = remove_cvref_t<VALUE>;
        return smd::tree::FringeTree<U>::leaf(std::forward<VALUE>(value));
    }

    /**
     * @brief Apply a tree of functions to a tree of arguments, shape-aware.
     * @param functions tree whose leaves contain callables
     * @param arguments tree whose leaves contain arguments
     * @return tree of results; empty if either operand is empty
     */
    template <class F, class A>
    auto apply(this auto &&self, const smd::tree::FringeTree<F> &functions,
               const smd::tree::FringeTree<A> &arguments)
        -> smd::tree::FringeTree<std::invoke_result_t<const F &, const A &>> {
        using R = std::invoke_result_t<const F &, const A &>;

        if (functions.is_empty() || arguments.is_empty()) {
            return smd::tree::FringeTree<R>::empty();
        }

        if (functions.is_leaf()) {
            auto function = functions.value();
            if (arguments.is_leaf()) {
                return smd::tree::FringeTree<R>::leaf(
                    function(arguments.value()));
            }
            return smd::tree::FringeTree<R>::branch(
                self.apply(functions, arguments.left()),
                self.apply(functions, arguments.right()));
        }

        if (arguments.is_leaf()) {
            return smd::tree::FringeTree<R>::branch(
                self.apply(functions.left(), arguments),
                self.apply(functions.right(), arguments));
        }

        return smd::tree::FringeTree<R>::branch(
            self.apply(functions.left(), arguments.left()),
            self.apply(functions.right(), arguments.right()));
    }
};

/** Applicative map exposing pure and apply for FringeTree<T>. */
template <class T>
struct FringeTreeApplicativeMap : Applicative<FringeTreeApplicativeImpl<T>> {
    using FringeTreeApplicativeImpl<T>::apply;
    using FringeTreeApplicativeImpl<T>::pure;
};

/** Registers FringeTreeApplicativeMap as the Applicative instance for FringeTree<T>. */
template <class T>
inline constexpr auto applicative_typeclass<smd::tree::FringeTree<T>> =
    FringeTreeApplicativeMap<T>{};

} // namespace smd

#endif

```

## smd/tree/fringe_tree_applicative.t.cpp

```cpp
#include <smd/tree/fringe_tree.hpp>
#include <smd/tree/fringe_tree.hpp> // Re-inclusion check
#include <smd/tree/fringe_tree_applicative.hpp>

#include <catch2/catch_test_macros.hpp>

#include <vector>

TEST_CASE("FringeTreeApplicativeTest - Invoke") {
    using Tree = smd::tree::FringeTree<int>;
    auto lhs = Tree::branch(Tree::leaf(1), Tree::leaf(2));
    auto rhs = Tree::branch(Tree::leaf(10), Tree::leaf(20));

    const auto &applicative = smd::applicative_typeclass<Tree>;
    auto summed =
        applicative.invoke([](int a, int b) { return a + b; }, lhs, rhs);

    CHECK(summed.flatten() == (std::vector<int>{11, 22}));
}

TEST_CASE("FringeTreeApplicativeTest - ApplyEmptyArgumentsOrFunctions") {
    using Tree = smd::tree::FringeTree<int>;
    const auto &applicative = smd::applicative_typeclass<Tree>;

    auto fs =
        smd::tree::FringeTree<int (*)(int)>::leaf(+[](int x) { return x + 1; });

    auto empty_args = applicative.apply(fs, Tree::empty());
    CHECK(empty_args.is_empty());

    auto empty_functions = applicative.apply(
        smd::tree::FringeTree<int (*)(int)>::empty(), Tree::leaf(1));
    CHECK(empty_functions.is_empty());
}

TEST_CASE("FringeTreeApplicativeTest - ApplyDistributesAcrossShapes") {
    using Tree = smd::tree::FringeTree<int>;
    const auto &applicative = smd::applicative_typeclass<Tree>;

    auto fs_leaf = smd::tree::FringeTree<int (*)(int)>::leaf(
        +[](int x) { return x * 10; });
    auto args_tree = Tree::branch(Tree::leaf(1), Tree::leaf(2));
    auto distributed = applicative.apply(fs_leaf, args_tree);
    CHECK(distributed.flatten() == (std::vector<int>{10, 20}));

    auto fs_tree = smd::tree::FringeTree<int (*)(int)>::branch(
        smd::tree::FringeTree<int (*)(int)>::leaf(+[](int x) { return x + 2; }),
        smd::tree::FringeTree<int (*)(int)>::leaf(
            +[](int x) { return x + 3; }));
    auto applied_to_leaf = applicative.apply(fs_tree, Tree::leaf(5));
    CHECK(applied_to_leaf.flatten() == (std::vector<int>{7, 8}));
}

```

## smd/tree/fringe_tree_foldable.hpp

```cpp
// src/smd/tree/fringe_tree_foldable.hpp                              -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_FRINGE_TREE_FOLDABLE
#define INCLUDED_SMD_TREE_FRINGE_TREE_FOLDABLE

#include <smd/tree/fringe_tree.hpp>
#include <smd/typeclass/foldable.hpp>

#include <functional>
#include <type_traits>
#include <utility>

namespace smd {

/** Foldable typeclass instance for FringeTree<T>.
 * fold_map maps @p function over leaf values and combines results with the
 * Monoid identity/combine. Empty trees yield the monoid identity; branches
 * combine left and right recursively.
 * @tparam T leaf element type
 */
template <class T>
struct FringeTreeFoldableImpl {
    template <class F>
    auto fold_map(this auto &&self, F &&function,
                  const smd::tree::FringeTree<T> &tree)
        -> remove_cvref_t<std::invoke_result_t<F, const T &>> {
        using Result = remove_cvref_t<std::invoke_result_t<F, const T &>>;

        if (tree.is_empty()) {
            return smd::typeclass::monoid_v<Result>.identity();
        }

        if (tree.is_leaf()) {
            return std::invoke(function, tree.value());
        }

        auto left = self.fold_map(function, tree.left());
        auto right = self.fold_map(function, tree.right());
        return smd::typeclass::monoid_v<Result>.combine(std::move(left),
                                                        std::move(right));
    }
};

/** Foldable map that exposes fold_map for FringeTree<T>. */
template <class T>
struct FringeTreeFoldableMap : Foldable<FringeTreeFoldableImpl<T>> {
    using FringeTreeFoldableImpl<T>::fold_map;
};

/** Registers FringeTreeFoldableMap as the Foldable instance for FringeTree<T>. */
template <class T>
inline constexpr auto foldable_typeclass<smd::tree::FringeTree<T>> =
    FringeTreeFoldableMap<T>{};

} // namespace smd

#endif

```

## smd/tree/fringe_tree_foldable.t.cpp

```cpp
// src/smd/tree/fringe_tree_foldable.t.cpp                            -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <smd/tree/fringe_tree_foldable.hpp>
#include <smd/tree/fringe_tree_foldable.hpp> // Re-inclusion check

#include <smd/typeclass/foldable.hpp>

#include <catch2/catch_test_macros.hpp>

#include <vector>

TEST_CASE("FringeTreeFoldableTest - SingleLeafLength") {
    using Tree = smd::tree::FringeTree<int>;
    const auto &foldable = smd::foldable_typeclass<Tree>;
    auto t = Tree::leaf(42);
    CHECK(foldable.length(t) == 1);
    CHECK(foldable.to_vector(t) == (std::vector<int>{42}));
}

TEST_CASE("FringeTreeFoldableTest - BranchToVector") {
    using Tree = smd::tree::FringeTree<int>;
    const auto &foldable = smd::foldable_typeclass<Tree>;
    auto t =
        Tree::branch(Tree::leaf(1), Tree::branch(Tree::leaf(2), Tree::leaf(3)));
    CHECK(foldable.length(t) == 3);
    CHECK(foldable.to_vector(t) == (std::vector<int>{1, 2, 3}));
}

TEST_CASE("FringeTreeFoldableTest - EmptyTree") {
    using Tree = smd::tree::FringeTree<int>;
    const auto &foldable = smd::foldable_typeclass<Tree>;
    auto t = Tree::empty();
    CHECK(foldable.length(t) == 0);
    CHECK(foldable.empty(t));
}

```

## smd/tree/fringe_tree.hpp

```cpp
// src/smd/tree/fringe_tree.hpp                                       -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_FRINGE_TREE
#define INCLUDED_SMD_TREE_FRINGE_TREE

#include <cassert>
#include <cstddef>
#include <memory>
#include <optional>
#include <utility>
#include <variant>
#include <vector>

namespace smd::tree {

/** Persistent balanced binary tree representing a sequence of leaf values.
 * 
 * The tree has three variants: Empty (no elements), Leaf (one element), and
 * Branch (two subtrees). Values live exclusively at leaves; branches carry
 * only structure and a cached measure (leaf count). This design supports
 * efficient deque operations (cons/snoc/head/tail/last/init) and O(1) concat
 * via structural sharing.
 * @tparam T element type stored at leaves
 */
template <class T>
class FringeTree {
    struct Empty {};
    struct Leaf {
        T d_value;
    };
    struct Branch {
        std::size_t d_measure;
        std::shared_ptr<FringeTree> d_left;
        std::shared_ptr<FringeTree> d_right;
    };

    std::variant<Empty, Leaf, Branch> d_data;

  public:
    using value_type = T;

    /** A deconstructed view of the front or back element plus the remaining tree. */
    struct View {
        T d_value;
        FringeTree d_rest;
    };

    /** Construct the empty tree (no elements). */
    static auto empty() -> FringeTree { return FringeTree(Empty{}); }

    /** Construct a single-element tree containing @p value. */
    static auto leaf(T value) -> FringeTree {
        return FringeTree(Leaf{std::move(value)});
    }

    /** Construct a branch joining two non-empty subtrees. */
    static auto branch(FringeTree left, FringeTree right) -> FringeTree {
        auto left_ptr = std::make_shared<FringeTree>(std::move(left));
        auto right_ptr = std::make_shared<FringeTree>(std::move(right));
        auto measure = left_ptr->measure() + right_ptr->measure();
        return FringeTree(
            Branch{measure, std::move(left_ptr), std::move(right_ptr)});
    }

    /** True when the tree is empty. */
    auto is_empty() const -> bool {
        return std::holds_alternative<Empty>(d_data);
    }
    /** True when the tree is a single leaf. */
    auto is_leaf() const -> bool {
        return std::holds_alternative<Leaf>(d_data);
    }
    /** True when the tree is an internal branch. */
    auto is_branch() const -> bool {
        return std::holds_alternative<Branch>(d_data);
    }

    /** Number of leaf elements in the tree (cached at branches). */
    auto measure() const -> std::size_t {
        if (is_empty()) {
            return 0U;
        }
        if (is_leaf()) {
            return 1U;
        }
        return std::get<Branch>(d_data).d_measure;
    }

    /** Return the leaf value; precondition: is_leaf(). */
    auto value() const -> const T & {
        assert(is_leaf());
        return std::get<Leaf>(d_data).d_value;
    }

    /** Return the left subtree; precondition: is_branch(). */
    auto left() const -> const FringeTree & {
        assert(is_branch());
        return *std::get<Branch>(d_data).d_left;
    }

    /** Return the right subtree; precondition: is_branch(). */
    auto right() const -> const FringeTree & {
        assert(is_branch());
        return *std::get<Branch>(d_data).d_right;
    }

    /** Synonym for measure() — number of leaf elements. */
    auto breadth() const -> std::size_t { return measure(); }

    /** Maximum depth from root to any leaf. */
    auto depth() const -> std::size_t {
        if (is_empty()) {
            return 0U;
        }
        if (is_leaf()) {
            return 1U;
        }
        const auto l = left().depth();
        const auto r = right().depth();
        return ((l > r) ? l : r) + 1U;
    }

    /** Collect all leaf values into a vector in left-to-right order. */
    auto flatten() const -> std::vector<T> {
        if (is_empty()) {
            return {};
        }
        if (is_leaf()) {
            return {value()};
        }

        auto l = left().flatten();
        auto r = right().flatten();
        l.insert(l.end(), r.begin(), r.end());
        return l;
    }

    /** Visit each leaf value left-to-right, calling @p callback on each. */
    template <typename F>
    void for_each(F &&callback) const {
        if (is_empty()) {
            return;
        }
        if (is_leaf()) {
            callback(value());
            return;
        }
        left().for_each(callback);
        right().for_each(callback);
    }

    /** Concatenate two trees; empty operands are identity elements. */
    static auto concat(const FringeTree &left_tree,
                       const FringeTree &right_tree) -> FringeTree {
        if (left_tree.is_empty()) {
            return right_tree;
        }
        if (right_tree.is_empty()) {
            return left_tree;
        }
        return branch(left_tree, right_tree);
    }

    /** Prepend @p value to @p tree (cons / push-front). */
    static auto prepend(T value, const FringeTree &tree) -> FringeTree {
        return concat(leaf(std::move(value)), tree);
    }

    /** Append @p value to @p tree (snoc / push-back), static form. */
    static auto append(const FringeTree &tree, T value) -> FringeTree {
        return concat(tree, leaf(std::move(value)));
    }

    /** Return a new tree with @p x prepended (deque cons). */
    auto cons(T x) const -> FringeTree {
        return concat(leaf(std::move(x)), *this);
    }

    /** Return a new tree with @p x appended (deque snoc). */
    auto snoc(T x) const -> FringeTree {
        return concat(*this, leaf(std::move(x)));
    }

    /** Return the concatenation of this tree with @p other. */
    auto append(const FringeTree &other) const -> FringeTree {
        return concat(*this, other);
    }

    /** Build a tree from a vector, appending elements left-to-right. */
    static auto from_sequence(std::vector<T> values) -> FringeTree {
        auto result = empty();
        for (auto &v : values) {
            result = result.snoc(std::move(v));
        }
        return result;
    }

    /**
     * @brief Destructure from the left: returns the front element and the rest.
     * @return nullopt if empty; otherwise View{front, tail}
     */
    auto view_l() const -> std::optional<View> {
        if (is_empty()) {
            return std::nullopt;
        }
        if (is_leaf()) {
            return View{value(), empty()};
        }

        auto left_view = left().view_l();
        if (left_view.has_value()) {
            return View{left_view->d_value, concat(left_view->d_rest, right())};
        }

        return right().view_l();
    }

    /**
     * @brief Destructure from the right: returns the back element and the rest.
     * @return nullopt if empty; otherwise View{back, init}
     */
    auto view_r() const -> std::optional<View> {
        if (is_empty()) {
            return std::nullopt;
        }
        if (is_leaf()) {
            return View{value(), empty()};
        }

        auto right_view = right().view_r();
        if (right_view.has_value()) {
            return View{right_view->d_value,
                        concat(left(), right_view->d_rest)};
        }

        return left().view_r();
    }

    /** Return the first (leftmost) leaf value; precondition: non-empty. */
    auto head() const -> T {
        auto v = view_l();
        assert(v.has_value());
        return v->d_value;
    }

    /** Return all but the first leaf; returns empty() when called on empty. */
    auto tail() const -> FringeTree {
        auto v = view_l();
        return v.has_value() ? v->d_rest : empty();
    }

    /** Return the last (rightmost) leaf value; precondition: non-empty. */
    auto last() const -> T {
        auto v = view_r();
        assert(v.has_value());
        return v->d_value;
    }

    /** Return all but the last leaf; returns empty() when called on empty. */
    auto init() const -> FringeTree {
        auto v = view_r();
        return v.has_value() ? v->d_rest : empty();
    }

  private:
    explicit FringeTree(Empty e) : d_data(std::move(e)) {}

    explicit FringeTree(Leaf l) : d_data(std::move(l)) {}

    explicit FringeTree(Branch b) : d_data(std::move(b)) {}
};

} // namespace smd::tree

#endif

```

## smd/tree/fringe_tree.t.cpp

```cpp
#include <smd/tree/fringe_tree.hpp>
#include <smd/tree/fringe_tree.hpp> // Re-inclusion check
#include <smd/tree/fringe_tree_foldable.hpp>
#include <smd/typeclass/foldable.hpp>

#include <catch2/catch_test_macros.hpp>

#include <vector>

TEST_CASE("FringeTreeTest - EmptyLeafAndPredicates") {
    using Tree = smd::tree::FringeTree<int>;

    auto empty = Tree::empty();
    CHECK(empty.is_empty());
    CHECK_FALSE(empty.is_leaf());
    CHECK_FALSE(empty.is_branch());
    CHECK(empty.measure() == 0U);
    CHECK(empty.breadth() == 0U);
    CHECK(empty.depth() == 0U);
    CHECK(empty.flatten() == (std::vector<int>{}));
    CHECK_FALSE(empty.view_l().has_value());
    CHECK_FALSE(empty.view_r().has_value());

    auto single = Tree::leaf(42);
    CHECK_FALSE(single.is_empty());
    CHECK(single.is_leaf());
    CHECK_FALSE(single.is_branch());
    CHECK(single.measure() == 1U);
    CHECK(single.value() == 42);
    CHECK(single.flatten() == (std::vector<int>{42}));
}

TEST_CASE("FringeTreeTest - BranchLeftRightAndMemberStyleOperations") {
    using Tree = smd::tree::FringeTree<int>;

    auto left = Tree::branch(Tree::leaf(1), Tree::leaf(2));
    auto right = Tree::branch(Tree::leaf(3), Tree::leaf(4));
    auto tree = Tree::branch(left, right);

    REQUIRE(tree.is_branch());
    CHECK(tree.left().flatten() == (std::vector<int>{1, 2}));
    CHECK(tree.right().flatten() == (std::vector<int>{3, 4}));

    auto prepended = Tree::prepend(0, tree);
    CHECK(prepended.flatten() == (std::vector<int>{0, 1, 2, 3, 4}));

    auto appended = Tree::append(tree, 5);
    CHECK(appended.flatten() == (std::vector<int>{1, 2, 3, 4, 5}));

    auto concatenated = Tree::concat(left, right);
    CHECK(concatenated.flatten() == (std::vector<int>{1, 2, 3, 4}));
}

TEST_CASE("FringeTreeTest - SingletonViewsAndEmptyTailInit") {
    using Tree = smd::tree::FringeTree<int>;

    auto single = Tree::leaf(7);
    auto left = single.view_l();
    REQUIRE(left.has_value());
    CHECK(left->d_value == 7);
    CHECK(left->d_rest.is_empty());

    auto right = single.view_r();
    REQUIRE(right.has_value());
    CHECK(right->d_value == 7);
    CHECK(right->d_rest.is_empty());

    auto empty = Tree::empty();
    CHECK(empty.tail().is_empty());
    CHECK(empty.init().is_empty());
}

TEST_CASE("FringeTreeTest - BasicMeasureDepthFlatten") {
    using Tree = smd::tree::FringeTree<int>;

    auto tree =
        Tree::branch(Tree::branch(Tree::leaf(1), Tree::leaf(2)), Tree::leaf(3));

    CHECK(tree.measure() == 3U);
    CHECK(tree.breadth() == 3U);
    CHECK(tree.depth() == 3U);
    CHECK(tree.flatten() == (std::vector<int>{1, 2, 3}));
}

TEST_CASE("FringeTreeTest - ViewsAndListOps") {
    using Tree = smd::tree::FringeTree<int>;

    auto tree =
        Tree::branch(Tree::branch(Tree::leaf(1), Tree::leaf(2)), Tree::leaf(3));

    auto left_view = tree.view_l();
    REQUIRE(left_view.has_value());
    CHECK(left_view->d_value == 1);
    CHECK(left_view->d_rest.flatten() == (std::vector<int>{2, 3}));

    auto right_view = tree.view_r();
    REQUIRE(right_view.has_value());
    CHECK(right_view->d_value == 3);
    CHECK(right_view->d_rest.flatten() == (std::vector<int>{1, 2}));

    CHECK(tree.head() == 1);
    CHECK(tree.last() == 3);
    CHECK(tree.tail().flatten() == (std::vector<int>{2, 3}));
    CHECK(tree.init().flatten() == (std::vector<int>{1, 2}));
}

TEST_CASE("FringeTreeTest - PrependAppendConcat") {
    using Tree = smd::tree::FringeTree<int>;

    auto tree = Tree::branch(Tree::leaf(1), Tree::leaf(2));

    auto prepended = Tree::prepend(0, tree);
    CHECK(prepended.flatten() == (std::vector<int>{0, 1, 2}));

    auto appended = Tree::append(tree, 3);
    CHECK(appended.flatten() == (std::vector<int>{1, 2, 3}));

    auto concatenated = Tree::concat(tree, tree);
    CHECK(concatenated.flatten() == (std::vector<int>{1, 2, 1, 2}));
}

TEST_CASE("FringeTreeTest - FoldableIntegration") {
    using Tree = smd::tree::FringeTree<int>;

    auto tree =
        Tree::branch(Tree::branch(Tree::leaf(1), Tree::leaf(2)), Tree::leaf(3));

    const auto &foldable = smd::foldable_typeclass<Tree>;
    CHECK(foldable.length(tree) == 3U);
    CHECK(foldable.fold_map([](int x) { return x; }, tree) == 6);
    CHECK(foldable.to_vector(tree) == (std::vector<int>{1, 2, 3}));
}

```

## smd/tree/fringe_tree_traversable.hpp

```cpp
// src/smd/tree/fringe_tree_traversable.hpp                           -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_FRINGE_TREE_TRAVERSABLE
#define INCLUDED_SMD_TREE_FRINGE_TREE_TRAVERSABLE

#include <smd/tree/fringe_tree.hpp>
#include <smd/tree/fringe_tree_applicative.hpp>
#include <smd/typeclass/traversable.hpp>

#include <functional>
#include <type_traits>
#include <utility>

namespace smd {

/** Traversable typeclass instance for FringeTree<T>.
 * traverse maps each leaf value into an applicative context and rebuilds a
 * FringeTree inside that context, preserving the original tree's structure.
 * Empty stays empty; leaves become single-element trees; branches combine
 * the traversed subtrees with FringeTree::branch inside the applicative.
 * @tparam T leaf element type of the tree being traversed
 */
template <class T>
struct FringeTreeTraversableImpl {
    using element_type = T;

    template <class APPLICATIVE, class F>
    auto traverse(this auto &&self, const APPLICATIVE &applicative,
                  F &&function, const smd::tree::FringeTree<T> &tree) {
        using Context = remove_cvref_t<std::invoke_result_t<F, const T &>>;
        using U = smd::applicative_value_t<Context>;

        if (tree.is_empty()) {
            return applicative.pure(smd::tree::FringeTree<U>::empty());
        }

        if (tree.is_leaf()) {
            return applicative.invoke(
                [](auto &&value) {
                    using U = remove_cvref_t<decltype(value)>;
                    return smd::tree::FringeTree<U>::leaf(
                        std::forward<decltype(value)>(value));
                },
                std::invoke(std::forward<F>(function), tree.value()));
        }

        auto left = self.traverse(applicative, function, tree.left());
        auto right = self.traverse(applicative, function, tree.right());

        return applicative.invoke(
            [](auto &&l, auto &&r) {
                return smd::tree::
                    FringeTree<remove_cvref_t<decltype(l.value())>>::branch(
                        std::forward<decltype(l)>(l),
                        std::forward<decltype(r)>(r));
            },
            left, right);
    }
};

/** Traversable map that exposes traverse for FringeTree<T>. */
template <class T>
struct FringeTreeTraversableMap : Traversable<FringeTreeTraversableImpl<T>> {
    using FringeTreeTraversableImpl<T>::traverse;
};

/** Registers FringeTreeTraversableMap as the Traversable instance for FringeTree<T>. */
template <class T>
inline constexpr auto traversable_typeclass<smd::tree::FringeTree<T>> =
    FringeTreeTraversableMap<T>{};

} // namespace smd

#endif

```

## smd/tree/fringe_tree_traversable.t.cpp

```cpp
#include <smd/tree/fringe_tree.hpp>
#include <smd/tree/fringe_tree.hpp> // Re-inclusion check
#include <smd/tree/fringe_tree_traversable.hpp>

#include <catch2/catch_test_macros.hpp>

#include <beman/optional/optional.hpp>

#include <optional>
#include <vector>

namespace {

struct PositiveTimesTen {
    auto operator()(int x) const -> std::optional<int> {
        return x > 0 ? std::optional<int>{x * 10} : std::optional<int>{};
    }
};

struct TimesTen {
    auto operator()(int x) const -> std::optional<int> {
        return std::optional<int>{x * 10};
    }
};

struct NonNegativeIdentity {
    auto operator()(int x) const -> std::optional<int> {
        return x >= 0 ? std::optional<int>{x} : std::optional<int>{};
    }
};

struct PlusOne {
    auto operator()(int x) const -> std::optional<int> {
        return std::optional<int>{x + 1};
    }
};

struct TimesTenBeman {
    auto operator()(int x) const -> beman::optional::optional<int> {
        return beman::optional::optional<int>{x * 10};
    }
};

struct PlusSevenBeman {
    auto operator()(int x) const -> beman::optional::optional<int> {
        return beman::optional::optional<int>{x + 7};
    }
};

} // namespace

TEST_CASE("FringeTreeTraversableTest - TraverseOptional") {
    using Tree = smd::tree::FringeTree<int>;
    auto tree =
        Tree::branch(Tree::leaf(1), Tree::branch(Tree::leaf(2), Tree::leaf(3)));

    auto traversed = smd::traverse(PositiveTimesTen{}, tree);

    REQUIRE(traversed.has_value());
    CHECK(traversed->flatten() == (std::vector<int>{10, 20, 30}));
}

TEST_CASE("FringeTreeTraversableTest - TraverseOptionalEmpty") {
    using Tree = smd::tree::FringeTree<int>;
    auto tree = Tree::empty();

    auto traversed = smd::traverse(TimesTen{}, tree);

    REQUIRE(traversed.has_value());
    CHECK(traversed->is_empty());
}

TEST_CASE("FringeTreeTraversableTest - TraverseBemanOptionalEmpty") {
    using Tree = smd::tree::FringeTree<int>;
    auto tree = Tree::empty();

    auto traversed = smd::traverse(TimesTenBeman{}, tree);

    REQUIRE(traversed.has_value());
    CHECK(traversed->is_empty());
}

TEST_CASE("FringeTreeTraversableTest - TraverseOptionalFailure") {
    using Tree = smd::tree::FringeTree<int>;
    auto tree = Tree::branch(Tree::leaf(1), Tree::leaf(-2));

    auto traversed = smd::traverse(NonNegativeIdentity{}, tree);

    CHECK_FALSE(traversed.has_value());
}

TEST_CASE("FringeTreeTraversableTest - TraverseLeaf") {
    using Tree = smd::tree::FringeTree<int>;
    auto tree = Tree::leaf(7);

    auto traversed = smd::traverse(PlusOne{}, tree);

    REQUIRE(traversed.has_value());
    REQUIRE(traversed->is_leaf());
    CHECK(traversed->value() == 8);
}

TEST_CASE("FringeTreeTraversableTest - TraverseBemanOptional") {
    using Tree = smd::tree::FringeTree<int>;
    auto tree = Tree::branch(Tree::leaf(2), Tree::leaf(5));

    auto traversed = smd::traverse(PlusSevenBeman{}, tree);

    REQUIRE(traversed.has_value());
    CHECK(traversed->flatten() == (std::vector<int>{9, 12}));
}

```

## smd/typeclass/applicative.hpp

```cpp
// src/smd/typeclass/applicative.hpp                                  -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TYPECLASS_APPLICATIVE
#define INCLUDED_SMD_TYPECLASS_APPLICATIVE

#include <smd/typeclass/typeclass_base.hpp>

#include <beman/optional/optional.hpp>

#include <concepts>
#include <functional>
#include <optional>
#include <tuple>
#include <type_traits>
#include <utility>

namespace smd {

namespace detail {

template <class FUNCTION, class... BOUND_ARGS>
struct terminating_partial {
    FUNCTION function;
    std::tuple<BOUND_ARGS...> bound_args;

    template <class NEXT_ARG>
    auto operator()(NEXT_ARG &&next_arg) {
        return invoke_or_extend(std::forward<NEXT_ARG>(next_arg),
                                std::index_sequence_for<BOUND_ARGS...>{});
    }

    template <class NEXT_ARG>
    auto operator()(NEXT_ARG &&next_arg) const {
        return invoke_or_extend_const(std::forward<NEXT_ARG>(next_arg),
                                      std::index_sequence_for<BOUND_ARGS...>{});
    }

  private:
    template <class NEXT_ARG, std::size_t... IDX>
    auto invoke_or_extend(NEXT_ARG &&next_arg, std::index_sequence<IDX...>) {
        if constexpr (std::invocable<FUNCTION &, BOUND_ARGS &..., NEXT_ARG>) {
            return std::invoke(function, std::get<IDX>(bound_args)...,
                               std::forward<NEXT_ARG>(next_arg));
        } else {
            using NEXT_PARTIAL = terminating_partial<FUNCTION, BOUND_ARGS...,
                                                     remove_cvref_t<NEXT_ARG>>;
            return NEXT_PARTIAL{
                function,
                std::tuple_cat(std::move(bound_args),
                               std::tuple<remove_cvref_t<NEXT_ARG>>{
                                   std::forward<NEXT_ARG>(next_arg)})};
        }
    }

    template <class NEXT_ARG, std::size_t... IDX>
    auto invoke_or_extend_const(NEXT_ARG &&next_arg,
                                std::index_sequence<IDX...>) const {
        if constexpr (std::invocable<const FUNCTION &, const BOUND_ARGS &...,
                                     NEXT_ARG>) {
            return std::invoke(function, std::get<IDX>(bound_args)...,
                               std::forward<NEXT_ARG>(next_arg));
        } else {
            using NEXT_PARTIAL = terminating_partial<FUNCTION, BOUND_ARGS...,
                                                     remove_cvref_t<NEXT_ARG>>;
            return NEXT_PARTIAL{
                function,
                std::tuple_cat(bound_args,
                               std::tuple<remove_cvref_t<NEXT_ARG>>{
                                   std::forward<NEXT_ARG>(next_arg)})};
        }
    }
};

template <class FUNCTION>
auto make_terminating_partial(FUNCTION &&function) {
    using STORED_FUNCTION = remove_cvref_t<FUNCTION>;
    return terminating_partial<STORED_FUNCTION>{
        std::forward<FUNCTION>(function), std::tuple<>{}};
}

} // namespace detail

// Applicative pattern invariants:
// - Minimal required operations are pure and apply.
// - invoke is derived from pure/apply via terminating partial application,
//   but an Impl may provide a custom invoke for different semantics or
//   efficiency.
// - Derived operations (lift/ap/zip_with/discard_*) live on that object.
// - Dispatch happens through a provided object or
// applicative_typeclass<Concrete>.
// - Do not introduce hidden alternate semantics without a distinct map/type.

/** CRTP base for Applicative instances.
 * `Impl` must provide `pure(value)` and `apply(f_in_context, arg_in_context)`.
 * All other operations are derived.
 */
template <class Impl>
struct Applicative : protected Impl {
    static_assert(!std::is_same_v<Impl, std::false_type>,
                  "No applicative_typeclass<T> specialization found. "
                  "Specialize smd::applicative_typeclass<T> for your type T "
                  "and provide pure(...) and apply(...) operations.");
    // Alternate-core: pure + apply are the primitives; invoke and all others
    // derive from them.
    using Impl::apply;
    using Impl::pure;

    // a11f7d8b-8f89-4f3e-9c92-f9f08ab7ef11
    // Teaching-friendly alias for "apply pure function to effectful arguments".
    // Prefer invoke as the primary C++ spelling (std::invoke model).
    template <class FUNCTION, class FIRST_ARGUMENT, class... REST_ARGUMENTS>
    auto apply_pure(this auto &&self, FUNCTION &&function,
                    FIRST_ARGUMENT &&first_argument,
                    REST_ARGUMENTS &&...rest_arguments) {
        return self.invoke(std::forward<FUNCTION>(function),
                           std::forward<FIRST_ARGUMENT>(first_argument),
                           std::forward<REST_ARGUMENTS>(rest_arguments)...);
    }

    /** Lifts `function` into the applicative and applies it to one or more
     * effectful arguments left-to-right, producing a single effectful result.
     * @param function  A plain callable; it is wrapped with `pure` internally.
     * @param first_argument  First effectful argument (e.g., optional or vector).
     * @param rest_arguments  Additional effectful arguments, if any.
     */
    template <class FUNCTION, class FIRST_ARGUMENT, class... REST_ARGUMENTS>
    auto invoke(this auto &&self, FUNCTION &&function,
                FIRST_ARGUMENT &&first_argument,
                REST_ARGUMENTS &&...rest_arguments) {
        using SELF = std::remove_reference_t<decltype(self)>;
        using IMPL_BASE =
            std::conditional_t<std::is_const_v<SELF>, const Impl, Impl>;

        if constexpr (requires(IMPL_BASE &impl) {
                          impl.invoke(
                              std::forward<FUNCTION>(function),
                              std::forward<FIRST_ARGUMENT>(first_argument),
                              std::forward<REST_ARGUMENTS>(rest_arguments)...);
                      }) {
            return static_cast<IMPL_BASE &>(self).invoke(
                std::forward<FUNCTION>(function),
                std::forward<FIRST_ARGUMENT>(first_argument),
                std::forward<REST_ARGUMENTS>(rest_arguments)...);
        } else {
            auto lifted_function = self.pure(detail::make_terminating_partial(
                std::forward<FUNCTION>(function)));
            return self.apply_chain(
                self.ap(std::move(lifted_function),
                        std::forward<FIRST_ARGUMENT>(first_argument)),
                std::forward<REST_ARGUMENTS>(rest_arguments)...);
        }
    }
    // a11f7d8b-8f89-4f3e-9c92-f9f08ab7ef11 end

  private:
    template <class ACCUMULATED>
    auto apply_chain(this auto &&, ACCUMULATED &&accumulated) {
        return std::forward<ACCUMULATED>(accumulated);
    }

    template <class ACCUMULATED, class NEXT_ARGUMENT, class... REST_ARGUMENTS>
    auto apply_chain(this auto &&self, ACCUMULATED &&accumulated,
                     NEXT_ARGUMENT &&next_argument,
                     REST_ARGUMENTS &&...rest_arguments) {
        auto next = self.ap(std::forward<ACCUMULATED>(accumulated),
                            std::forward<NEXT_ARGUMENT>(next_argument));
        if constexpr (sizeof...(REST_ARGUMENTS) == 0) {
            return next;
        } else {
            return self.apply_chain(
                std::move(next),
                std::forward<REST_ARGUMENTS>(rest_arguments)...);
        }
    }

  public:
    /** Single-argument fmap via invoke; applies `function` to one effectful arg. */
    template <class FUNCTION, class ARGUMENT>
    auto map(this auto &&self, FUNCTION &&function, ARGUMENT &&argument) {
        return self.invoke(std::forward<FUNCTION>(function),
                           std::forward<ARGUMENT>(argument));
    }

    /** Alias for `pure`; embeds a plain value into the applicative context. */
    template <class VALUE>
    auto lift(this auto &&self, VALUE &&value) {
        return self.pure(std::forward<VALUE>(value));
    }

    /** Alias for the primitive `apply`: applies an effectful function to an
     * effectful argument.
     */
    template <class FUNCTION_IN_CONTEXT, class ARGUMENT_IN_CONTEXT>
    auto ap(this auto &&self, FUNCTION_IN_CONTEXT &&function,
            ARGUMENT_IN_CONTEXT &&argument) {
        return self.apply(std::forward<FUNCTION_IN_CONTEXT>(function),
                          std::forward<ARGUMENT_IN_CONTEXT>(argument));
    }

    /** Lifts a binary function and applies it to two effectful arguments. */
    template <class FUNCTION, class FIRST_ARGUMENT, class SECOND_ARGUMENT>
    auto zip_with(this auto &&self, FUNCTION &&function,
                  FIRST_ARGUMENT &&first_argument,
                  SECOND_ARGUMENT &&second_argument) {
        return self.invoke(std::forward<FUNCTION>(function),
                           std::forward<FIRST_ARGUMENT>(first_argument),
                           std::forward<SECOND_ARGUMENT>(second_argument));
    }

    /** Sequences two effectful values; returns the second, ignoring the first. */
    template <class FIRST_ARGUMENT, class SECOND_ARGUMENT>
    auto discard_first(this auto &&self, FIRST_ARGUMENT &&first_argument,
                       SECOND_ARGUMENT &&second_argument) {
        return self.invoke(
            [](const auto &, auto &&rhs) {
                return std::forward<decltype(rhs)>(rhs);
            },
            std::forward<FIRST_ARGUMENT>(first_argument),
            std::forward<SECOND_ARGUMENT>(second_argument));
    }

    /** Sequences two effectful values; returns the first, ignoring the second. */
    template <class FIRST_ARGUMENT, class SECOND_ARGUMENT>
    auto discard_second(this auto &&self, FIRST_ARGUMENT &&first_argument,
                        SECOND_ARGUMENT &&second_argument) {
        return self.invoke(
            [](auto &&lhs, const auto &) {
                return std::forward<decltype(lhs)>(lhs);
            },
            std::forward<FIRST_ARGUMENT>(first_argument),
            std::forward<SECOND_ARGUMENT>(second_argument));
    }

    /** Delegates invoke to a different applicative instance at runtime. */
    template <class APPLICATIVE_MAP, class FUNCTION, class FIRST_ARGUMENT,
              class... REST_ARGUMENTS>
    auto invoke_with(this auto &&, const APPLICATIVE_MAP &applicative_map,
                     FUNCTION &&function, FIRST_ARGUMENT &&first_argument,
                     REST_ARGUMENTS &&...rest_arguments) {
        return applicative_map.invoke(
            std::forward<FUNCTION>(function),
            std::forward<FIRST_ARGUMENT>(first_argument),
            std::forward<REST_ARGUMENTS>(rest_arguments)...);
    }

    /** Delegates apply_pure to a different applicative instance at runtime. */
    template <class APPLICATIVE_MAP, class FUNCTION, class FIRST_ARGUMENT,
              class... REST_ARGUMENTS>
    auto apply_pure_with(this auto &&, const APPLICATIVE_MAP &applicative_map,
                         FUNCTION &&function, FIRST_ARGUMENT &&first_argument,
                         REST_ARGUMENTS &&...rest_arguments) {
        return applicative_map.invoke(
            std::forward<FUNCTION>(function),
            std::forward<FIRST_ARGUMENT>(first_argument),
            std::forward<REST_ARGUMENTS>(rest_arguments)...);
    }

    /** Delegates invoke to a compile-time constant applicative instance. */
    template <const auto &APPLICATIVE_MAP, class FUNCTION, class FIRST_ARGUMENT,
              class... REST_ARGUMENTS>
    auto invoke_with(this auto &&, FUNCTION &&function,
                     FIRST_ARGUMENT &&first_argument,
                     REST_ARGUMENTS &&...rest_arguments) {
        return APPLICATIVE_MAP.invoke(
            std::forward<FUNCTION>(function),
            std::forward<FIRST_ARGUMENT>(first_argument),
            std::forward<REST_ARGUMENTS>(rest_arguments)...);
    }

    /** Delegates apply_pure to a compile-time constant applicative instance. */
    template <const auto &APPLICATIVE_MAP, class FUNCTION, class FIRST_ARGUMENT,
              class... REST_ARGUMENTS>
    auto apply_pure_with(this auto &&, FUNCTION &&function,
                         FIRST_ARGUMENT &&first_argument,
                         REST_ARGUMENTS &&...rest_arguments) {
        return APPLICATIVE_MAP.invoke(
            std::forward<FUNCTION>(function),
            std::forward<FIRST_ARGUMENT>(first_argument),
            std::forward<REST_ARGUMENTS>(rest_arguments)...);
    }
};

/** Typeclass lookup variable for Applicative; specialize for each type. */
template <class T>
inline constexpr auto applicative_typeclass = std::false_type{};

template <class VALUE_TYPE>
struct OptionalApplicativeImpl {
    template <class VALUE>
    auto pure(this auto &&, VALUE &&value)
        -> std::optional<remove_cvref_t<VALUE>> {
        return std::optional<remove_cvref_t<VALUE>>{std::forward<VALUE>(value)};
    }

    template <class FUNCTION_IN_CONTEXT, class ARGUMENT_IN_CONTEXT>
    auto apply(this auto &&, FUNCTION_IN_CONTEXT &&function,
               ARGUMENT_IN_CONTEXT &&argument) {
        using Result =
            std::invoke_result_t<decltype(*function), decltype(*argument)>;

        if (!function || !argument) {
            return std::optional<remove_cvref_t<Result>>{};
        }

        return std::optional<remove_cvref_t<Result>>{
            std::invoke(*std::forward<FUNCTION_IN_CONTEXT>(function),
                        *std::forward<ARGUMENT_IN_CONTEXT>(argument))};
    }
};

template <class VALUE_TYPE>
struct OptionalApplicativeMap
    : Applicative<OptionalApplicativeImpl<VALUE_TYPE>> {
    using OptionalApplicativeImpl<VALUE_TYPE>::apply;
    using OptionalApplicativeImpl<VALUE_TYPE>::pure;
};

template <class VALUE_TYPE>
    requires(!std::same_as<beman::optional::optional<VALUE_TYPE>,
                           std::optional<VALUE_TYPE>>)
struct BemanOptionalApplicativeImpl {
    template <class VALUE>
    auto pure(this auto &&, VALUE &&value)
        -> beman::optional::optional<remove_cvref_t<VALUE>> {
        return beman::optional::optional<remove_cvref_t<VALUE>>{
            std::forward<VALUE>(value)};
    }

    template <class FUNCTION_IN_CONTEXT, class ARGUMENT_IN_CONTEXT>
    auto apply(this auto &&, FUNCTION_IN_CONTEXT &&function,
               ARGUMENT_IN_CONTEXT &&argument) {
        using Result =
            std::invoke_result_t<decltype(*function), decltype(*argument)>;

        if (!function || !argument) {
            return beman::optional::optional<remove_cvref_t<Result>>{};
        }

        return beman::optional::optional<remove_cvref_t<Result>>{
            std::invoke(*std::forward<FUNCTION_IN_CONTEXT>(function),
                        *std::forward<ARGUMENT_IN_CONTEXT>(argument))};
    }
};

template <class VALUE_TYPE>
    requires(!std::same_as<beman::optional::optional<VALUE_TYPE>,
                           std::optional<VALUE_TYPE>>)
struct BemanOptionalApplicativeMap
    : Applicative<BemanOptionalApplicativeImpl<VALUE_TYPE>> {
    using BemanOptionalApplicativeImpl<VALUE_TYPE>::apply;
    using BemanOptionalApplicativeImpl<VALUE_TYPE>::pure;
};

/** Applicative instance for `std::optional<VALUE_TYPE>`. */
template <class VALUE_TYPE>
inline constexpr auto applicative_typeclass<std::optional<VALUE_TYPE>> =
    OptionalApplicativeMap<VALUE_TYPE>{};

/** Applicative instance for `beman::optional::optional<VALUE_TYPE>`. */
template <class VALUE_TYPE>
    requires(!std::same_as<beman::optional::optional<VALUE_TYPE>,
                           std::optional<VALUE_TYPE>>)
inline constexpr auto
    applicative_typeclass<beman::optional::optional<VALUE_TYPE>> =
        BemanOptionalApplicativeMap<VALUE_TYPE>{};

} // namespace smd

#endif

```

## smd/typeclass/applicative.t.cpp

```cpp
#include <smd/typeclass/applicative.hpp>
#include <smd/typeclass/applicative.hpp> // Re-inclusion check
#include <smd/typeclass/test/test_support.hpp>

#include <catch2/catch_test_macros.hpp>

#include <beman/optional/optional.hpp>

#include <cmath>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace {

template <class VALUE_TYPE>
struct DirectInvokeIdentityApplicativeImpl {
    template <class VALUE>
    auto pure(this auto &&, VALUE &&value) {
        return smd::typeclass::test::Identity<smd::remove_cvref_t<VALUE>>{
            std::forward<VALUE>(value)};
    }

    template <class FUNCTION_IN_CONTEXT, class ARGUMENT_IN_CONTEXT>
    auto apply(this auto &&, const FUNCTION_IN_CONTEXT &function,
               const ARGUMENT_IN_CONTEXT &argument) {
        using Result =
            std::invoke_result_t<const typename smd::remove_cvref_t<
                                     FUNCTION_IN_CONTEXT>::value_type &,
                                 const typename smd::remove_cvref_t<
                                     ARGUMENT_IN_CONTEXT>::value_type &>;

        return smd::typeclass::test::Identity<smd::remove_cvref_t<Result>>{
            std::invoke(function.value, argument.value)};
    }

    template <class FUNCTION, class FIRST_ARGUMENT, class... REST_ARGUMENTS>
    auto invoke(this auto &&self, FUNCTION &&function,
                const FIRST_ARGUMENT &first_argument,
                const REST_ARGUMENTS &...rest_arguments) {
        return self.pure(std::invoke(std::forward<FUNCTION>(function),
                                     first_argument.value,
                                     rest_arguments.value...));
    }
};

template <class VALUE_TYPE>
struct DirectInvokeIdentityApplicativeMap
    : smd::Applicative<DirectInvokeIdentityApplicativeImpl<VALUE_TYPE>> {
    using DirectInvokeIdentityApplicativeImpl<VALUE_TYPE>::apply;
    using DirectInvokeIdentityApplicativeImpl<VALUE_TYPE>::invoke;
    using DirectInvokeIdentityApplicativeImpl<VALUE_TYPE>::pure;
};

inline constexpr DirectInvokeIdentityApplicativeMap<int> direct_invoke_map{};

template <class A, class B, class C>
void run_bare_identity_matrix_case(A a, B b, C c) {
    using BareA = smd::typeclass::test::BareIdentity<A>;
    const auto &applicative = smd::applicative_typeclass<BareA>;

    auto summed = applicative.invoke(
        [](const A &x, const B &y, const C &z) {
            return static_cast<long double>(x) + static_cast<long double>(y) +
                   static_cast<long double>(z);
        },
        BareA{a}, smd::typeclass::test::BareIdentity<B>{b},
        smd::typeclass::test::BareIdentity<C>{c});
    auto expected = static_cast<long double>(a) + static_cast<long double>(b) +
                    static_cast<long double>(c);
    CHECK(std::abs(summed.value - expected) < 1e-9L);

    auto mapped = applicative.map(
        [](const A &x) { return std::to_string(static_cast<long double>(x)); },
        BareA{a});
    CHECK_FALSE(mapped.value.empty());

    auto applied = applicative.ap(
        smd::typeclass::test::BareIdentity<std::string (*)(A)>{+[](A x) {
            return std::to_string(static_cast<long double>(x + x));
        }},
        BareA{a});
    CHECK_FALSE(applied.value.empty());
}

} // namespace

TEST_CASE("ApplicativeTypeclassTest - PureOptional") {
    const auto &applicative = smd::applicative_typeclass<std::optional<int>>;
    auto lifted = applicative.pure(7);
    REQUIRE(lifted.has_value());
    CHECK(*lifted == 7);
}

TEST_CASE("ApplicativeTypeclassTest - ApplyOptional") {
    std::optional<int (*)(int)> function{+[](int x) { return x + 3; }};
    std::optional<int> argument{4};
    const auto &applicative =
        smd::applicative_typeclass<std::optional<int (*)(int)>>;

    auto result = applicative.apply(function, argument);
    REQUIRE(result.has_value());
    CHECK(*result == 7);
}

TEST_CASE("ApplicativeTypeclassTest - InvokeOptional") {
    // f6c2b5e1-9a3d-4f8c-b2e6-1d9c5b3f7a02
    std::optional<int> ax{10};
    std::optional<int> ay{5};
    const auto &applicative = smd::applicative_typeclass<std::optional<int>>;

    auto result =
        applicative.invoke([](int a, int b) { return a - b; }, ax, ay);
    REQUIRE(result.has_value());
    CHECK(*result == 5);
    // f6c2b5e1-9a3d-4f8c-b2e6-1d9c5b3f7a02 end
}

TEST_CASE(
    "ApplicativeTypeclassTest - InvokeOptionalTernaryUsesPartialApplication") {
    std::optional<int> ax{2};
    std::optional<int> ay{3};
    std::optional<int> az{4};
    const auto &applicative = smd::applicative_typeclass<std::optional<int>>;

    auto result = applicative.invoke(
        [](int a, int b, int c) { return a * b + c; }, ax, ay, az);
    REQUIRE(result.has_value());
    CHECK(*result == 10);
}

TEST_CASE("ApplicativeTypeclassTest - ApplyPureOptionalTernary") {
    // 6e8bde7b-a9f1-4c98-8f1a-807d9ee0a93b
    std::optional<int> ax{2};
    std::optional<int> ay{3};
    std::optional<int> az{4};
    const auto &applicative = smd::applicative_typeclass<std::optional<int>>;

    auto result = applicative.apply_pure(
        [](int a, int b, int c) { return a * b + c; }, ax, ay, az);
    REQUIRE(result.has_value());
    CHECK(*result == 10);
    // 6e8bde7b-a9f1-4c98-8f1a-807d9ee0a93b end
}

TEST_CASE("ApplicativeTypeclassTest - MapOptional") {
    std::optional<int> value{21};
    const auto &applicative = smd::applicative_typeclass<std::optional<int>>;

    auto result = applicative.map([](int x) { return x * 2; }, value);
    REQUIRE(result.has_value());
    CHECK(*result == 42);
}

TEST_CASE("ApplicativeTypeclassTest - InvokeWithExplicitMap") {
    std::optional<int> ax{10};
    std::optional<int> ay{5};
    const auto &default_applicative =
        smd::applicative_typeclass<std::optional<int>>;
    const auto &optional_applicative =
        smd::applicative_typeclass<std::optional<int>>;

    auto result = default_applicative.invoke_with(
        optional_applicative, [](int a, int b) { return a + b; }, ax, ay);
    REQUIRE(result.has_value());
    CHECK(*result == 15);
}

TEST_CASE("ApplicativeTypeclassTest - OptionalEmptyPaths") {
    const auto &applicative = smd::applicative_typeclass<std::optional<int>>;

    std::optional<int (*)(int)> no_function{};
    std::optional<int> argument{4};
    auto no_function_result = applicative.apply(no_function, argument);
    CHECK_FALSE(no_function_result.has_value());

    std::optional<int (*)(int)> function{+[](int x) { return x + 3; }};
    std::optional<int> no_argument{};
    auto no_argument_result = applicative.apply(function, no_argument);
    CHECK_FALSE(no_argument_result.has_value());

    // b4a8c2f7-6d3e-4c1b-9f5a-7e2d4b8a6c09
    std::optional<int> ax{1};
    std::optional<int> ay{};
    auto invoke_result =
        applicative.invoke([](int a, int b) { return a + b; }, ax, ay);
    CHECK_FALSE(invoke_result.has_value());
    // b4a8c2f7-6d3e-4c1b-9f5a-7e2d4b8a6c09 end
}

TEST_CASE("ApplicativeTypeclassTest - DerivedOperations") {
    const auto &applicative = smd::applicative_typeclass<std::optional<int>>;

    auto lifted = applicative.lift(9);
    REQUIRE(lifted.has_value());
    CHECK(*lifted == 9);

    std::optional<int (*)(int)> function{+[](int x) { return x * 3; }};
    auto applied = applicative.ap(function, std::optional<int>{7});
    REQUIRE(applied.has_value());
    CHECK(*applied == 21);

    auto zipped =
        applicative.zip_with([](int a, int b) { return a * b; },
                             std::optional<int>{6}, std::optional<int>{5});
    REQUIRE(zipped.has_value());
    CHECK(*zipped == 30);

    auto keep_right =
        applicative.discard_first(std::optional<int>{1}, std::optional<int>{2});
    REQUIRE(keep_right.has_value());
    CHECK(*keep_right == 2);

    auto keep_left = applicative.discard_second(std::optional<int>{1},
                                                std::optional<int>{2});
    REQUIRE(keep_left.has_value());
    CHECK(*keep_left == 1);
}

TEST_CASE("ApplicativeTypeclassTest - InvokeWithNttpMap") {
    const auto &default_applicative =
        smd::applicative_typeclass<std::optional<int>>;

    auto result =
        default_applicative
            .invoke_with<smd::applicative_typeclass<std::optional<int>>>(
                [](int a, int b, int c) { return a + b + c; },
                std::optional<int>{1}, std::optional<int>{2},
                std::optional<int>{3});
    REQUIRE(result.has_value());
    CHECK(*result == 6);

    auto apply_pure_result =
        default_applicative
            .apply_pure_with<smd::applicative_typeclass<std::optional<int>>>(
                [](int a, int b) { return a - b; }, std::optional<int>{8},
                std::optional<int>{5});
    REQUIRE(apply_pure_result.has_value());
    CHECK(*apply_pure_result == 3);
}

TEST_CASE("ApplicativeTypeclassTest - BemanOptional") {
    using BemanOptional = beman::optional::optional<int>;
    const auto &applicative = smd::applicative_typeclass<BemanOptional>;

    auto lifted = applicative.pure(11);
    REQUIRE(lifted.has_value());
    CHECK(*lifted == 11);

    beman::optional::optional<int (*)(int)> function{
        +[](int x) { return x + 5; }};
    BemanOptional argument{7};
    auto applied = applicative.apply(function, argument);
    REQUIRE(applied.has_value());
    CHECK(*applied == 12);

    beman::optional::optional<int (*)(int)> no_function{};
    auto no_function_applied = applicative.apply(no_function, argument);
    CHECK_FALSE(no_function_applied.has_value());

    BemanOptional no_argument{};
    auto no_argument_applied = applicative.apply(function, no_argument);
    CHECK_FALSE(no_argument_applied.has_value());

    auto invoked = applicative.invoke([](int a, int b) { return a * b; },
                                      BemanOptional{3}, BemanOptional{4});
    REQUIRE(invoked.has_value());
    CHECK(*invoked == 12);

    auto empty_invoked = applicative.invoke([](int a, int b) { return a * b; },
                                            BemanOptional{}, BemanOptional{4});
    CHECK_FALSE(empty_invoked.has_value());
}

TEST_CASE("ApplicativeTypeclassTest - ApplyPureWithExplicitMap") {
    const auto &default_applicative =
        smd::applicative_typeclass<std::optional<int>>;
    const auto &optional_applicative =
        smd::applicative_typeclass<std::optional<int>>;

    auto result = default_applicative.apply_pure_with(
        optional_applicative, [](int a, int b, int c) { return a + b + c; },
        std::optional<int>{4}, std::optional<int>{5}, std::optional<int>{6});
    REQUIRE(result.has_value());
    CHECK(*result == 15);
}

TEST_CASE("ApplicativeTypeclassTest - TerminatingPartialExtendsAndInvokes") {
    // c9f3b1a7-4e8d-4c2a-b6f1-7d3e9c5b2a48
    auto partial = smd::detail::make_terminating_partial(
        [](int a, int b, int c) { return a * 100 + b * 10 + c; });

    auto partial2 = partial(1);
    auto partial3 = partial2(2);
    CHECK(partial3(3) == 123);
    // c9f3b1a7-4e8d-4c2a-b6f1-7d3e9c5b2a48 end

    const auto const_partial = smd::detail::make_terminating_partial(
        [](int a, int b) { return a - b; });
    auto const_partial2 = const_partial(9);
    const auto const_partial3 = const_partial2;
    CHECK(const_partial3(4) == 5);
}

TEST_CASE("ApplicativeTypeclassTest - IdentityMapUsesDerivedInvokePath") {
    using Identity = smd::typeclass::test::Identity<int>;
    const auto &applicative = smd::applicative_typeclass<Identity>;

    auto binary = applicative.invoke([](int a, int b) { return a + b; },
                                     Identity{2}, Identity{3});
    CHECK(binary.value == 5);

    auto ternary = applicative.apply_pure(
        [](int a, int b, int c) { return a * 100 + b * 10 + c; }, Identity{1},
        Identity{2}, Identity{3});
    CHECK(ternary.value == 123);
}

TEST_CASE("ApplicativeTypeclassTest - CustomInvokeDispatchPath") {
    const auto &default_applicative =
        smd::applicative_typeclass<std::optional<int>>;

    auto result = default_applicative.invoke_with(
        direct_invoke_map, [](int a, int b, int c) { return a + b + c; },
        smd::typeclass::test::Identity<int>{4},
        smd::typeclass::test::Identity<int>{5},
        smd::typeclass::test::Identity<int>{6});
    CHECK(result.value == 15);

    auto nttp_result = default_applicative.invoke_with<direct_invoke_map>(
        [](int a, int b) { return a * b; },
        smd::typeclass::test::Identity<int>{7},
        smd::typeclass::test::Identity<int>{8});
    CHECK(nttp_result.value == 56);
}

TEST_CASE(
    "ApplicativeTypeclassTest - OptionalAndBemanVectorInstantiationPaths") {
    const auto &optional_applicative =
        smd::applicative_typeclass<std::optional<std::vector<int>>>;

    auto lifted_vector = optional_applicative.pure(std::vector<int>{1, 2, 3});
    REQUIRE(lifted_vector.has_value());
    CHECK(lifted_vector->size() == 3);

    std::optional<std::vector<int> (*)(std::vector<int>)> append_value{
        +[](std::vector<int> v) {
            v.push_back(4);
            return v;
        }};
    auto applied_vector =
        optional_applicative.apply(append_value, lifted_vector);
    REQUIRE(applied_vector.has_value());
    CHECK(applied_vector->size() == 4);

    using BemanVectorOptional = beman::optional::optional<std::vector<int>>;
    const auto &beman_applicative =
        smd::applicative_typeclass<BemanVectorOptional>;

    auto beman_lifted = beman_applicative.pure(std::vector<int>{8, 9});
    REQUIRE(beman_lifted.has_value());
    CHECK(beman_lifted->size() == 2);

    beman::optional::optional<std::vector<int> (*)(std::vector<int>)>
        beman_append{+[](std::vector<int> v) {
            v.push_back(10);
            return v;
        }};
    auto beman_applied = beman_applicative.apply(beman_append, beman_lifted);
    REQUIRE(beman_applied.has_value());
    CHECK(beman_applied->size() == 3);
}

TEST_CASE("ApplicativeTypeclassTest - IdentityWrapperMethods") {
    using Identity = smd::typeclass::test::Identity<int>;
    const auto &applicative = smd::applicative_typeclass<Identity>;

    auto mapped = applicative.map([](int x) { return x + 1; }, Identity{9});
    CHECK(mapped.value == 10);

    auto zipped = applicative.zip_with([](int a, int b) { return a - b; },
                                       Identity{20}, Identity{3});
    CHECK(zipped.value == 17);

    auto ap_result =
        applicative.ap(smd::typeclass::test::Identity<int (*)(int)>{+[](int x) {
                           return x * 5;
                       }},
                       Identity{6});
    CHECK(ap_result.value == 30);
}

TEST_CASE("ApplicativeTypeclassTest - BareIdentityInvokeAndApplyChain") {
    using BareIdentity = smd::typeclass::test::BareIdentity<int>;
    const auto &applicative = smd::applicative_typeclass<BareIdentity>;

    auto unary =
        applicative.invoke([](int x) { return x + 1; }, BareIdentity{4});
    CHECK(unary.value == 5);

    auto ternary =
        applicative.invoke([](int a, int b, int c) { return a * b + c; },
                           BareIdentity{2}, BareIdentity{3}, BareIdentity{4});
    CHECK(ternary.value == 10);

    auto quaternary = applicative.apply_pure(
        [](int a, int b, int c, int d) { return a + b + c + d; },
        BareIdentity{1}, BareIdentity{2}, BareIdentity{3}, BareIdentity{4});
    CHECK(quaternary.value == 10);
}

TEST_CASE("ApplicativeTypeclassTest - BareIdentityWrapperCoverage") {
    using BareIdentity = smd::typeclass::test::BareIdentity<int>;
    const auto &applicative = smd::applicative_typeclass<BareIdentity>;

    auto lifted = applicative.lift(33);
    CHECK(lifted.value == 33);

    auto mapped =
        applicative.map([](int x) { return x * 2; }, BareIdentity{11});
    CHECK(mapped.value == 22);

    auto applied = applicative.ap(
        smd::typeclass::test::BareIdentity<int (*)(int)>{
            +[](int x) { return x - 2; }},
        BareIdentity{9});
    CHECK(applied.value == 7);

    auto zipped = applicative.zip_with([](int a, int b) { return a - b; },
                                       BareIdentity{40}, BareIdentity{8});
    CHECK(zipped.value == 32);

    auto keep_right =
        applicative.discard_first(BareIdentity{5}, BareIdentity{6});
    CHECK(keep_right.value == 6);

    auto keep_left =
        applicative.discard_second(BareIdentity{5}, BareIdentity{6});
    CHECK(keep_left.value == 5);
}

TEST_CASE("ApplicativeTypeclassTest - BareIdentityInvokeWithMapCoverage") {
    using BareIdentity = smd::typeclass::test::BareIdentity<int>;
    const auto &default_applicative =
        smd::applicative_typeclass<std::optional<int>>;
    const auto &bare_identity_applicative =
        smd::applicative_typeclass<BareIdentity>;

    auto explicit_map_result = default_applicative.invoke_with(
        bare_identity_applicative,
        [](int a, int b, int c) { return a + b + c; }, BareIdentity{3},
        BareIdentity{4}, BareIdentity{5});
    CHECK(explicit_map_result.value == 12);

    auto explicit_apply_pure_result = default_applicative.apply_pure_with(
        bare_identity_applicative, [](int a, int b) { return a * b; },
        BareIdentity{7}, BareIdentity{6});
    CHECK(explicit_apply_pure_result.value == 42);

    auto nttp_map_result =
        default_applicative.invoke_with<bare_identity_applicative>(
            [](int a, int b) { return a - b; }, BareIdentity{20},
            BareIdentity{9});
    CHECK(nttp_map_result.value == 11);

    auto nttp_apply_pure_result =
        default_applicative.apply_pure_with<bare_identity_applicative>(
            [](int a, int b, int c) { return a + b * c; }, BareIdentity{2},
            BareIdentity{3}, BareIdentity{4});
    CHECK(nttp_apply_pure_result.value == 14);
}

TEST_CASE("ApplicativeTypeclassTest - BareIdentityTypeMatrixCoverage") {
    run_bare_identity_matrix_case<int, short, unsigned>(3, 4, 5U);
    run_bare_identity_matrix_case<long, int, long long>(10L, 20, 30LL);
    run_bare_identity_matrix_case<float, double, int>(1.5F, 2.25, 3);
}

TEST_CASE("ApplicativeBehaviorTest - OptionalIdentityHomomorphismAndInvoke") {
    CHECK(smd::typeclass::test::check_applicative_identity_law(
        std::optional<int>{8}));
    CHECK(smd::typeclass::test::check_applicative_homomorphism_law<
          std::optional<int>>(+[](int x) { return x + 3; }, 5));
    CHECK(smd::typeclass::test::check_applicative_invoke_binary_law(
        [](int a, int b) { return a * 10 + b; }, std::optional<int>{2},
        std::optional<int>{7}));
}

TEST_CASE(
    "ApplicativeBehaviorTest - BareIdentityIdentityHomomorphismAndInvoke") {
    using BareIdentity = smd::typeclass::test::BareIdentity<int>;
    CHECK(
        smd::typeclass::test::check_applicative_identity_law(BareIdentity{11}));
    CHECK(
        smd::typeclass::test::check_applicative_homomorphism_law<BareIdentity>(
            +[](int x) { return x * 4; }, 3));
    CHECK(smd::typeclass::test::check_applicative_invoke_binary_law(
        [](int a, int b) { return a - b; }, BareIdentity{20}, BareIdentity{6}));
}

TEST_CASE("ApplicativeBehaviorTest - BemanIdentityHomomorphismAndInvoke") {
    using BemanOptional = beman::optional::optional<int>;

    CHECK(smd::typeclass::test::check_applicative_identity_law(
        BemanOptional{11}));
    CHECK(
        smd::typeclass::test::check_applicative_homomorphism_law<BemanOptional>(
            +[](int x) { return x * 4; }, 3));
    CHECK(smd::typeclass::test::check_applicative_invoke_binary_law(
        [](int a, int b) { return a - b; }, BemanOptional{20},
        BemanOptional{6}));
}

TEST_CASE("ApplicativeBehaviorTest - OptionalShortCircuit") {
    const auto &applicative = smd::applicative_typeclass<std::optional<int>>;

    std::optional<std::function<int(int)>> no_function{};
    auto no_function_result =
        applicative.ap(no_function, std::optional<int>{4});
    CHECK_FALSE(no_function_result.has_value());

    std::optional<std::function<int(int)>> function{
        [](int x) { return x + 1; }};
    auto no_argument_result = applicative.ap(function, std::optional<int>{});
    CHECK_FALSE(no_argument_result.has_value());

    int calls = 0;
    auto invoke_result = applicative.invoke(
        [&calls](int lhs, int rhs) {
            ++calls;
            return lhs + rhs;
        },
        std::optional<int>{3}, std::optional<int>{});
    CHECK_FALSE(invoke_result.has_value());
    CHECK(calls == 0);
}

TEST_CASE("ApplicativeLaws - InterchangeLaw") {
    // Interchange: ap(u, pure(y)) == ap(pure(λf. f(y)), u)
    // Ensures that applying a contextual function to a pure value is symmetric.
    using Fn = std::function<int(int)>;
    const int y = 7;

    {
        // b8e3d6a1-2c5f-4b7e-a2d8-7f6c2b3e5d15
        const auto &ap = smd::applicative_typeclass<std::optional<int>>;
        std::optional<Fn> u{[](int x) { return x * 3; }};

        auto lhs = ap.ap(u, ap.pure(y));
        auto rhs = ap.ap(ap.pure([](const Fn &fn) { return fn(y); }), u);

        REQUIRE(lhs.has_value());
        CHECK(*lhs == 21);
        CHECK(lhs == rhs);
        // b8e3d6a1-2c5f-4b7e-a2d8-7f6c2b3e5d15 end
    }
    {
        // empty function: both sides propagate the absence
        const auto &ap = smd::applicative_typeclass<std::optional<int>>;
        std::optional<Fn> empty{};
        auto lhs = ap.ap(empty, ap.pure(y));
        auto rhs = ap.ap(ap.pure([](const Fn &fn) { return fn(y); }), empty);
        CHECK_FALSE(lhs.has_value());
        CHECK(lhs == rhs);
    }
    {
        using BemanFn = beman::optional::optional<Fn>;
        const auto &ap =
            smd::applicative_typeclass<beman::optional::optional<int>>;
        BemanFn u{[](int x) { return x + 8; }};

        auto lhs = ap.ap(u, ap.pure(y));
        auto rhs = ap.ap(ap.pure([](const Fn &fn) { return fn(y); }), u);

        REQUIRE(lhs.has_value());
        CHECK(*lhs == 15);
        CHECK(lhs == rhs);
    }
    {
        using BI = smd::typeclass::test::BareIdentity<int>;
        using BIFn = smd::typeclass::test::BareIdentity<Fn>;
        const auto &ap = smd::applicative_typeclass<BI>;
        BIFn u{[](int x) { return x - 2; }};

        auto lhs = ap.ap(u, ap.pure(y));
        auto rhs = ap.ap(ap.pure([](const Fn &fn) { return fn(y); }), u);

        CHECK(lhs.value == 5);
        CHECK(lhs == rhs);
    }
}

TEST_CASE("ApplicativeLaws - CompositionLaw") {
    // Composition: ap(invoke(∘, u, v), w) == ap(u, ap(v, w))
    // Composing effectful functions then applying equals sequencing the
    // applications.
    using Fn = std::function<int(int)>;
    auto compose = [](const Fn &f, const Fn &g) {
        return Fn{[f, g](int x) { return f(g(x)); }};
    };

    {
        // e2c7f5b3-4a1d-4e8c-b3f5-9d6a5c2e3b02
        const auto &ap = smd::applicative_typeclass<std::optional<int>>;
        std::optional<Fn> u{[](int x) { return x + 10; }};
        std::optional<Fn> v{[](int x) { return x * 2; }};
        std::optional<int> w{3};

        auto lhs = ap.ap(ap.invoke(compose, u, v), w);
        auto rhs = ap.ap(u, ap.ap(v, w));

        REQUIRE(lhs.has_value());
        CHECK(*lhs == 16); // (3 * 2) + 10
        CHECK(lhs == rhs);
        // e2c7f5b3-4a1d-4e8c-b3f5-9d6a5c2e3b02 end
    }
    {
        // empty u propagates to both sides
        const auto &ap = smd::applicative_typeclass<std::optional<int>>;
        std::optional<Fn> empty_u{};
        std::optional<Fn> v{[](int x) { return x * 2; }};
        std::optional<int> w{3};

        auto lhs = ap.ap(ap.invoke(compose, empty_u, v), w);
        auto rhs = ap.ap(empty_u, ap.ap(v, w));
        CHECK_FALSE(lhs.has_value());
        CHECK(lhs == rhs);
    }
    {
        using BI = smd::typeclass::test::BareIdentity<int>;
        using BIFn = smd::typeclass::test::BareIdentity<Fn>;
        const auto &ap = smd::applicative_typeclass<BI>;
        BIFn u{[](int x) { return x + 10; }};
        BIFn v{[](int x) { return x * 2; }};
        BI w{3};

        auto lhs = ap.ap(ap.invoke(compose, u, v), w);
        auto rhs = ap.ap(u, ap.ap(v, w));

        CHECK(lhs.value == 16);
        CHECK(lhs == rhs);
    }
}

TEST_CASE("ApplicativeBehaviorTest - BemanShortCircuit") {
    using BemanOptional = beman::optional::optional<int>;
    const auto &applicative = smd::applicative_typeclass<BemanOptional>;

    beman::optional::optional<std::function<int(int)>> no_function{};
    auto no_function_result = applicative.ap(no_function, BemanOptional{5});
    CHECK_FALSE(no_function_result.has_value());

    beman::optional::optional<std::function<int(int)>> function{
        [](int x) { return x * 2; }};
    auto no_argument_result = applicative.ap(function, BemanOptional{});
    CHECK_FALSE(no_argument_result.has_value());

    int calls = 0;
    auto invoke_result = applicative.invoke(
        [&calls](int lhs, int rhs) {
            ++calls;
            return lhs - rhs;
        },
        BemanOptional{9}, BemanOptional{});
    CHECK_FALSE(invoke_result.has_value());
    CHECK(calls == 0);
}

TEST_CASE(
    "ApplicativeBehaviorTest - InvokeDispatchThroughBaseAndDerivedPaths") {
    DirectInvokeIdentityApplicativeMap<int> custom_map{};
    auto &custom_base = static_cast<
        smd::Applicative<DirectInvokeIdentityApplicativeImpl<int>> &>(
        custom_map);

    auto custom_dispatched =
        custom_base.invoke([](int a, int b, int c) { return a + b + c; },
                           smd::typeclass::test::Identity<int>{1},
                           smd::typeclass::test::Identity<int>{2},
                           smd::typeclass::test::Identity<int>{3});
    CHECK(custom_dispatched.value == 6);

    smd::BareIdentityApplicativeMap<int> bare_map{};
    auto &bare_base =
        static_cast<smd::Applicative<smd::BareIdentityApplicativeImpl<int>> &>(
            bare_map);

    auto derived_dispatched = bare_base.invoke(
        [](int a, int b, int c) { return a * 100 + b * 10 + c; },
        smd::typeclass::test::BareIdentity<int>{4},
        smd::typeclass::test::BareIdentity<int>{5},
        smd::typeclass::test::BareIdentity<int>{6});
    CHECK(derived_dispatched.value == 456);
}

TEST_CASE("ApplicativeBehaviorTest - BareIdentityConstAndNonConstInvokeApMap") {
    smd::BareIdentityApplicativeMap<int> mutable_map{};
    auto &mutable_base =
        static_cast<smd::Applicative<smd::BareIdentityApplicativeImpl<int>> &>(
            mutable_map);

    auto non_const_invoke =
        mutable_base.invoke([](int a, int b) { return a + b; },
                            smd::typeclass::test::BareIdentity<int>{10},
                            smd::typeclass::test::BareIdentity<int>{4});
    CHECK(non_const_invoke.value == 14);

    auto non_const_map =
        mutable_base.map([](int x) { return x * 3; },
                         smd::typeclass::test::BareIdentity<int>{7});
    CHECK(non_const_map.value == 21);

    auto non_const_ap = mutable_base.ap(
        smd::typeclass::test::BareIdentity<std::function<int(int)>>{
            [](int x) { return x - 5; }},
        smd::typeclass::test::BareIdentity<int>{12});
    CHECK(non_const_ap.value == 7);

    const smd::BareIdentityApplicativeMap<int> const_map{};
    const auto &const_base = static_cast<
        const smd::Applicative<smd::BareIdentityApplicativeImpl<int>> &>(
        const_map);

    auto const_invoke =
        const_base.invoke([](int a, int b, int c) { return a * b + c; },
                          smd::typeclass::test::BareIdentity<int>{3},
                          smd::typeclass::test::BareIdentity<int>{5},
                          smd::typeclass::test::BareIdentity<int>{2});
    CHECK(const_invoke.value == 17);

    auto const_map_result =
        const_base.map([](int x) { return x + 8; },
                       smd::typeclass::test::BareIdentity<int>{1});
    CHECK(const_map_result.value == 9);

    auto const_ap_result = const_base.ap(
        smd::typeclass::test::BareIdentity<std::function<int(int)>>{
            [](int x) { return x * x; }},
        smd::typeclass::test::BareIdentity<int>{6});
    CHECK(const_ap_result.value == 36);
}

```

## smd/typeclass/examples/applicative_bad.cpp

```cpp
#include <smd/typeclass/examples/examples.hpp>

#include <smd/tree/fringe_tree.hpp>

#include <cstddef>
#include <utility>

namespace smd::typeclass::examples {

template <class LEFT, class RIGHT>
auto cartesian_product(const smd::tree::FringeTree<LEFT> &,
                       const smd::tree::FringeTree<RIGHT> &)
    -> smd::tree::FringeTree<std::pair<LEFT, RIGHT>> {
    using PairTree = smd::tree::FringeTree<std::pair<LEFT, RIGHT>>;
    return PairTree::leaf(std::pair<LEFT, RIGHT>{});
}

auto bad_applicative_example() -> std::size_t {
    using IntTree = smd::tree::FringeTree<int>;
    auto tx = IntTree::branch(IntTree::leaf(1), IntTree::leaf(2));
    auto ty = IntTree::leaf(3);

    // d2e7a1c9-0f3b-4b2e-9d55-1a8e7c4b2f90
    // Hypothetical: expands structure instead of preserving shape.
    auto bad = cartesian_product(tx, ty);
    // d2e7a1c9-0f3b-4b2e-9d55-1a8e7c4b2f90 end

    return bad.is_leaf() ? 1U : 0U;
}

} // namespace smd::typeclass::examples

```

## smd/typeclass/examples/applicative_examples.cpp

```cpp
#include <smd/typeclass/examples/examples.hpp>

#include <smd/typeclass/applicative.hpp>

#include <beman/optional/optional.hpp>

namespace smd::typeclass::examples {

auto applicative_invoke_example() -> beman::optional::optional<int> {
    using beman::optional::optional;
    const auto &applicative = smd::applicative_typeclass<optional<int>>;

    optional<int> ax = 1;
    optional<int> ay = 2;
    optional<int> az = 3;

    // 3f0c8d0e-9a6b-4a3e-9c2a-0c1e9c3d4f11
    auto sum = applicative.invoke([](int a, int b, int c) { return a + b + c; },
                                  ax, ay, az);
    // 3f0c8d0e-9a6b-4a3e-9c2a-0c1e9c3d4f11 end

    return sum;
}

} // namespace smd::typeclass::examples

```

## smd/typeclass/examples/examples.hpp

```cpp
// src/smd/typeclass/examples/examples.hpp                            -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TYPECLASS_EXAMPLES_EXAMPLES
#define INCLUDED_SMD_TYPECLASS_EXAMPLES_EXAMPLES

#include <beman/optional/optional.hpp>

#include <cstddef>
#include <optional>

namespace smd::typeclass::examples {

/** Count elements in a range using the generic Foldable `length`. */
auto generic_length_example() -> std::size_t;
/** Count elements in a BinaryTree using the generic Foldable `length`. */
auto generic_length_binary_tree_example() -> std::size_t;
/** Count elements in a FringeTree using the generic Foldable `length`. */
auto generic_length_fringe_tree_example() -> std::size_t;
/** Demonstrate that `fold_map` linearises tree shape (order is depth-first). */
auto foldable_flattens_shape_example() -> bool;
/** Lift a binary function into optional using `invoke`; returns empty if any arg is empty. */
auto applicative_invoke_example() -> beman::optional::optional<int>;
/** Relabel tree nodes with sequential indices by traversing through optional. */
auto traversable_relabel_example() -> beman::optional::optional<std::size_t>;
/** Verify that `traverse` rebuilds a tree of the same shape as the input. */
auto traversable_preserves_shape_example() -> bool;
/** Illustrate a semantically wrong tree Applicative that compiles but violates laws. */
auto bad_applicative_example() -> std::size_t;
/** Call a generic algorithm passing the typeclass object explicitly at the call site. */
auto explicit_object_lookup_example() -> std::optional<int>;
/** Call a generic algorithm with the typeclass object bound as an NTTP default. */
auto nttp_object_lookup_example() -> std::optional<int>;

} // namespace smd::typeclass::examples

#endif // INCLUDED_SMD_TYPECLASS_EXAMPLES_EXAMPLES

```

## smd/typeclass/examples/examples.t.cpp

```cpp
#include <smd/typeclass/examples/examples.hpp>
#include <smd/typeclass/examples/examples.hpp> // Re-inclusion check

#include <catch2/catch_test_macros.hpp>

TEST_CASE("TypeclassExamples - SlideExamplesRemainExecutable") {
    using namespace smd::typeclass::examples;

    CHECK(generic_length_example() == 3U);
    CHECK(generic_length_binary_tree_example() == 4U);
    CHECK(generic_length_fringe_tree_example() == 3U);
    auto applicative_result = applicative_invoke_example();
    REQUIRE(applicative_result);
    CHECK(*applicative_result == 6);
    auto traversable_result = traversable_relabel_example();
    REQUIRE(traversable_result);
    CHECK(*traversable_result == 2U);
    CHECK(traversable_preserves_shape_example());
    CHECK(foldable_flattens_shape_example());
    CHECK(bad_applicative_example() == 1U);

    auto explicit_lookup = explicit_object_lookup_example();
    REQUIRE(explicit_lookup);
    CHECK(*explicit_lookup == 42);

    auto nttp_lookup = nttp_object_lookup_example();
    REQUIRE(nttp_lookup);
    CHECK(*nttp_lookup == 10);
}

```

## smd/typeclass/examples/foldable_examples.cpp

```cpp
#include <smd/typeclass/examples/examples.hpp>

#include <smd/tree/binary_tree.hpp>
#include <smd/tree/binary_tree_foldable.hpp>
#include <smd/tree/fixpoint_tree.hpp>
#include <smd/tree/fixpoint_tree_foldable.hpp>
#include <smd/tree/fringe_tree.hpp>
#include <smd/tree/fringe_tree_foldable.hpp>
#include <smd/typeclass/foldable.hpp>

namespace smd::typeclass::examples {

using smd::tree::add_expr;
using smd::tree::const_expr;
using smd::tree::Expr;
using smd::tree::mul_expr;

auto generic_length_example() -> std::size_t {
    auto tree =
        add_expr(const_expr(1.0), add_expr(const_expr(2.0), const_expr(3.0)));
    const auto &foldable = smd::foldable_typeclass<Expr>;

    // 9a1c4e2b-2c7e-4b1a-9f55-8b6a4d2e91aa
    auto n = foldable.length(tree);
    // 9a1c4e2b-2c7e-4b1a-9f55-8b6a4d2e91aa end

    return n;
}

auto generic_length_binary_tree_example() -> std::size_t {
    using IntBinaryTree = smd::tree::BinaryTree<int>;
    auto tree = IntBinaryTree::from_children_ptrs(
        2, IntBinaryTree::make_ptr(IntBinaryTree::leaf(1)),
        IntBinaryTree::make_ptr(IntBinaryTree::from_children_ptrs(
            3, {}, IntBinaryTree::make_ptr(IntBinaryTree::leaf(4)))));

    const auto &foldable = smd::foldable_typeclass<IntBinaryTree>;

    // 53b9f5b4-3b3a-4e18-9b3c-07b7e2c980f4
    auto n = foldable.length(tree);
    // 53b9f5b4-3b3a-4e18-9b3c-07b7e2c980f4 end

    return n;
}

auto generic_length_fringe_tree_example() -> std::size_t {
    using Fringe = smd::tree::FringeTree<int>;
    auto tree = Fringe::branch(Fringe::branch(Fringe::leaf(1), Fringe::leaf(2)),
                               Fringe::leaf(3));

    const auto &foldable = smd::foldable_typeclass<Fringe>;

    // 7c2f11d9-ef09-45e2-80da-9229f3c8d82c
    auto n = foldable.length(tree);
    // 7c2f11d9-ef09-45e2-80da-9229f3c8d82c end

    return n;
}

auto foldable_flattens_shape_example() -> bool {
    // Two differently-structured expressions with the same leaf constants.
    auto right_deep =
        add_expr(const_expr(1.0), add_expr(const_expr(2.0), const_expr(3.0)));
    auto left_deep =
        add_expr(add_expr(const_expr(1.0), const_expr(2.0)), const_expr(3.0));

    const auto &foldable = smd::foldable_typeclass<Expr>;

    // b1fd4b92-b060-4c47-8c08-97328ec02329
    auto right_flat = foldable.to_vector(right_deep);
    auto left_flat = foldable.to_vector(left_deep);
    // b1fd4b92-b060-4c47-8c08-97328ec02329 end

    return right_flat == left_flat;
}

} // namespace smd::typeclass::examples

```

## smd/typeclass/examples/lookup_modes_examples.cpp

```cpp
#include <smd/typeclass/examples/examples.hpp>

#include <smd/typeclass/functor.hpp>

#include <optional>
#include <type_traits>
#include <utility>

namespace smd::typeclass::examples {

struct OptionalFunctorObject {
    template <class F, class T>
    auto fmap(F &&function, const std::optional<T> &value) const {
        const auto &functor = smd::functor_typeclass<std::optional<T>>;
        return functor.fmap(std::forward<F>(function), value);
    }
};

template <class CONTEXT>
inline constexpr auto functor_typeclass = std::false_type{};

template <class T>
inline constexpr auto functor_typeclass<std::optional<T>> =
    OptionalFunctorObject{};

template <class CONTEXT, const auto &FUNCTOR = functor_typeclass<CONTEXT>>
auto fmap_plus_one_nttp(const CONTEXT &value) {
    return FUNCTOR.fmap([](int x) { return x + 1; }, value);
}

auto explicit_object_lookup_example() -> std::optional<int> {
    OptionalFunctorObject functor;
    std::optional<int> value{41};

    return functor.fmap([](int x) { return x + 1; }, value);
}

auto nttp_object_lookup_example() -> std::optional<int> {
    std::optional<int> value{9};
    return fmap_plus_one_nttp(value);
}

} // namespace smd::typeclass::examples

```

## smd/typeclass/examples/traversable_examples.cpp

```cpp
#include <smd/typeclass/examples/examples.hpp>

#include <smd/tree/fixpoint_tree.hpp>
#include <smd/tree/fixpoint_tree_foldable.hpp>
#include <smd/tree/fixpoint_tree_traversable.hpp>
#include <smd/tree/fringe_tree.hpp>
#include <smd/tree/fringe_tree_foldable.hpp>
#include <smd/tree/fringe_tree_traversable.hpp>
#include <smd/typeclass/applicative.hpp>
#include <smd/typeclass/traversable.hpp>

#include <beman/optional/optional.hpp>

namespace smd::typeclass::examples {

using smd::tree::add_expr;
using smd::tree::const_expr;
using smd::tree::eval;
using smd::tree::Expr;

auto traversable_relabel_example() -> beman::optional::optional<std::size_t> {
    using Fringe = smd::tree::FringeTree<int>;
    auto tree = Fringe::branch(Fringe::leaf(1), Fringe::leaf(2));

    // 5c6b2d3e-7a44-4c8a-9c31-3d1e2a9b77c2
    using beman::optional::optional;

    auto relabelled = smd::traverse(
        [](int x) -> optional<int> {
            return x >= 0 ? optional<int>{x + 1} : optional<int>{};
        },
        tree);
    // 5c6b2d3e-7a44-4c8a-9c31-3d1e2a9b77c2 end

    if (!relabelled) {
        return {};
    }

    const auto &foldable = smd::foldable_typeclass<Fringe>;
    return foldable.length(*relabelled);
}

auto traversable_preserves_shape_example() -> bool {
    // (1 + (2 + 3)) — traverse maps each constant, rebuilding the same Expr
    // shape.
    auto tree =
        add_expr(const_expr(1.0), add_expr(const_expr(2.0), const_expr(3.0)));

    using beman::optional::optional;

    // d804ec63-77d1-4fa0-99a6-9effce6f741b
    auto mapped = smd::traverse(
        [](double x) -> optional<double> { return optional<double>{x + 10.0}; },
        tree);
    // d804ec63-77d1-4fa0-99a6-9effce6f741b end

    if (!mapped) {
        return false;
    }

    // Shape is preserved: (11 + (12 + 13)) = 36
    return eval(*mapped) == 36.0;
}

} // namespace smd::typeclass::examples

```

## smd/typeclass/foldable.hpp

```cpp
// src/smd/typeclass/foldable.hpp                                     -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TYPECLASS_FOLDABLE
#define INCLUDED_SMD_TYPECLASS_FOLDABLE

#include <smd/typeclass/monoid.hpp>
#include <smd/typeclass/typeclass_base.hpp>

#include <cstddef>
#include <functional>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

namespace smd::detail {

// Identity function for fold composition - no type erasure needed
template <class STATE>
struct IdentityFoldFunc {
    constexpr auto operator()(STATE s) const -> STATE { return s; }
};

// Compose two fold functions without type erasure
template <class F1, class F2>
struct ComposedFoldFunc {
    F1 d_f1;
    F2 d_f2;

    template <class STATE>
    auto operator()(STATE s) const -> STATE {
        return d_f2(d_f1(std::move(s)));
    }
};

// Left fold composition (same order as f1 then f2)
template <class STATE>
struct LeftFoldProgram {
    std::function<STATE(STATE)> d_run;

    auto operator()(STATE state) const -> STATE {
        return d_run(std::move(state));
    }
};

// Template-based version that avoids type erasure - used internally
template <class F>
struct LeftFoldProgramT {
    F d_run;

    template <class STATE>
    auto operator()(STATE state) const -> STATE {
        return d_run(std::move(state));
    }
};

template <class STATE>
struct RightFoldProgram {
    std::function<STATE(STATE)> d_run;

    auto operator()(STATE state) const -> STATE {
        return d_run(std::move(state));
    }
};

// Template-based version that avoids type erasure - used internally
template <class F>
struct RightFoldProgramT {
    F d_run;

    template <class STATE>
    auto operator()(STATE state) const -> STATE {
        return d_run(std::move(state));
    }
};

struct Any {
    bool d_value;
};

struct All {
    bool d_value;
};

template <class VALUE_TYPE>
struct First {
    std::optional<VALUE_TYPE> d_value;
};

} // namespace smd::detail

namespace smd::typeclass {

template <class STATE>
struct Monoid<smd::detail::LeftFoldProgram<STATE>> {
    auto identity() const -> smd::detail::LeftFoldProgram<STATE> {
        return smd::detail::LeftFoldProgram<STATE>{[](STATE s) { return s; }};
    }

    auto combine(const smd::detail::LeftFoldProgram<STATE> &lhs,
                 const smd::detail::LeftFoldProgram<STATE> &rhs) const
        -> smd::detail::LeftFoldProgram<STATE> {
        return smd::detail::LeftFoldProgram<STATE>{
            [lhs, rhs](STATE s) { return rhs(lhs(std::move(s))); }};
    }
};

// Template specialization for LeftFoldProgramT - avoids type erasure
template <class F>
struct Monoid<smd::detail::LeftFoldProgramT<F>> {
    auto identity() const
        -> smd::detail::LeftFoldProgramT<smd::detail::IdentityFoldFunc<int>> {
        return smd::detail::LeftFoldProgramT<
            smd::detail::IdentityFoldFunc<int>>{
            smd::detail::IdentityFoldFunc<int>{}};
    }

    template <class G>
    auto combine(const smd::detail::LeftFoldProgramT<F> &lhs,
                 const smd::detail::LeftFoldProgramT<G> &rhs) const
        -> smd::detail::LeftFoldProgramT<smd::detail::ComposedFoldFunc<F, G>> {
        return smd::detail::LeftFoldProgramT<
            smd::detail::ComposedFoldFunc<F, G>>{
            smd::detail::ComposedFoldFunc<F, G>{lhs.d_run, rhs.d_run}};
    }
};

template <class STATE>
struct Monoid<smd::detail::RightFoldProgram<STATE>> {
    auto identity() const -> smd::detail::RightFoldProgram<STATE> {
        return smd::detail::RightFoldProgram<STATE>{[](STATE s) { return s; }};
    }

    auto combine(const smd::detail::RightFoldProgram<STATE> &lhs,
                 const smd::detail::RightFoldProgram<STATE> &rhs) const
        -> smd::detail::RightFoldProgram<STATE> {
        return smd::detail::RightFoldProgram<STATE>{
            [lhs, rhs](STATE s) { return lhs(rhs(std::move(s))); }};
    }
};

// Template specialization for RightFoldProgramT - avoids type erasure
template <class F>
struct Monoid<smd::detail::RightFoldProgramT<F>> {
    auto identity() const
        -> smd::detail::RightFoldProgramT<smd::detail::IdentityFoldFunc<int>> {
        return smd::detail::RightFoldProgramT<
            smd::detail::IdentityFoldFunc<int>>{
            smd::detail::IdentityFoldFunc<int>{}};
    }

    template <class G>
    auto combine(const smd::detail::RightFoldProgramT<F> &lhs,
                 const smd::detail::RightFoldProgramT<G> &rhs) const
        -> smd::detail::RightFoldProgramT<smd::detail::ComposedFoldFunc<F, G>> {
        return smd::detail::RightFoldProgramT<
            smd::detail::ComposedFoldFunc<F, G>>{
            smd::detail::ComposedFoldFunc<F, G>{lhs.d_run, rhs.d_run}};
    }
};

template <>
struct Monoid<smd::detail::Any> {
    constexpr auto identity() const -> smd::detail::Any { return {false}; }

    constexpr auto combine(smd::detail::Any lhs, smd::detail::Any rhs) const
        -> smd::detail::Any {
        return {lhs.d_value || rhs.d_value};
    }
};

template <>
struct Monoid<smd::detail::All> {
    constexpr auto identity() const -> smd::detail::All { return {true}; }

    constexpr auto combine(smd::detail::All lhs, smd::detail::All rhs) const
        -> smd::detail::All {
        return {lhs.d_value && rhs.d_value};
    }
};

template <class VALUE_TYPE>
struct Monoid<smd::detail::First<VALUE_TYPE>> {
    auto identity() const -> smd::detail::First<VALUE_TYPE> { return {{}}; }

    auto combine(const smd::detail::First<VALUE_TYPE> &lhs,
                 const smd::detail::First<VALUE_TYPE> &rhs) const
        -> smd::detail::First<VALUE_TYPE> {
        if (lhs.d_value) {
            return lhs;
        }
        return rhs;
    }
};

} // namespace smd::typeclass

namespace smd {

// Foldable pattern invariants:
// - Generic entry point is fold_map(F, T) via foldable_typeclass<T>.
// - Instances provide an object with fold_map(F, T) and specialize
//   foldable_typeclass<Concrete>.
// - Derived APIs (length, fold_left, fold_right, combine_all, any_of, all_of,
//   empty, to_vector, find_first) live on the same looked-up object.
// - Traversal order is instance-defined but must be coherent per instance.

/** CRTP base for Foldable instances.
 * `Impl` must provide either `fold_map(f, container)` or `fold_right` +
 * `element_type`; all other operations are derived from whichever is the
 * primitive.
 */
template <class Impl>
struct Foldable : protected Impl {
    static_assert(!std::is_same_v<Impl, std::false_type>,
                  "No foldable_typeclass<T> specialization found. "
                  "Specialize smd::foldable_typeclass<T> for your type T "
                  "and provide fold_map(F, T) or fold_right(T, STATE, F) + "
                  "element_type.");
    // Alternate-core: Impl provides either fold_map or fold_right as primitive.
    // The Map class's using-declaration selects which; the base derives the
    // other. Haskell equivalent: {-# MINIMAL foldMap | foldr #-}

    // Derived fold_map from fold_right. Active when a fold_right-primitive
    // Impl's using-declaration shadows the base's derived fold_right with the
    // real one. Requires element_type to deduce the monoid result type. foldMap
    // f = foldr (\x acc -> f x <> acc) mempty
    template <class F, class T>
    auto fold_map(this auto &&self, F &&function, T &&value)
        requires requires { typename Impl::element_type; }
    {
        using Result = remove_cvref_t<
            std::invoke_result_t<F, const typename Impl::element_type &>>;
        return self.fold_right(
            std::forward<T>(value), monoid_identity<Result>(),
            [&function](const auto &elem, Result acc) {
                return monoid_combine(std::invoke(function, elem),
                                      std::move(acc));
            });
    }

    // e3a1b1a2-6adf-4cb9-8c85-c0e39a7b98f2

    // c1e5b4a7-4d3f-4c2b-a7e1-7f9d4c6b3e08
    /** Returns the number of elements in the foldable container. */
    template <class T>
    auto length(this auto &&self, T &&value) -> std::size_t {
        const auto count =
            self.fold_map([](const auto &) { return typeclass::Count{1}; },
                          std::forward<T>(value));
        return count.d_value;
    }
    // c1e5b4a7-4d3f-4c2b-a7e1-7f9d4c6b3e08 end

    /** Left-associative fold: applies `function(state, element)` for each
     * element in traversal order, starting from `initial_state`.
     */
    template <class T, class STATE, class F>
    auto fold_left(this auto &&self, T &&value, STATE initial_state,
                   F &&function) {
        using StateType = remove_cvref_t<STATE>;
        auto step = std::forward<F>(function);

        const auto program = self.fold_map(
            [&step](const auto &x) {
                using ValueType = remove_cvref_t<decltype(x)>;
                return detail::LeftFoldProgram<StateType>{
                    [x_copy = ValueType(x), &step](StateType s) {
                        return std::invoke(step, std::move(s), x_copy);
                    }};
            },
            std::forward<T>(value));

        return program(StateType(std::move(initial_state)));
    }

    /** Right-associative fold: applies `function(element, state)` for each
     * element in reverse traversal order, starting from `initial_state`.
     */
    template <class T, class STATE, class F>
    auto fold_right(this auto &&self, T &&value, STATE initial_state,
                    F &&function) {
        using StateType = remove_cvref_t<STATE>;
        auto step = std::forward<F>(function);

        const auto program = self.fold_map(
            [&step](const auto &x) {
                using ValueType = remove_cvref_t<decltype(x)>;
                return detail::RightFoldProgram<StateType>{
                    [x_copy = ValueType(x), &step](StateType s) {
                        return std::invoke(step, x_copy, std::move(s));
                    }};
            },
            std::forward<T>(value));

        return program(StateType(std::move(initial_state)));
    }

    /** Combines all elements using the Monoid of the element type
     * (requires elements themselves to be Monoid values).
     */
    template <class T>
    auto combine_all(this auto &&self, T &&value) {
        return self.fold_map([](const auto &x) { return x; },
                             std::forward<T>(value));
    }

    /** Alias for `combine_all`. */
    template <class T>
    auto fold(this auto &&self, T &&value) {
        return self.combine_all(std::forward<T>(value));
    }

    /** Returns `true` if any element satisfies `predicate`. */
    template <class T, class PREDICATE>
    auto any_of(this auto &&self, T &&value, PREDICATE &&predicate) -> bool {
        const auto result = self.fold_map(
            [&predicate](const auto &x) {
                return detail::Any{std::invoke(predicate, x)};
            },
            std::forward<T>(value));

        return result.d_value;
    }

    /** Returns `true` if all elements satisfy `predicate`. */
    template <class T, class PREDICATE>
    auto all_of(this auto &&self, T &&value, PREDICATE &&predicate) -> bool {
        const auto result = self.fold_map(
            [&predicate](const auto &x) {
                return detail::All{std::invoke(predicate, x)};
            },
            std::forward<T>(value));

        return result.d_value;
    }

    /** Returns `true` if the container holds no elements. */
    template <class T>
    auto empty(this auto &&self, T &&value) -> bool {
        return !self.any_of(std::forward<T>(value),
                            [](const auto &) { return true; });
    }

    // a6d2c8f3-1e7b-4a5d-b9f4-3c8e2a7d1b09
    /** Collects all elements into a `std::vector` in traversal order. */
    template <class T>
    auto to_vector(this auto &&self, T &&value) {
        return self.fold_map(
            [](const auto &x) {
                using ValueType = remove_cvref_t<decltype(x)>;
                return std::vector<ValueType>{x};
            },
            std::forward<T>(value));
    }
    // a6d2c8f3-1e7b-4a5d-b9f4-3c8e2a7d1b09 end
    // e3a1b1a2-6adf-4cb9-8c85-c0e39a7b98f2 end

    /** Returns the first element satisfying `predicate`, or an empty optional. */
    template <class T, class PREDICATE>
    auto find_first(this auto &&self, T &&value, PREDICATE &&predicate) {
        const auto result = self.fold_map(
            [&predicate](const auto &x) {
                using X = remove_cvref_t<decltype(x)>;
                if (std::invoke(predicate, x)) {
                    return detail::First<X>{{x}};
                }
                return detail::First<X>{{}};
            },
            std::forward<T>(value));

        return result.d_value;
    }
};

/** Typeclass lookup variable for Foldable; specialize for each container type. */
template <class T>
inline constexpr auto foldable_typeclass = std::false_type{};

} // namespace smd

#endif

```

## smd/typeclass/foldable.t.cpp

```cpp
#include <smd/typeclass/foldable.hpp>
#include <smd/typeclass/foldable.hpp> // Re-inclusion check
#include <smd/typeclass/test/test_support.hpp>

#include <catch2/catch_test_macros.hpp>

#include <vector>

namespace {

// a7c3e1f5-8d4b-4a2c-9e7f-3b5d1c8a4e06
template <class STRUCTURE,
          const auto &FOLDABLE = smd::foldable_typeclass<STRUCTURE>>
auto sum_with_nttp_lookup(const STRUCTURE &structure) {
    return FOLDABLE.fold_map([](int x) { return x; }, structure);
}
// a7c3e1f5-8d4b-4a2c-9e7f-3b5d1c8a4e06 end

template <class STRUCTURE,
          const auto &FOLDABLE = smd::foldable_typeclass<STRUCTURE>>
auto fold_left_with_nttp_lookup(const STRUCTURE &structure) {
    return FOLDABLE.fold_left(structure, 0,
                              [](int acc, int x) { return acc * 10 + x; });
}

template <class STRUCTURE,
          const auto &FOLDABLE = smd::foldable_typeclass<STRUCTURE>>
auto fold_right_with_nttp_lookup(const STRUCTURE &structure) {
    return FOLDABLE.fold_right(structure, 0,
                               [](int x, int acc) { return x * 10 + acc; });
}

} // namespace

TEST_CASE("FoldableTypeclassTest - LengthOnSequence") {
    using Sequence = smd::typeclass::test::Sequence<int>;
    auto sequence = Sequence{{1, 2, 3}};

    const auto &foldable = smd::foldable_typeclass<Sequence>;
    CHECK(foldable.length(sequence) == 3U);
}

TEST_CASE("FoldableTypeclassTest - FoldMapSumOnSequence") {
    using Sequence = smd::typeclass::test::Sequence<int>;
    auto sequence = Sequence{{1, 2, 3}};

    const auto &foldable = smd::foldable_typeclass<Sequence>;
    const auto sum = foldable.fold_map([](int x) { return x; }, sequence);
    CHECK(sum == 6);
}

TEST_CASE("FoldableTypeclassTest - FoldMapWithExplicitObject") {
    using Sequence = smd::typeclass::test::Sequence<int>;
    auto sequence = Sequence{{1, 2, 3}};

    const auto &foldable = smd::foldable_typeclass<Sequence>;
    const auto sum = foldable.fold_map([](int x) { return x; }, sequence);
    CHECK(sum == 6);
}

TEST_CASE("FoldableTypeclassTest - FoldMapWithNttpLookup") {
    using Sequence = smd::typeclass::test::Sequence<int>;
    auto sequence = Sequence{{1, 2, 3}};

    CHECK(sum_with_nttp_lookup(sequence) == 6);
}

TEST_CASE("FoldableTypeclassTest - FoldLeftAndRight") {
    using Sequence = smd::typeclass::test::Sequence<int>;
    auto sequence = Sequence{{1, 2, 3}};
    const auto &foldable = smd::foldable_typeclass<Sequence>;

    const auto left = foldable.fold_left(
        sequence, 0, [](int acc, int x) { return acc * 10 + x; });
    const auto right = foldable.fold_right(
        sequence, 0, [](int x, int acc) { return x * 10 + acc; });

    CHECK(left == 123);
    CHECK(right == 60);
}

TEST_CASE("FoldableTypeclassTest - FoldLeftRightWithExplicitObject") {
    using Sequence = smd::typeclass::test::Sequence<int>;
    auto sequence = Sequence{{1, 2, 3}};

    const auto &foldable = smd::foldable_typeclass<Sequence>;
    const auto left = foldable.fold_left(
        sequence, 0, [](int acc, int x) { return acc * 10 + x; });
    const auto right = foldable.fold_right(
        sequence, 0, [](int x, int acc) { return x * 10 + acc; });

    CHECK(left == 123);
    CHECK(right == 60);
}

TEST_CASE("FoldableTypeclassTest - FoldLeftRightWithNttpLookup") {
    using Sequence = smd::typeclass::test::Sequence<int>;
    auto sequence = Sequence{{1, 2, 3}};

    CHECK(fold_left_with_nttp_lookup(sequence) == 123);
    CHECK(fold_right_with_nttp_lookup(sequence) == 60);
}

TEST_CASE("FoldableTypeclassTest - PredicatesAndFind") {
    using Sequence = smd::typeclass::test::Sequence<int>;
    auto sequence = Sequence{{1, 2, 3}};
    const auto &foldable = smd::foldable_typeclass<Sequence>;

    CHECK(foldable.any_of(sequence, [](int x) { return x == 2; }));
    CHECK(foldable.all_of(sequence, [](int x) { return x > 0; }));
    CHECK_FALSE(foldable.empty(sequence));

    auto found = foldable.find_first(sequence, [](int x) { return x > 1; });
    REQUIRE(found.has_value());
    CHECK(*found == 2);
}

TEST_CASE("FoldableTypeclassTest - ToVectorAndCombineAll") {
    // 4c8a5f77-8a62-4f1b-a9cf-95452c4b8ea4
    // a3d5c9e1-6b2f-4a4d-c8e3-5b1d3a7f2c46
    using IntSequence = smd::typeclass::test::Sequence<int>;
    auto sequence = IntSequence{{1, 2, 3}};
    const auto &int_foldable = smd::foldable_typeclass<IntSequence>;

    const auto as_vector = int_foldable.to_vector(sequence);
    CHECK(as_vector == (std::vector<int>{1, 2, 3}));
    // a3d5c9e1-6b2f-4a4d-c8e3-5b1d3a7f2c46 end

    using VectorSequence = smd::typeclass::test::Sequence<std::vector<int>>;
    auto vectors = VectorSequence{{{1, 2}, {3}}};
    const auto &vector_foldable = smd::foldable_typeclass<VectorSequence>;
    const auto combined = vector_foldable.combine_all(vectors);
    CHECK(combined == (std::vector<int>{1, 2, 3}));

    const auto folded = vector_foldable.fold(vectors);
    CHECK(folded == (std::vector<int>{1, 2, 3}));
    // 4c8a5f77-8a62-4f1b-a9cf-95452c4b8ea4 end
}

TEST_CASE("FoldableTypeclassTest - AllOfAndFindFirstEdgeCases") {
    using Sequence = smd::typeclass::test::Sequence<int>;
    const auto &foldable = smd::foldable_typeclass<Sequence>;

    auto mixed = Sequence{{2, -1, 4}};
    CHECK_FALSE(foldable.all_of(mixed, [](int x) { return x > 0; }));

    auto found_even =
        foldable.find_first(mixed, [](int x) { return x % 2 == 0; });
    REQUIRE(found_even.has_value());
    CHECK(*found_even == 2);

    auto found_large =
        foldable.find_first(mixed, [](int x) { return x > 100; });
    CHECK_FALSE(found_large.has_value());
}

// -- Alternate core: fold_right as primitive --

namespace {

// A Foldable instance where fold_right is the primitive, not fold_map.
// Demonstrates the alternate-core pattern: same Sequence type, different core.
template <class VALUE_TYPE>
struct RightFoldSequenceImpl {
    using element_type = VALUE_TYPE;

    template <class STATE, class F>
    auto fold_right(this auto &&,
                    const smd::typeclass::test::Sequence<VALUE_TYPE> &seq,
                    STATE initial, F &&step) {
        auto acc = std::move(initial);
        for (auto it = seq.values.rbegin(); it != seq.values.rend(); ++it) {
            acc = std::invoke(step, *it, std::move(acc));
        }
        return acc;
    }
};

// Alternate-core: using Impl::fold_right selects fold_right as primitive.
// The base Foldable<Impl> derives fold_map from fold_right automatically.
template <class VALUE_TYPE>
struct RightFoldSequenceMap : smd::Foldable<RightFoldSequenceImpl<VALUE_TYPE>> {
    using RightFoldSequenceImpl<VALUE_TYPE>::fold_right;
};

template <class VALUE_TYPE>
constexpr auto right_fold_sequence_map = RightFoldSequenceMap<VALUE_TYPE>{};

} // namespace

TEST_CASE("FoldableAlternateCore - FoldRightPrimitiveDerivesFoldMap") {
    using Sequence = smd::typeclass::test::Sequence<int>;
    auto seq = Sequence{{1, 2, 3}};
    const auto &foldable = right_fold_sequence_map<int>;

    const auto sum = foldable.fold_map([](int x) { return x; }, seq);
    CHECK(sum == 6);
}

TEST_CASE("FoldableAlternateCore - FoldRightPrimitiveDerivesLength") {
    using Sequence = smd::typeclass::test::Sequence<int>;
    auto seq = Sequence{{1, 2, 3}};
    const auto &foldable = right_fold_sequence_map<int>;

    CHECK(foldable.length(seq) == 3U);
}

TEST_CASE("FoldableAlternateCore - FoldRightPrimitiveDerivesToVector") {
    using Sequence = smd::typeclass::test::Sequence<int>;
    auto seq = Sequence{{1, 2, 3}};
    const auto &foldable = right_fold_sequence_map<int>;

    CHECK(foldable.to_vector(seq) == (std::vector<int>{1, 2, 3}));
}

TEST_CASE("FoldableAlternateCore - FoldRightPrimitiveDerivesFoldLeft") {
    using Sequence = smd::typeclass::test::Sequence<int>;
    auto seq = Sequence{{1, 2, 3}};
    const auto &foldable = right_fold_sequence_map<int>;

    const auto result =
        foldable.fold_left(seq, 0, [](int acc, int x) { return acc * 10 + x; });
    CHECK(result == 123);
}

TEST_CASE("FoldableAlternateCore - FoldRightPrimitiveDerivesPredicates") {
    using Sequence = smd::typeclass::test::Sequence<int>;
    auto seq = Sequence{{1, 2, 3}};
    const auto &foldable = right_fold_sequence_map<int>;

    CHECK(foldable.any_of(seq, [](int x) { return x == 2; }));
    CHECK(foldable.all_of(seq, [](int x) { return x > 0; }));
    CHECK_FALSE(foldable.empty(seq));
}

TEST_CASE("FoldableAlternateCore - MatchesFoldMapPrimitiveResults") {
    using Sequence = smd::typeclass::test::Sequence<int>;
    auto seq = Sequence{{4, 1, 7, 2}};

    const auto &fold_map_foldable = smd::foldable_typeclass<Sequence>;
    const auto &fold_right_foldable = right_fold_sequence_map<int>;

    CHECK(fold_map_foldable.length(seq) == fold_right_foldable.length(seq));
    CHECK(fold_map_foldable.to_vector(seq) ==
          fold_right_foldable.to_vector(seq));
    CHECK(fold_map_foldable.fold_map([](int x) { return x; }, seq) ==
          fold_right_foldable.fold_map([](int x) { return x; }, seq));
}

```

## smd/typeclass/functor.hpp

```cpp
// src/smd/typeclass/functor.hpp                                      -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TYPECLASS_FUNCTOR
#define INCLUDED_SMD_TYPECLASS_FUNCTOR

#include <smd/typeclass/typeclass_base.hpp>

#include <beman/optional/optional.hpp>

#include <algorithm>
#include <concepts>
#include <functional>
#include <iterator>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

namespace smd {

// Functor pattern invariants:
// - Instances are single lookup objects that provide fmap(F, T).
// - replace is a derived object operation implemented from fmap.
// - Dispatch happens through a provided object or functor_typeclass<Concrete>.
// - Keep lookup explicit through typeclass objects, not ADL overloads.

/** CRTP base for Functor instances.
 * `Impl` must provide `fmap(f, container)`; `replace` is derived from it.
 */
template <class Impl>
struct Functor : protected Impl {
    using Impl::fmap;

    // e4c7a3f1-8b2d-4e1a-b6f4-1c8d7a5e3b02
    /** Replaces every element of `value` with `replacement`, ignoring the
     * original element values.
     */
    template <class T, class U>
    auto replace(this auto &&self, T &&value, U &&replacement) {
        return self.fmap([replacement = std::forward<U>(replacement)](
                             const auto &) { return replacement; },
                         std::forward<T>(value));
    }
    // e4c7a3f1-8b2d-4e1a-b6f4-1c8d7a5e3b02 end
};

/** Typeclass lookup variable for Functor; specialize for each container type. */
template <class T>
inline constexpr auto functor_typeclass = std::false_type{};

template <class VALUE_TYPE>
struct OptionalFunctorImpl {
    template <class F>
    auto fmap(this auto &&, F &&function,
              const std::optional<VALUE_TYPE> &value) {
        using Result = std::invoke_result_t<F, const VALUE_TYPE &>;
        if (!value) {
            return std::optional<remove_cvref_t<Result>>{};
        }
        return std::optional<remove_cvref_t<Result>>{
            std::invoke(std::forward<F>(function), *value)};
    }
};

template <class VALUE_TYPE>
    requires(!std::same_as<beman::optional::optional<VALUE_TYPE>,
                           std::optional<VALUE_TYPE>>)
struct BemanOptionalFunctorImpl {
    template <class F>
    auto fmap(this auto &&, F &&function,
              const beman::optional::optional<VALUE_TYPE> &value) {
        using Result = std::invoke_result_t<F, const VALUE_TYPE &>;
        if (!value) {
            return beman::optional::optional<remove_cvref_t<Result>>{};
        }
        return beman::optional::optional<remove_cvref_t<Result>>{
            std::invoke(std::forward<F>(function), *value)};
    }
};

template <class VALUE_TYPE>
struct VectorFunctorImpl {
    template <class F>
    auto fmap(this auto &&, F &&function,
              const std::vector<VALUE_TYPE> &values) {
        using Result = std::invoke_result_t<F, const VALUE_TYPE &>;
        std::vector<remove_cvref_t<Result>> output;
        output.reserve(values.size());

        std::ranges::transform(values, std::back_inserter(output),
                               [&function](const VALUE_TYPE &v) {
                                   return std::invoke(function, v);
                               });

        return output;
    }
};

template <class VALUE_TYPE>
struct OptionalFunctorMap : Functor<OptionalFunctorImpl<VALUE_TYPE>> {
    using OptionalFunctorImpl<VALUE_TYPE>::fmap;
};

template <class VALUE_TYPE>
    requires(!std::same_as<beman::optional::optional<VALUE_TYPE>,
                           std::optional<VALUE_TYPE>>)
struct BemanOptionalFunctorMap : Functor<BemanOptionalFunctorImpl<VALUE_TYPE>> {
    using BemanOptionalFunctorImpl<VALUE_TYPE>::fmap;
};

template <class VALUE_TYPE>
struct VectorFunctorMap : Functor<VectorFunctorImpl<VALUE_TYPE>> {
    using VectorFunctorImpl<VALUE_TYPE>::fmap;
};

/** Functor instance for `std::optional<VALUE_TYPE>`. */
template <class VALUE_TYPE>
inline constexpr auto functor_typeclass<std::optional<VALUE_TYPE>> =
    OptionalFunctorMap<VALUE_TYPE>{};

/** Functor instance for `beman::optional::optional<VALUE_TYPE>`. */
template <class VALUE_TYPE>
    requires(!std::same_as<beman::optional::optional<VALUE_TYPE>,
                           std::optional<VALUE_TYPE>>)
inline constexpr auto functor_typeclass<beman::optional::optional<VALUE_TYPE>> =
    BemanOptionalFunctorMap<VALUE_TYPE>{};

/** Functor instance for `std::vector<VALUE_TYPE>`. */
template <class VALUE_TYPE>
inline constexpr auto functor_typeclass<std::vector<VALUE_TYPE>> =
    VectorFunctorMap<VALUE_TYPE>{};

} // namespace smd

#endif // INCLUDED_SMD_TYPECLASS_FUNCTOR

```

## smd/typeclass/functor.t.cpp

```cpp
#include <smd/typeclass/functor.hpp>
#include <smd/typeclass/functor.hpp> // Re-inclusion check

#include <catch2/catch_test_macros.hpp>

#include <beman/optional/optional.hpp>

#include <optional>
#include <vector>

TEST_CASE("FunctorTypeclassTest - OptionalBreathing") {
    std::optional<int> value{5};
    const auto &functor = smd::functor_typeclass<std::optional<int>>;
    auto mapped = functor.fmap([](int x) { return x + 1; }, value);

    REQUIRE(mapped.has_value());
    CHECK(*mapped == 6);
}

TEST_CASE("FunctorTypeclassTest - ReplaceVector") {
    std::vector<int> input{1, 2, 3};
    const auto &functor = smd::functor_typeclass<std::vector<int>>;
    auto replaced = functor.replace(input, 9);

    CHECK(replaced == (std::vector<int>{9, 9, 9}));
}

TEST_CASE("FunctorTypeclassTest - OptionalFmapShortCircuit") {
    std::optional<int> empty{};
    const auto &functor = smd::functor_typeclass<std::optional<int>>;

    int calls = 0;
    auto mapped = functor.fmap(
        [&calls](int x) {
            ++calls;
            return x + 10;
        },
        empty);

    CHECK_FALSE(mapped.has_value());
    CHECK(calls == 0);
}

TEST_CASE("FunctorTypeclassTest - VectorFmapMapsAndPreservesEmpty") {
    const auto &functor = smd::functor_typeclass<std::vector<int>>;

    std::vector<int> input{1, 2, 3};
    auto mapped = functor.fmap([](int x) { return x * x; }, input);
    CHECK(mapped == (std::vector<int>{1, 4, 9}));

    std::vector<int> empty_input{};
    auto empty_mapped = functor.fmap([](int x) { return x + 1; }, empty_input);
    CHECK(empty_mapped.empty());
}

TEST_CASE("FunctorTypeclassTest - BemanOptionalBreathing") {
    beman::optional::optional<int> value{5};
    const auto &functor =
        smd::functor_typeclass<beman::optional::optional<int>>;
    auto mapped = functor.fmap([](int x) { return x + 2; }, value);

    REQUIRE(mapped.has_value());
    CHECK(*mapped == 7);
}

TEST_CASE("FunctorTypeclassTest - BemanOptionalFmapShortCircuit") {
    beman::optional::optional<int> empty{};
    const auto &functor =
        smd::functor_typeclass<beman::optional::optional<int>>;

    int calls = 0;
    auto mapped = functor.fmap(
        [&calls](int x) {
            ++calls;
            return x + 10;
        },
        empty);

    CHECK_FALSE(mapped.has_value());
    CHECK(calls == 0);
}

TEST_CASE("FunctorTypeclassTest - ReplaceOptionalAndBemanOptional") {
    const auto &optional_functor = smd::functor_typeclass<std::optional<int>>;
    auto replaced_present = optional_functor.replace(std::optional<int>{1}, 42);
    REQUIRE(replaced_present.has_value());
    CHECK(*replaced_present == 42);

    auto replaced_empty = optional_functor.replace(std::optional<int>{}, 42);
    CHECK_FALSE(replaced_empty.has_value());

    const auto &beman_functor =
        smd::functor_typeclass<beman::optional::optional<int>>;
    auto beman_replaced_present =
        beman_functor.replace(beman::optional::optional<int>{2}, 99);
    REQUIRE(beman_replaced_present.has_value());
    CHECK(*beman_replaced_present == 99);

    auto beman_replaced_empty =
        beman_functor.replace(beman::optional::optional<int>{}, 99);
    CHECK_FALSE(beman_replaced_empty.has_value());
}

TEST_CASE("FunctorLaws - IdentityLaw") {
    // fmap(id, x) == x for all instances and shapes
    auto id = [](int x) { return x; };

    // d8b6e1f2-7a3c-4d5e-b2a8-3f4c1d9e5b65
    {
        const auto &functor = smd::functor_typeclass<std::optional<int>>;
        CHECK(functor.fmap(id, std::optional<int>{42}) ==
              std::optional<int>{42});
        CHECK(functor.fmap(id, std::optional<int>{}) == std::optional<int>{});
    }
    // d8b6e1f2-7a3c-4d5e-b2a8-3f4c1d9e5b65 end
    {
        const auto &functor =
            smd::functor_typeclass<beman::optional::optional<int>>;
        const beman::optional::optional<int> present{7};
        const beman::optional::optional<int> empty{};
        CHECK(functor.fmap(id, present) == present);
        CHECK(functor.fmap(id, empty) == empty);
    }
    {
        const auto &functor = smd::functor_typeclass<std::vector<int>>;
        const std::vector<int> v{1, 2, 3};
        CHECK(functor.fmap(id, v) == v);
        CHECK(functor.fmap(id, std::vector<int>{}) == std::vector<int>{});
    }
}

TEST_CASE("FunctorLaws - CompositionLaw") {
    // fmap(f ∘ g, x) == fmap(f, fmap(g, x))
    auto g = [](int x) { return x + 1; };
    auto f = [](int x) { return x * 2; };
    auto fog = [](int x) { return (x + 1) * 2; };

    {
        const auto &functor = smd::functor_typeclass<std::optional<int>>;
        const std::optional<int> present{5};
        const std::optional<int> empty{};
        CHECK(functor.fmap(fog, present) ==
              functor.fmap(f, functor.fmap(g, present)));
        CHECK(functor.fmap(fog, empty) ==
              functor.fmap(f, functor.fmap(g, empty)));
    }
    {
        const auto &functor =
            smd::functor_typeclass<beman::optional::optional<int>>;
        const beman::optional::optional<int> present{5};
        const beman::optional::optional<int> empty{};
        CHECK(functor.fmap(fog, present) ==
              functor.fmap(f, functor.fmap(g, present)));
        CHECK(functor.fmap(fog, empty) ==
              functor.fmap(f, functor.fmap(g, empty)));
    }
    {
        const auto &functor = smd::functor_typeclass<std::vector<int>>;
        const std::vector<int> v{1, 2, 3};
        CHECK(functor.fmap(fog, v) == functor.fmap(f, functor.fmap(g, v)));
        CHECK(functor.fmap(fog, std::vector<int>{}) ==
              functor.fmap(f, functor.fmap(g, std::vector<int>{})));
    }
}

```

## smd/typeclass/monad.hpp

```cpp
// src/smd/typeclass/monad.hpp                                       -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TYPECLASS_MONAD
#define INCLUDED_SMD_TYPECLASS_MONAD

#include <smd/typeclass/applicative.hpp>
#include <smd/typeclass/typeclass_base.hpp>

#include <beman/optional/optional.hpp>

#include <concepts>
#include <functional>
#include <optional>
#include <type_traits>
#include <utility>

namespace smd {

/** CRTP base for Monad instances.
 * `Impl` must provide `pure(value)` and `bind(ma, f)`.
 * `apply` is synthesized; `join` and `kleisli` are derived.
 * Monad does not inherit from Applicative, but provides equivalent
 * operations once `apply` is synthesized from `bind` + `pure`.
 */
template <class Impl>
struct Monad : protected Impl {
    static_assert(!std::is_same_v<Impl, std::false_type>,
                  "No monad_typeclass<T> specialization found. "
                  "Specialize smd::monad_typeclass<T> for your type T "
                  "and provide pure(...) and bind(...) operations.");

    using Impl::bind;
    using Impl::pure;

    // apply: synthesized from bind + pure.
    // ap mf mx = mf >>= \f -> mx >>= \a -> pure (f a)
    template <class MF, class MA>
    auto apply(this auto &&self, MF &&mf, MA &&ma) {
        return self.bind(
            std::forward<MF>(mf),
            [&self, &ma](auto &&f) {
                return self.bind(
                    ma,
                    [&self, &f](auto &&a) {
                        return self.pure(
                            std::invoke(std::forward<decltype(f)>(f),
                                        std::forward<decltype(a)>(a)));
                    });
            });
    }

    // join: flatten nested monad.
    // join mma = mma >>= id
    template <class MMA>
    auto join(this auto &&self, MMA &&mma) {
        return self.bind(
            std::forward<MMA>(mma),
            [](auto &&inner) { return inner; });
    }

    // kleisli: forward Kleisli composition (>=>).
    // (f >=> g) a = f a >>= g
    template <class F, class G>
    auto kleisli(this auto &&self, F f, G g) {
        return [&self, f = std::move(f), g = std::move(g)](auto &&a) {
            return self.bind(
                f(std::forward<decltype(a)>(a)), g);
        };
    }

    // bind_with: explicit monad object override.
    template <class MONAD_MAP, class MA, class F>
    auto bind_with(this auto &&, const MONAD_MAP &monad_map,
                   MA &&ma, F &&f) {
        return monad_map.bind(std::forward<MA>(ma), std::forward<F>(f));
    }
};

/** Typeclass lookup variable for Monad; specialize for each type. */
template <class T>
inline constexpr auto monad_typeclass = std::false_type{};

// -- std::optional monad instance --
// Delegates pure to the existing applicative_typeclass.

template <class VALUE_TYPE>
struct OptionalMonadImpl {
    using element_type = VALUE_TYPE;

    template <class VALUE>
    auto pure(this auto &&, VALUE &&value)
        -> std::optional<remove_cvref_t<VALUE>> {
        return applicative_typeclass<std::optional<VALUE_TYPE>>
            .pure(std::forward<VALUE>(value));
    }

    template <class A, class F>
    auto bind(this auto &&, const std::optional<A> &ma, F &&f) {
        using Result =
            remove_cvref_t<std::invoke_result_t<F, const A &>>;
        if (!ma)
            return Result{};
        return Result{std::invoke(std::forward<F>(f), *ma)};
    }
};

template <class VALUE_TYPE>
struct OptionalMonadMap : Monad<OptionalMonadImpl<VALUE_TYPE>> {
    using OptionalMonadImpl<VALUE_TYPE>::bind;
    using OptionalMonadImpl<VALUE_TYPE>::pure;
};

/** Monad instance for `std::optional<VALUE_TYPE>`. */
template <class VALUE_TYPE>
inline constexpr auto monad_typeclass<std::optional<VALUE_TYPE>> =
    OptionalMonadMap<VALUE_TYPE>{};

// -- beman::optional monad instance --

template <class VALUE_TYPE>
    requires(!std::same_as<beman::optional::optional<VALUE_TYPE>,
                           std::optional<VALUE_TYPE>>)
struct BemanOptionalMonadImpl {
    using element_type = VALUE_TYPE;

    template <class VALUE>
    auto pure(this auto &&, VALUE &&value)
        -> beman::optional::optional<remove_cvref_t<VALUE>> {
        return applicative_typeclass<beman::optional::optional<VALUE_TYPE>>
            .pure(std::forward<VALUE>(value));
    }

    template <class A, class F>
    auto bind(this auto &&, const beman::optional::optional<A> &ma, F &&f) {
        using Result =
            remove_cvref_t<std::invoke_result_t<F, const A &>>;
        if (!ma)
            return Result{};
        return Result{std::invoke(std::forward<F>(f), *ma)};
    }
};

template <class VALUE_TYPE>
    requires(!std::same_as<beman::optional::optional<VALUE_TYPE>,
                           std::optional<VALUE_TYPE>>)
struct BemanOptionalMonadMap
    : Monad<BemanOptionalMonadImpl<VALUE_TYPE>> {
    using BemanOptionalMonadImpl<VALUE_TYPE>::bind;
    using BemanOptionalMonadImpl<VALUE_TYPE>::pure;
};

/** Monad instance for `beman::optional::optional<VALUE_TYPE>`. */
template <class VALUE_TYPE>
    requires(!std::same_as<beman::optional::optional<VALUE_TYPE>,
                           std::optional<VALUE_TYPE>>)
inline constexpr auto
    monad_typeclass<beman::optional::optional<VALUE_TYPE>> =
        BemanOptionalMonadMap<VALUE_TYPE>{};

// -- Free-function API --

/** Sequences a monadic value `ma` through function `f` (Haskell's `>>=`). */
template <class MA, class F>
auto mbind(MA &&ma, F &&f) {
    const auto &map = monad_typeclass<remove_cvref_t<MA>>;
    return map.bind(std::forward<MA>(ma), std::forward<F>(f));
}

/** Flattens a nested monadic value; equivalent to `bind(mma, id)`. */
template <class MMA>
auto join(MMA &&mma) {
    const auto &map = monad_typeclass<remove_cvref_t<MMA>>;
    return map.join(std::forward<MMA>(mma));
}

} // namespace smd

#endif

```

## smd/typeclass/monad.t.cpp

```cpp
#include <smd/typeclass/monad.hpp>
#include <smd/typeclass/monad.hpp> // Re-inclusion check

#include <catch2/catch_test_macros.hpp>

#include <functional>
#include <optional>
#include <string>

namespace {

// Test-only type with a monad instance but no applicative_typeclass.
// Proves the synthesized apply from bind + pure works standalone.
template <class T>
struct MonadOnly {
    T value;
    using value_type = T;
    auto operator==(const MonadOnly &) const -> bool = default;
};

template <class VALUE_TYPE>
struct MonadOnlyImpl {
    using element_type = VALUE_TYPE;

    template <class VALUE>
    auto pure(this auto &&, VALUE &&value)
        -> MonadOnly<smd::remove_cvref_t<VALUE>> {
        return MonadOnly<smd::remove_cvref_t<VALUE>>{
            std::forward<VALUE>(value)};
    }

    template <class A, class F>
    auto bind(this auto &&, const MonadOnly<A> &ma, F &&f)
        -> std::invoke_result_t<F, const A &> {
        return std::invoke(std::forward<F>(f), ma.value);
    }
};

template <class VALUE_TYPE>
struct MonadOnlyMap : smd::Monad<MonadOnlyImpl<VALUE_TYPE>> {
    using MonadOnlyImpl<VALUE_TYPE>::bind;
    using MonadOnlyImpl<VALUE_TYPE>::pure;
};

constexpr MonadOnlyMap<int> monad_only_int{};

} // namespace

// -- Breathing tests --

TEST_CASE("MonadTypeclassTest - BindOptionalPresent") {
    const auto &monad = smd::monad_typeclass<std::optional<int>>;
    auto result = monad.bind(
        std::optional<int>{5},
        [](int x) { return std::optional<int>{x * 2}; });
    REQUIRE(result.has_value());
    CHECK(*result == 10);
}

TEST_CASE("MonadTypeclassTest - BindOptionalAbsent") {
    const auto &monad = smd::monad_typeclass<std::optional<int>>;
    auto result = monad.bind(
        std::optional<int>{},
        [](int x) { return std::optional<int>{x * 2}; });
    CHECK_FALSE(result.has_value());
}

TEST_CASE("MonadTypeclassTest - PureDelegatesToApplicative") {
    const auto &monad = smd::monad_typeclass<std::optional<int>>;
    auto lifted = monad.pure(42);
    REQUIRE(lifted.has_value());
    CHECK(*lifted == 42);
}

// -- Join --

TEST_CASE("MonadTypeclassTest - JoinOptionalPresent") {
    const auto &monad = smd::monad_typeclass<std::optional<int>>;
    std::optional<std::optional<int>> nested{std::optional<int>{7}};
    auto result = monad.join(nested);
    REQUIRE(result.has_value());
    CHECK(*result == 7);
}

TEST_CASE("MonadTypeclassTest - JoinOptionalOuterAbsent") {
    const auto &monad = smd::monad_typeclass<std::optional<int>>;
    std::optional<std::optional<int>> nested{};
    auto result = monad.join(nested);
    CHECK_FALSE(result.has_value());
}

TEST_CASE("MonadTypeclassTest - JoinOptionalInnerAbsent") {
    const auto &monad = smd::monad_typeclass<std::optional<int>>;
    std::optional<std::optional<int>> nested{std::optional<int>{}};
    auto result = monad.join(nested);
    CHECK_FALSE(result.has_value());
}

// -- Monad laws (optional) --

TEST_CASE("MonadTypeclassTest - LeftIdentityLaw") {
    // bind(pure(a), f) == f(a)
    const auto &monad = smd::monad_typeclass<std::optional<int>>;
    auto f = [](int x) { return std::optional<std::string>{std::to_string(x)}; };
    int a = 42;

    auto lhs = monad.bind(monad.pure(a), f);
    auto rhs = f(a);
    CHECK(lhs == rhs);
}

TEST_CASE("MonadTypeclassTest - RightIdentityLaw") {
    // bind(ma, pure) == ma
    const auto &monad = smd::monad_typeclass<std::optional<int>>;
    std::optional<int> ma{99};

    auto result = monad.bind(ma,
        [](int x) { return std::optional<int>{x}; });
    CHECK(result == ma);
}

TEST_CASE("MonadTypeclassTest - RightIdentityLawAbsent") {
    const auto &monad = smd::monad_typeclass<std::optional<int>>;
    std::optional<int> ma{};

    auto result = monad.bind(ma,
        [](int x) { return std::optional<int>{x}; });
    CHECK(result == ma);
}

TEST_CASE("MonadTypeclassTest - AssociativityLaw") {
    // bind(bind(ma, f), g) == bind(ma, [](a) { return bind(f(a), g); })
    const auto &monad = smd::monad_typeclass<std::optional<int>>;
    std::optional<int> ma{10};
    auto f = [](int x) { return std::optional<int>{x + 5}; };
    auto g = [](int x) { return std::optional<int>{x * 2}; };

    auto lhs = monad.bind(monad.bind(ma, f), g);
    auto rhs = monad.bind(ma, [&f, &g](int a) {
        return smd::monad_typeclass<std::optional<int>>.bind(f(a), g);
    });
    CHECK(lhs == rhs);
}

TEST_CASE("MonadTypeclassTest - AssociativityLawWithFailure") {
    const auto &monad = smd::monad_typeclass<std::optional<int>>;
    std::optional<int> ma{3};
    auto f = [](int) { return std::optional<int>{}; };
    auto g = [](int x) { return std::optional<int>{x * 100}; };

    auto lhs = monad.bind(monad.bind(ma, f), g);
    auto rhs = monad.bind(ma, [&f, &g](int a) {
        return smd::monad_typeclass<std::optional<int>>.bind(f(a), g);
    });
    CHECK(lhs == rhs);
    CHECK_FALSE(lhs.has_value());
}

// -- Synthesized apply --

TEST_CASE("MonadTypeclassTest - SynthesizedApplyOptional") {
    const auto &monad = smd::monad_typeclass<std::optional<int>>;
    std::optional<int (*)(int)> mf{+[](int x) { return x + 100; }};
    std::optional<int> ma{7};

    auto result = monad.apply(mf, ma);
    REQUIRE(result.has_value());
    CHECK(*result == 107);
}

TEST_CASE("MonadTypeclassTest - SynthesizedApplyOptionalFunctionAbsent") {
    const auto &monad = smd::monad_typeclass<std::optional<int>>;
    std::optional<int (*)(int)> mf{};
    std::optional<int> ma{7};

    auto result = monad.apply(mf, ma);
    CHECK_FALSE(result.has_value());
}

TEST_CASE("MonadTypeclassTest - SynthesizedApplyOptionalArgumentAbsent") {
    const auto &monad = smd::monad_typeclass<std::optional<int>>;
    std::optional<int (*)(int)> mf{+[](int x) { return x + 100; }};
    std::optional<int> ma{};

    auto result = monad.apply(mf, ma);
    CHECK_FALSE(result.has_value());
}

// -- MonadOnly: standalone monad with no applicative_typeclass --

TEST_CASE("MonadOnlyTest - BindBreathing") {
    auto result = monad_only_int.bind(
        MonadOnly<int>{5},
        [](int x) { return MonadOnly<int>{x * 3}; });
    CHECK(result == MonadOnly<int>{15});
}

TEST_CASE("MonadOnlyTest - PureBreathing") {
    auto result = monad_only_int.pure(42);
    CHECK(result == MonadOnly<int>{42});
}

TEST_CASE("MonadOnlyTest - JoinBreathing") {
    MonadOnly<MonadOnly<int>> nested{MonadOnly<int>{99}};
    auto result = monad_only_int.join(nested);
    CHECK(result == MonadOnly<int>{99});
}

TEST_CASE("MonadOnlyTest - SynthesizedApplyWithoutApplicative") {
    auto mf = MonadOnly<int (*)(int)>{+[](int x) { return x + 10; }};
    auto ma = MonadOnly<int>{5};
    auto result = monad_only_int.apply(mf, ma);
    CHECK(result == MonadOnly<int>{15});
}

TEST_CASE("MonadOnlyTest - LeftIdentityLaw") {
    auto f = [](int x) { return MonadOnly<int>{x * 2}; };
    int a = 7;
    auto lhs = monad_only_int.bind(monad_only_int.pure(a), f);
    auto rhs = f(a);
    CHECK(lhs == rhs);
}

TEST_CASE("MonadOnlyTest - RightIdentityLaw") {
    MonadOnly<int> ma{42};
    auto result = monad_only_int.bind(ma,
        [](int x) { return MonadOnly<int>{x}; });
    CHECK(result == ma);
}

TEST_CASE("MonadOnlyTest - AssociativityLaw") {
    MonadOnly<int> ma{3};
    auto f = [](int x) { return MonadOnly<int>{x + 10}; };
    auto g = [](int x) { return MonadOnly<int>{x * 2}; };

    auto lhs = monad_only_int.bind(monad_only_int.bind(ma, f), g);
    auto rhs = monad_only_int.bind(ma, [&](int a) {
        return monad_only_int.bind(f(a), g);
    });
    CHECK(lhs == rhs);
}

// -- Kleisli composition --

TEST_CASE("MonadTypeclassTest - KleisliComposition") {
    const auto &monad = smd::monad_typeclass<std::optional<int>>;
    auto f = [](int x) { return std::optional<int>{x + 1}; };
    auto g = [](int x) { return std::optional<std::string>{std::to_string(x)}; };

    auto composed = monad.kleisli(f, g);
    auto result = composed(9);
    REQUIRE(result.has_value());
    CHECK(*result == "10");
}

TEST_CASE("MonadTypeclassTest - KleisliCompositionShortCircuit") {
    const auto &monad = smd::monad_typeclass<std::optional<int>>;
    auto f = [](int) { return std::optional<int>{}; };
    auto g = [](int x) { return std::optional<std::string>{std::to_string(x)}; };

    auto composed = monad.kleisli(f, g);
    auto result = composed(9);
    CHECK_FALSE(result.has_value());
}

// -- Free-function API --

TEST_CASE("MonadFreeFunctionTest - MBind") {
    auto result = smd::mbind(
        std::optional<int>{5},
        [](int x) { return std::optional<int>{x * 2}; });
    REQUIRE(result.has_value());
    CHECK(*result == 10);
}

TEST_CASE("MonadFreeFunctionTest - Join") {
    std::optional<std::optional<int>> nested{std::optional<int>{42}};
    auto result = smd::join(nested);
    REQUIRE(result.has_value());
    CHECK(*result == 42);
}

// -- bind_with: explicit monad object override --

TEST_CASE("MonadTypeclassTest - BindWithExplicitMap") {
    const auto &monad = smd::monad_typeclass<std::optional<int>>;
    auto result = monad.bind_with(
        monad,
        std::optional<int>{5},
        [](int x) { return std::optional<int>{x + 1}; });
    REQUIRE(result.has_value());
    CHECK(*result == 6);
}

```

## smd/typeclass/monoid.hpp

```cpp
// src/smd/typeclass/monoid.hpp                                       -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TYPECLASS_MONOID
#define INCLUDED_SMD_TYPECLASS_MONOID

#include <smd/typeclass/typeclass_base.hpp>

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

namespace smd::typeclass {

// Monoid pattern invariants:
// - Monoid<T> is the customization point; specialize identity/combine together.
// - monoid_v<T> is the canonical lookup object used by generic algorithms.
// - identity and combine must stay coherent for a single associative law
// domain.
// - Prefer adding new Monoid<T> specializations over ad hoc free functions.

/** Customization point for the Monoid typeclass.
 * Specialize this struct for type `VALUE_TYPE` and provide
 * `identity()` and `combine(lhs, rhs)` to make that type usable
 * wherever a Monoid is required (e.g., as the result type of fold_map).
 */
template <class VALUE_TYPE>
struct Monoid;

/** Canonical lookup object for Monoid<VALUE_TYPE>; used by generic algorithms. */
template <class VALUE_TYPE>
inline constexpr Monoid<VALUE_TYPE> monoid_v = Monoid<VALUE_TYPE>{};

/** Opaque count accumulator; the Monoid combines by addition. */
struct Count {
    std::size_t d_value;

    friend constexpr bool operator==(const Count &lhs,
                                     const Count &rhs) = default;
};

// c3a1e0f8-6b5d-4c2a-a8e3-3d7b9f4a1c06
/** Monoid<Count>: identity is zero, combine adds counts. */
template <>
struct Monoid<Count> {
    constexpr auto identity() const -> Count { return Count{0}; }

    constexpr auto combine(const Count &lhs, const Count &rhs) const -> Count {
        return Count{lhs.d_value + rhs.d_value};
    }
};
// c3a1e0f8-6b5d-4c2a-a8e3-3d7b9f4a1c06 end

/** Monoid<int>: additive monoid with identity 0. */
template <>
struct Monoid<int> {
    constexpr auto identity() const -> int { return 0; }

    constexpr auto combine(int lhs, int rhs) const -> int { return lhs + rhs; }
};

/** Monoid<long>: additive monoid with identity 0. */
template <>
struct Monoid<long> {
    constexpr auto identity() const -> long { return 0L; }

    constexpr auto combine(long lhs, long rhs) const -> long {
        return lhs + rhs;
    }
};

/** Monoid<std::size_t>: additive monoid with identity 0. */
template <>
struct Monoid<std::size_t> {
    constexpr auto identity() const -> std::size_t { return 0U; }

    constexpr auto combine(std::size_t lhs, std::size_t rhs) const
        -> std::size_t {
        return lhs + rhs;
    }
};

/** Monoid<std::string>: concatenation monoid with identity "". */
template <>
struct Monoid<std::string> {
    auto identity() const -> std::string { return {}; }

    auto combine(const std::string &lhs, const std::string &rhs) const
        -> std::string {
        return lhs + rhs;
    }
};

/** Monoid<std::vector<T>>: concatenation monoid with identity empty vector. */
template <class VALUE_TYPE>
struct Monoid<std::vector<VALUE_TYPE>> {
    auto identity() const -> std::vector<VALUE_TYPE> { return {}; }

    auto combine(std::vector<VALUE_TYPE> lhs,
                 const std::vector<VALUE_TYPE> &rhs) const
        -> std::vector<VALUE_TYPE> {
        lhs.insert(lhs.end(), rhs.begin(), rhs.end());
        return lhs;
    }
};

} // namespace smd::typeclass

namespace smd {

// b5f3d1a9-7c4e-4b2f-9a5d-6e3c7b8d4f02
/** Returns the identity element for the Monoid of VALUE_TYPE. */
template <class VALUE_TYPE>
auto monoid_identity() -> VALUE_TYPE {
    return typeclass::monoid_v<VALUE_TYPE>.identity();
}

/** Combines two values using the Monoid of VALUE_TYPE. */
template <class VALUE_TYPE>
auto monoid_combine(const VALUE_TYPE &lhs, const VALUE_TYPE &rhs)
    -> VALUE_TYPE {
    return typeclass::monoid_v<VALUE_TYPE>.combine(lhs, rhs);
}
// b5f3d1a9-7c4e-4b2f-9a5d-6e3c7b8d4f02 end

} // namespace smd

#endif // INCLUDED_SMD_TYPECLASS_MONOID

```

## smd/typeclass/monoid.t.cpp

```cpp
#include <smd/typeclass/monoid.hpp>
#include <smd/typeclass/monoid.hpp> // Re-inclusion check

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <vector>

TEST_CASE("MonoidTypeclassTest - CountBreathing") {
    // a1d6e3f7-3c2b-4a8e-b4f1-7c5d3a9e6b84
    const smd::typeclass::Count one{1};
    const smd::typeclass::Count two{2};

    const auto result = smd::monoid_combine(one, two);
    CHECK(result.d_value == 3U);
    // a1d6e3f7-3c2b-4a8e-b4f1-7c5d3a9e6b84 end
}

TEST_CASE("MonoidTypeclassTest - StringCombine") {
    const auto joined =
        smd::monoid_combine(std::string{"hello"}, std::string{" world"});
    CHECK(joined == "hello world");
}

TEST_CASE("MonoidTypeclassTest - VectorCombine") {
    const auto joined =
        smd::monoid_combine(std::vector<int>{1, 2}, std::vector<int>{3});
    CHECK(joined == (std::vector<int>{1, 2, 3}));
}

TEST_CASE("MonoidLaws - IdentityElement") {
    // f3b4e6a2-1c7d-4e5b-8a3f-2d9c5b8e3f36
    {
        const auto &m = smd::typeclass::monoid_v<int>;
        CHECK(m.combine(m.identity(), 42) == 42);
        CHECK(m.combine(42, m.identity()) == 42);
    }
    // f3b4e6a2-1c7d-4e5b-8a3f-2d9c5b8e3f36 end
    {
        const auto &m = smd::typeclass::monoid_v<std::string>;
        CHECK(m.combine(m.identity(), std::string{"hello"}) == "hello");
        CHECK(m.combine(std::string{"hello"}, m.identity()) == "hello");
    }
    {
        const auto &m = smd::typeclass::monoid_v<std::vector<int>>;
        const std::vector<int> v{1, 2, 3};
        CHECK(m.combine(m.identity(), v) == v);
        CHECK(m.combine(v, m.identity()) == v);
    }
    {
        const auto &m = smd::typeclass::monoid_v<smd::typeclass::Count>;
        CHECK(m.combine(m.identity(), smd::typeclass::Count{5}) ==
              smd::typeclass::Count{5});
        CHECK(m.combine(smd::typeclass::Count{5}, m.identity()) ==
              smd::typeclass::Count{5});
    }
}

TEST_CASE("MonoidLaws - Associativity") {
    {
        const auto &m = smd::typeclass::monoid_v<int>;
        CHECK(m.combine(m.combine(1, 2), 3) == m.combine(1, m.combine(2, 3)));
        CHECK(m.combine(m.combine(-5, 10), -3) ==
              m.combine(-5, m.combine(10, -3)));
    }
    {
        const auto &m = smd::typeclass::monoid_v<std::string>;
        CHECK(m.combine(m.combine(std::string{"foo"}, std::string{"bar"}),
                        std::string{"baz"}) ==
              m.combine(std::string{"foo"},
                        m.combine(std::string{"bar"}, std::string{"baz"})));
    }
    {
        const auto &m = smd::typeclass::monoid_v<std::vector<int>>;
        const std::vector<int> a{1, 2};
        const std::vector<int> b{3, 4};
        const std::vector<int> c{5, 6};
        CHECK(m.combine(m.combine(a, b), c) == m.combine(a, m.combine(b, c)));
    }
}

```

## smd/typeclass/test/test_support.hpp

```cpp
// src/smd/typeclass/test/test_support.hpp                            -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TYPECLASS_TEST_TEST_SUPPORT
#define INCLUDED_SMD_TYPECLASS_TEST_TEST_SUPPORT

#include <smd/typeclass/applicative.hpp>
#include <smd/typeclass/foldable.hpp>
#include <smd/typeclass/traversable.hpp>

#include <algorithm>
#include <functional>
#include <utility>
#include <vector>

namespace smd::typeclass::test {

/** Return true if @p left == @p right; thin wrapper for use in law checks. */
template <class LEFT, class RIGHT>
auto are_equal(LEFT &&left, RIGHT &&right) -> bool {
    return std::forward<LEFT>(left) == std::forward<RIGHT>(right);
}

/** Verify the Applicative identity law: `pure(id) <*> v == v`.
 * @tparam CONTEXT an applicative context type with a registered typeclass
 */
template <class CONTEXT>
auto check_applicative_identity_law(const CONTEXT &value) -> bool {
    const auto &applicative =
        smd::applicative_typeclass<smd::remove_cvref_t<CONTEXT>>;
    auto result = applicative.ap(
        applicative.pure([](const auto &x) { return x; }), value);
    return result == value;
}

/** Verify the Applicative homomorphism law: `pure(f) <*> pure(x) == pure(f x)`.
 * @tparam CONTEXT the applicative context whose typeclass is under test
 */
template <class CONTEXT, class FUNCTION, class VALUE>
auto check_applicative_homomorphism_law(const FUNCTION &function,
                                        const VALUE &value) -> bool {
    const auto &applicative =
        smd::applicative_typeclass<smd::remove_cvref_t<CONTEXT>>;
    auto left =
        applicative.ap(applicative.pure(function), applicative.pure(value));
    auto right = applicative.pure(std::invoke(function, value));
    return left == right;
}

/** Verify that `invoke(f, u, v)` agrees with the desugared `ap`/`pure` form
 * for a binary @p function applied to effectful arguments @p first and @p second.
 * @tparam CONTEXT the applicative context whose typeclass is under test
 */
template <class CONTEXT, class FUNCTION>
auto check_applicative_invoke_binary_law(const FUNCTION &function,
                                         const CONTEXT &first,
                                         const CONTEXT &second) -> bool {
    const auto &applicative =
        smd::applicative_typeclass<smd::remove_cvref_t<CONTEXT>>;
    auto invoke_result = applicative.invoke(function, first, second);

    auto ap_result = applicative.ap(
        applicative.ap(applicative.pure([function](const auto &lhs) {
            return [function, lhs](const auto &rhs) {
                return std::invoke(function, lhs, rhs);
            };
        }),
                       first),
        second);

    return invoke_result == ap_result;
}

/** Minimal single-element applicative context used in law tests.
 * `pure(x)` wraps @p x; `apply` unwraps and invokes the stored function.
 */
template <class VALUE_TYPE>
struct Identity {
    using value_type = VALUE_TYPE;

    VALUE_TYPE value;

    friend auto operator==(const Identity &, const Identity &)
        -> bool = default;
};

/** Like Identity but uses forwarding (rvalue-ref) apply semantics.
 * Distinguishes dispatch paths in tests that require move-only contexts.
 */
template <class VALUE_TYPE>
struct BareIdentity {
    using value_type = VALUE_TYPE;

    VALUE_TYPE value;

    friend auto operator==(const BareIdentity &, const BareIdentity &)
        -> bool = default;
};

/** Ordered multi-element foldable context backed by `std::vector`.
 * fold_map accumulates left-to-right in `values` order.
 */
template <class VALUE_TYPE>
struct Sequence {
    using value_type = VALUE_TYPE;

    std::vector<VALUE_TYPE> values;

    friend auto operator==(const Sequence &, const Sequence &)
        -> bool = default;
};

using smd::typeclass::Count;

/** Convenience alias so tests can write `Vector<int>` instead of `std::vector<int>`. */
template <class VALUE_TYPE>
using Vector = std::vector<VALUE_TYPE>;

} // namespace smd::typeclass::test

namespace smd {

/** Applicative implementation for Identity<V>: pure wraps, apply unwraps and invokes. */
template <class VALUE_TYPE>
struct TestIdentityApplicativeImpl {
    template <class VALUE>
    auto pure(this auto &&, VALUE &&value) {
        return smd::typeclass::test::Identity<remove_cvref_t<VALUE>>{
            std::forward<VALUE>(value)};
    }

    template <class FUNCTION_IN_CONTEXT, class ARGUMENT_IN_CONTEXT>
    auto apply(this auto &&, const FUNCTION_IN_CONTEXT &function,
               const ARGUMENT_IN_CONTEXT &argument) {
        using Result = std::invoke_result_t<
            const typename remove_cvref_t<FUNCTION_IN_CONTEXT>::value_type &,
            const typename remove_cvref_t<ARGUMENT_IN_CONTEXT>::value_type &>;

        return smd::typeclass::test::Identity<remove_cvref_t<Result>>{
            std::invoke(function.value, argument.value)};
    }
};

/** Applicative typeclass record for Identity<V>; exposes pure and apply. */
template <class VALUE_TYPE>
struct TestIdentityApplicativeMap
    : Applicative<TestIdentityApplicativeImpl<VALUE_TYPE>> {
    using TestIdentityApplicativeImpl<VALUE_TYPE>::apply;
    using TestIdentityApplicativeImpl<VALUE_TYPE>::pure;
};

template <class VALUE_TYPE>
inline constexpr auto
    applicative_typeclass<smd::typeclass::test::Identity<VALUE_TYPE>> =
        TestIdentityApplicativeMap<VALUE_TYPE>{};

/** Applicative implementation for BareIdentity<V>: apply uses forwarding references. */
template <class VALUE_TYPE>
struct BareIdentityApplicativeImpl {
    template <class VALUE>
    auto pure(this auto &&, VALUE &&value) {
        return smd::typeclass::test::BareIdentity<remove_cvref_t<VALUE>>{
            std::forward<VALUE>(value)};
    }

    template <class FUNCTION_IN_CONTEXT, class ARGUMENT_IN_CONTEXT>
    auto apply(this auto &&, FUNCTION_IN_CONTEXT &&function,
               ARGUMENT_IN_CONTEXT &&argument) {
        using Result = std::invoke_result_t<
            decltype(std::forward<FUNCTION_IN_CONTEXT>(function).value),
            decltype(std::forward<ARGUMENT_IN_CONTEXT>(argument).value)>;

        return smd::typeclass::test::BareIdentity<remove_cvref_t<Result>>{
            std::invoke(std::forward<FUNCTION_IN_CONTEXT>(function).value,
                        std::forward<ARGUMENT_IN_CONTEXT>(argument).value)};
    }
};

/** Applicative typeclass record for BareIdentity<V>; exposes pure and apply. */
template <class VALUE_TYPE>
struct BareIdentityApplicativeMap
    : Applicative<BareIdentityApplicativeImpl<VALUE_TYPE>> {
    using BareIdentityApplicativeImpl<VALUE_TYPE>::apply;
    using BareIdentityApplicativeImpl<VALUE_TYPE>::pure;
};

template <class VALUE_TYPE>
inline constexpr auto
    applicative_typeclass<smd::typeclass::test::BareIdentity<VALUE_TYPE>> =
        BareIdentityApplicativeMap<VALUE_TYPE>{};

/** Foldable implementation for Sequence<V>: fold_map walks values left-to-right. */
template <class VALUE_TYPE>
struct TestSequenceFoldableImpl {
    template <class FUNCTION>
    auto fold_map(this auto &&, FUNCTION &&function,
                  const smd::typeclass::test::Sequence<VALUE_TYPE> &sequence) {
        using Result =
            remove_cvref_t<std::invoke_result_t<FUNCTION, const VALUE_TYPE &>>;
        return std::ranges::fold_left(
            sequence.values, smd::monoid_identity<Result>(),
            [&](Result acc, const VALUE_TYPE &value) {
                return smd::monoid_combine(std::move(acc),
                                           std::invoke(function, value));
            });
    }
};

/** Foldable typeclass record for Sequence<V>; exposes fold_map. */
template <class VALUE_TYPE>
struct TestSequenceFoldableMap
    : Foldable<TestSequenceFoldableImpl<VALUE_TYPE>> {
    using TestSequenceFoldableImpl<VALUE_TYPE>::fold_map;
};

template <class VALUE_TYPE>
inline constexpr auto
    foldable_typeclass<smd::typeclass::test::Sequence<VALUE_TYPE>> =
        TestSequenceFoldableMap<VALUE_TYPE>{};

/** Traversable implementation for Identity<V>: traverse applies f to the single value
 * and wraps the result back in Identity inside the applicative effect.
 */
template <class VALUE_TYPE>
struct TestIdentityTraversableImpl {
    using element_type = VALUE_TYPE;

    template <class APPLICATIVE, class FUNCTION>
    auto traverse(this auto &&, const APPLICATIVE &applicative,
                  FUNCTION &&function,
                  const smd::typeclass::test::Identity<VALUE_TYPE> &identity) {
        return applicative.invoke(
            [](auto &&value) {
                using U = remove_cvref_t<decltype(value)>;
                return smd::typeclass::test::Identity<U>{
                    std::forward<decltype(value)>(value)};
            },
            std::invoke(std::forward<FUNCTION>(function), identity.value));
    }
};

/** Traversable typeclass record for Identity<V>; exposes traverse. */
template <class VALUE_TYPE>
struct TestIdentityTraversableMap
    : Traversable<TestIdentityTraversableImpl<VALUE_TYPE>> {
    using TestIdentityTraversableImpl<VALUE_TYPE>::traverse;
};

template <class VALUE_TYPE>
inline constexpr auto
    traversable_typeclass<smd::typeclass::test::Identity<VALUE_TYPE>> =
        TestIdentityTraversableMap<VALUE_TYPE>{};

} // namespace smd

#endif // INCLUDED_SMD_TYPECLASS_TEST_TEST_SUPPORT

```

## smd/typeclass/traversable.hpp

```cpp
// src/smd/typeclass/traversable.hpp                                  -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TYPECLASS_TRAVERSABLE
#define INCLUDED_SMD_TYPECLASS_TRAVERSABLE

#include <smd/typeclass/applicative.hpp>
#include <smd/typeclass/functor.hpp>
#include <smd/typeclass/typeclass_base.hpp>

#include <type_traits>
#include <utility>

namespace smd {

// Traversable pattern invariants:
// - Instances are single lookup objects that provide traverse(F, T).
// - sequence is a derived object operation implemented from traverse(identity).
// - Dispatch happens through a provided object or
// traversable_typeclass<Concrete>.
// - Traversal must preserve container shape while sequencing effects.

/** CRTP base for Traversable instances.
 * `Impl` must provide `traverse(applicative, f, container)` and declare
 * `element_type`. All other operations (`sequence`, `for_each`,
 * `traverse_with`, `sequence_with`) are derived.
 */
template <class Impl>
struct Traversable : protected Impl {
    static_assert(!std::is_same_v<Impl, std::false_type>,
                  "No traversable_typeclass<T> specialization found. "
                  "Specialize smd::traversable_typeclass<T> for your type T, "
                  "provide traverse(applicative, F, T), and declare 'using "
                  "element_type = T;'.");
    static_assert(
        requires { typename Impl::element_type; },
        "Traversable Impl must declare 'using element_type = T;' "
        "so that sequence() and traverse_with() can deduce the element type.");
    // Alternate-core: Impl::traverse is the primitive; sequence is derived from
    // it. A sequence-primitive Impl would shadow sequence instead.
    using Impl::traverse;
    using element_type = typename Impl::element_type;

    // 8f1d5c4a-1a7e-4b9e-8cb4-908f4ab0ca11

    // d5a2c1f8-7e3b-4d1a-c6b2-2f9e5d7a1c46
    /** Applies `function` to each element and sequences the resulting effects;
     * the applicative is inferred from the return type of `function`.
     */
    template <class T, class F>
    auto for_each(this auto &&self, T &&value, F &&function) {
        using Context =
            remove_cvref_t<std::invoke_result_t<F, const element_type &>>;
        const auto &applicative = smd::applicative_typeclass<Context>;
        return self.traverse(applicative, std::forward<F>(function),
                             std::forward<T>(value));
    }
    // d5a2c1f8-7e3b-4d1a-c6b2-2f9e5d7a1c46 end

    // c1f8e7a2-9b6d-4c4f-a5e3-1b2d9c8f6a79
    /** Sequences a container of effectful values into a single effect containing
     * the container. The element type must itself be an applicative context.
     */
    template <class T>
    auto sequence(this auto &&self, T &&value) {
        using Context = element_type;
        const auto &applicative = smd::applicative_typeclass<Context>;
        return self.traverse(
            applicative, [](auto &&x) { return std::forward<decltype(x)>(x); },
            std::forward<T>(value));
    }
    // c1f8e7a2-9b6d-4c4f-a5e3-1b2d9c8f6a79 end

    /** Traverses using a different traversable instance; applicative is inferred
     * from the return type of `function`.
     */
    template <class TRAVERSABLE_MAP, class T, class F>
    auto traverse_with(this auto &&, const TRAVERSABLE_MAP &traversable_map,
                       F &&function, T &&value) {
        using Context = remove_cvref_t<std::invoke_result_t<
            F, const typename remove_cvref_t<TRAVERSABLE_MAP>::element_type &>>;
        const auto &applicative = smd::applicative_typeclass<Context>;
        return traversable_map.traverse(applicative, std::forward<F>(function),
                                        std::forward<T>(value));
    }

    /** Traverses using explicit traversable and applicative instances. */
    template <class TRAVERSABLE_MAP, class APPLICATIVE_MAP, class T, class F>
    auto traverse_with(this auto &&, const TRAVERSABLE_MAP &traversable_map,
                       const APPLICATIVE_MAP &applicative_map, F &&function,
                       T &&value) {
        return traversable_map.traverse(
            applicative_map, std::forward<F>(function), std::forward<T>(value));
    }

    /** Sequences using a different traversable instance; applicative is inferred
     * from the container's element type.
     */
    template <class TRAVERSABLE_MAP, class T>
    auto sequence_with(this auto &&self, const TRAVERSABLE_MAP &traversable_map,
                       T &&value) {
        return self.traverse_with(
            traversable_map,
            [](auto &&x) { return std::forward<decltype(x)>(x); },
            std::forward<T>(value));
    }
    // 8f1d5c4a-1a7e-4b9e-8cb4-908f4ab0ca11 end
};

/** Typeclass lookup variable for Traversable; specialize for each container type. */
template <class T>
inline constexpr auto traversable_typeclass = std::false_type{};

/** @brief Maps `function` over `value`, sequences effects left-to-right,
 *         and preserves container shape.
 *
 * @param function  A callable returning an applicative effect for each element.
 * @param value     The traversable container to process.
 * @return          The container shape lifted into the applicative effect.
 */
template <class F, class T>
auto traverse(F &&function, T &&value) {
    const auto &map = traversable_typeclass<remove_cvref_t<T>>;
    using element_type = typename remove_cvref_t<decltype(map)>::element_type;
    using Context =
        remove_cvref_t<std::invoke_result_t<F, const element_type &>>;
    const auto &applicative = applicative_typeclass<Context>;
    return map.traverse(applicative, std::forward<F>(function),
                        std::forward<T>(value));
}

} // namespace smd

#endif

```

## smd/typeclass/traversable.t.cpp

```cpp
#include <smd/typeclass/traversable.hpp>
#include <smd/typeclass/traversable.hpp> // Re-inclusion check

#include <catch2/catch_test_macros.hpp>
#include <smd/typeclass/test/test_support.hpp>

#include <beman/optional/optional.hpp>

#include <optional>

TEST_CASE("TraversableTypeclassTest - TraverseOptionalSuccess") {
    using Identity = smd::typeclass::test::Identity<int>;
    auto identity = Identity{1};

    auto traversed = smd::traverse(
        [](int x) -> std::optional<int> {
            return x >= 0 ? std::optional<int>{x + 1} : std::optional<int>{};
        },
        identity);

    REQUIRE(traversed.has_value());
    CHECK(traversed->value == 2);
}

TEST_CASE("TraversableTypeclassTest - TraverseOptionalFailure") {
    using Identity = smd::typeclass::test::Identity<int>;
    auto identity = Identity{-2};

    auto traversed = smd::traverse(
        [](int x) -> std::optional<int> {
            return x >= 0 ? std::optional<int>{x + 1} : std::optional<int>{};
        },
        identity);

    CHECK_FALSE(traversed.has_value());
}

TEST_CASE("TraversableTypeclassTest - ForEachOptionalSuccess") {
    using Identity = smd::typeclass::test::Identity<int>;
    auto identity = Identity{3};
    const auto &traversable = smd::traversable_typeclass<Identity>;

    auto traversed =
        traversable.for_each(identity, [](int x) -> std::optional<int> {
            return std::optional<int>{x * 2};
        });

    REQUIRE(traversed.has_value());
    CHECK(traversed->value == 6);
}

TEST_CASE("TraversableTypeclassTest - SequenceAndSequenceWith") {
    // f1de12e0-2287-4568-98c7-75be4f6f7446
    // e7b4a1f9-3c8d-4e2a-b5f7-1d9c3e5a7b28
    using IdentityOpt = smd::typeclass::test::Identity<std::optional<int>>;
    auto identity = IdentityOpt{std::optional<int>{1}};
    const auto &traversable = smd::traversable_typeclass<IdentityOpt>;

    auto sequenced = traversable.sequence(identity);
    REQUIRE(sequenced.has_value());
    CHECK(sequenced->value == 1);
    // e7b4a1f9-3c8d-4e2a-b5f7-1d9c3e5a7b28 end

    auto sequenced_with = traversable.sequence_with(traversable, identity);
    REQUIRE(sequenced_with.has_value());
    CHECK(sequenced_with->value == 1);
    // f1de12e0-2287-4568-98c7-75be4f6f7446 end
}

TEST_CASE("TraversableTypeclassTest - ForEachMatchesTraverse") {
    using Identity = smd::typeclass::test::Identity<int>;
    auto identity = Identity{4};
    const auto &traversable = smd::traversable_typeclass<Identity>;

    auto via_traverse = smd::traverse(
        [](int x) -> std::optional<int> { return std::optional<int>{x + 7}; },
        identity);
    auto via_for_each =
        traversable.for_each(identity, [](int x) -> std::optional<int> {
            return std::optional<int>{x + 7};
        });

    CHECK(via_traverse == via_for_each);
}

TEST_CASE("TraversableTypeclassTest - SequenceMatchesTraverseIdentity") {
    using IdentityOpt = smd::typeclass::test::Identity<std::optional<int>>;
    auto identity = IdentityOpt{std::optional<int>{5}};
    const auto &traversable = smd::traversable_typeclass<IdentityOpt>;

    auto via_sequence = traversable.sequence(identity);
    auto via_traverse_identity = smd::traverse(
        [](auto &&x) { return std::forward<decltype(x)>(x); }, identity);

    CHECK(via_sequence == via_traverse_identity);
}

TEST_CASE("TraversableTypeclassTest - IdentityLawWithIdentityApplicative") {
    using Identity = smd::typeclass::test::Identity<int>;
    const auto &applicative = smd::applicative_typeclass<Identity>;

    auto value = Identity{42};

    auto lhs = smd::traverse([](int x) { return applicative.pure(x); }, value);
    auto rhs = applicative.pure(value);

    CHECK(lhs == rhs);
}

TEST_CASE("TraversableTypeclassTest - TraverseMapCoherence") {
    using Identity = smd::typeclass::test::Identity<int>;

    auto value = Identity{7};

    auto via_traverse = smd::traverse(
        [](int x) -> std::optional<int> { return std::optional<int>{x + 1}; },
        value);

    auto via_mapped_traverse = smd::traverse(
        [](int x) -> std::optional<int> {
            return std::optional<int>{(x + 1) * 3};
        },
        value);

    REQUIRE(via_traverse.has_value());
    auto mapped = std::optional<smd::typeclass::test::Identity<int>>{
        smd::typeclass::test::Identity<int>{via_traverse->value * 3}};

    CHECK(mapped == via_mapped_traverse);
}

TEST_CASE("TraversableTypeclassTest - CompositionLawViaNestedOptional") {
    using Identity = smd::typeclass::test::Identity<int>;

    auto value = Identity{9};

    auto f = [](int x) -> std::optional<int> {
        return x >= 0 ? std::optional<int>{x + 2} : std::optional<int>{};
    };
    auto g = [](int x) -> std::optional<int> {
        return x % 2 == 0 ? std::optional<int>{x / 2} : std::optional<int>{};
    };

    auto lhs = smd::traverse(
        [&](int x) -> std::optional<std::optional<int>> {
            auto fx = f(x);
            if (!fx.has_value()) {
                return std::optional<std::optional<int>>{std::optional<int>{}};
            }
            return std::optional<std::optional<int>>{g(*fx)};
        },
        value);

    auto rhs = [&]() -> std::optional<std::optional<Identity>> {
        auto traversed_once = smd::traverse(f, value);
        if (!traversed_once.has_value()) {
            return std::optional<std::optional<Identity>>{
                std::optional<Identity>{}};
        }

        auto traversed_twice = smd::traverse(g, *traversed_once);
        return std::optional<std::optional<Identity>>{traversed_twice};
    }();

    auto unwrap_identity =
        [](const std::optional<std::optional<Identity>> &nested)
        -> std::optional<std::optional<int>> {
        if (!nested.has_value()) {
            return std::optional<std::optional<int>>{};
        }
        if (!nested->has_value()) {
            return std::optional<std::optional<int>>{std::optional<int>{}};
        }
        return std::optional<std::optional<int>>{
            std::optional<int>{nested->value().value}};
    };

    auto unwrap_traversed =
        [](const std::optional<
            smd::typeclass::test::Identity<std::optional<int>>> &traversed)
        -> std::optional<std::optional<int>> {
        if (!traversed.has_value()) {
            return std::optional<std::optional<int>>{};
        }
        return std::optional<std::optional<int>>{traversed->value};
    };

    CHECK(unwrap_traversed(lhs) == unwrap_identity(rhs));
}

TEST_CASE("TraversableTypeclassTest - NaturalityLawWithOptional") {
    using Identity = smd::typeclass::test::Identity<int>;

    auto value = Identity{8};

    auto effectful = [](int x) -> std::optional<int> {
        return x >= 0 ? std::optional<int>{x + 5} : std::optional<int>{};
    };
    auto natural = [](int x) { return x * 3; };

    auto lhs = smd::traverse(effectful, value);
    auto lhs_mapped = std::optional<Identity>{};
    if (lhs.has_value()) {
        lhs_mapped = Identity{natural(lhs->value)};
    }

    auto rhs = smd::traverse(
        [&](int x) -> std::optional<int> {
            auto result = effectful(x);
            if (!result.has_value()) {
                return std::optional<int>{};
            }
            return std::optional<int>{natural(*result)};
        },
        value);

    CHECK(lhs_mapped == rhs);
}

TEST_CASE("TraversableLaws - NaturalityLaw") {
    // Naturality law: an applicative morphism commutes with traverse.
    // to_beman: std::optional<B> → beman::optional<B> is one such morphism.
    // Law: to_beman(traverse f t) == traverse (f_returning_beman) t
    using Identity = smd::typeclass::test::Identity<int>;

    auto f = [](int x) -> std::optional<int> {
        return x > 0 ? std::optional<int>{x * 2} : std::optional<int>{};
    };
    auto f_returning_beman = [](int x) -> beman::optional::optional<int> {
        return x > 0 ? beman::optional::optional<int>{x * 2}
                     : beman::optional::optional<int>{};
    };
    auto to_beman =
        [](std::optional<Identity> o) -> beman::optional::optional<Identity> {
        return o.has_value() ? beman::optional::optional<Identity>{*o}
                             : beman::optional::optional<Identity>{};
    };

    // Present case: f(3) == {6}, to_beman({Identity{6}}) == {Identity{6}}
    // a9e4c2f1-7d6b-4a3c-e5b2-8f3d1e9c6a43
    {
        auto value = Identity{3};
        CHECK(to_beman(smd::traverse(f, value)) ==
              smd::traverse(f_returning_beman, value));
    }
    // a9e4c2f1-7d6b-4a3c-e5b2-8f3d1e9c6a43 end

    // Absent case: f(-1) == {}, to_beman({}) == {}
    {
        auto value = Identity{-1};
        CHECK(to_beman(smd::traverse(f, value)) ==
              smd::traverse(f_returning_beman, value));
    }
}

// NullOpt applicative: pure/apply always return an empty optional.
// Used to guard that traverse_with honors the explicitly passed applicative
// rather than re-looking up applicative_typeclass<std::optional<int>>.
struct NullOptImpl {
    template <class V>
    auto pure(this auto &&, V &&) -> std::optional<std::remove_cvref_t<V>> {
        return {};
    }

    template <class FUNCTION_IN_CONTEXT, class ARGUMENT_IN_CONTEXT>
    auto apply(this auto &&, FUNCTION_IN_CONTEXT &&function,
               ARGUMENT_IN_CONTEXT &&argument) {
        using Result =
            std::invoke_result_t<decltype(*function), decltype(*argument)>;
        return std::optional<std::remove_cvref_t<Result>>{};
    }
};

// Alternate-core: both pure and apply are primitives here (symmetric
// Applicative).
struct NullOptMap : smd::Applicative<NullOptImpl> {
    using NullOptImpl::apply;
    using NullOptImpl::pure;
};

TEST_CASE("TraversableTypeclassTest - TraverseWithHonorsExplicitApplicative") {
    using Identity = smd::typeclass::test::Identity<int>;
    const auto &traversable = smd::traversable_typeclass<Identity>;

    auto value = Identity{10};
    auto f = [](int x) -> std::optional<int> {
        return std::optional<int>{x + 1};
    };

    // Default path — must succeed.
    auto default_result = smd::traverse(f, value);
    REQUIRE(default_result.has_value());
    CHECK(default_result->value == 11);

    // traverse_with with NullOptMap must return empty: if the impl uses the
    // explicit applicative, pure() returns nullopt and the result is empty.
    // If it re-looks up the default applicative_typeclass, the result is
    // non-empty.
    NullOptMap null_opt{};
    auto custom_result =
        traversable.traverse_with(traversable, null_opt, f, value);
    CHECK_FALSE(custom_result.has_value());
}

```

## smd/typeclass/typeclass_base.hpp

```cpp
// src/smd/typeclass/typeclass_base.hpp                               -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TYPECLASS_TYPECLASS_BASE
#define INCLUDED_SMD_TYPECLASS_TYPECLASS_BASE

#include <beman/optional/optional.hpp>

#include <optional>
#include <type_traits>

namespace smd {

// Design invariants for the typeclass object pattern:
// - Per-concept lookup objects (for example *_typeclass<T>) are the
//   customization lookup points for typeclass dispatch.
// - Generic algorithms call through looked-up typeclass objects.
// - New concepts should keep lookup static and explicit.
// - Avoid adding parallel ADL-only customization paths for the same concept.

template <class T>
using remove_cvref_t = std::remove_cvref_t<T>;

/** Trait that extracts the element type from an applicative container.
 * Primary template uses the nested `value_type` alias when present.
 */
template <class T, class = void>
struct applicative_value;

template <class T>
struct applicative_value<T,
                         std::void_t<typename remove_cvref_t<T>::value_type>> {
    using type = typename remove_cvref_t<T>::value_type;
};

template <class T>
struct applicative_value<std::optional<T>, void> {
    using type = T;
};

template <class T>
struct applicative_value<beman::optional::optional<T>, void> {
    using type = T;
};

/** Convenience alias for `applicative_value<T>::type`. */
template <class T>
using applicative_value_t = typename applicative_value<remove_cvref_t<T>>::type;

} // namespace smd

#endif // INCLUDED_SMD_TYPECLASS_TYPECLASS_BASE

```

## smd/typeclass/typeclass_base.t.cpp

```cpp
#include <smd/typeclass/typeclass_base.hpp>
#include <smd/typeclass/typeclass_base.hpp> // Re-inclusion check

#include <catch2/catch_test_macros.hpp>

#include <type_traits>

TEST_CASE("TypeclassBaseTest - RemoveCvrefAlias") {
    static_assert(std::is_same_v<smd::remove_cvref_t<const int &>, int>);
}

```

## smd/ziplist/zip_list_applicative.hpp

```cpp
// src/smd/ziplist/zip_list_applicative.hpp                           -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_ZIPLIST_ZIP_LIST_APPLICATIVE
#define INCLUDED_SMD_ZIPLIST_ZIP_LIST_APPLICATIVE

#include <smd/typeclass/applicative.hpp>
#include <smd/ziplist/zip_list.hpp>

#include <algorithm>
#include <cstddef>
#include <functional>
#include <optional>
#include <type_traits>
#include <utility>

namespace smd {

namespace detail {

template <class T>
auto zip_list_finite_length(const zip_list<T> &list)
    -> std::optional<std::size_t> {
    if (list.is_repeating()) {
        return std::nullopt;
    }
    return list.finite_size();
}

template <class T>
auto zip_list_value_at(const zip_list<T> &list, std::size_t index)
    -> const T & {
    if (list.is_repeating()) {
        return *list.repeated;
    }
    return list.data[index];
}

template <class FIRST, class... REST>
auto zip_list_result_size(const FIRST &first, const REST &...rest)
    -> std::optional<std::size_t> {
    auto count = zip_list_finite_length(first);
    ((count = count
                  ? std::optional<std::size_t>{std::min(
                        *count, zip_list_finite_length(rest).value_or(*count))}
                  : zip_list_finite_length(rest)),
     ...);
    return count;
}

} // namespace detail

/** Applicative typeclass instance for zip_list<T> with positional (zip)
 * semantics.
 * 
 * pure(x) = infinite repetition of x (zip_list::repeat(x)).
 * apply(fs, xs) zips functions and arguments positionally, truncating to the
 * length of the shortest finite operand. If all operands are infinite the
 * result is also infinite (repeating f(x) for the first positions).
 * @tparam T element type of the zip_list holding function values
 */
template <class T>
struct ZipListApplicativeImpl {
    /** Lift a value into an infinite zip_list repeating that value. */
    template <class VALUE>
    auto pure(this auto &&, VALUE &&value) {
        using U = remove_cvref_t<VALUE>;
        return zip_list<U>::repeat(U(std::forward<VALUE>(value)));
    }

    /**
     * @brief Zip functions and arguments positionally; truncate to shortest finite.
     * @param functions zip_list of callables
     * @param arguments zip_list of arguments
     * @return zip_list of results; infinite only when both operands are infinite
     */
    template <class F, class A>
    auto apply(this auto &&, const zip_list<F> &functions,
               const zip_list<A> &arguments) {
        using Result = std::invoke_result_t<const F &, const A &>;
        using U = remove_cvref_t<Result>;

        const auto count = detail::zip_list_result_size(functions, arguments);
        if (!count.has_value()) {
            return zip_list<U>::repeat(
                std::invoke(detail::zip_list_value_at(functions, 0),
                            detail::zip_list_value_at(arguments, 0)));
        }

        zip_list<U> result;
        result.data.reserve(*count);

        for (std::size_t index = 0; index < *count; ++index) {
            result.data.push_back(
                std::invoke(detail::zip_list_value_at(functions, index),
                            detail::zip_list_value_at(arguments, index)));
        }

        return result;
    }

    /**
     * @brief N-ary positional application: apply @p function over all zip_lists
     *        element-wise, truncating to the shortest finite operand.
     * @param function  callable accepting one element from each input list
     * @param first     first zip_list
     * @param rest      remaining zip_lists
     * @return zip_list of results
     */
    template <class FUNCTION, class FIRST, class... REST>
    auto invoke(this auto &&, FUNCTION &&function, const FIRST &first,
                const REST &...rest) {
        using Result =
            std::invoke_result_t<FUNCTION, const typename FIRST::value_type &,
                                 const typename REST::value_type &...>;

        using U = remove_cvref_t<Result>;
        auto callable = std::forward<FUNCTION>(function);
        const auto count = detail::zip_list_result_size(first, rest...);

        if (!count.has_value()) {
            return zip_list<U>::repeat(
                std::invoke(callable, detail::zip_list_value_at(first, 0),
                            detail::zip_list_value_at(rest, 0)...));
        }

        zip_list<U> result;
        result.data.reserve(*count);

        for (std::size_t index = 0; index < *count; ++index) {
            result.data.push_back(
                std::invoke(callable, detail::zip_list_value_at(first, index),
                            detail::zip_list_value_at(rest, index)...));
        }

        return result;
    }
};

/** Applicative map exposing pure, apply, and invoke for zip_list<T>. */
template <class T>
struct ZipListApplicativeMap : Applicative<ZipListApplicativeImpl<T>> {
    using ZipListApplicativeImpl<T>::apply;
    using ZipListApplicativeImpl<T>::pure;
};

/** Registers ZipListApplicativeMap as the Applicative instance for zip_list<T>. */
template <class T>
inline constexpr auto applicative_typeclass<zip_list<T>> =
    ZipListApplicativeMap<T>{};

} // namespace smd

#endif

```

## smd/ziplist/zip_list_applicative.t.cpp

```cpp
#include <smd/ziplist/zip_list_applicative.hpp>
#include <smd/ziplist/zip_list_applicative.hpp> // Re-inclusion check

#include <catch2/catch_test_macros.hpp>
#include <smd/typeclass/applicative.hpp>
#include <smd/typeclass/test/test_support.hpp>
#include <smd/ziplist/zip_list.hpp>

#include <functional>
#include <vector>

TEST_CASE("ZipListApplicativeTest - PureBreathing") {
    const auto &applicative = smd::applicative_typeclass<smd::zip_list<int>>;
    auto lifted = applicative.pure(9);

    CHECK(lifted.is_repeating());
    REQUIRE(lifted.repeated.has_value());
    CHECK(*lifted.repeated == 9);
}

TEST_CASE("ZipListApplicativeTest - ApplyZips") {
    smd::zip_list<int (*)(int)> functions{{
        +[](int x) { return x + 1; },
        +[](int x) { return x * 2; },
        +[](int x) { return x - 3; },
    }};
    smd::zip_list<int> arguments{{10, 10}};
    const auto &applicative =
        smd::applicative_typeclass<smd::zip_list<int (*)(int)>>;

    auto result = applicative.apply(functions, arguments);
    CHECK(result.data == (std::vector<int>{11, 20}));
}

TEST_CASE("ZipListApplicativeTest - PureBroadcastsAcrossFiniteInput") {
    const auto &applicative = smd::applicative_typeclass<smd::zip_list<int>>;
    smd::zip_list<int> xs{{1, 2, 3}};

    auto result =
        applicative.ap(applicative.pure(+[](int x) { return x + 10; }), xs);

    CHECK(result.data == (std::vector<int>{11, 12, 13}));
}

TEST_CASE("ZipListApplicativeTest - IdentityLawOnFiniteInput") {
    const auto &applicative = smd::applicative_typeclass<smd::zip_list<int>>;
    smd::zip_list<int> xs{{4, 5, 6}};

    auto result =
        applicative.ap(applicative.pure(+[](int x) { return x; }), xs);

    CHECK(result.data == xs.data);
}

TEST_CASE("ZipListApplicativeTest - BothPureProducesRepeatingResult") {
    const auto &applicative = smd::applicative_typeclass<smd::zip_list<int>>;

    auto result = applicative.ap(applicative.pure(+[](int x) { return x * 2; }),
                                 applicative.pure(7));

    CHECK(result.is_repeating());
    REQUIRE(result.repeated.has_value());
    CHECK(*result.repeated == 14);
}

TEST_CASE("ZipListApplicativeTest - InvokeZipsMultipleArguments") {
    smd::zip_list<int> xs{{1, 2, 3}};
    smd::zip_list<int> ys{{10, 20}};
    smd::zip_list<int> zs{{100, 200, 300, 400}};
    const auto &applicative = smd::applicative_typeclass<smd::zip_list<int>>;

    auto result = applicative.invoke(
        [](int x, int y, int z) { return x + y + z; }, xs, ys, zs);

    CHECK(result.data == (std::vector<int>{111, 222}));
}

TEST_CASE("ZipListApplicativeTest - InvokeWithPureAndFiniteArguments") {
    const auto &applicative = smd::applicative_typeclass<smd::zip_list<int>>;
    smd::zip_list<int> ys{{10, 20, 30}};

    auto result =
        applicative.invoke([](int x, int y, int z) { return x + y + z; },
                           applicative.pure(1), ys, applicative.pure(100));

    CHECK(result.data == (std::vector<int>{111, 121, 131}));
}

TEST_CASE("ZipListApplicativeTest - InterchangeLaw") {
    const auto &applicative = smd::applicative_typeclass<smd::zip_list<int>>;

    smd::zip_list<std::function<int(int)>> functions{{
        [](int x) { return x + 1; },
        [](int x) { return x * 3; },
        [](int x) { return x - 2; },
    }};
    const int value = 7;

    auto lhs = applicative.ap(functions, applicative.pure(value));
    auto rhs = applicative.ap(
        applicative.pure([](const std::function<int(int)> &function) {
            return function(value);
        }),
        functions);

    CHECK(lhs.data == rhs.data);
}

TEST_CASE("ZipListApplicativeTest - CompositionLaw") {
    const auto &applicative = smd::applicative_typeclass<smd::zip_list<int>>;

    smd::zip_list<std::function<int(int)>> u{{
        [](int x) { return x + 10; },
        [](int x) { return x * 2; },
    }};
    smd::zip_list<std::function<int(int)>> v{{
        [](int x) { return x - 3; },
        [](int x) { return x + 4; },
    }};
    smd::zip_list<int> w{{5, 6, 7}};

    auto compose = [](const std::function<int(int)> &f) {
        return [f](const std::function<int(int)> &g) {
            return [f, g](int x) { return f(g(x)); };
        };
    };

    auto lhs = applicative.ap(
        applicative.ap(applicative.ap(applicative.pure(compose), u), v), w);

    auto rhs = applicative.ap(u, applicative.ap(v, w));

    CHECK(lhs.data == rhs.data);
}

TEST_CASE("ZipListApplicativeTest - IdentityHomomorphismAndInvokeViaHarness") {
    CHECK(smd::typeclass::test::check_applicative_identity_law(
        smd::zip_list<int>{{4, 5, 6}}));
    CHECK(smd::typeclass::test::check_applicative_homomorphism_law<
          smd::zip_list<int>>(+[](int x) { return x + 9; }, 3));
    CHECK(smd::typeclass::test::check_applicative_invoke_binary_law(
        [](int a, int b) { return a * 10 + b; }, smd::zip_list<int>{{1, 2, 3}},
        smd::zip_list<int>{{7, 8, 9}}));
}

```

## smd/ziplist/zip_list.hpp

```cpp
// src/smd/ziplist/zip_list.hpp                                       -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_ZIPLIST_ZIP_LIST
#define INCLUDED_SMD_ZIPLIST_ZIP_LIST

#include <cstddef>
#include <optional>
#include <utility>
#include <vector>

namespace smd {

/** List with positional (zip) applicative semantics that can represent an
 * infinite repetition of a single value.
 * 
 * A zip_list is either finite (elements stored in @c data) or infinite
 * (a single @c repeated value that logically occupies every position). The
 * ZipList applicative applies functions positionally and truncates to the
 * shortest finite operand; pure(x) yields the infinite repetition of x so
 * that it acts as an identity for truncation.
 * 
 * Invariant: when @c repeated has a value, @c data is ignored and the
 * zip_list models an infinite repetition of @c repeated.
 * @tparam T element type
 */
template <class T>
struct zip_list {
    using value_type = T;

    // Invariant: when repeated has a value, this zip_list models an infinite
    // repetition of that value and data is ignored.
    std::vector<T> data;
    std::optional<T> repeated{};

    /** Construct an infinite zip_list repeating @p value at every position. */
    static auto repeat(T value) -> zip_list {
        return zip_list{{}, std::move(value)};
    }

    /** True when this zip_list represents an infinite repetition. */
    auto is_repeating() const -> bool { return repeated.has_value(); }

    /** Number of elements in the finite representation; 0 for infinite lists. */
    auto finite_size() const -> std::size_t { return data.size(); }

    /** Equality: two infinite lists are equal iff they repeat the same value;
     * two finite lists use element-wise comparison; mixed always false.
     */
    friend auto operator==(const zip_list &left, const zip_list &right)
        -> bool {
        if (left.is_repeating() || right.is_repeating()) {
            return left.repeated == right.repeated;
        }
        return left.data == right.data;
    }
};

} // namespace smd

#endif

```

## smd/ziplist/zip_list.t.cpp

```cpp
// src/smd/ziplist/zip_list.t.cpp                                     -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <smd/ziplist/zip_list.hpp>
#include <smd/ziplist/zip_list.hpp> // Re-inclusion check

#include <catch2/catch_test_macros.hpp>

TEST_CASE("ZipListTest - FiniteListConstruction") {
    smd::zip_list<int> zl{{1, 2, 3}};
    CHECK_FALSE(zl.is_repeating());
    CHECK(zl.finite_size() == 3);
    CHECK(zl.data == (std::vector<int>{1, 2, 3}));
}

TEST_CASE("ZipListTest - InfiniteRepeat") {
    auto zl = smd::zip_list<int>::repeat(42);
    CHECK(zl.is_repeating());
    CHECK(zl.repeated == std::optional{42});
}

TEST_CASE("ZipListTest - EqualityFinite") {
    smd::zip_list<int> a{{1, 2, 3}};
    smd::zip_list<int> b{{1, 2, 3}};
    smd::zip_list<int> c{{1, 2}};
    CHECK(a == b);
    CHECK_FALSE(a == c);
}

TEST_CASE("ZipListTest - EqualityRepeating") {
    auto r1 = smd::zip_list<int>::repeat(7);
    auto r2 = smd::zip_list<int>::repeat(7);
    auto r3 = smd::zip_list<int>::repeat(9);
    CHECK(r1 == r2);
    CHECK_FALSE(r1 == r3);
}

```

