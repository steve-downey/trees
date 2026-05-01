---
title: Live Source Snapshot (main, strict built-target graph)
summary: Point-in-time fenced dump of trees/src C++ files explicitly listed in CMake target_sources() on main.
source_of_truth: git HEAD on branch main
strictness: only explicit target_sources entries; excludes transitively included headers
scope:
  include:
    - files directly named in trees/src/**/CMakeLists.txt target_sources()
  exclude:
    - trees/src/deadcode/**
    - trees/src/smd/conceptmap/**
update_policy:
  when_to_update:
    - Any time target_sources lists in trees/src/**/CMakeLists.txt change.
    - Any time a listed file changes on main and this snapshot is used as a baseline.
  how_to_update:
    - Rebuild the explicit built-file manifest from CMakeLists and regenerate from git HEAD.
notes:
  - Section headers are canonical paths without the leading src/ prefix.
  - File contents are copied from git (HEAD), not the working tree.
---

# Live Source Snapshot (main, strict built-target graph)

Generated from main at commit 6b0ab0b.

This file includes only source files explicitly listed in CMake target_sources() entries under trees/src.

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

template <class VIEW>
struct ListRangeApplicativeImpl {
    template <class VALUE>
    auto pure(this auto&&, VALUE&& value)
    {
        using Stored = remove_cvref_t<VALUE>;
        return smd::ranges::from_vector(
            std::vector<Stored>{std::forward<VALUE>(value)});
    }

    template <class FUNCTION_VIEW, class ARGUMENT_VIEW>
    auto apply(this auto&&,
               const smd::ranges::list_range<FUNCTION_VIEW>& functions,
               const smd::ranges::list_range<ARGUMENT_VIEW>& arguments)
    {
        using Function = std::ranges::range_value_t<smd::ranges::list_range<FUNCTION_VIEW> >;
        using Argument = std::ranges::range_value_t<smd::ranges::list_range<ARGUMENT_VIEW> >;
        using Result = std::invoke_result_t<const Function&, const Argument&>;

        auto function_values = smd::ranges::detail::materialize(functions);
        auto argument_values = smd::ranges::detail::materialize(arguments);
        std::vector<remove_cvref_t<Result> > output;
        output.reserve(function_values.size() * argument_values.size());

        for (const auto& function : function_values) {
            for (const auto& argument : argument_values) {
                output.push_back(std::invoke(function, argument));
            }
        }

        return smd::ranges::from_vector(std::move(output));
    }
};

template <class VIEW>
struct ListRangeApplicativeMap : Applicative<ListRangeApplicativeImpl<VIEW> > {
    using ListRangeApplicativeImpl<VIEW>::apply;
    using ListRangeApplicativeImpl<VIEW>::pure;
};

template <class VIEW>
inline constexpr auto applicative_typeclass<smd::ranges::list_range<VIEW> > =
    ListRangeApplicativeMap<VIEW>{};

}  // close namespace smd

#endif

```

## smd/ranges/range_applicative.t.cpp

```cpp
#include <smd/ranges/range_applicative.hpp>

#include <catch2/catch_test_macros.hpp>

#include <vector>

namespace {

template <std::ranges::input_range RANGE>
auto collect(RANGE&& range)
{
    using Value = std::ranges::range_value_t<RANGE>;
    std::vector<Value> values;

    for (auto&& value : range) {
        values.emplace_back(value);
    }

    return values;
}

}  // namespace

TEST_CASE("RangeApplicativeTest - PureCreatesSingletonRange")
{
    const auto& applicative =
        smd::applicative_typeclass<decltype(smd::ranges::single(1))>;

    auto singleton = applicative.pure(7);

    CHECK(collect(singleton) == (std::vector<int>{7}));
}

TEST_CASE("RangeApplicativeTest - InvokeUsesListNondeterminism")
{
    auto lhs = smd::ranges::from_vector(std::vector<int>{1, 2});
    auto rhs = smd::ranges::from_vector(std::vector<int>{10, 20});
    const auto& applicative = smd::applicative_typeclass<decltype(lhs)>;

    auto summed = applicative.invoke(
        [](int left, int right) { return left + right; },
        lhs,
        rhs);

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

template <class VIEW>
struct ListRangeFoldableImpl {
    template <class FUNCTION>
    auto fold_map(this auto&& self,
                  FUNCTION&& function,
                  const smd::ranges::list_range<VIEW>& values)
    {
        using Result = remove_cvref_t<
            std::invoke_result_t<FUNCTION, const typename smd::ranges::list_range<VIEW>::value_type&> >;

        return self.fold_left(
            values,
            smd::monoid_identity<Result>(),
            [&function](Result acc, const auto& value) {
                return smd::monoid_combine(
                    std::move(acc),
                    std::invoke(function, value));
            });
    }

    auto length(this auto&&,
                const smd::ranges::list_range<VIEW>& values) -> std::size_t
    {
        return static_cast<std::size_t>(std::ranges::distance(values));
    }

    template <class STATE, class FUNCTION>
    auto fold_left(this auto&&,
                   const smd::ranges::list_range<VIEW>& values,
                   STATE initial_state,
                   FUNCTION&& function)
    {
        return std::ranges::fold_left(
            values,
            std::move(initial_state),
            std::forward<FUNCTION>(function));
    }

    template <class STATE, class FUNCTION>
    auto fold_right(this auto&&,
                    const smd::ranges::list_range<VIEW>& values,
                    STATE initial_state,
                    FUNCTION&& function)
    {
        return std::ranges::fold_right(
            smd::ranges::detail::materialize(values),
            std::move(initial_state),
            std::forward<FUNCTION>(function));
    }

    template <class PREDICATE>
    auto any_of(this auto&&,
                const smd::ranges::list_range<VIEW>& values,
                PREDICATE&& predicate) -> bool
    {
        return std::ranges::any_of(values, std::forward<PREDICATE>(predicate));
    }

    template <class PREDICATE>
    auto all_of(this auto&&,
                const smd::ranges::list_range<VIEW>& values,
                PREDICATE&& predicate) -> bool
    {
        return std::ranges::all_of(values, std::forward<PREDICATE>(predicate));
    }

    auto empty(this auto&&,
               const smd::ranges::list_range<VIEW>& values) -> bool
    {
        return std::ranges::empty(values);
    }

    auto to_vector(this auto&&,
                   const smd::ranges::list_range<VIEW>& values)
    {
        return smd::ranges::detail::materialize(values);
    }

    template <class PREDICATE>
    auto find_first(this auto&&,
                    const smd::ranges::list_range<VIEW>& values,
                    PREDICATE&& predicate)
    {
        auto it = std::ranges::find_if(values, std::forward<PREDICATE>(predicate));
        if (it == std::ranges::end(values)) {
            return std::optional<typename smd::ranges::list_range<VIEW>::value_type>{};
        }
        return std::optional<typename smd::ranges::list_range<VIEW>::value_type>{*it};
    }
};

template <class VIEW>
struct ListRangeFoldableMap : Foldable<ListRangeFoldableImpl<VIEW> > {
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

template <class VIEW>
inline constexpr auto foldable_typeclass<smd::ranges::list_range<VIEW> > =
    ListRangeFoldableMap<VIEW>{};

}  // close namespace smd

#endif

```

## smd/ranges/range_foldable.t.cpp

```cpp
#include <smd/ranges/range_foldable.hpp>

#include <catch2/catch_test_macros.hpp>

#include <ranges>
#include <vector>

TEST_CASE("RangeFoldableTest - LengthAndFoldLeftFollowRangeOrder")
{
    auto values = smd::ranges::all(std::views::iota(1, 5));
    const auto& foldable = smd::foldable_typeclass<decltype(values)>;

    CHECK(foldable.length(values) == 4U);

    const auto folded = foldable.fold_left(values, 0, [](int acc, int value) {
        return acc * 10 + value;
    });
    CHECK(folded == 1234);
}

TEST_CASE("RangeFoldableTest - ToVectorMaterializesValues")
{
    auto values = smd::ranges::from_vector(std::vector<int>{3, 1, 4});
    const auto& foldable = smd::foldable_typeclass<decltype(values)>;

    CHECK(foldable.to_vector(values) == (std::vector<int>{3, 1, 4}));
}

TEST_CASE("RangeFoldableTest - PredicatesAndFindUseRangeSemantics")
{
    auto values = smd::ranges::all(std::views::iota(1, 6));
    const auto& foldable = smd::foldable_typeclass<decltype(values)>;

    CHECK(foldable.any_of(values, [](int value) { return value == 4; }));
    CHECK_FALSE(foldable.all_of(values, [](int value) { return value < 5; }));
    CHECK_FALSE(foldable.empty(values));

    auto found = foldable.find_first(values, [](int value) { return value > 3; });
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

template <class VIEW>
struct ListRangeFunctorImpl {
    template <class FUNCTION>
    auto fmap(this auto&&,
              FUNCTION&& function,
              const smd::ranges::list_range<VIEW>& values)
    {
        return smd::ranges::all(
            values | std::views::transform(std::forward<FUNCTION>(function)));
    }
};

template <class VIEW>
struct ListRangeFunctorMap : Functor<ListRangeFunctorImpl<VIEW> > {
    using ListRangeFunctorImpl<VIEW>::fmap;
};

template <class VIEW>
inline constexpr auto functor_typeclass<smd::ranges::list_range<VIEW> > =
    ListRangeFunctorMap<VIEW>{};

}  // close namespace smd

#endif

```

## smd/ranges/range_functor.t.cpp

```cpp
#include <smd/ranges/range_functor.hpp>

#include <catch2/catch_test_macros.hpp>

#include <ranges>
#include <vector>

namespace {

template <std::ranges::input_range RANGE>
auto collect(RANGE&& range)
{
    using Value = std::ranges::range_value_t<RANGE>;
    std::vector<Value> values;

    for (auto&& value : range) {
        values.emplace_back(value);
    }

    return values;
}

}  // namespace

TEST_CASE("RangeFunctorTest - FmapUsesLazyRangeSemantics")
{
    auto values = smd::ranges::all(std::views::iota(1, 5));
    const auto& functor = smd::functor_typeclass<decltype(values)>;

    auto mapped = functor.fmap([](int value) { return value * 10; }, values);

    CHECK(collect(mapped) == (std::vector<int>{10, 20, 30, 40}));
}

TEST_CASE("RangeFunctorTest - ReplaceKeepsRangeShape")
{
    auto values = smd::ranges::all(std::views::iota(0, 3));
    const auto& functor = smd::functor_typeclass<decltype(values)>;

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
auto materialize(RANGE&& range)
{
    using Value = std::ranges::range_value_t<RANGE>;
    std::vector<Value> values;

    if constexpr (std::ranges::sized_range<RANGE>) {
        values.reserve(std::ranges::size(range));
    }

    std::ranges::copy(range, std::back_inserter(values));

    return values;
}

}  // close namespace detail

template <class VIEW>
    requires(std::ranges::view<VIEW> && std::ranges::input_range<VIEW>)
class list_range : public std::ranges::view_interface<list_range<VIEW> > {
    VIEW d_view;

  public:
    using value_type = std::ranges::range_value_t<VIEW>;
    using view_type = VIEW;

    list_range()
        requires std::default_initializable<VIEW>
    = default;

    constexpr explicit list_range(VIEW view)
        : d_view(std::move(view))
    {
    }

    constexpr auto begin()
    {
        return std::ranges::begin(d_view);
    }

    constexpr auto begin() const
        requires std::ranges::range<const VIEW>
    {
        return std::ranges::begin(d_view);
    }

    constexpr auto end()
    {
        return std::ranges::end(d_view);
    }

    constexpr auto end() const
        requires std::ranges::range<const VIEW>
    {
        return std::ranges::end(d_view);
    }

    constexpr auto base() const&
        requires std::copy_constructible<VIEW>
    {
        return d_view;
    }

    constexpr auto base() &&
    {
        return std::move(d_view);
    }
};

template <std::ranges::viewable_range RANGE>
auto all(RANGE&& range)
{
    using View = std::views::all_t<RANGE>;
    return list_range<View>{std::views::all(std::forward<RANGE>(range))};
}

template <class VALUE>
auto single(VALUE&& value)
{
    using Stored = std::remove_cvref_t<VALUE>;
    return list_range<std::ranges::single_view<Stored> >{
        std::views::single(std::forward<VALUE>(value))};
}

template <class VALUE>
auto from_vector(std::vector<VALUE> values)
{
    return all(std::move(values));
}

}  // close namespace smd::ranges

#endif

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

template <class VIEW>
    requires std::ranges::forward_range<VIEW>
struct ListRangeTraversableImpl {
    using element_type = typename smd::ranges::list_range<VIEW>::value_type;

    template <class APPLICATIVE, class FUNCTION>
    auto traverse(this auto&&,
                  const APPLICATIVE& applicative,
                  FUNCTION&& function,
                  const smd::ranges::list_range<VIEW>& values)
    {
        using Value = element_type;
        using Context = remove_cvref_t<std::invoke_result_t<FUNCTION, const Value&> >;
        using ResultValue = smd::applicative_value_t<Context>;

        auto current = std::ranges::begin(values);
        const auto last = std::ranges::end(values);

        if (current == last) {
            return applicative.map(
                [](auto&& materialized) {
                    return smd::ranges::from_vector(
                        std::forward<decltype(materialized)>(materialized));
                },
                applicative.pure(std::vector<ResultValue>{}));
        }

        auto collected = applicative.map(
            [](auto&& first_value) {
                using U = remove_cvref_t<decltype(first_value)>;
                return std::vector<U>{std::forward<decltype(first_value)>(first_value)};
            },
            std::invoke(function, *current));
        ++current;

        for (; current != last; ++current) {
            auto lifted_value = std::invoke(function, *current);
            collected = applicative.invoke(
                [](std::vector<ResultValue> acc, auto&& next_value) {
                    acc.push_back(std::forward<decltype(next_value)>(next_value));
                    return acc;
                },
                std::move(collected),
                std::move(lifted_value));
        }

        return applicative.map(
            [](auto&& materialized) {
                return smd::ranges::from_vector(
                    std::forward<decltype(materialized)>(materialized));
            },
            std::move(collected));
    }
};

template <class VIEW>
    requires std::ranges::forward_range<VIEW>
struct ListRangeTraversableMap : Traversable<ListRangeTraversableImpl<VIEW> > {
    using ListRangeTraversableImpl<VIEW>::traverse;
};

template <class VIEW>
    requires std::ranges::forward_range<VIEW>
inline constexpr auto traversable_typeclass<smd::ranges::list_range<VIEW> > =
    ListRangeTraversableMap<VIEW>{};

}  // close namespace smd

#endif

```

## smd/ranges/range_traversable.t.cpp

```cpp
#include <smd/ranges/range_traversable.hpp>
#include <smd/ziplist/zip_list_applicative.hpp>

#include <catch2/catch_test_macros.hpp>

#include <optional>
#include <sstream>
#include <ranges>
#include <type_traits>
#include <vector>

#include <algorithm>

namespace {

template <std::ranges::input_range RANGE>
auto collect(RANGE&& range)
{
    using Value = std::ranges::range_value_t<RANGE>;
    std::vector<Value> values;

    for (auto&& value : range) {
        values.emplace_back(value);
    }

    return values;
}

template <std::ranges::input_range OUTER_RANGE>
auto collect_nested(OUTER_RANGE&& outer_range)
{
    std::vector<std::vector<std::ranges::range_value_t<std::ranges::range_value_t<OUTER_RANGE> > > > collected;

    for (auto&& inner_range : outer_range) {
        collected.push_back(collect(inner_range));
    }

    return collected;
}

auto to_vector_of_ziplists(const smd::zip_list<std::vector<int> >& zip_of_vectors)
    -> std::vector<smd::zip_list<int> >
{
    std::vector<smd::zip_list<int> > rows;
    if (zip_of_vectors.data.empty()) {
        return rows;
    }

    std::size_t row_count = zip_of_vectors.data.front().size();
    for (const auto& column : zip_of_vectors.data) {
        row_count = std::min(row_count, column.size());
    }

    rows.assign(row_count, smd::zip_list<int>{});
    for (auto& row : rows) {
        row.data.reserve(zip_of_vectors.data.size());
    }

    for (std::size_t index = 0; index < row_count; ++index) {
        for (const auto& column : zip_of_vectors.data) {
            rows[index].data.push_back(column[index]);
        }
    }

    return rows;
}

}  // namespace

TEST_CASE("RangeTraversableTest - TraverseOptionalSuccess")
{
    // c7f3a1e8-2b5d-4f9c-a4e7-1b3d6c8a5f02
    auto values = smd::ranges::from_vector(std::vector<int>{1, 2, 3});
    const auto& traversable = smd::traversable_typeclass<decltype(values)>;

    auto traversed = smd::traverse(
        [](int value) -> std::optional<int> {
            return std::optional<int>{value + 1};
        },
        values);

    REQUIRE(traversed.has_value());
    CHECK(collect(*traversed) == (std::vector<int>{2, 3, 4}));
    // c7f3a1e8-2b5d-4f9c-a4e7-1b3d6c8a5f02 end
}

TEST_CASE("RangeTraversableTest - TraverseOptionalFailure")
{
    // e9b1d4f2-7c3a-4e8b-f6c2-5d1a9e3b7f04
    auto values = smd::ranges::from_vector(std::vector<int>{1, -2, 3});
    const auto& traversable = smd::traversable_typeclass<decltype(values)>;

    auto traversed = smd::traverse(
        [](int value) -> std::optional<int> {
            return value >= 0 ? std::optional<int>{value + 1}
                              : std::optional<int>{};
        },
        values);

    CHECK_FALSE(traversed.has_value());
    // e9b1d4f2-7c3a-4e8b-f6c2-5d1a9e3b7f04 end
}

TEST_CASE("RangeTraversableTest - TraverseWithRangeApplicativeEnumeratesChoices")
{
    auto values = smd::ranges::from_vector(std::vector<int>{1, 2});
    const auto& traversable = smd::traversable_typeclass<decltype(values)>;

    auto traversed = smd::traverse(
        [](int value) {
            return smd::ranges::from_vector(std::vector<int>{value, value + 10});
        },
        values);

    CHECK(
        collect_nested(traversed) ==
        (std::vector<std::vector<int> >{{1, 2}, {1, 12}, {11, 2}, {11, 12}}));
}

TEST_CASE("RangeTraversableTest - TraversableIsNotDefinedForInputOnlyRanges")
{
    using InputView = std::ranges::basic_istream_view<int, char>;
    using InputList = smd::ranges::list_range<InputView>;

    static_assert(std::is_same_v<
        decltype(smd::traversable_typeclass<InputList>),
        const std::false_type>);
}

TEST_CASE("RangeTraversableTest - SequenceConvertsRangeOfZiplistsToZiplistOfRanges")
{
    // 0e9a7d13-9082-4b9e-b93f-86ef0e0ba20a
    using Zip = smd::zip_list<int>;
    auto values = smd::ranges::from_vector(std::vector<Zip>{
        Zip{{1, 2, 3}},
        Zip{{10, 20}},
        Zip{{100, 200, 300, 400}}});

    const auto& traversable = smd::traversable_typeclass<decltype(values)>;
    // d4f9b1e3-8c2a-4d7f-b6e1-3a5c9d2b7f48
    auto sequenced = traversable.sequence(values);

    REQUIRE(sequenced.data.size() == 2U);
    CHECK(collect(sequenced.data[0]) == (std::vector<int>{1, 10, 100}));
    CHECK(collect(sequenced.data[1]) == (std::vector<int>{2, 20, 200}));
    // d4f9b1e3-8c2a-4d7f-b6e1-3a5c9d2b7f48 end
    // 0e9a7d13-9082-4b9e-b93f-86ef0e0ba20a end
}

TEST_CASE("RangeTraversableTest - SequenceConvertsRangeOfZiplistsToZiplistOfRangesLengthFive")
{
    using Zip = smd::zip_list<int>;
    auto values = smd::ranges::from_vector(std::vector<Zip>{
        Zip{{1, 2, 3, 4, 5}},
        Zip{{10, 20, 30, 40, 50}},
        Zip{{100, 200, 300, 400, 500}}});

    const auto& traversable = smd::traversable_typeclass<decltype(values)>;
    auto sequenced = traversable.sequence(values);

    REQUIRE(sequenced.data.size() == 5U);
    CHECK(collect(sequenced.data[0]) == (std::vector<int>{1, 10, 100}));
    CHECK(collect(sequenced.data[1]) == (std::vector<int>{2, 20, 200}));
    CHECK(collect(sequenced.data[2]) == (std::vector<int>{3, 30, 300}));
    CHECK(collect(sequenced.data[3]) == (std::vector<int>{4, 40, 400}));
    CHECK(collect(sequenced.data[4]) == (std::vector<int>{5, 50, 500}));
}

TEST_CASE("RangeTraversableTest - ConvertZiplistOfVectorsToVectorOfZiplists")
{
    // 4be89584-35cc-4933-b3de-6d524d54371d
    smd::zip_list<std::vector<int> > zip_of_vectors{
        {{1, 10, 100}, {2, 20, 200}}};

    auto as_rows = to_vector_of_ziplists(zip_of_vectors);

    REQUIRE(as_rows.size() == 3U);
    CHECK(as_rows[0].data == (std::vector<int>{1, 2}));
    CHECK(as_rows[1].data == (std::vector<int>{10, 20}));
    CHECK(as_rows[2].data == (std::vector<int>{100, 200}));
    // 4be89584-35cc-4933-b3de-6d524d54371d end
}

TEST_CASE("RangeTraversableTest - ConvertZiplistOfVectorsToVectorOfZiplistsLengthFive")
{
    smd::zip_list<std::vector<int> > zip_of_vectors{
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

template <class T>
struct BinaryTreeApplicativeImpl {
  template <class VALUE>
  auto pure(this auto&&, VALUE&& value)
  {
    using U = remove_cvref_t<VALUE>;
    return smd::tree::BinaryTree<U>::leaf(std::forward<VALUE>(value));
  }

  template <class F, class A>
  auto apply(this auto&& self,
             const smd::tree::BinaryTree<F>& functions,
             const smd::tree::BinaryTree<A>& arguments)
    -> smd::tree::BinaryTree<std::invoke_result_t<const F&, const A&>>
  {
    using R = std::invoke_result_t<const F&, const A&>;

    std::shared_ptr<smd::tree::BinaryTree<R> > left{};
    std::shared_ptr<smd::tree::BinaryTree<R> > right{};

    const auto function_is_leaf = !functions.has_left() && !functions.has_right();
    const auto argument_is_leaf = !arguments.has_left() && !arguments.has_right();

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
      // A non-leaf function tree can be applied pointwise to a single argument.
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
      functions.value()(arguments.value()),
      std::move(left),
      std::move(right));
  }
};

template <class T>
struct BinaryTreeApplicativeMap : Applicative<BinaryTreeApplicativeImpl<T> > {
  using BinaryTreeApplicativeImpl<T>::apply;
  using BinaryTreeApplicativeImpl<T>::pure;
};

template <class T>
inline constexpr auto applicative_typeclass<smd::tree::BinaryTree<T> > =
  BinaryTreeApplicativeMap<T>{};

}  // close namespace smd

#endif

```

## smd/tree/binary_tree_applicative.t.cpp

```cpp
#include <smd/tree/binary_tree.hpp>
#include <smd/tree/binary_tree_applicative.hpp>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("BinaryTreeApplicativeTest - InvokeAndApply")
{
    using Tree = smd::tree::BinaryTree<int>;
    auto lhs = Tree::from_children_ptrs(
        10,
        Tree::make_ptr(Tree::leaf(1)),
        Tree::make_ptr(Tree::leaf(2)));
    auto rhs = Tree::from_children_ptrs(
        3,
        Tree::make_ptr(Tree::leaf(4)),
        Tree::make_ptr(Tree::leaf(5)));

    const auto& applicative = smd::applicative_typeclass<Tree>;
    auto summed = applicative.invoke([](int a, int b) { return a + b; }, lhs, rhs);

    CHECK(summed.value() == 13);
    REQUIRE(summed.has_left());
    REQUIRE(summed.has_right());
    CHECK(summed.left().value() == 5);
    CHECK(summed.right().value() == 7);

    auto fs = smd::tree::BinaryTree<int(*)(int)>::from_children_ptrs(
        +[](int x) { return x * 2; },
        smd::tree::BinaryTree<int(*)(int)>::make_ptr(
            smd::tree::BinaryTree<int(*)(int)>::leaf(+[](int x) { return x + 1; })),
        {});
    auto applied = applicative.apply(fs, lhs);
    CHECK(applied.value() == 20);
    REQUIRE(applied.has_left());
    CHECK(applied.left().value() == 2);
    CHECK_FALSE(applied.has_right());
}

TEST_CASE("BinaryTreeApplicativeTest - PureFunctionDistributesOverArgumentShape")
{
    using Tree = smd::tree::BinaryTree<int>;
    const auto& applicative = smd::applicative_typeclass<Tree>;

    auto fs = smd::tree::BinaryTree<int(*)(int)>::leaf(+[](int x) { return x + 10; });
    auto xs = Tree::from_children_ptrs(
        1,
        Tree::make_ptr(Tree::leaf(2)),
        Tree::make_ptr(Tree::leaf(3)));

    auto applied = applicative.apply(fs, xs);
    CHECK(applied.value() == 11);
    REQUIRE(applied.has_left());
    REQUIRE(applied.has_right());
    CHECK(applied.left().value() == 12);
    CHECK(applied.right().value() == 13);
}

TEST_CASE("BinaryTreeApplicativeTest - FunctionTreeAppliesPointwiseToLeafArgument")
{
    using Tree = smd::tree::BinaryTree<int>;
    const auto& applicative = smd::applicative_typeclass<Tree>;

    auto fs = smd::tree::BinaryTree<int(*)(int)>::from_children_ptrs(
        +[](int x) { return x * 2; },
        smd::tree::BinaryTree<int(*)(int)>::make_ptr(
            smd::tree::BinaryTree<int(*)(int)>::leaf(+[](int x) { return x + 1; })),
        smd::tree::BinaryTree<int(*)(int)>::make_ptr(
            smd::tree::BinaryTree<int(*)(int)>::leaf(+[](int x) { return x - 1; })));

    auto applied = applicative.apply(fs, Tree::leaf(10));
    CHECK(applied.value() == 20);
    REQUIRE(applied.has_left());
    REQUIRE(applied.has_right());
    CHECK(applied.left().value() == 11);
    CHECK(applied.right().value() == 9);
}

TEST_CASE("BinaryTreeApplicativeTest - PairwiseApplyRequiresMatchingChildren")
{
    using Tree = smd::tree::BinaryTree<int>;
    const auto& applicative = smd::applicative_typeclass<Tree>;

    auto fs = smd::tree::BinaryTree<int(*)(int)>::from_children_ptrs(
        +[](int x) { return x + 100; },
        smd::tree::BinaryTree<int(*)(int)>::make_ptr(
            smd::tree::BinaryTree<int(*)(int)>::leaf(+[](int x) { return x + 1; })),
        {});

    auto xs = Tree::from_children_ptrs(
        1,
        {},
        Tree::make_ptr(Tree::leaf(2)));

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

template <class T>
struct BinaryTreeFoldableImpl {
  template <class F>
  auto fold_map(this auto&& self,
                F&& function,
                const smd::tree::BinaryTree<T>& tree)
    -> remove_cvref_t<decltype(std::invoke(function, tree.value()))>
  {
    auto value_result = std::invoke(function, tree.value());
    using Result = remove_cvref_t<decltype(value_result)>;

    Result acc = tree.has_left()
      ? smd::typeclass::monoid_v<Result>.combine(
          self.fold_map(function, tree.left()),
          std::move(value_result))
      : std::move(value_result);

    if (tree.has_right()) {
      acc = smd::typeclass::monoid_v<Result>.combine(
        std::move(acc),
        self.fold_map(function, tree.right()));
    }

    return acc;
  }
};

template <class T>
struct BinaryTreeFoldableMap : Foldable<BinaryTreeFoldableImpl<T> > {
  using BinaryTreeFoldableImpl<T>::fold_map;
};

template <class T>
inline constexpr auto foldable_typeclass<smd::tree::BinaryTree<T> > =
  BinaryTreeFoldableMap<T>{};

}  // close namespace smd

#endif

```

## smd/tree/binary_tree_foldable.t.cpp

```cpp
#include <smd/tree/binary_tree.hpp>
#include <smd/tree/binary_tree_foldable.hpp>
#include <smd/typeclass/foldable.hpp>

#include <catch2/catch_test_macros.hpp>

#include <vector>

TEST_CASE("BinaryTreeFoldableTest - InorderFoldAndLength")
{
    using Tree = smd::tree::BinaryTree<int>;
    auto tree = Tree::from_children_ptrs(
      2,
      Tree::make_ptr(Tree::leaf(1)),
      Tree::make_ptr(Tree::from_children_ptrs(
        3,
        {},
        Tree::make_ptr(Tree::leaf(4)))));

    const auto& foldable = smd::foldable_typeclass<Tree>;
    CHECK(foldable.length(tree) == 4U);

    const auto as_vector = foldable.to_vector(tree);
    CHECK(as_vector == (std::vector<int>{1, 2, 3, 4}));

    const auto left = foldable.fold_left(tree, 0, [](int acc, int x) {
        return acc * 10 + x;
    });
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

template <class T>
class BinaryTree {
  T d_value;
  std::shared_ptr<BinaryTree> d_left;
  std::shared_ptr<BinaryTree> d_right;

 public:
  using value_type = T;

  static auto leaf(T value) -> BinaryTree
  {
    return BinaryTree(std::move(value), {}, {});
  }

  static auto node(T value, BinaryTree left, BinaryTree right) -> BinaryTree
  {
    return BinaryTree(std::move(value),
                      std::make_shared<BinaryTree>(std::move(left)),
                      std::make_shared<BinaryTree>(std::move(right)));
  }

  static auto branch(T value, BinaryTree left, BinaryTree right) -> BinaryTree
  {
    return node(std::move(value), std::move(left), std::move(right));
  }

  static auto from_children_ptrs(T value,
                                 std::shared_ptr<BinaryTree> left,
                                 std::shared_ptr<BinaryTree> right)
    -> BinaryTree
  {
    return BinaryTree(std::move(value), std::move(left), std::move(right));
  }

  static auto make_ptr(BinaryTree tree) -> std::shared_ptr<BinaryTree>
  {
    return std::make_shared<BinaryTree>(std::move(tree));
  }

  auto value() const -> const T& { return d_value; }

  auto has_left() const -> bool { return static_cast<bool>(d_left); }
  auto has_right() const -> bool { return static_cast<bool>(d_right); }

  auto left() const -> const BinaryTree&
  {
    assert(d_left);
    return *d_left;
  }

  auto right() const -> const BinaryTree&
  {
    assert(d_right);
    return *d_right;
  }

  auto left_ptr() const -> const std::shared_ptr<BinaryTree>& { return d_left; }
  auto right_ptr() const -> const std::shared_ptr<BinaryTree>& { return d_right; }

 private:
  BinaryTree(T value,
             std::shared_ptr<BinaryTree> left,
             std::shared_ptr<BinaryTree> right)
      : d_value(std::move(value))
      , d_left(std::move(left))
      , d_right(std::move(right))
  {
  }
};

}  // close namespace smd::tree

#endif

```

## smd/tree/binary_tree_traversable.hpp

```cpp
// src/smd/tree/binary_tree_traversable.hpp                           -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_BINARY_TREE_TRAVERSABLE
#define INCLUDED_SMD_TREE_BINARY_TREE_TRAVERSABLE

#include <smd/tree/binary_tree_applicative.hpp>
#include <smd/tree/binary_tree.hpp>
#include <smd/typeclass/traversable.hpp>

#include <functional>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>

namespace smd {

template <class T>
struct BinaryTreeTraversableImpl {
  using element_type = T;

  template <class APPLICATIVE, class F>
  auto traverse(this auto&& self,
                const APPLICATIVE& applicative,
                F&& function,
                const smd::tree::BinaryTree<T>& tree)
  {
    auto value_context = std::invoke(std::forward<F>(function), tree.value());
    using Context = remove_cvref_t<decltype(value_context)>;
    using U = smd::applicative_value_t<Context>;
    using TreeContext = decltype(applicative.invoke(
      [](auto&& value) {
        using V = remove_cvref_t<decltype(value)>;
        return smd::tree::BinaryTree<V>::leaf(std::forward<decltype(value)>(value));
      },
      value_context));

    if (!tree.has_left() && !tree.has_right()) {
      return applicative.invoke(
        [](auto&& value) {
          using V = remove_cvref_t<decltype(value)>;
          return smd::tree::BinaryTree<V>::leaf(std::forward<decltype(value)>(value));
        },
        value_context);
    }

    std::optional<TreeContext> left_tree_context;
    if (tree.has_left()) {
      left_tree_context.emplace(self.traverse(applicative, function, tree.left()));
    }

    std::optional<TreeContext> right_tree_context;
    if (tree.has_right()) {
      right_tree_context.emplace(self.traverse(applicative, function, tree.right()));
    }

    auto to_child_ptr = [&](const auto& child_tree_context) {
      return applicative.invoke(
        [](auto&& subtree) {
          using SubTree = remove_cvref_t<decltype(subtree)>;
          return std::make_shared<SubTree>(
            std::forward<decltype(subtree)>(subtree));
        },
        child_tree_context);
    };

    auto empty_child_like = [&](const auto& child_tree_context) {
      return applicative.invoke(
        [](const auto&) {
          return std::shared_ptr<smd::tree::BinaryTree<U> >{};
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
      [](auto&& value, auto&& left, auto&& right) {
        using U = remove_cvref_t<decltype(value)>;
        return smd::tree::BinaryTree<U>::from_children_ptrs(
          std::forward<decltype(value)>(value),
          std::forward<decltype(left)>(left),
          std::forward<decltype(right)>(right));
      },
      value_context,
      left_context,
      right_context);
  }
};

template <class T>
struct BinaryTreeTraversableMap : Traversable<BinaryTreeTraversableImpl<T> > {
  using BinaryTreeTraversableImpl<T>::traverse;
};

template <class T>
inline constexpr auto traversable_typeclass<smd::tree::BinaryTree<T> > =
  BinaryTreeTraversableMap<T>{};

}  // close namespace smd

#endif

```

## smd/tree/binary_tree_traversable.t.cpp

```cpp
#include <smd/tree/binary_tree.hpp>
#include <smd/tree/binary_tree_traversable.hpp>

#include <catch2/catch_test_macros.hpp>

#include <optional>

namespace {

struct PositiveTimesTen {
    auto operator()(int x) const -> std::optional<int>
    {
        return x > 0 ? std::optional<int>{x * 10} : std::optional<int>{};
    }
};

struct TimesTen {
    auto operator()(int x) const -> std::optional<int>
    {
        return std::optional<int>{x * 10};
    }
};

struct PlusOne {
    auto operator()(int x) const -> std::optional<int>
    {
        return std::optional<int>{x + 1};
    }
};

struct NonNegativeIdentity {
    auto operator()(int x) const -> std::optional<int>
    {
        return x >= 0 ? std::optional<int>{x} : std::optional<int>{};
    }
};

}  // namespace

TEST_CASE("BinaryTreeTraversableTest - TraverseOptionalPreservesShape")
{
    using Tree = smd::tree::BinaryTree<int>;
    auto tree = Tree::from_children_ptrs(
        2,
        Tree::make_ptr(Tree::leaf(1)),
        Tree::make_ptr(Tree::from_children_ptrs(3, {}, Tree::make_ptr(Tree::leaf(4)))));

    const auto& traversable = smd::traversable_typeclass<Tree>;
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

TEST_CASE("BinaryTreeTraversableTest - TraverseOptionalDoesNotDuplicateRootEffect")
{
    using Tree = smd::tree::BinaryTree<int>;
    auto tree = Tree::from_children_ptrs(
        2,
        {},
        Tree::make_ptr(Tree::leaf(5)));

    int invocations = 0;
    const auto& traversable = smd::traversable_typeclass<Tree>;
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

TEST_CASE("BinaryTreeTraversableTest - TraverseOptionalLeaf")
{
    using Tree = smd::tree::BinaryTree<int>;
    auto tree = Tree::leaf(9);

    const auto& traversable = smd::traversable_typeclass<Tree>;
    auto traversed = smd::traverse(PlusOne{}, tree);

    REQUIRE(traversed.has_value());
    CHECK(traversed->value() == 10);
    CHECK_FALSE(traversed->has_left());
    CHECK_FALSE(traversed->has_right());
}

TEST_CASE("BinaryTreeTraversableTest - TraverseOptionalLeftOnly")
{
    using Tree = smd::tree::BinaryTree<int>;
    auto tree = Tree::from_children_ptrs(
        2,
        Tree::make_ptr(Tree::leaf(3)),
        {});

    const auto& traversable = smd::traversable_typeclass<Tree>;
    auto traversed = smd::traverse(TimesTen{}, tree);

    REQUIRE(traversed.has_value());
    CHECK(traversed->value() == 20);
    REQUIRE(traversed->has_left());
    CHECK_FALSE(traversed->has_right());
    CHECK(traversed->left().value() == 30);
}

TEST_CASE("BinaryTreeTraversableTest - TraverseOptionalFailure")
{
    using Tree = smd::tree::BinaryTree<int>;
    auto tree = Tree::from_children_ptrs(
        2,
        Tree::make_ptr(Tree::leaf(-1)),
        {});

    const auto& traversable = smd::traversable_typeclass<Tree>;
    auto traversed = smd::traverse(NonNegativeIdentity{}, tree);

    CHECK_FALSE(traversed.has_value());
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

#include <algorithm>
#include <functional>
#include <type_traits>
#include <utility>

namespace smd {

template <class T, class TAG_TYPE, class MEASURE_POLICY>
struct FingerTreeFoldableImpl {
  template <class F>
  auto fold_map(this auto&&,
                F&& function,
                const smd::tree::FingerTree<T, TAG_TYPE, MEASURE_POLICY>& tree)
    -> remove_cvref_t<std::invoke_result_t<F, const T&>>
  {
    using Result = remove_cvref_t<std::invoke_result_t<F, const T&>>;

    return std::ranges::fold_left(
        tree.flatten(),
        smd::typeclass::monoid_v<Result>.identity(),
        [&](Result acc, const auto& value) {
          return smd::typeclass::monoid_v<Result>.combine(
              std::move(acc), std::invoke(function, value));
        });
  }
};

template <class T, class TAG_TYPE, class MEASURE_POLICY>
struct FingerTreeFoldableMap
  : Foldable<FingerTreeFoldableImpl<T, TAG_TYPE, MEASURE_POLICY>> {
  using FingerTreeFoldableImpl<T, TAG_TYPE, MEASURE_POLICY>::fold_map;
};

template <class T, class TAG_TYPE, class MEASURE_POLICY>
inline constexpr auto foldable_typeclass<
  smd::tree::FingerTree<T, TAG_TYPE, MEASURE_POLICY>> =
  FingerTreeFoldableMap<T, TAG_TYPE, MEASURE_POLICY>{};

}  // close namespace smd

#endif

```

## smd/tree/finger_tree.hpp

```cpp
// src/smd/tree/finger_tree.hpp                                       -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_FINGER_TREE
#define INCLUDED_SMD_TREE_FINGER_TREE

#include <smd/tree/memoized_thunk.hpp>
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

template <typename T>
using Boxed = std::shared_ptr<const T>;

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
using Digit = std::variant<One<T>, Two<T>, Three<T>>;

template <typename T>
struct Node2 {
  Boxed<T> a;
  Boxed<T> b;
};

template <typename T>
struct Node3 {
  Boxed<T> a;
  Boxed<T> b;
  Boxed<T> c;
};

template <typename T>
using Node = std::variant<Node2<T>, Node3<T>>;

template <typename VALUE_TYPE, typename TAG_TYPE>
struct UnitMeasure {
  auto operator()(const VALUE_TYPE&) const -> TAG_TYPE { return TAG_TYPE{1}; }
};

template <typename T,
          typename TAG_TYPE = std::size_t,
          typename MEASURE_POLICY = UnitMeasure<T, TAG_TYPE> >
class FingerTree {
  static_assert(std::is_default_constructible_v<MEASURE_POLICY>,
                "FingerTree measure policy must be default-constructible");

  using Tag = TAG_TYPE;
  using MeasurePolicy = MEASURE_POLICY;

  struct Segment;
  using SegmentPtr = std::shared_ptr<const Segment>;
  using SegmentThunk = detail::erased_thunk<SegmentPtr>;

  static auto tag_identity() -> Tag
  {
    return smd::typeclass::monoid_v<Tag>.identity();
  }

  static auto tag_combine(const Tag& lhs, const Tag& rhs) -> Tag
  {
    return smd::typeclass::monoid_v<Tag>.combine(lhs, rhs);
  }

  static auto tag_value(const T& value) -> Tag
  {
    return MeasurePolicy{}(value);
  }

  static auto tag_range(const std::shared_ptr<const std::vector<T> >& values,
                        std::size_t begin,
                        std::size_t end) -> Tag
  {
    auto result = tag_identity();
    for (auto i = begin; i < end; ++i) {
      result = tag_combine(result, tag_value((*values)[i]));
    }
    return result;
  }

  static auto balanced_depth(std::size_t count) -> std::size_t
  {
    if (count == 0U) {
      return 0U;
    }
    if (count == 1U) {
      return 1U;
    }

    auto left_count = count / 2U;
    auto right_count = count - left_count;
    return std::max(balanced_depth(left_count), balanced_depth(right_count)) + 1U;
  }

  struct SegmentMetadata {
    std::size_t d_size;
    std::size_t d_depth;
    Tag d_tag;
  };

  struct Segment {
    virtual ~Segment() = default;

    [[nodiscard]] virtual auto size() const -> std::size_t = 0;
    [[nodiscard]] virtual auto tag() const -> const Tag& = 0;
    [[nodiscard]] virtual auto depth() const -> std::size_t = 0;
    virtual void flatten_into(std::vector<T>& out) const = 0;
    [[nodiscard]] virtual auto pop_left() const -> std::optional<std::pair<T, SegmentPtr>> = 0;
    [[nodiscard]] virtual auto pop_right() const -> std::optional<std::pair<T, SegmentPtr>> = 0;
  };

  struct FlatSegment final : Segment {
    std::shared_ptr<const std::vector<T>> d_values;
    std::size_t d_begin;
    std::size_t d_end;
    Tag d_tag;

    explicit FlatSegment(std::vector<T> values)
      : d_values(std::make_shared<const std::vector<T>>(std::move(values)))
      , d_begin(0)
      , d_end(d_values->size())
      , d_tag(tag_range(d_values, d_begin, d_end))
    {
    }

    FlatSegment(std::shared_ptr<const std::vector<T>> values, std::size_t begin, std::size_t end)
      : d_values(std::move(values))
      , d_begin(begin)
      , d_end(end)
      , d_tag(tag_range(d_values, d_begin, d_end))
    {
    }

    [[nodiscard]] auto size() const -> std::size_t override { return d_end - d_begin; }
    [[nodiscard]] auto tag() const -> const Tag& override { return d_tag; }

    [[nodiscard]] auto depth() const -> std::size_t override
    {
      return size() == 0 ? std::size_t{0} : std::size_t{1};
    }

    void flatten_into(std::vector<T>& out) const override
    {
      out.insert(out.end(),
                 d_values->begin() + static_cast<std::ptrdiff_t>(d_begin),
                 d_values->begin() + static_cast<std::ptrdiff_t>(d_end));
    }

    [[nodiscard]] auto pop_left() const -> std::optional<std::pair<T, SegmentPtr>> override
    {
      if (size() == 0) {
        return std::nullopt;
      }

      SegmentPtr rest;
      if (size() > 1) {
        rest = std::make_shared<const FlatSegment>(d_values, d_begin + 1, d_end);
      }

      return std::pair<T, SegmentPtr>{(*d_values)[d_begin], std::move(rest)};
    }

    [[nodiscard]] auto pop_right() const -> std::optional<std::pair<T, SegmentPtr>> override
    {
      if (size() == 0) {
        return std::nullopt;
      }

      SegmentPtr rest;
      if (size() > 1) {
        rest = std::make_shared<const FlatSegment>(d_values, d_begin, d_end - 1);
      }

      return std::pair<T, SegmentPtr>{(*d_values)[d_end - 1], std::move(rest)};
    }
  };

  static auto seg_size(const SegmentPtr& seg) -> std::size_t
  {
    return seg ? seg->size() : std::size_t{0};
  }

  static auto seg_depth(const SegmentPtr& seg) -> std::size_t
  {
    return seg ? seg->depth() : std::size_t{0};
  }

  static auto seg_tag(const SegmentPtr& seg) -> Tag
  {
    return seg ? seg->tag() : tag_identity();
  }

  static auto make_flat(std::vector<T> values) -> SegmentPtr
  {
    if (values.empty()) {
      return nullptr;
    }
    return std::make_shared<const FlatSegment>(std::move(values));
  }

  static auto make_flat_range(const std::shared_ptr<const std::vector<T>>& values,
                              std::size_t begin,
                              std::size_t end) -> SegmentPtr
  {
    if (begin >= end) {
      return nullptr;
    }
    return std::make_shared<const FlatSegment>(values, begin, end);
  }

  static auto segment_metadata(const SegmentPtr& seg) -> SegmentMetadata
  {
    return SegmentMetadata{seg_size(seg), seg_depth(seg), seg_tag(seg)};
  }

  static auto range_metadata(const std::shared_ptr<const std::vector<T>>& values,
                             std::size_t begin,
                             std::size_t end) -> SegmentMetadata
  {
    auto count = end > begin ? end - begin : 0U;
    return SegmentMetadata{count, balanced_depth(count), tag_range(values, begin, end)};
  }

  static auto concat_metadata(const SegmentMetadata& left,
                              const SegmentMetadata& right) -> SegmentMetadata
  {
    return SegmentMetadata{left.d_size + right.d_size,
                           std::max(left.d_depth, right.d_depth) + std::size_t{1},
                           tag_combine(left.d_tag, right.d_tag)};
  }

  struct MiddleEdge {
    SegmentMetadata d_metadata;
    mutable SegmentThunk d_force;

    [[nodiscard]] auto size() const -> std::size_t { return d_metadata.d_size; }
    [[nodiscard]] auto depth() const -> std::size_t { return d_metadata.d_depth; }
    [[nodiscard]] auto tag() const -> const Tag& { return d_metadata.d_tag; }
    [[nodiscard]] auto force() const -> const SegmentPtr& { return d_force(); }
  };

  static auto make_middle_from_segment(SegmentPtr seg) -> MiddleEdge
  {
    auto metadata = segment_metadata(seg);
    return MiddleEdge{
      std::move(metadata),
      detail::thunk([seg = std::move(seg)]() -> SegmentPtr { return seg; })};
  }

  static auto make_middle_from_range(const std::shared_ptr<const std::vector<T>>& values,
                                     std::size_t begin,
                                     std::size_t end) -> MiddleEdge
  {
    return MiddleEdge{
      range_metadata(values, begin, end),
      detail::thunk([values, begin, end]() -> SegmentPtr {
        return FingerTree::build_balanced(values, begin, end);
      })};
  }

  struct SuspendedSegment final : Segment {
    SegmentMetadata d_metadata;
    mutable SegmentThunk d_force;

    SuspendedSegment(SegmentMetadata metadata, SegmentThunk thunk)
      : d_metadata(std::move(metadata))
      , d_force(std::move(thunk))
    {
    }

    [[nodiscard]] auto size() const -> std::size_t override { return d_metadata.d_size; }
    [[nodiscard]] auto tag() const -> const Tag& override { return d_metadata.d_tag; }
    [[nodiscard]] auto depth() const -> std::size_t override { return d_metadata.d_depth; }

    [[nodiscard]] auto force() const -> const SegmentPtr& { return d_force(); }

    void flatten_into(std::vector<T>& out) const override
    {
      force()->flatten_into(out);
    }

    [[nodiscard]] auto pop_left() const -> std::optional<std::pair<T, SegmentPtr>> override
    {
      return force()->pop_left();
    }

    [[nodiscard]] auto pop_right() const -> std::optional<std::pair<T, SegmentPtr>> override
    {
      return force()->pop_right();
    }
  };

  static auto force_segment(const SegmentPtr& seg) -> SegmentPtr
  {
    auto current = seg;
    while (const auto* suspended = dynamic_cast<const SuspendedSegment*>(current.get())) {
      current = suspended->force();
    }
    return current;
  }

  static auto make_segment_from_middle(const MiddleEdge& edge) -> SegmentPtr
  {
    if (edge.size() == 0U) {
      return nullptr;
    }
    return make_suspended_segment(edge.d_metadata, edge.d_force);
  }

  static auto make_delayed_concat_segment(const MiddleEdge& left,
                                          SegmentPtr right) -> SegmentPtr
  {
    if (left.size() == 0U) {
      return right;
    }
    if (!right) {
      return make_segment_from_middle(left);
    }

    auto metadata = concat_metadata(left.d_metadata, segment_metadata(right));

    return make_suspended_segment(
      std::move(metadata),
      detail::thunk(
        [left, right = std::move(right)]() mutable -> SegmentPtr {
          return make_concat(left.force(), right);
        }));
  }

  static auto make_delayed_concat_pair(SegmentPtr left, SegmentPtr right) -> SegmentPtr
  {
    if (!left) {
      return right;
    }
    if (!right) {
      return left;
    }

    auto metadata = concat_metadata(segment_metadata(left), segment_metadata(right));

    return make_suspended_segment(
      std::move(metadata),
      detail::thunk([left = std::move(left), right = std::move(right)]() mutable -> SegmentPtr {
        return make_concat(std::move(left), std::move(right));
      }));
  }

  struct ConcatSegment final : Segment {
    SegmentPtr d_left;
    MiddleEdge d_right;
    std::size_t d_size;
    std::size_t d_depth;
    Tag d_tag;

    ConcatSegment(SegmentPtr left, MiddleEdge right)
      : d_left(std::move(left))
      , d_right(std::move(right))
      , d_size(seg_size(d_left) + d_right.size())
      , d_depth(std::max(seg_depth(d_left), d_right.depth()) + std::size_t{1})
      , d_tag(tag_combine(seg_tag(d_left), d_right.tag()))
    {
    }

    [[nodiscard]] auto size() const -> std::size_t override { return d_size; }
    [[nodiscard]] auto tag() const -> const Tag& override { return d_tag; }
    [[nodiscard]] auto depth() const -> std::size_t override { return d_depth; }

    void flatten_into(std::vector<T>& out) const override
    {
      if (d_left) {
        d_left->flatten_into(out);
      }
      if (d_right.size() != 0U) {
        d_right.force()->flatten_into(out);
      }
    }

    [[nodiscard]] auto pop_left() const -> std::optional<std::pair<T, SegmentPtr>> override
    {
      if (!d_left && d_right.size() == 0U) {
        return std::nullopt;
      }

      if (d_left) {
        auto l = d_left->pop_left();
        if (l.has_value()) {
          return std::pair<T, SegmentPtr>{std::move(l->first), make_concat(std::move(l->second), d_right)};
        }
      }

      return d_right.size() != 0U ? d_right.force()->pop_left() : std::nullopt;
    }

    [[nodiscard]] auto pop_right() const -> std::optional<std::pair<T, SegmentPtr>> override
    {
      if (!d_left && d_right.size() == 0U) {
        return std::nullopt;
      }

      if (d_right.size() != 0U) {
        auto r = d_right.force()->pop_right();
        if (r.has_value()) {
          return std::pair<T, SegmentPtr>{std::move(r->first), make_concat(d_left, std::move(r->second))};
        }
      }

      return d_left ? d_left->pop_right() : std::nullopt;
    }
  };

  static auto make_concat(SegmentPtr left, MiddleEdge right) -> SegmentPtr
  {
    if (!left) {
      return make_segment_from_middle(right);
    }
    if (right.size() == 0U) {
      return left;
    }
    return std::make_shared<const ConcatSegment>(std::move(left), std::move(right));
  }

  static auto make_suspended_segment(SegmentMetadata metadata, SegmentThunk thunk) -> SegmentPtr
  {
    if (metadata.d_size == 0U) {
      return nullptr;
    }
    return std::make_shared<const SuspendedSegment>(std::move(metadata), std::move(thunk));
  }

  static auto make_concat(SegmentPtr left, SegmentPtr right) -> SegmentPtr
  {
    auto make_node = [](SegmentPtr lhs, SegmentPtr rhs) -> SegmentPtr {
      lhs = force_segment(lhs);
      rhs = force_segment(rhs);

      if (!lhs) {
        return rhs;
      }
      if (!rhs) {
        return lhs;
      }
      return std::make_shared<const ConcatSegment>(std::move(lhs), make_middle_from_segment(std::move(rhs)));
    };

    auto as_concat = [](const SegmentPtr& seg) -> const ConcatSegment* {
      return dynamic_cast<const ConcatSegment*>(seg.get());
    };

    auto balance = [&](SegmentPtr lhs, SegmentPtr rhs) -> SegmentPtr {
      while (true) {
        auto lhs_depth = seg_depth(lhs);
        auto rhs_depth = seg_depth(rhs);

        if (lhs_depth <= rhs_depth + 1 && rhs_depth <= lhs_depth + 1) {
          return make_node(std::move(lhs), std::move(rhs));
        }

        if (lhs_depth > rhs_depth + 1) {
          lhs = force_segment(lhs);
          const auto* l = as_concat(lhs);
          if (!l) {
            return make_node(std::move(lhs), std::move(rhs));
          }

          if (seg_depth(l->d_left) < l->d_right.depth()) {
            const auto* lr = as_concat(l->d_right.force());
            if (lr) {
              lhs = make_node(l->d_left, lr->d_left);
              rhs = make_delayed_concat_segment(lr->d_right, std::move(rhs));
              continue;
            }
          }

          rhs = make_delayed_concat_segment(l->d_right, std::move(rhs));
          lhs = l->d_left;
          continue;
        }

        rhs = force_segment(rhs);
        const auto* r = as_concat(rhs);
        if (!r) {
          return make_node(std::move(lhs), std::move(rhs));
        }

        if (r->d_right.depth() < seg_depth(r->d_left)) {
          auto forced_left = force_segment(r->d_left);
          const auto* rl = as_concat(forced_left);
          if (rl) {
            lhs = make_node(std::move(lhs), rl->d_left);
            rhs = make_delayed_concat_segment(rl->d_right, make_segment_from_middle(r->d_right));
            continue;
          }
        }

        lhs = make_node(std::move(lhs), r->d_left);
        rhs = make_segment_from_middle(r->d_right);
      }
    };

    if (!left) {
      return right;
    }
    if (!right) {
      return left;
    }

    auto left_depth = seg_depth(left);
    auto right_depth = seg_depth(right);

    if (left_depth > right_depth + 1) {
      if (dynamic_cast<const SuspendedSegment*>(left.get()) != nullptr) {
        return make_delayed_concat_pair(std::move(left), std::move(right));
      }

      auto forced_left = force_segment(left);
      if (const auto* l = dynamic_cast<const ConcatSegment*>(forced_left.get())) {
        return balance(l->d_left, make_delayed_concat_segment(l->d_right, std::move(right)));
      }
    }

    if (right_depth > left_depth + 1) {
      if (dynamic_cast<const SuspendedSegment*>(right.get()) != nullptr) {
        return make_delayed_concat_pair(std::move(left), std::move(right));
      }

      auto forced_right = force_segment(right);
      if (const auto* r = dynamic_cast<const ConcatSegment*>(forced_right.get())) {
        return balance(make_concat(std::move(left), r->d_left), make_segment_from_middle(r->d_right));
      }
    }

    return balance(std::move(left), std::move(right));
  }

  static auto build_balanced(const std::shared_ptr<const std::vector<T>>& values,
                             std::size_t begin,
                             std::size_t end) -> SegmentPtr
  {
    if (begin >= end) {
      return nullptr;
    }
    if (end - begin == 1) {
      return make_flat_range(values, begin, end);
    }

    auto mid = begin + (end - begin) / 2;
    return make_concat(build_balanced(values, begin, mid),
                       make_middle_from_range(values, mid, end));
  }

  template <typename PREDICATE>
  static auto search_segment(const SegmentPtr& seg,
                             const PREDICATE& predicate,
                             Tag prefix) -> std::optional<T>
  {
    auto current = force_segment(seg);
    if (!current) {
      return std::nullopt;
    }

    if (const auto* flat = dynamic_cast<const FlatSegment*>(current.get())) {
      for (std::size_t i = flat->d_begin; i < flat->d_end; ++i) {
        prefix = tag_combine(prefix, tag_value((*flat->d_values)[i]));
        if (predicate(prefix)) {
          return (*flat->d_values)[i];
        }
      }
      return std::nullopt;
    }

    const auto* concat = dynamic_cast<const ConcatSegment*>(current.get());
    assert(concat != nullptr);

    auto left_prefix = tag_combine(prefix, seg_tag(concat->d_left));
    if (predicate(left_prefix)) {
      return search_segment(concat->d_left, predicate, prefix);
    }

    return search_segment(make_segment_from_middle(concat->d_right), predicate, left_prefix);
  }

  struct SegmentSplit {
    SegmentPtr d_left;
    T d_pivot;
    SegmentPtr d_right;
  };

  static auto is_consistent_split(const SegmentPtr& source,
                                  const SegmentSplit& split) -> bool
  {
    auto source_size = seg_size(source);
    auto split_size = seg_size(split.d_left) + std::size_t{1} + seg_size(split.d_right);
    if (split_size != source_size) {
      return false;
    }

    if constexpr (requires(const Tag& lhs, const Tag& rhs) {
                   { lhs == rhs } -> std::convertible_to<bool>;
                 }) {
      auto split_tag = tag_combine(
        tag_combine(seg_tag(split.d_left), tag_value(split.d_pivot)),
        seg_tag(split.d_right));
      if (!(split_tag == seg_tag(source))) {
        return false;
      }
    }

    return true;
  }

  template <typename PREDICATE>
  static auto split_segment(const SegmentPtr& seg,
                            const PREDICATE& predicate,
                            Tag prefix) -> std::optional<SegmentSplit>
  {
    auto current = force_segment(seg);
    if (!current) {
      return std::nullopt;
    }

    if (const auto* flat = dynamic_cast<const FlatSegment*>(current.get())) {
      auto running = prefix;
      for (std::size_t i = flat->d_begin; i < flat->d_end; ++i) {
        running = tag_combine(running, tag_value((*flat->d_values)[i]));
        if (predicate(running)) {
          auto split = SegmentSplit{
            make_flat_range(flat->d_values, flat->d_begin, i),
            (*flat->d_values)[i],
            make_flat_range(flat->d_values, i + 1, flat->d_end)};
          if (!is_consistent_split(current, split)) {
            return std::nullopt;
          }
          return split;
        }
      }
      return std::nullopt;
    }

    const auto* concat = dynamic_cast<const ConcatSegment*>(current.get());
    assert(concat != nullptr);

    auto left_prefix = tag_combine(prefix, seg_tag(concat->d_left));
    if (predicate(left_prefix)) {
      auto left_split = split_segment(concat->d_left, predicate, prefix);
      if (!left_split.has_value()) {
        return std::nullopt;
      }

      auto split = SegmentSplit{left_split->d_left,
                                left_split->d_pivot,
                                make_concat(left_split->d_right, concat->d_right)};
      if (!is_consistent_split(current, split)) {
        return std::nullopt;
      }
      return split;
    }

    auto right_split = split_segment(make_segment_from_middle(concat->d_right), predicate, left_prefix);
    if (!right_split.has_value()) {
      return std::nullopt;
    }

    auto split = SegmentSplit{make_concat(concat->d_left, right_split->d_left),
                              right_split->d_pivot,
                              right_split->d_right};
    if (!is_consistent_split(current, split)) {
      return std::nullopt;
    }
    return split;
  }

  static auto split_at_count(const SegmentPtr& seg,
                             std::size_t index) -> std::pair<SegmentPtr, SegmentPtr>
  {
    auto current = force_segment(seg);
    if (!current) {
      return {nullptr, nullptr};
    }

    if (const auto* flat = dynamic_cast<const FlatSegment*>(current.get())) {
      auto size = flat->d_end - flat->d_begin;
      auto pivot = index > size ? size : index;
      return {
        make_flat_range(flat->d_values, flat->d_begin, flat->d_begin + pivot),
        make_flat_range(flat->d_values, flat->d_begin + pivot, flat->d_end)};
    }

    const auto* concat = dynamic_cast<const ConcatSegment*>(current.get());
    assert(concat != nullptr);

    auto left_size = seg_size(concat->d_left);
    if (index < left_size) {
      auto split_left = split_at_count(concat->d_left, index);
      return {split_left.first, make_concat(split_left.second, concat->d_right)};
    }

    if (index == left_size) {
      return {concat->d_left, make_segment_from_middle(concat->d_right)};
    }

    auto split_right = split_at_count(make_segment_from_middle(concat->d_right), index - left_size);
    return {make_concat(concat->d_left, split_right.first), split_right.second};
  }

  SegmentPtr d_root;

  explicit FingerTree(SegmentPtr root)
    : d_root(std::move(root))
  {
  }

  explicit FingerTree(std::vector<T> values)
    : d_root(make_flat(std::move(values)))
  {
  }

  FingerTree() = default;

 public:
  // Current complexity contract (prototype implementation):
  // - O(1): empty, leaf, is_empty/is_leaf/is_branch,
  //         is_empty/is_leaf/is_branch, measure, breadth, depth, value.
  // - O(log n): cons, snoc, append/branch/concat,
  //             view_l/view_r, head/last, tail/init,
  //             search, split, split_at, split_at_index, split_at_measure.
  // - O(n): flatten, from_sequence.
  //
  // This keeps a stable API while leaving room for future asymptotic
  // improvements in search/split without changing call sites.
  //
  // Original finger-tree papers target stronger bounds with measured search:
  // amortized O(1) for end operations, O(log(min(n,m))) concatenation,
  // and O(log n) split/search.

  using value_type = T;
  using tag_type = Tag;

  struct View {
    T d_value;
    FingerTree d_rest;
  };

  struct Split {
    FingerTree d_left;
    T d_pivot;
    FingerTree d_right;
  };

  struct SplitAt {
    FingerTree d_left;
    FingerTree d_right;
  };

  static auto empty() -> FingerTree { return FingerTree(std::vector<T>{}); }

  static auto leaf(T value) -> FingerTree
  {
    return FingerTree(std::vector<T>{std::move(value)});
  }

  auto cons(T x) const -> FingerTree
  {
    return FingerTree(make_concat(make_flat(std::vector<T>{std::move(x)}), d_root));
  }

  auto snoc(T x) const -> FingerTree
  {
    return FingerTree(make_concat(d_root, make_flat(std::vector<T>{std::move(x)})));
  }

  auto append(const FingerTree& right) const -> FingerTree
  {
    return FingerTree(make_concat(d_root, right.d_root));
  }

  static auto branch(const FingerTree& left, const FingerTree& right) -> FingerTree
  {
    return left.append(right);
  }

  static auto prepend(T value, const FingerTree& tree) -> FingerTree
  {
    return tree.cons(std::move(value));
  }

  static auto append(const FingerTree& tree, T value) -> FingerTree
  {
    return tree.snoc(std::move(value));
  }

  static auto concat(const FingerTree& left, const FingerTree& right) -> FingerTree
  {
    return left.append(right);
  }

  auto is_empty() const -> bool { return seg_size(d_root) == 0; }
  auto is_leaf() const -> bool { return seg_size(d_root) == 1; }
  auto is_branch() const -> bool { return seg_size(d_root) > 1; }

  auto measure() const -> Tag { return seg_tag(d_root); }

  auto breadth() const -> std::size_t { return seg_size(d_root); }

  auto depth() const -> std::size_t { return seg_depth(d_root); }

  auto value() const -> const T&
  {
    assert(is_leaf());
    const auto* flat = dynamic_cast<const FlatSegment*>(d_root.get());
    assert(flat != nullptr);
    return (*(flat->d_values))[flat->d_begin];
  }

  auto flatten() const -> std::vector<T>
  {
    if (!d_root) {
      return {};
    }

    std::vector<T> out;
    out.reserve(breadth());
    d_root->flatten_into(out);
    return out;
  }

  template <typename PREDICATE>
  auto search(PREDICATE&& predicate) const -> std::optional<T>
  {
    return search_segment(d_root, predicate, tag_identity());
  }

  template <typename PREDICATE>
  auto split(PREDICATE&& predicate) const -> std::optional<Split>
  {
    auto split_result = split_segment(d_root, predicate, tag_identity());
    if (!split_result.has_value()) {
      return std::nullopt;
    }

    return Split{FingerTree(split_result->d_left),
                 std::move(split_result->d_pivot),
                 FingerTree(split_result->d_right)};
  }

  template <typename PREDICATE>
  auto split_at(PREDICATE&& predicate) const -> SplitAt
  {
    auto split_result = split_segment(d_root, predicate, tag_identity());
    if (!split_result.has_value()) {
      return SplitAt{*this, empty()};
    }

    auto right_with_pivot = make_concat(
      make_flat(std::vector<T>{split_result->d_pivot}), split_result->d_right);

    return SplitAt{FingerTree(split_result->d_left), FingerTree(right_with_pivot)};
  }

  auto split_at_index(std::size_t index) const -> SplitAt
  {
    auto clamped = index > breadth() ? breadth() : index;
    auto split_result = split_at_count(d_root, clamped);
    return SplitAt{FingerTree(split_result.first), FingerTree(split_result.second)};
  }

  auto split_at_measure(const Tag& threshold) const -> SplitAt
    requires requires(const Tag& lhs, const Tag& rhs) {
      { lhs >= rhs } -> std::convertible_to<bool>;
    }
  {
    return split_at([&threshold](const Tag& prefix) { return prefix >= threshold; });
  }

  static auto from_sequence(std::vector<T> values) -> FingerTree
  {
    auto shared = std::make_shared<const std::vector<T>>(std::move(values));
    return FingerTree(build_balanced(shared, 0, shared->size()));
  }

  auto view_l() const -> std::optional<View>
  {
    if (!d_root) {
      return std::nullopt;
    }

    auto left = d_root->pop_left();
    if (!left.has_value()) {
      return std::nullopt;
    }

    return View{std::move(left->first), FingerTree(std::move(left->second))};
  }

  auto view_r() const -> std::optional<View>
  {
    if (!d_root) {
      return std::nullopt;
    }

    auto right = d_root->pop_right();
    if (!right.has_value()) {
      return std::nullopt;
    }

    return View{std::move(right->first), FingerTree(std::move(right->second))};
  }

  auto head() const -> T
  {
    auto v = view_l();
    assert(v.has_value());
    return std::move(v->d_value);
  }

  auto tail() const -> FingerTree
  {
    auto v = view_l();
    return v.has_value() ? std::move(v->d_rest) : empty();
  }

  auto last() const -> T
  {
    auto v = view_r();
    assert(v.has_value());
    return std::move(v->d_value);
  }

  auto init() const -> FingerTree
  {
    auto v = view_r();
    return v.has_value() ? std::move(v->d_rest) : empty();
  }
};

}  // namespace smd::tree

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

#include <algorithm>
#include <cstddef>
#include <functional>
#include <iterator>
#include <ranges>
#include <type_traits>
#include <utility>
#include <vector>

namespace smd::tree {

template <typename PAYLOAD_TYPE>
struct Interval {
  std::size_t d_start;
  std::size_t d_end;
  PAYLOAD_TYPE d_payload;
};

template <typename PAYLOAD_TYPE>
struct IntervalMaxEndTag {
  std::size_t d_max_end;

  friend bool operator==(const IntervalMaxEndTag&, const IntervalMaxEndTag&) =
    default;
};

template <typename PAYLOAD_TYPE>
struct IntervalMeasure {
  auto operator()(const Interval<PAYLOAD_TYPE>& interval) const
    -> IntervalMaxEndTag<PAYLOAD_TYPE>
  {
    return IntervalMaxEndTag<PAYLOAD_TYPE>{interval.d_end};
  }
};

template <typename PAYLOAD_TYPE>
class FingerTreeIntervalIndex {
  using Entry = Interval<PAYLOAD_TYPE>;
  using Tree =
    FingerTree<Entry, IntervalMaxEndTag<PAYLOAD_TYPE>, IntervalMeasure<PAYLOAD_TYPE>>;

  Tree d_tree;

 public:
  FingerTreeIntervalIndex()
    : d_tree(Tree::empty())
  {
  }

  static auto from_intervals(std::vector<Entry> entries) -> FingerTreeIntervalIndex
  {
    return FingerTreeIntervalIndex{Tree::from_sequence(std::move(entries))};
  }

  auto insert(Entry entry) const -> FingerTreeIntervalIndex
  {
    return FingerTreeIntervalIndex{d_tree.snoc(std::move(entry))};
  }

  auto query_point(std::size_t point) const -> std::vector<PAYLOAD_TYPE>
  {
    std::vector<PAYLOAD_TYPE> out;

    // Use measure-based pruning: skip intervals where max_end <= point
    // These intervals cannot contain the point by definition
    auto parts = d_tree.split_at([point](const IntervalMaxEndTag<PAYLOAD_TYPE>& prefix) {
      return prefix.d_max_end > point;
    });

    // Only search within intervals with d_max_end > point
    auto flat = parts.d_right.flatten();
    std::ranges::copy(
        flat | std::views::filter([point](const auto& e) {
                   return e.d_start <= point && point < e.d_end;
               })
             | std::views::transform([](const auto& e) { return e.d_payload; }),
        std::back_inserter(out));

    return out;
  }

  auto query_overlap(std::size_t start, std::size_t end) const
    -> std::vector<PAYLOAD_TYPE>
  {
    std::vector<PAYLOAD_TYPE> out;

    // Use measure-based pruning: skip intervals where max_end <= start
    // These intervals cannot overlap with [start, end) by definition
    auto parts = d_tree.split_at([start](const IntervalMaxEndTag<PAYLOAD_TYPE>& prefix) {
      return prefix.d_max_end > start;
    });

    // Only search within intervals with d_max_end > start
    auto flat = parts.d_right.flatten();
    std::ranges::copy(
        flat | std::views::filter([start, end](const auto& e) {
                   return e.d_start < end && start < e.d_end;
               })
             | std::views::transform([](const auto& e) { return e.d_payload; }),
        std::back_inserter(out));

    return out;
  }

  auto entries() const -> std::vector<Entry> { return d_tree.flatten(); }

 private:
  explicit FingerTreeIntervalIndex(Tree tree)
    : d_tree(std::move(tree))
  {
  }
};

}  // namespace smd::tree

namespace smd::typeclass {

template <typename PAYLOAD_TYPE>
struct Monoid<smd::tree::IntervalMaxEndTag<PAYLOAD_TYPE>> {
  auto identity() const -> smd::tree::IntervalMaxEndTag<PAYLOAD_TYPE>
  {
    return {0U};
  }

  auto combine(const smd::tree::IntervalMaxEndTag<PAYLOAD_TYPE>& lhs,
               const smd::tree::IntervalMaxEndTag<PAYLOAD_TYPE>& rhs) const
    -> smd::tree::IntervalMaxEndTag<PAYLOAD_TYPE>
  {
    return {std::max(lhs.d_max_end, rhs.d_max_end)};
  }
};

}  // namespace smd::typeclass

#endif

#include <smd/tree/finger_tree_interval_index_foldable.hpp>
#include <smd/tree/finger_tree_interval_index_traversable.hpp>

```

## smd/tree/finger_tree_interval_index.t.cpp

```cpp
#include <smd/tree/finger_tree_interval_index.hpp>

#include <catch2/catch_test_macros.hpp>

#include <optional>
#include <string>
#include <vector>

TEST_CASE("FingerTreeIntervalIndexTest - WrapperOperations")
{
    using Index = smd::tree::FingerTreeIntervalIndex<std::string>;
    using Entry = smd::tree::Interval<std::string>;

    auto idx = Index::from_intervals({
        Entry{0U, 5U, "A"},
        Entry{3U, 10U, "B"},
        Entry{8U, 12U, "C"}
    });

    CHECK(idx.query_point(2U) == (std::vector<std::string>{"A"}));
    CHECK(idx.query_point(4U) == (std::vector<std::string>{"A", "B"}));
    CHECK(idx.query_overlap(9U, 11U) == (std::vector<std::string>{"B", "C"}));
}

TEST_CASE("FingerTreeIntervalIndexTest - FoldableTypeclass")
{
    using Index = smd::tree::FingerTreeIntervalIndex<std::string>;
    using Entry = smd::tree::Interval<std::string>;

    auto idx = Index::from_intervals({
      Entry{0U, 5U, "A"},
      Entry{3U, 10U, "B"},
      Entry{8U, 12U, "C"}
    });
    const auto& foldable = smd::foldable_typeclass<Index>;

    CHECK(
      foldable.fold_map([](const std::string& payload) { return payload; }, idx) ==
      "ABC");
    CHECK(foldable.length(idx) == 3U);
}

TEST_CASE("FingerTreeIntervalIndexTest - TraversableTypeclass")
{
    using Index = smd::tree::FingerTreeIntervalIndex<std::string>;
    using Entry = smd::tree::Interval<std::string>;

    auto idx = Index::from_intervals({
      Entry{0U, 5U, "A"},
      Entry{3U, 10U, "B"},
      Entry{8U, 12U, "C"}
    });
    const auto& traversable = smd::traversable_typeclass<Index>;

    auto success = smd::traverse(
      [](const std::string& payload) -> std::optional<std::string> {
          return payload + "!";
      },
      idx);
    REQUIRE(success.has_value());
    CHECK(success->query_point(4U) == (std::vector<std::string>{"A!", "B!"}));
    CHECK(success->query_overlap(9U, 11U) ==
              (std::vector<std::string>{"B!", "C!"}));

    auto failure = smd::traverse(
      [](const std::string& payload) -> std::optional<std::string> {
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
#include <smd/typeclass/monoid.hpp>

#include <algorithm>
#include <cstddef>
#include <optional>
#include <utility>
#include <vector>

namespace smd::tree {

template <typename T>
struct MinTag {
  std::optional<T> d_value;

  friend bool operator==(const MinTag&, const MinTag&) = default;
};

template <typename T>
struct MaxTag {
  std::optional<T> d_value;

  friend bool operator==(const MaxTag&, const MaxTag&) = default;
};

template <typename T>
struct MinMeasure {
  auto operator()(const T& value) const -> MinTag<T> { return MinTag<T>{value}; }
};

template <typename T>
struct MaxMeasure {
  auto operator()(const T& value) const -> MaxTag<T> { return MaxTag<T>{value}; }
};

template <typename T>
class FingerTreePriorityQueue {
  using MinTree = FingerTree<T, MinTag<T>, MinMeasure<T>>;
  using MaxTree = FingerTree<T, MaxTag<T>, MaxMeasure<T>>;

  MinTree d_min_tree;
  MaxTree d_max_tree;

  template <typename TREE>
  static auto remove_one_rebuild(const TREE& tree, const T& needle) -> TREE
  {
    auto values = tree.flatten();
    auto it = std::find(values.begin(), values.end(), needle);
    if (it == values.end()) {
      return tree;
    }

    values.erase(it);
    return TREE::from_sequence(std::move(values));
  }

 public:
  FingerTreePriorityQueue()
    : d_min_tree(MinTree::empty())
    , d_max_tree(MaxTree::empty())
  {
  }

  static auto from_values(std::vector<T> values) -> FingerTreePriorityQueue
  {
    return FingerTreePriorityQueue{
      MinTree::from_sequence(values),
      MaxTree::from_sequence(std::move(values))
    };
  }

  auto empty() const -> bool { return d_min_tree.is_empty(); }

  auto size() const -> std::size_t { return d_min_tree.breadth(); }

  auto min() const -> std::optional<T>
  {
    auto m = d_min_tree.measure().d_value;
    return m.has_value() ? std::optional<T>{*m} : std::nullopt;
  }

  auto max() const -> std::optional<T>
  {
    auto m = d_max_tree.measure().d_value;
    return m.has_value() ? std::optional<T>{*m} : std::nullopt;
  }

  auto push(T value) const -> FingerTreePriorityQueue
  {
    return FingerTreePriorityQueue{
      d_min_tree.snoc(value),
      d_max_tree.snoc(std::move(value))
    };
  }

  // Removes the minimum element. Both trees are rebuilt from the flattened
  // sequence (O(n)); this is a correctness-first implementation pending O(log n)
  // split-based optimization that does not change the call-site API.
  auto pop_min() const -> std::optional<std::pair<T, FingerTreePriorityQueue>>
  {
    auto m = min();
    if (!m.has_value()) {
      return std::nullopt;
    }

    auto new_min_tree = remove_one_rebuild(d_min_tree, *m);
    auto new_max_tree = remove_one_rebuild(d_max_tree, *m);

    return std::pair<T, FingerTreePriorityQueue>{
      *m,
      FingerTreePriorityQueue{std::move(new_min_tree), std::move(new_max_tree)}
    };
  }

  // Same semantics as pop_min but returns the maximum element.
  auto pop_max() const -> std::optional<std::pair<T, FingerTreePriorityQueue>>
  {
    auto m = max();
    if (!m.has_value()) {
      return std::nullopt;
    }

    auto rebuilt_max = remove_one_rebuild(d_max_tree, *m);
    auto rebuilt_min = remove_one_rebuild(d_min_tree, *m);

    return std::pair<T, FingerTreePriorityQueue>{
      *m,
      FingerTreePriorityQueue{std::move(rebuilt_min), std::move(rebuilt_max)}
    };
  }

  auto to_vector() const -> std::vector<T> { return d_min_tree.flatten(); }

 private:
  FingerTreePriorityQueue(MinTree min_tree, MaxTree max_tree)
    : d_min_tree(std::move(min_tree))
    , d_max_tree(std::move(max_tree))
  {
  }
};

}  // namespace smd::tree

namespace smd::typeclass {

template <typename T>
struct Monoid<smd::tree::MinTag<T>> {
  auto identity() const -> smd::tree::MinTag<T> { return {std::nullopt}; }

  auto combine(const smd::tree::MinTag<T>& lhs,
               const smd::tree::MinTag<T>& rhs) const -> smd::tree::MinTag<T>
  {
    if (!lhs.d_value.has_value()) {
      return rhs;
    }
    if (!rhs.d_value.has_value()) {
      return lhs;
    }

    return lhs.d_value.value() <= rhs.d_value.value() ? lhs : rhs;
  }
};

template <typename T>
struct Monoid<smd::tree::MaxTag<T>> {
  auto identity() const -> smd::tree::MaxTag<T> { return {std::nullopt}; }

  auto combine(const smd::tree::MaxTag<T>& lhs,
               const smd::tree::MaxTag<T>& rhs) const -> smd::tree::MaxTag<T>
  {
    if (!lhs.d_value.has_value()) {
      return rhs;
    }
    if (!rhs.d_value.has_value()) {
      return lhs;
    }

    return lhs.d_value.value() >= rhs.d_value.value() ? lhs : rhs;
  }
};

}  // namespace smd::typeclass

#endif

#include <smd/tree/finger_tree_priority_queue_foldable.hpp>
#include <smd/tree/finger_tree_priority_queue_traversable.hpp>

```

## smd/tree/finger_tree_priority_queue.t.cpp

```cpp
#include <smd/tree/finger_tree_priority_queue.hpp>

#include <catch2/catch_test_macros.hpp>

#include <optional>
#include <set>
#include <vector>

TEST_CASE("FingerTreePriorityQueueTest - WrapperOperations")
{
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

TEST_CASE("FingerTreePriorityQueueTest - FoldableTypeclass")
{
    using Queue = smd::tree::FingerTreePriorityQueue<int>;

    auto q = Queue::from_values({5, 2, 8, 2, 7});
    const auto& foldable = smd::foldable_typeclass<Queue>;

    CHECK(foldable.fold_map([](int value) { return value; }, q) == 24);
    CHECK(foldable.length(q) == 5U);
}

TEST_CASE("FingerTreePriorityQueueTest - TraversableTypeclass")
{
    using Queue = smd::tree::FingerTreePriorityQueue<int>;

    auto q = Queue::from_values({5, 2, 8});

    auto success = smd::traverse(
      [](int value) -> std::optional<int> { return value * 10; },
      q);
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

TEST_CASE("FingerTreePriorityQueueTest - RepeatedPushPopMatchesMultiset")
{
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

TEST_CASE("FingerTreePriorityQueueTest - PopMinWithDuplicates")
{
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

TEST_CASE("FingerTreePriorityQueueTest - PopMaxWithDuplicates")
{
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

#include <cstddef>
#include <functional>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

namespace smd::tree {

template <typename T>
class FingerTreeRandomAccess {
  FingerTree<T> d_tree;

 public:
  FingerTreeRandomAccess()
    : d_tree(FingerTree<T>::empty())
  {
  }

  explicit FingerTreeRandomAccess(FingerTree<T> tree)
    : d_tree(std::move(tree))
  {
  }

  static auto from_sequence(std::vector<T> values) -> FingerTreeRandomAccess
  {
    return FingerTreeRandomAccess(FingerTree<T>::from_sequence(std::move(values)));
  }

  auto size() const -> std::size_t { return d_tree.breadth(); }

  auto empty() const -> bool { return d_tree.is_empty(); }

  auto at(std::size_t index) const -> std::optional<T>
  {
    if (index >= size()) {
      return std::nullopt;
    }
    // Use indexed split to minimize materialization
    // Split at index+1 to get everything up to and including the element
    auto parts = d_tree.split_at_index(index + 1);
    // The element at index is the last element of the left part
    auto left_vec = parts.d_left.flatten();
    return left_vec.back();
  }

  auto push_back(T value) const -> FingerTreeRandomAccess
  {
    return FingerTreeRandomAccess(d_tree.snoc(std::move(value)));
  }

  auto push_front(T value) const -> FingerTreeRandomAccess
  {
    return FingerTreeRandomAccess(d_tree.cons(std::move(value)));
  }

  auto insert(std::size_t index, T value) const -> FingerTreeRandomAccess
  {
    auto parts = d_tree.split_at_index(index);
    auto middle = FingerTree<T>::leaf(std::move(value));
    return FingerTreeRandomAccess(FingerTree<T>::concat(FingerTree<T>::concat(parts.d_left, middle), parts.d_right));
  }

  auto erase(std::size_t index) const -> FingerTreeRandomAccess
  {
    if (index >= size()) {
      return *this;
    }

    auto left_right = d_tree.split_at_index(index);
    auto drop_rest = left_right.d_right.tail();
    return FingerTreeRandomAccess(FingerTree<T>::concat(left_right.d_left, drop_rest));
  }

  auto update(std::size_t index, T value) const -> FingerTreeRandomAccess
  {
    return erase(index).insert(index, std::move(value));
  }

  auto to_vector() const -> std::vector<T> { return d_tree.flatten(); }
};

}  // namespace smd::tree

#endif

#include <smd/tree/finger_tree_random_access_foldable.hpp>
#include <smd/tree/finger_tree_random_access_traversable.hpp>

```

## smd/tree/finger_tree_random_access.t.cpp

```cpp
#include <smd/tree/finger_tree_random_access.hpp>

#include <catch2/catch_test_macros.hpp>

#include <optional>
#include <vector>

TEST_CASE("FingerTreeRandomAccessTest - WrapperOperations")
{
    // f2a6c9b3-8d1e-4f7a-c5b9-4e2d7a3c6f01
    using Seq = smd::tree::FingerTreeRandomAccess<int>;

    auto seq = Seq::from_sequence({1, 2, 3});
    REQUIRE(seq.at(0).has_value());
    CHECK(*seq.at(0) == 1);
    CHECK_FALSE(seq.at(99).has_value());

    auto edited = seq.push_back(4).push_front(0).insert(2, 9).update(3, 7).erase(1);
    CHECK(edited.to_vector() == (std::vector<int>{0, 9, 7, 3, 4}));
    // f2a6c9b3-8d1e-4f7a-c5b9-4e2d7a3c6f01 end
}

TEST_CASE("FingerTreeRandomAccessTest - FoldableTypeclass")
{
    using Seq = smd::tree::FingerTreeRandomAccess<int>;

    auto seq = Seq::from_sequence({1, 2, 3, 4});
    const auto& foldable = smd::foldable_typeclass<Seq>;

    CHECK(foldable.fold_map([](int value) { return value; }, seq) == 10);
    CHECK(foldable.length(seq) == 4U);
}

TEST_CASE("FingerTreeRandomAccessTest - TraversableTypeclass")
{
    using Seq = smd::tree::FingerTreeRandomAccess<int>;

    auto seq = Seq::from_sequence({1, 2, 3});
    const auto& traversable = smd::traversable_typeclass<Seq>;

    auto success = smd::traverse(
      [](int value) -> std::optional<int> { return value * 10; },
      seq);
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

struct RopeChunkMeasure {
  auto operator()(const std::string& value) const -> std::size_t
  {
    return value.size();
  }
};

class FingerTreeRope {
  using Tree = FingerTree<std::string, std::size_t, RopeChunkMeasure>;

  Tree d_tree;

  auto split_chars(std::size_t pos) const -> std::pair<FingerTreeRope, FingerTreeRope>
  {
    if (pos == 0) {
      return {FingerTreeRope{}, *this};
    }

    if (pos >= size_bytes()) {
      return {*this, FingerTreeRope{}};
    }

    auto split = d_tree.split([pos](std::size_t prefix) {
      return prefix > pos;
    });

    if (!split.has_value()) {
      return {*this, FingerTreeRope{}};
    }

    auto left_prefix_bytes = split->d_left.measure();
    auto local = pos - left_prefix_bytes;

    const auto& pivot = split->d_pivot;
    assert(local < pivot.size());

    auto left = split->d_left;
    if (local > 0) {
      left = left.snoc(pivot.substr(0, local));
    }

    auto right = split->d_right;
    if (local < pivot.size()) {
      right = right.cons(pivot.substr(local));
    }

    return {FingerTreeRope{std::move(left)}, FingerTreeRope{std::move(right)}};
  }

 public:
  FingerTreeRope()
    : d_tree(Tree::empty())
  {
  }

  static auto from_chunks(std::vector<std::string> chunks) -> FingerTreeRope
  {
    return FingerTreeRope{Tree::from_sequence(std::move(chunks))};
  }

  static auto from_text(std::string_view text, std::size_t chunk_size = 16)
    -> FingerTreeRope
  {
    std::vector<std::string> chunks;
    chunks.reserve((text.size() / chunk_size) + 1);

    for (std::size_t i = 0; i < text.size(); i += chunk_size) {
      const auto n = std::min(chunk_size, text.size() - i);
      chunks.emplace_back(text.substr(i, n));
    }

    return from_chunks(std::move(chunks));
  }

  auto size_bytes() const -> std::size_t { return d_tree.measure(); }

  auto to_string() const -> std::string
  {
    std::string out;
    out.reserve(size_bytes());
    std::ranges::for_each(d_tree.flatten(),
                          [&out](const std::string& chunk) { out += chunk; });
    return out;
  }

  auto insert(std::size_t pos, std::string_view text) const -> FingerTreeRope
  {
    auto [left, right] = split_chars(pos);
    auto middle = from_text(text);
    return FingerTreeRope{Tree::concat(Tree::concat(left.d_tree, middle.d_tree),
                                        right.d_tree)};
  }

  auto erase(std::size_t pos, std::size_t count) const -> FingerTreeRope
  {
    auto [left, rest] = split_chars(pos);
    auto [drop, right] = rest.split_chars(count);
    static_cast<void>(drop);
    return FingerTreeRope{Tree::concat(left.d_tree, right.d_tree)};
  }

  auto replace(std::size_t pos, std::size_t count, std::string_view text) const
    -> FingerTreeRope
  {
    return erase(pos, count).insert(pos, text);
  }

  auto chunks() const -> std::vector<std::string> { return d_tree.flatten(); }

 private:
  explicit FingerTreeRope(Tree tree)
    : d_tree(std::move(tree))
  {
  }
};

}  // namespace smd::tree

#endif

#include <smd/tree/finger_tree_rope_foldable.hpp>
#include <smd/tree/finger_tree_rope_traversable.hpp>

```

## smd/tree/finger_tree_rope.t.cpp

```cpp
#include <smd/tree/finger_tree_rope.hpp>

#include <catch2/catch_test_macros.hpp>

#include <optional>
#include <string>

TEST_CASE("FingerTreeRopeTest - WrapperOperations")
{
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

TEST_CASE("FingerTreeRopeTest - FoldableTypeclass")
{
    using Rope = smd::tree::FingerTreeRope;

    auto rope = Rope::from_text("abcdefgh", 2);
    const auto& foldable = smd::foldable_typeclass<Rope>;

    CHECK(
      foldable.fold_map(
        [](const std::string& chunk) { return chunk.size(); },
        rope) ==
      8U);
    CHECK(foldable.length(rope) == 4U);
}

TEST_CASE("FingerTreeRopeTest - TraversableTypeclass")
{
    using Rope = smd::tree::FingerTreeRope;

    auto rope = Rope::from_text("abcd", 2);
    const auto& traversable = smd::traversable_typeclass<Rope>;

    auto success = smd::traverse(
      [](const std::string& chunk) -> std::optional<std::string> {
          return chunk + "!";
      },
      rope);
    REQUIRE(success.has_value());
    CHECK(success->to_string() == "ab!cd!");

    auto failure = smd::traverse(
      [](const std::string& chunk) -> std::optional<std::string> {
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
#include <smd/tree/finger_tree.hpp>
#include <smd/tree/finger_tree_foldable.hpp>
#include <smd/tree/memoized_thunk.hpp>
#include <smd/tree/finger_tree_traversable.hpp>

#include <catch2/catch_test_macros.hpp>

#include <atomic>
#include <cstddef>
#include <optional>
#include <vector>

namespace {

auto ceil_log2(std::size_t n) -> std::size_t
{
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

    friend bool operator==(const Weighted&, const Weighted&) = default;
    friend bool operator>=(const Weighted& lhs, const Weighted& rhs)
    {
        return lhs.d_total >= rhs.d_total;
    }
};

struct WeightedMeasure {
    auto operator()(int value) const -> Weighted
    {
        return Weighted{static_cast<std::size_t>(value * 10)};
    }
};

}  // namespace

namespace smd::typeclass {

template <>
struct Monoid<Weighted> {
    constexpr auto identity() const -> Weighted { return Weighted{0U}; }

    constexpr auto combine(const Weighted& lhs, const Weighted& rhs) const
        -> Weighted
    {
        return Weighted{lhs.d_total + rhs.d_total};
    }
};

}  // namespace smd::typeclass

TEST_CASE("FingerTreeTest - EmptyLeafAndPredicates")
{
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

TEST_CASE("FingerTreeStrictnessTest - MemoizedThunkForcesOnce")
{
    std::atomic<int> evaluations{0};
    auto delayed = smd::tree::detail::thunk([
        &evaluations]() {
        evaluations.fetch_add(1, std::memory_order_relaxed);
        return 42;
    });

    CHECK(delayed() == 42);
    CHECK(delayed() == 42);
    CHECK(evaluations.load(std::memory_order_relaxed) == 1);
}

TEST_CASE("FingerTreeStrictnessTest - MemoizedThunkSharesAcrossCopies")
{
    std::atomic<int> evaluations{0};
    auto delayed = smd::tree::detail::thunk([
        &evaluations]() {
        evaluations.fetch_add(1, std::memory_order_relaxed);
        return 7;
    });
    auto alias = delayed;

    CHECK(delayed() == 7);
    CHECK(alias() == 7);
    CHECK(evaluations.load(std::memory_order_relaxed) == 1);
}

TEST_CASE("FingerTreeStrictnessTest - MeasuredThunkExposesCachedMeasureWithoutForce")
{
    std::atomic<int> evaluations{0};
    auto delayed = smd::tree::detail::measured_thunk(
        std::size_t{99},
        [&evaluations]() {
            evaluations.fetch_add(1, std::memory_order_relaxed);
            return 123;
        });

    CHECK(delayed.cached_measure() == 99U);
    CHECK(evaluations.load(std::memory_order_relaxed) == 0);
    CHECK(delayed.force() == 123);
    CHECK(delayed.cached_measure() == 99U);
    CHECK(evaluations.load(std::memory_order_relaxed) == 1);
}

TEST_CASE("FingerTreeTest - FromSequenceConsSnocAndMemberAppend")
{
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

TEST_CASE("FingerTreeTest - SingletonViewsAndEmptyTailInit")
{
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

TEST_CASE("FingerTreeTest - BasicMeasureDepthFlatten")
{
    using Tree = smd::tree::FingerTree<int>;

    auto tree = Tree::branch(
        Tree::branch(Tree::leaf(1), Tree::leaf(2)),
        Tree::leaf(3));

    CHECK(tree.measure() == 3U);
    CHECK(tree.breadth() == 3U);
    CHECK(tree.depth() >= 1U);
    CHECK(tree.flatten() == (std::vector<int>{1, 2, 3}));
}

TEST_CASE("FingerTreeTest - ViewsAndListOps")
{
    using Tree = smd::tree::FingerTree<int>;

    auto tree = Tree::branch(
        Tree::branch(Tree::leaf(1), Tree::leaf(2)),
        Tree::leaf(3));

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

TEST_CASE("FingerTreeTest - PrependAppendConcat")
{
    using Tree = smd::tree::FingerTree<int>;

    auto tree = Tree::branch(Tree::leaf(1), Tree::leaf(2));

    auto prepended = Tree::prepend(0, tree);
    CHECK(prepended.flatten() == (std::vector<int>{0, 1, 2}));

    auto appended = Tree::append(tree, 3);
    CHECK(appended.flatten() == (std::vector<int>{1, 2, 3}));

    auto concatenated = Tree::concat(tree, tree);
    CHECK(concatenated.flatten() == (std::vector<int>{1, 2, 1, 2}));
}

TEST_CASE("FingerTreeTest - MonoidTaggedMeasure")
{
    using Tree = smd::tree::FingerTree<int, Weighted, WeightedMeasure>;

    auto tree = Tree::from_sequence({1, 2, 3});
    CHECK(tree.measure() == Weighted{60U});

    auto prepended = Tree::prepend(4, tree);
    CHECK(prepended.measure() == Weighted{100U});

    auto concatenated = Tree::concat(tree, Tree::leaf(5));
    CHECK(concatenated.measure() == Weighted{110U});
}

TEST_CASE("FingerTreeTest - MeasureGuidedSearchAndSplit")
{
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

    CHECK_FALSE(tree.search([](std::size_t prefix) { return prefix >= 6U; }).has_value());
    CHECK_FALSE(tree.split([](std::size_t prefix) { return prefix >= 6U; }).has_value());
}

TEST_CASE("FingerTreeTest - MeasureGuidedSearchAndSplitWithCustomTag")
{
    using Tree = smd::tree::FingerTree<int, Weighted, WeightedMeasure>;

    auto tree = Tree::from_sequence({1, 2, 3, 4});

    auto found = tree.search([](Weighted prefix) { return prefix.d_total >= 35U; });
    REQUIRE(found.has_value());
    CHECK(*found == 3);

    auto split = tree.split([](Weighted prefix) { return prefix.d_total >= 35U; });
    REQUIRE(split.has_value());
    CHECK(split->d_left.flatten() == (std::vector<int>{1, 2}));
    CHECK(split->d_left.measure() == Weighted{30U});
    CHECK(split->d_pivot == 3);
    CHECK(split->d_right.flatten() == (std::vector<int>{4}));
    CHECK(split->d_right.measure() == Weighted{40U});
}

TEST_CASE("FingerTreeTest - SplitAtCountBoundary")
{
    using Tree = smd::tree::FingerTree<int>;

    auto tree = Tree::from_sequence({1, 2, 3, 4, 5});

    auto at_three = tree.split_at([](std::size_t prefix) { return prefix >= 3U; });
    CHECK(at_three.d_left.flatten() == (std::vector<int>{1, 2}));
    CHECK(at_three.d_right.flatten() == (std::vector<int>{3, 4, 5}));

    auto at_one = tree.split_at([](std::size_t prefix) { return prefix >= 1U; });
    CHECK(at_one.d_left.is_empty());
    CHECK(at_one.d_right.flatten() == (std::vector<int>{1, 2, 3, 4, 5}));

    auto none = tree.split_at([](std::size_t prefix) { return prefix >= 6U; });
    CHECK(none.d_left.flatten() == (std::vector<int>{1, 2, 3, 4, 5}));
    CHECK(none.d_right.is_empty());
}

TEST_CASE("FingerTreeTest - SplitAtWeightedBoundary")
{
    using Tree = smd::tree::FingerTree<int, Weighted, WeightedMeasure>;

    auto tree = Tree::from_sequence({1, 2, 3, 4});

    auto split = tree.split_at([](Weighted prefix) { return prefix.d_total >= 35U; });
    CHECK(split.d_left.flatten() == (std::vector<int>{1, 2}));
    CHECK(split.d_left.measure() == Weighted{30U});
    CHECK(split.d_right.flatten() == (std::vector<int>{3, 4}));
    CHECK(split.d_right.measure() == Weighted{70U});
}

TEST_CASE("FingerTreeTest - SplitAtIndexConvenience")
{
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

TEST_CASE("FingerTreeTest - SplitAtMeasureConvenience")
{
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

TEST_CASE("FingerTreePersistenceTest - SharedVersionsSurviveAppendAndPops")
{
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

TEST_CASE("FingerTreePersistenceTest - SharedVersionsSurviveSearchAndSplit")
{
    using Tree = smd::tree::FingerTree<int>;

    auto base = Tree::from_sequence({1, 2, 3, 4, 5, 6});
    auto appended = base.append(Tree::from_sequence({7, 8, 9}));
    auto split = appended.split([](std::size_t prefix) { return prefix >= 7U; });
    REQUIRE(split.has_value());

    auto count_split = appended.split_at_index(4U);
    auto found = appended.search([](std::size_t prefix) { return prefix >= 8U; });

    REQUIRE(found.has_value());
    CHECK(*found == 8);
    CHECK(split->d_left.flatten() == (std::vector<int>{1, 2, 3, 4, 5, 6}));
    CHECK(split->d_pivot == 7);
    CHECK(split->d_right.flatten() == (std::vector<int>{8, 9}));
    CHECK(count_split.d_left.flatten() == (std::vector<int>{1, 2, 3, 4}));
    CHECK(count_split.d_right.flatten() == (std::vector<int>{5, 6, 7, 8, 9}));

    CHECK(base.flatten() == (std::vector<int>{1, 2, 3, 4, 5, 6}));
    CHECK(base.search([](std::size_t prefix) { return prefix >= 4U; }) == std::optional<int>{4});
}

TEST_CASE("FingerTreePersistenceTest - WeightedSharedVersionsKeepMeasures")
{
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

TEST_CASE("FingerTreePersistenceTest - RepeatedSplitPopAcrossSharedVersions")
{
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

TEST_CASE("FingerTreeTest - DepthRemainsLogarithmic")
{
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

TEST_CASE("FingerTreeFoldableTest - FoldMapAndDerivedOperations")
{
    using Tree = smd::tree::FingerTree<int>;

    auto tree = Tree::from_sequence({1, 2, 3, 4});
    const auto& foldable = smd::foldable_typeclass<Tree>;

    CHECK(foldable.length(tree) == 4U);
    CHECK(foldable.fold_map([](int x) { return x; }, tree) == 10);
    CHECK(foldable.to_vector(tree) == (std::vector<int>{1, 2, 3, 4}));

    const auto left = foldable.fold_left(tree, 0, [](int acc, int x) {
        return acc * 10 + x;
    });
    CHECK(left == 1234);
}

TEST_CASE("FingerTreeTraversableTest - TraverseOptionalSuccess")
{
    using Tree = smd::tree::FingerTree<int>;

    auto tree = Tree::from_sequence({1, 2, 3});
    const auto& traversable = smd::traversable_typeclass<Tree>;

    auto traversed = smd::traverse(
        [](int x) -> std::optional<int> {
            return x > 0 ? std::optional<int>{x * 10} : std::optional<int>{};
        },
        tree);

    REQUIRE(traversed.has_value());
    CHECK(traversed->flatten() == (std::vector<int>{10, 20, 30}));
}

TEST_CASE("FingerTreeTraversableTest - TraverseOptionalFailure")
{
    using Tree = smd::tree::FingerTree<int>;

    auto tree = Tree::from_sequence({1, -2, 3});
    const auto& traversable = smd::traversable_typeclass<Tree>;

    auto traversed = smd::traverse(
        [](int x) -> std::optional<int> {
            return x > 0 ? std::optional<int>{x * 10} : std::optional<int>{};
        },
        tree);

    CHECK_FALSE(traversed.has_value());
}

```

## smd/tree/finger_tree_traversable.hpp

```cpp
// src/smd/tree/finger_tree_traversable.hpp                           -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_FINGER_TREE_TRAVERSABLE
#define INCLUDED_SMD_TREE_FINGER_TREE_TRAVERSABLE

// Traversable constraint for FingerTree core:
// - Materialization: traverse materializes the tree via flatten() into a vector.
// - Preservation: monoid measure semantics are preserved through the traversal.
// - Reconstruction: results are rebuilt via FingerTree<U>::from_sequence() with same measure policy.
// - Applicative semantics: all traversals follow left-to-right order independent of tree structure.
//
// Rationale: FingerTree provides efficient structural operations (cons, snoc, split);
// traversal is not a primary performance path, so O(n) materialization is acceptable.
// Wrapper types override with specialized traversal that preserves wrapper invariants.

#include <smd/tree/finger_tree.hpp>
#include <smd/typeclass/applicative.hpp>
#include <smd/typeclass/traversable.hpp>

#include <functional>
#include <type_traits>
#include <utility>
#include <vector>

namespace smd {

template <class T, class TAG_TYPE, class MEASURE_POLICY>
struct FingerTreeTraversableImpl {
  using element_type = T;

  template <class APPLICATIVE, class F>
  auto traverse(this auto&&,
                const APPLICATIVE& applicative,
                F&& function,
                const smd::tree::FingerTree<T, TAG_TYPE, MEASURE_POLICY>& tree)
  {
    using Context = remove_cvref_t<std::invoke_result_t<F, const T&>>;
    using U = smd::applicative_value_t<Context>;

    auto accumulated = applicative.pure(std::vector<U>{});

    for (const auto& value : tree.flatten()) {
      auto lifted = std::invoke(function, value);
      accumulated = applicative.invoke(
        [](std::vector<U> values, U element) {
          values.push_back(std::move(element));
          return values;
        },
        std::move(accumulated),
        std::move(lifted));
    }

    return applicative.invoke(
      [](std::vector<U> values) {
        return smd::tree::FingerTree<U>::from_sequence(std::move(values));
      },
      std::move(accumulated));
  }
};

template <class T, class TAG_TYPE, class MEASURE_POLICY>
struct FingerTreeTraversableMap
  : Traversable<FingerTreeTraversableImpl<T, TAG_TYPE, MEASURE_POLICY>> {
  using FingerTreeTraversableImpl<T, TAG_TYPE, MEASURE_POLICY>::traverse;
};

template <class T, class TAG_TYPE, class MEASURE_POLICY>
inline constexpr auto traversable_typeclass<
  smd::tree::FingerTree<T, TAG_TYPE, MEASURE_POLICY>> =
  FingerTreeTraversableMap<T, TAG_TYPE, MEASURE_POLICY>{};

}  // close namespace smd

#endif

```

## smd/tree/finger_tree_wrappers.hpp

```cpp
// src/smd/tree/finger_tree_wrappers.hpp                              -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_FINGER_TREE_WRAPPERS
#define INCLUDED_SMD_TREE_FINGER_TREE_WRAPPERS

// Core wrapper classes
#include <smd/tree/finger_tree_interval_index.hpp>
#include <smd/tree/finger_tree_priority_queue.hpp>
#include <smd/tree/finger_tree_random_access.hpp>
#include <smd/tree/finger_tree_rope.hpp>

// Wrapper-specific typeclass implementations
#include <smd/tree/finger_tree_interval_index_foldable.hpp>
#include <smd/tree/finger_tree_interval_index_traversable.hpp>
#include <smd/tree/finger_tree_priority_queue_foldable.hpp>
#include <smd/tree/finger_tree_priority_queue_traversable.hpp>
#include <smd/tree/finger_tree_random_access_foldable.hpp>
#include <smd/tree/finger_tree_random_access_traversable.hpp>
#include <smd/tree/finger_tree_rope_foldable.hpp>
#include <smd/tree/finger_tree_rope_traversable.hpp>

#endif

```

## smd/tree/fix_tree_applicative.hpp

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
  auto pure(this auto&&, VALUE&& x)
  {
    using U = std::remove_cvref_t<VALUE>;
    return smd::tree::FixTree<U>::leaf(std::forward<VALUE>(x));
  }

  template <class F, class A>
  auto apply(this auto&& self,
             const smd::tree::FixTree<F>& fs,
             const smd::tree::FixTree<A>& xs)
  {
    using R = decltype(fs.value()(xs.value()));
    if (fs.is_leaf()) {
      auto f = fs.value();
      if (xs.is_leaf()) {
        return smd::tree::FixTree<R>::leaf(f(xs.value()));
      }
      return smd::tree::FixTree<R>::node(
        self.apply(fs, xs.left()), self.apply(fs, xs.right()));
    }
    return smd::tree::FixTree<R>::node(self.apply(fs.left(), xs),
                                       self.apply(fs.right(), xs));
  }
};

template <class T>
struct FixTreeApplicativeMap : Applicative<FixTreeApplicativeImpl<T> > {
  using FixTreeApplicativeImpl<T>::apply;
  using FixTreeApplicativeImpl<T>::pure;
};

template <class T>
inline constexpr auto applicative_typeclass<smd::tree::FixTree<T> > =
  FixTreeApplicativeMap<T>{};

}

#endif

```

## smd/tree/fix_tree_applicative.t.cpp

```cpp
#include <smd/tree/fix_tree.hpp>
#include <smd/tree/fix_tree_applicative.hpp>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("FixTreeApplicativeTest - InvokeDistributesLeafOverShape")
{
    using Tree = smd::tree::FixTree<int>;
    auto scalar = Tree::leaf(10);
    auto shaped = Tree::node(Tree::leaf(1), Tree::leaf(2));

    const auto& applicative = smd::applicative_typeclass<Tree>;
    auto summed = applicative.invoke(
        [](int lhs, int rhs) { return lhs + rhs; },
        scalar,
        shaped);

    REQUIRE_FALSE(summed.is_leaf());
    CHECK(summed.left().value() == 11);
    CHECK(summed.right().value() == 12);
}

```

## smd/tree/fix_tree_foldable.hpp

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
    auto fold_map(this auto&& self, F&& f, const smd::tree::FixTree<T>& t)
    {
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
struct FixTreeFoldableMap : Foldable<FixTreeFoldableImpl<T> > {
    using FixTreeFoldableImpl<T>::fold_map;
};

// d6e2b9f4-1a7c-4b3e-8f5d-3c9a2e7b6f08
template <class T>
inline constexpr auto foldable_typeclass<smd::tree::FixTree<T> > =
    FixTreeFoldableMap<T>{};
// d6e2b9f4-1a7c-4b3e-8f5d-3c9a2e7b6f08 end

}  // close namespace smd

#endif

```

## smd/tree/fix_tree_foldable.t.cpp

```cpp
#include <smd/tree/fix_tree.hpp>
#include <smd/tree/fix_tree_foldable.hpp>
#include <smd/typeclass/foldable.hpp>

#include <catch2/catch_test_macros.hpp>

#include <vector>

namespace {

template <class TREE,
          const auto& FOLDABLE = smd::foldable_typeclass<TREE> >
auto sum_with_nttp_lookup(const TREE& tree)
{
    return FOLDABLE.fold_map([](int x) { return x; }, tree);
}

template <class TREE,
          const auto& FOLDABLE = smd::foldable_typeclass<TREE> >
auto fold_left_with_nttp_lookup(const TREE& tree)
{
    return FOLDABLE.fold_left(tree, 0, [](int acc, int x) {
        return acc * 10 + x;
    });
}

template <class TREE,
          const auto& FOLDABLE = smd::foldable_typeclass<TREE> >
auto fold_right_with_nttp_lookup(const TREE& tree)
{
    return FOLDABLE.fold_right(tree, 0, [](int x, int acc) {
        return x * 10 + acc;
    });
}

}  // namespace

TEST_CASE("FixTreeFoldableTest - Length")
{
    using Tree = smd::tree::FixTree<int>;
    auto tree = Tree::branch(Tree::leaf(1), Tree::branch(Tree::leaf(2), Tree::leaf(3)));

    const auto& foldable = smd::foldable_typeclass<Tree>;
    CHECK(foldable.length(tree) == 3U);
}

TEST_CASE("FixTreeFoldableTest - FoldMapSum")
{
    using Tree = smd::tree::FixTree<int>;
    auto tree = Tree::branch(Tree::leaf(1), Tree::branch(Tree::leaf(2), Tree::leaf(3)));

    const auto& foldable = smd::foldable_typeclass<Tree>;
    const auto sum = foldable.fold_map([](int x) { return x; }, tree);
    CHECK(sum == 6);
}

TEST_CASE("FixTreeFoldableTest - FoldMapWithExplicitObject")
{
    using Tree = smd::tree::FixTree<int>;
    auto tree = Tree::branch(Tree::leaf(1), Tree::branch(Tree::leaf(2), Tree::leaf(3)));

    const auto& foldable = smd::foldable_typeclass<Tree>;
    const auto sum = foldable.fold_map([](int x) { return x; }, tree);
    CHECK(sum == 6);
}

TEST_CASE("FixTreeFoldableTest - FoldMapWithNttpLookup")
{
    using Tree = smd::tree::FixTree<int>;
    auto tree = Tree::branch(Tree::leaf(1), Tree::branch(Tree::leaf(2), Tree::leaf(3)));

    CHECK(sum_with_nttp_lookup(tree) == 6);
}

TEST_CASE("FixTreeFoldableTest - FoldLeftAndRight")
{
    using Tree = smd::tree::FixTree<int>;
    auto tree = Tree::branch(Tree::leaf(1), Tree::branch(Tree::leaf(2), Tree::leaf(3)));
    const auto& foldable = smd::foldable_typeclass<Tree>;

    const auto left = foldable.fold_left(tree, 0, [](int acc, int x) {
        return acc * 10 + x;
    });
    const auto right = foldable.fold_right(tree, 0, [](int x, int acc) {
        return x * 10 + acc;
    });

    CHECK(left == 123);
    CHECK(right == 60);
}

TEST_CASE("FixTreeFoldableTest - FoldLeftRightWithExplicitObject")
{
    using Tree = smd::tree::FixTree<int>;
    auto tree = Tree::branch(Tree::leaf(1), Tree::branch(Tree::leaf(2), Tree::leaf(3)));

    const auto& foldable = smd::foldable_typeclass<Tree>;
    const auto left = foldable.fold_left(tree, 0, [](int acc, int x) {
        return acc * 10 + x;
    });
    const auto right = foldable.fold_right(tree, 0, [](int x, int acc) {
        return x * 10 + acc;
    });

    CHECK(left == 123);
    CHECK(right == 60);
}

TEST_CASE("FixTreeFoldableTest - FoldLeftRightWithNttpLookup")
{
    using Tree = smd::tree::FixTree<int>;
    auto tree = Tree::branch(Tree::leaf(1), Tree::branch(Tree::leaf(2), Tree::leaf(3)));

    CHECK(fold_left_with_nttp_lookup(tree) == 123);
    CHECK(fold_right_with_nttp_lookup(tree) == 60);
}

TEST_CASE("FixTreeFoldableTest - PredicatesAndFind")
{
    using Tree = smd::tree::FixTree<int>;
    auto tree = Tree::branch(Tree::leaf(1), Tree::branch(Tree::leaf(2), Tree::leaf(3)));
    const auto& foldable = smd::foldable_typeclass<Tree>;

    CHECK(foldable.any_of(tree, [](int x) { return x == 2; }));
    CHECK(foldable.all_of(tree, [](int x) { return x > 0; }));
    CHECK_FALSE(foldable.empty(tree));

    auto found = foldable.find_first(tree, [](int x) { return x > 1; });
    REQUIRE(found.has_value());
    CHECK(*found == 2);
}

TEST_CASE("FixTreeTest - CoreConstructionAndAccess")
{
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

## smd/tree/fix_tree.hpp

```cpp
// src/smd/tree/fix_tree.hpp                                          -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_FIX_TREE
#define INCLUDED_SMD_TREE_FIX_TREE
#include <memory>
#include <variant>
namespace smd::tree {
template<class T>
class FixTree{
 struct Leaf{T v;};
 struct Node{std::shared_ptr<FixTree> l,r;};
 std::variant<Leaf,Node> d;
public:
 using value_type = T;

 static FixTree leaf(T v){return FixTree(Leaf{v});}
 static FixTree node(FixTree a,FixTree b){
  return FixTree(Node{std::make_shared<FixTree>(a),
                      std::make_shared<FixTree>(b)});
 }
 static FixTree branch(FixTree a,FixTree b){
  return node(std::move(a), std::move(b));
 }
 bool is_leaf()const{return std::holds_alternative<Leaf>(d);}
 const T& value()const{return std::get<Leaf>(d).v;}
 const FixTree& left()const{return *std::get<Node>(d).l;}
 const FixTree& right()const{return *std::get<Node>(d).r;}
private:
 FixTree(Leaf l):d(l){}
 FixTree(Node n):d(n){}
};
}
#endif

```

## smd/tree/fix_tree_traversable.hpp

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
  auto traverse(this auto&& self,
                const APPLICATIVE& applicative,
                F&& f,
                const smd::tree::FixTree<T>& t)
  {
    if (t.is_leaf()) {
      return applicative.invoke(
        [](auto&& value) {
          using U = std::remove_cvref_t<decltype(value)>;
          return smd::tree::FixTree<U>::leaf(
            std::forward<decltype(value)>(value));
        },
        std::invoke(std::forward<F>(f), t.value()));
    }

    auto left = self.traverse(applicative, f, t.left());
    auto right = self.traverse(applicative, f, t.right());

    return applicative.invoke(
      [](auto&& l, auto&& r) {
        using U = std::remove_cvref_t<decltype(l.value())>;
        return smd::tree::FixTree<U>::node(std::forward<decltype(l)>(l),
                                           std::forward<decltype(r)>(r));
      },
      left,
      right);
  }
};

template <class T>
struct FixTreeTraversableMap : Traversable<FixTreeTraversableImpl<T> > {
  using FixTreeTraversableImpl<T>::traverse;
};

template <class T>
inline constexpr auto traversable_typeclass<smd::tree::FixTree<T> > =
    FixTreeTraversableMap<T>{};

}  // close namespace smd

#endif

```

## smd/tree/fix_tree_traversable.t.cpp

```cpp
#include <smd/tree/fix_tree.hpp>
#include <smd/tree/fix_tree_traversable.hpp>

#include <catch2/catch_test_macros.hpp>

#include <beman/optional/optional.hpp>

#include <optional>

namespace {

struct NonNegativePlusOne {
    auto operator()(int x) const -> std::optional<int>
    {
        return x >= 0 ? std::optional<int>{x + 1} : std::optional<int>{};
    }
};

struct TimesTwo {
    auto operator()(int x) const -> std::optional<int>
    {
        return std::optional<int>{x * 2};
    }
};

struct MinusTwo {
    auto operator()(int x) const -> std::optional<int>
    {
        return std::optional<int>{x - 2};
    }
};

struct TimesFiveBeman {
    auto operator()(int x) const -> beman::optional::optional<int>
    {
        return beman::optional::optional<int>{x * 5};
    }
};

}  // namespace

TEST_CASE("FixTreeTraversableTest - TraverseOptionalSuccess")
{
    using Tree = smd::tree::FixTree<int>;
    auto tree = Tree::branch(Tree::leaf(1), Tree::leaf(2));
    const auto& traversable = smd::traversable_typeclass<Tree>;

    auto traversed = smd::traverse(NonNegativePlusOne{}, tree);

    REQUIRE(traversed.has_value());
    CHECK(traversed->left().value() == 2);
    CHECK(traversed->right().value() == 3);
}

TEST_CASE("FixTreeTraversableTest - TraverseOptionalFailure")
{
    using Tree = smd::tree::FixTree<int>;
    auto tree = Tree::branch(Tree::leaf(1), Tree::leaf(-2));
    const auto& traversable = smd::traversable_typeclass<Tree>;

    auto traversed = smd::traverse(NonNegativePlusOne{}, tree);

    CHECK_FALSE(traversed.has_value());
}

TEST_CASE("FixTreeTraversableTest - ForEachOptionalSuccess")
{
    using Tree = smd::tree::FixTree<int>;
    auto tree = Tree::branch(Tree::leaf(3), Tree::leaf(4));
    const auto& traversable = smd::traversable_typeclass<Tree>;

    auto traversed = traversable.for_each(tree, TimesTwo{});

    REQUIRE(traversed.has_value());
    CHECK(traversed->left().value() == 6);
    CHECK(traversed->right().value() == 8);
}

TEST_CASE("FixTreeTraversableTest - TraverseLeaf")
{
    using Tree = smd::tree::FixTree<int>;
    auto tree = Tree::leaf(9);
    const auto& traversable = smd::traversable_typeclass<Tree>;

    auto traversed = smd::traverse(MinusTwo{}, tree);

    REQUIRE(traversed.has_value());
    REQUIRE(traversed->is_leaf());
    CHECK(traversed->value() == 7);
}

TEST_CASE("FixTreeTraversableTest - TraverseBemanOptional")
{
    using Tree = smd::tree::FixTree<int>;
    auto tree = Tree::branch(Tree::leaf(2), Tree::leaf(3));
    const auto& traversable = smd::traversable_typeclass<Tree>;

    auto traversed = smd::traverse(TimesFiveBeman{}, tree);

    REQUIRE(traversed.has_value());
    CHECK(traversed->left().value() == 10);
    CHECK(traversed->right().value() == 15);
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

template <class T>
struct FringeTreeApplicativeImpl {
  template <class VALUE>
  auto pure(this auto&&, VALUE&& value)
  {
    using U = remove_cvref_t<VALUE>;
    return smd::tree::FringeTree<U>::leaf(std::forward<VALUE>(value));
  }

  template <class F, class A>
  auto apply(this auto&& self,
             const smd::tree::FringeTree<F>& functions,
             const smd::tree::FringeTree<A>& arguments)
    -> smd::tree::FringeTree<std::invoke_result_t<const F&, const A&>>
  {
    using R = std::invoke_result_t<const F&, const A&>;

    if (functions.is_empty() || arguments.is_empty()) {
      return smd::tree::FringeTree<R>::empty();
    }

    if (functions.is_leaf()) {
      auto function = functions.value();
      if (arguments.is_leaf()) {
        return smd::tree::FringeTree<R>::leaf(function(arguments.value()));
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

template <class T>
struct FringeTreeApplicativeMap : Applicative<FringeTreeApplicativeImpl<T> > {
  using FringeTreeApplicativeImpl<T>::apply;
  using FringeTreeApplicativeImpl<T>::pure;
};

template <class T>
inline constexpr auto applicative_typeclass<smd::tree::FringeTree<T> > =
  FringeTreeApplicativeMap<T>{};

}  // close namespace smd

#endif

```

## smd/tree/fringe_tree_applicative.t.cpp

```cpp
#include <smd/tree/fringe_tree.hpp>
#include <smd/tree/fringe_tree_applicative.hpp>

#include <catch2/catch_test_macros.hpp>

#include <vector>

TEST_CASE("FringeTreeApplicativeTest - Invoke")
{
    using Tree = smd::tree::FringeTree<int>;
    auto lhs = Tree::branch(Tree::leaf(1), Tree::leaf(2));
    auto rhs = Tree::branch(Tree::leaf(10), Tree::leaf(20));

    const auto& applicative = smd::applicative_typeclass<Tree>;
    auto summed = applicative.invoke([](int a, int b) { return a + b; }, lhs, rhs);

    CHECK(summed.flatten() == (std::vector<int>{11, 22}));
}

TEST_CASE("FringeTreeApplicativeTest - ApplyEmptyArgumentsOrFunctions")
{
    using Tree = smd::tree::FringeTree<int>;
    const auto& applicative = smd::applicative_typeclass<Tree>;

    auto fs = smd::tree::FringeTree<int(*)(int)>::leaf(+[](int x) { return x + 1; });

    auto empty_args = applicative.apply(fs, Tree::empty());
    CHECK(empty_args.is_empty());

    auto empty_functions = applicative.apply(smd::tree::FringeTree<int(*)(int)>::empty(), Tree::leaf(1));
    CHECK(empty_functions.is_empty());
}

TEST_CASE("FringeTreeApplicativeTest - ApplyDistributesAcrossShapes")
{
    using Tree = smd::tree::FringeTree<int>;
    const auto& applicative = smd::applicative_typeclass<Tree>;

    auto fs_leaf = smd::tree::FringeTree<int(*)(int)>::leaf(+[](int x) { return x * 10; });
    auto args_tree = Tree::branch(Tree::leaf(1), Tree::leaf(2));
    auto distributed = applicative.apply(fs_leaf, args_tree);
    CHECK(distributed.flatten() == (std::vector<int>{10, 20}));

    auto fs_tree = smd::tree::FringeTree<int(*)(int)>::branch(
        smd::tree::FringeTree<int(*)(int)>::leaf(+[](int x) { return x + 2; }),
        smd::tree::FringeTree<int(*)(int)>::leaf(+[](int x) { return x + 3; }));
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

template <class T>
struct FringeTreeFoldableImpl {
  template <class F>
  auto fold_map(this auto&& self,
                F&& function,
                const smd::tree::FringeTree<T>& tree)
    -> remove_cvref_t<std::invoke_result_t<F, const T&>>
  {
    using Result = remove_cvref_t<std::invoke_result_t<F, const T&>>;

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

template <class T>
struct FringeTreeFoldableMap : Foldable<FringeTreeFoldableImpl<T> > {
  using FringeTreeFoldableImpl<T>::fold_map;
};

template <class T>
inline constexpr auto foldable_typeclass<smd::tree::FringeTree<T> > =
  FringeTreeFoldableMap<T>{};

}  // close namespace smd

#endif

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

  struct View {
    T d_value;
    FringeTree d_rest;
  };

  static auto empty() -> FringeTree { return FringeTree(Empty{}); }

  static auto leaf(T value) -> FringeTree { return FringeTree(Leaf{std::move(value)}); }

  static auto branch(FringeTree left, FringeTree right) -> FringeTree
  {
    auto left_ptr = std::make_shared<FringeTree>(std::move(left));
    auto right_ptr = std::make_shared<FringeTree>(std::move(right));
    auto measure = left_ptr->measure() + right_ptr->measure();
    return FringeTree(Branch{measure, std::move(left_ptr), std::move(right_ptr)});
  }

  auto is_empty() const -> bool { return std::holds_alternative<Empty>(d_data); }
  auto is_leaf() const -> bool { return std::holds_alternative<Leaf>(d_data); }
  auto is_branch() const -> bool { return std::holds_alternative<Branch>(d_data); }

  auto measure() const -> std::size_t
  {
    if (is_empty()) {
      return 0U;
    }
    if (is_leaf()) {
      return 1U;
    }
    return std::get<Branch>(d_data).d_measure;
  }

  auto value() const -> const T&
  {
    assert(is_leaf());
    return std::get<Leaf>(d_data).d_value;
  }

  auto left() const -> const FringeTree&
  {
    assert(is_branch());
    return *std::get<Branch>(d_data).d_left;
  }

  auto right() const -> const FringeTree&
  {
    assert(is_branch());
    return *std::get<Branch>(d_data).d_right;
  }

  auto breadth() const -> std::size_t { return measure(); }

  auto depth() const -> std::size_t
  {
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

  auto flatten() const -> std::vector<T>
  {
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

  static auto concat(const FringeTree& left_tree, const FringeTree& right_tree)
    -> FringeTree
  {
    if (left_tree.is_empty()) {
      return right_tree;
    }
    if (right_tree.is_empty()) {
      return left_tree;
    }
    return branch(left_tree, right_tree);
  }

  static auto prepend(T value, const FringeTree& tree) -> FringeTree
  {
    return concat(leaf(std::move(value)), tree);
  }

  static auto append(const FringeTree& tree, T value) -> FringeTree
  {
    return concat(tree, leaf(std::move(value)));
  }

  auto view_l() const -> std::optional<View>
  {
    if (is_empty()) {
      return std::nullopt;
    }
    if (is_leaf()) {
      return View{value(), empty()};
    }

    auto left_view = left().view_l();
    if (left_view.has_value()) {
      return View{left_view->d_value,
                  concat(left_view->d_rest, right())};
    }

    return right().view_l();
  }

  auto view_r() const -> std::optional<View>
  {
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

  auto head() const -> T
  {
    auto v = view_l();
    assert(v.has_value());
    return v->d_value;
  }

  auto tail() const -> FringeTree
  {
    auto v = view_l();
    return v.has_value() ? v->d_rest : empty();
  }

  auto last() const -> T
  {
    auto v = view_r();
    assert(v.has_value());
    return v->d_value;
  }

  auto init() const -> FringeTree
  {
    auto v = view_r();
    return v.has_value() ? v->d_rest : empty();
  }

 private:
  explicit FringeTree(Empty e)
      : d_data(std::move(e))
  {
  }

  explicit FringeTree(Leaf l)
      : d_data(std::move(l))
  {
  }

  explicit FringeTree(Branch b)
      : d_data(std::move(b))
  {
  }
};

}  // close namespace smd::tree

#endif

```

## smd/tree/fringe_tree.t.cpp

```cpp
#include <smd/tree/fringe_tree.hpp>
#include <smd/tree/fringe_tree_foldable.hpp>
#include <smd/typeclass/foldable.hpp>

#include <catch2/catch_test_macros.hpp>

#include <vector>

TEST_CASE("FringeTreeTest - EmptyLeafAndPredicates")
{
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

TEST_CASE("FringeTreeTest - BranchLeftRightAndMemberStyleOperations")
{
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

TEST_CASE("FringeTreeTest - SingletonViewsAndEmptyTailInit")
{
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

TEST_CASE("FringeTreeTest - BasicMeasureDepthFlatten")
{
    using Tree = smd::tree::FringeTree<int>;

    auto tree = Tree::branch(
        Tree::branch(Tree::leaf(1), Tree::leaf(2)),
        Tree::leaf(3));

    CHECK(tree.measure() == 3U);
    CHECK(tree.breadth() == 3U);
    CHECK(tree.depth() == 3U);
    CHECK(tree.flatten() == (std::vector<int>{1, 2, 3}));
}

TEST_CASE("FringeTreeTest - ViewsAndListOps")
{
    using Tree = smd::tree::FringeTree<int>;

    auto tree = Tree::branch(
        Tree::branch(Tree::leaf(1), Tree::leaf(2)),
        Tree::leaf(3));

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

TEST_CASE("FringeTreeTest - PrependAppendConcat")
{
    using Tree = smd::tree::FringeTree<int>;

    auto tree = Tree::branch(Tree::leaf(1), Tree::leaf(2));

    auto prepended = Tree::prepend(0, tree);
    CHECK(prepended.flatten() == (std::vector<int>{0, 1, 2}));

    auto appended = Tree::append(tree, 3);
    CHECK(appended.flatten() == (std::vector<int>{1, 2, 3}));

    auto concatenated = Tree::concat(tree, tree);
    CHECK(concatenated.flatten() == (std::vector<int>{1, 2, 1, 2}));
}

TEST_CASE("FringeTreeTest - FoldableIntegration")
{
    using Tree = smd::tree::FringeTree<int>;

    auto tree = Tree::branch(
        Tree::branch(Tree::leaf(1), Tree::leaf(2)),
        Tree::leaf(3));

    const auto& foldable = smd::foldable_typeclass<Tree>;
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

template <class T>
struct FringeTreeTraversableImpl {
  using element_type = T;

  template <class APPLICATIVE, class F>
  auto traverse(this auto&& self,
                const APPLICATIVE& applicative,
                F&& function,
                const smd::tree::FringeTree<T>& tree)
  {
    using Context = remove_cvref_t<std::invoke_result_t<F, const T&>>;
    using U = smd::applicative_value_t<Context>;

    if (tree.is_empty()) {
      return applicative.pure(smd::tree::FringeTree<U>::empty());
    }

    if (tree.is_leaf()) {
      return applicative.invoke(
        [](auto&& value) {
          using U = remove_cvref_t<decltype(value)>;
          return smd::tree::FringeTree<U>::leaf(std::forward<decltype(value)>(value));
        },
        std::invoke(std::forward<F>(function), tree.value()));
    }

    auto left = self.traverse(applicative, function, tree.left());
    auto right = self.traverse(applicative, function, tree.right());

    return applicative.invoke(
      [](auto&& l, auto&& r) {
        return smd::tree::FringeTree<remove_cvref_t<decltype(l.value())> >::branch(
          std::forward<decltype(l)>(l),
          std::forward<decltype(r)>(r));
      },
      left,
      right);
  }
};

template <class T>
struct FringeTreeTraversableMap : Traversable<FringeTreeTraversableImpl<T> > {
  using FringeTreeTraversableImpl<T>::traverse;
};

template <class T>
inline constexpr auto traversable_typeclass<smd::tree::FringeTree<T> > =
  FringeTreeTraversableMap<T>{};

}  // close namespace smd

#endif

```

## smd/tree/fringe_tree_traversable.t.cpp

```cpp
#include <smd/tree/fringe_tree.hpp>
#include <smd/tree/fringe_tree_traversable.hpp>

#include <catch2/catch_test_macros.hpp>

#include <beman/optional/optional.hpp>

#include <optional>
#include <vector>

namespace {

struct PositiveTimesTen {
    auto operator()(int x) const -> std::optional<int>
    {
        return x > 0 ? std::optional<int>{x * 10} : std::optional<int>{};
    }
};

struct TimesTen {
    auto operator()(int x) const -> std::optional<int>
    {
        return std::optional<int>{x * 10};
    }
};

struct NonNegativeIdentity {
    auto operator()(int x) const -> std::optional<int>
    {
        return x >= 0 ? std::optional<int>{x} : std::optional<int>{};
    }
};

struct PlusOne {
    auto operator()(int x) const -> std::optional<int>
    {
        return std::optional<int>{x + 1};
    }
};

struct TimesTenBeman {
    auto operator()(int x) const -> beman::optional::optional<int>
    {
        return beman::optional::optional<int>{x * 10};
    }
};

struct PlusSevenBeman {
    auto operator()(int x) const -> beman::optional::optional<int>
    {
        return beman::optional::optional<int>{x + 7};
    }
};

}  // namespace

TEST_CASE("FringeTreeTraversableTest - TraverseOptional")
{
    using Tree = smd::tree::FringeTree<int>;
    auto tree = Tree::branch(Tree::leaf(1), Tree::branch(Tree::leaf(2), Tree::leaf(3)));

    const auto& traversable = smd::traversable_typeclass<Tree>;
    auto traversed = smd::traverse(PositiveTimesTen{}, tree);

    REQUIRE(traversed.has_value());
    CHECK(traversed->flatten() == (std::vector<int>{10, 20, 30}));
}

TEST_CASE("FringeTreeTraversableTest - TraverseOptionalEmpty")
{
    using Tree = smd::tree::FringeTree<int>;
    auto tree = Tree::empty();

    const auto& traversable = smd::traversable_typeclass<Tree>;
    auto traversed = smd::traverse(TimesTen{}, tree);

    REQUIRE(traversed.has_value());
    CHECK(traversed->is_empty());
}

TEST_CASE("FringeTreeTraversableTest - TraverseBemanOptionalEmpty")
{
    using Tree = smd::tree::FringeTree<int>;
    auto tree = Tree::empty();

    const auto& traversable = smd::traversable_typeclass<Tree>;
    auto traversed = smd::traverse(TimesTenBeman{}, tree);

    REQUIRE(traversed.has_value());
    CHECK(traversed->is_empty());
}

TEST_CASE("FringeTreeTraversableTest - TraverseOptionalFailure")
{
    using Tree = smd::tree::FringeTree<int>;
    auto tree = Tree::branch(Tree::leaf(1), Tree::leaf(-2));

    const auto& traversable = smd::traversable_typeclass<Tree>;
    auto traversed = smd::traverse(NonNegativeIdentity{}, tree);

    CHECK_FALSE(traversed.has_value());
}

TEST_CASE("FringeTreeTraversableTest - TraverseLeaf")
{
    using Tree = smd::tree::FringeTree<int>;
    auto tree = Tree::leaf(7);

    const auto& traversable = smd::traversable_typeclass<Tree>;
    auto traversed = smd::traverse(PlusOne{}, tree);

    REQUIRE(traversed.has_value());
    REQUIRE(traversed->is_leaf());
    CHECK(traversed->value() == 8);
}

TEST_CASE("FringeTreeTraversableTest - TraverseBemanOptional")
{
    using Tree = smd::tree::FringeTree<int>;
    auto tree = Tree::branch(Tree::leaf(2), Tree::leaf(5));

    const auto& traversable = smd::traversable_typeclass<Tree>;
    auto traversed = smd::traverse(PlusSevenBeman{}, tree);

    REQUIRE(traversed.has_value());
    CHECK(traversed->flatten() == (std::vector<int>{9, 12}));
}

```

## smd/tree/memoized_thunk.hpp

```cpp
// src/smd/tree/memoized_thunk.hpp                                    -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_MEMOIZED_THUNK
#define INCLUDED_SMD_TREE_MEMOIZED_THUNK

#include <cassert>
#include <functional>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

namespace smd::tree::detail {

template <typename Result>
class erased_thunk {
  struct ThunkBase {
    virtual ~ThunkBase() = default;
    virtual auto invoke() -> const Result& = 0;
  };

  template <typename Callable>
  struct ThunkModel final : ThunkBase {
    Callable d_callable;

    explicit ThunkModel(Callable callable)
      : d_callable(std::move(callable))
    {
    }

    auto invoke() -> const Result& override
    {
      return d_callable();
    }
  };

  std::shared_ptr<ThunkBase> d_impl;

 public:
  erased_thunk() = default;

  template <typename Callable,
            typename CallableT = std::remove_cvref_t<Callable>,
            std::enable_if_t<!std::is_same_v<CallableT, erased_thunk>, int> = 0>
  erased_thunk(Callable&& callable)
    : d_impl(std::make_shared<ThunkModel<CallableT>>(std::forward<Callable>(callable)))
  {
  }

  [[nodiscard]] auto operator()() const -> const Result&
  {
    assert(d_impl != nullptr);
    return d_impl->invoke();
  }
};

template <typename Callable, typename... Args>
auto delay(Callable&& c, Args&&... args)
{
  using CallableT = std::remove_cvref_t<Callable>;
  using ArgsTuple = std::tuple<std::remove_cvref_t<Args>...>;

  return [callable = CallableT(std::forward<Callable>(c)),
          arguments = ArgsTuple(std::forward<Args>(args)...)]() mutable {
    return std::apply(
      [&](auto&... unpacked) {
        return std::invoke(callable, unpacked...);
      },
      arguments);
  };
}

template <typename Callable, typename... Args>
auto thunk(Callable&& c, Args&&... args)
{
  using Closure = decltype(delay(std::forward<Callable>(c), std::forward<Args>(args)...));
  using Result = std::invoke_result_t<Closure&>;
  using State = std::variant<std::monostate, Closure, Result>;

  auto state = std::make_shared<State>(
    std::in_place_index<1>,
    delay(std::forward<Callable>(c), std::forward<Args>(args)...));

  return [state = std::move(state)]() mutable -> const Result& {
    if (state->index() == 1U) {
      state->template emplace<2>(std::get<1>(*state)());
    }
    return std::get<2>(*state);
  };
}

template <typename Measure, typename Callable, typename... Args>
auto measured_thunk(Measure measure, Callable&& c, Args&&... args)
{
  auto delayed = thunk(std::forward<Callable>(c), std::forward<Args>(args)...);

  return [measure = std::move(measure), delayed = std::move(delayed)]() mutable {
    struct MeasuredThunkAccess {
      Measure d_measure;
      mutable decltype(delayed) d_force;

      [[nodiscard]] auto cached_measure() const -> const Measure&
      {
        return d_measure;
      }

      [[nodiscard]] auto force() const -> decltype(auto)
      {
        return d_force();
      }
    };

    return MeasuredThunkAccess{std::move(measure), std::move(delayed)};
  }();
}

}  // namespace smd::tree::detail

#endif

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
  auto operator()(NEXT_ARG&& next_arg)
  {
    return invoke_or_extend(std::forward<NEXT_ARG>(next_arg),
                            std::index_sequence_for<BOUND_ARGS...>{});
  }

  template <class NEXT_ARG>
  auto operator()(NEXT_ARG&& next_arg) const
  {
    return invoke_or_extend_const(std::forward<NEXT_ARG>(next_arg),
                                  std::index_sequence_for<BOUND_ARGS...>{});
  }

 private:
  template <class NEXT_ARG, std::size_t... IDX>
  auto invoke_or_extend(NEXT_ARG&& next_arg, std::index_sequence<IDX...>)
  {
    if constexpr (std::invocable<FUNCTION&, BOUND_ARGS&..., NEXT_ARG>) {
      return std::invoke(function,
                         std::get<IDX>(bound_args)...,
                         std::forward<NEXT_ARG>(next_arg));
    } else {
      using NEXT_PARTIAL =
        terminating_partial<FUNCTION, BOUND_ARGS..., remove_cvref_t<NEXT_ARG> >;
      return NEXT_PARTIAL{
        function,
        std::tuple_cat(std::move(bound_args),
                       std::tuple<remove_cvref_t<NEXT_ARG> >{
                         std::forward<NEXT_ARG>(next_arg)})};
    }
  }

  template <class NEXT_ARG, std::size_t... IDX>
  auto invoke_or_extend_const(NEXT_ARG&& next_arg,
                              std::index_sequence<IDX...>) const
  {
    if constexpr (std::invocable<const FUNCTION&, const BOUND_ARGS&..., NEXT_ARG>) {
      return std::invoke(function,
                         std::get<IDX>(bound_args)...,
                         std::forward<NEXT_ARG>(next_arg));
    } else {
      using NEXT_PARTIAL =
        terminating_partial<FUNCTION, BOUND_ARGS..., remove_cvref_t<NEXT_ARG> >;
      return NEXT_PARTIAL{
        function,
        std::tuple_cat(bound_args,
                       std::tuple<remove_cvref_t<NEXT_ARG> >{
                         std::forward<NEXT_ARG>(next_arg)})};
    }
  }
};

template <class FUNCTION>
auto make_terminating_partial(FUNCTION&& function)
{
  using STORED_FUNCTION = remove_cvref_t<FUNCTION>;
  return terminating_partial<STORED_FUNCTION>{
    std::forward<FUNCTION>(function),
    std::tuple<>{}};
}

}  // namespace detail

// Applicative pattern invariants:
// - Minimal required operations are pure and apply.
// - invoke is derived from pure/apply via terminating partial application,
//   but an Impl may provide a custom invoke for different semantics or efficiency.
// - Derived operations (lift/ap/zip_with/discard_*) live on that object.
// - Dispatch happens through a provided object or applicative_typeclass<Concrete>.
// - Do not introduce hidden alternate semantics without a distinct map/type.

template <class Impl>
struct Applicative : protected Impl {
  using Impl::apply;
  using Impl::pure;

  // a11f7d8b-8f89-4f3e-9c92-f9f08ab7ef11
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
  // a11f7d8b-8f89-4f3e-9c92-f9f08ab7ef11 end

 private:
  template <class ACCUMULATED>
  auto apply_chain(this auto&&, ACCUMULATED&& accumulated)
  {
    return std::forward<ACCUMULATED>(accumulated);
  }

  template <class ACCUMULATED, class NEXT_ARGUMENT, class... REST_ARGUMENTS>
  auto apply_chain(this auto&& self,
                   ACCUMULATED&& accumulated,
                   NEXT_ARGUMENT&& next_argument,
                   REST_ARGUMENTS&&... rest_arguments)
  {
    auto next = self.ap(std::forward<ACCUMULATED>(accumulated),
                        std::forward<NEXT_ARGUMENT>(next_argument));
    if constexpr (sizeof...(REST_ARGUMENTS) == 0) {
      return next;
    } else {
      return self.apply_chain(std::move(next),
                              std::forward<REST_ARGUMENTS>(rest_arguments)...);
    }
  }

 public:
  template <class FUNCTION, class ARGUMENT>
  auto map(this auto&& self, FUNCTION&& function, ARGUMENT&& argument)
  {
    return self.invoke(std::forward<FUNCTION>(function),
                       std::forward<ARGUMENT>(argument));
  }

  template <class VALUE>
  auto lift(this auto&& self, VALUE&& value)
  {
    return self.pure(std::forward<VALUE>(value));
  }

  template <class FUNCTION_IN_CONTEXT, class ARGUMENT_IN_CONTEXT>
  auto ap(this auto&& self,
          FUNCTION_IN_CONTEXT&& function,
          ARGUMENT_IN_CONTEXT&& argument)
  {
    return self.apply(std::forward<FUNCTION_IN_CONTEXT>(function),
                      std::forward<ARGUMENT_IN_CONTEXT>(argument));
  }

  template <class FUNCTION, class FIRST_ARGUMENT, class SECOND_ARGUMENT>
  auto zip_with(this auto&& self,
                FUNCTION&& function,
                FIRST_ARGUMENT&& first_argument,
                SECOND_ARGUMENT&& second_argument)
  {
    return self.invoke(std::forward<FUNCTION>(function),
                       std::forward<FIRST_ARGUMENT>(first_argument),
                       std::forward<SECOND_ARGUMENT>(second_argument));
  }

  template <class FIRST_ARGUMENT, class SECOND_ARGUMENT>
  auto discard_first(this auto&& self,
                     FIRST_ARGUMENT&& first_argument,
                     SECOND_ARGUMENT&& second_argument)
  {
    return self.invoke(
      [](const auto&, auto&& rhs) {
        return std::forward<decltype(rhs)>(rhs);
      },
      std::forward<FIRST_ARGUMENT>(first_argument),
      std::forward<SECOND_ARGUMENT>(second_argument));
  }

  template <class FIRST_ARGUMENT, class SECOND_ARGUMENT>
  auto discard_second(this auto&& self,
                      FIRST_ARGUMENT&& first_argument,
                      SECOND_ARGUMENT&& second_argument)
  {
    return self.invoke(
      [](auto&& lhs, const auto&) {
        return std::forward<decltype(lhs)>(lhs);
      },
      std::forward<FIRST_ARGUMENT>(first_argument),
      std::forward<SECOND_ARGUMENT>(second_argument));
  }

  template <class APPLICATIVE_MAP,
            class FUNCTION,
            class FIRST_ARGUMENT,
            class... REST_ARGUMENTS>
  auto invoke_with(this auto&&,
                   const APPLICATIVE_MAP& applicative_map,
                   FUNCTION&& function,
                   FIRST_ARGUMENT&& first_argument,
                   REST_ARGUMENTS&&... rest_arguments)
  {
    return applicative_map.invoke(std::forward<FUNCTION>(function),
                                  std::forward<FIRST_ARGUMENT>(first_argument),
                                  std::forward<REST_ARGUMENTS>(rest_arguments)...);
  }

  template <class APPLICATIVE_MAP,
            class FUNCTION,
            class FIRST_ARGUMENT,
            class... REST_ARGUMENTS>
  auto apply_pure_with(this auto&&,
                       const APPLICATIVE_MAP& applicative_map,
                       FUNCTION&& function,
                       FIRST_ARGUMENT&& first_argument,
                       REST_ARGUMENTS&&... rest_arguments)
  {
    return applicative_map.invoke(std::forward<FUNCTION>(function),
                                  std::forward<FIRST_ARGUMENT>(first_argument),
                                  std::forward<REST_ARGUMENTS>(rest_arguments)...);
  }

  template <const auto& APPLICATIVE_MAP,
            class FUNCTION,
            class FIRST_ARGUMENT,
            class... REST_ARGUMENTS>
  auto invoke_with(this auto&&,
                   FUNCTION&& function,
                   FIRST_ARGUMENT&& first_argument,
                   REST_ARGUMENTS&&... rest_arguments)
  {
    return APPLICATIVE_MAP.invoke(std::forward<FUNCTION>(function),
                                  std::forward<FIRST_ARGUMENT>(first_argument),
                                  std::forward<REST_ARGUMENTS>(rest_arguments)...);
  }

  template <const auto& APPLICATIVE_MAP,
            class FUNCTION,
            class FIRST_ARGUMENT,
            class... REST_ARGUMENTS>
  auto apply_pure_with(this auto&&,
                       FUNCTION&& function,
                       FIRST_ARGUMENT&& first_argument,
                       REST_ARGUMENTS&&... rest_arguments)
  {
    return APPLICATIVE_MAP.invoke(std::forward<FUNCTION>(function),
                                  std::forward<FIRST_ARGUMENT>(first_argument),
                                  std::forward<REST_ARGUMENTS>(rest_arguments)...);
  }
};

template <class T>
inline constexpr auto applicative_typeclass = std::false_type{};

template <class VALUE_TYPE>
struct OptionalApplicativeImpl {
  template <class VALUE>
  auto pure(this auto&&, VALUE&& value) -> std::optional<remove_cvref_t<VALUE> >
  {
    return std::optional<remove_cvref_t<VALUE> >{std::forward<VALUE>(value)};
  }

  template <class FUNCTION_IN_CONTEXT, class ARGUMENT_IN_CONTEXT>
  auto apply(this auto&&,
             FUNCTION_IN_CONTEXT&& function,
             ARGUMENT_IN_CONTEXT&& argument)
  {
    using Result =
      std::invoke_result_t<decltype(*function), decltype(*argument)>;

    if (!function || !argument) {
      return std::optional<remove_cvref_t<Result> >{};
    }

    return std::optional<remove_cvref_t<Result> >{
      std::invoke(*std::forward<FUNCTION_IN_CONTEXT>(function),
            *std::forward<ARGUMENT_IN_CONTEXT>(argument))};
  }
};

template <class VALUE_TYPE>
struct OptionalApplicativeMap : Applicative<OptionalApplicativeImpl<VALUE_TYPE> > {
  using OptionalApplicativeImpl<VALUE_TYPE>::apply;
  using OptionalApplicativeImpl<VALUE_TYPE>::pure;
};

template <class VALUE_TYPE>
  requires(!std::same_as<beman::optional::optional<VALUE_TYPE>,
                         std::optional<VALUE_TYPE> >)
struct BemanOptionalApplicativeImpl {
  template <class VALUE>
  auto pure(this auto&&, VALUE&& value)
    -> beman::optional::optional<remove_cvref_t<VALUE> >
  {
    return beman::optional::optional<remove_cvref_t<VALUE> >{
      std::forward<VALUE>(value)};
  }

  template <class FUNCTION_IN_CONTEXT, class ARGUMENT_IN_CONTEXT>
  auto apply(this auto&&,
             FUNCTION_IN_CONTEXT&& function,
             ARGUMENT_IN_CONTEXT&& argument)
  {
    using Result =
      std::invoke_result_t<decltype(*function), decltype(*argument)>;

    if (!function || !argument) {
      return beman::optional::optional<remove_cvref_t<Result> >{};
    }

    return beman::optional::optional<remove_cvref_t<Result> >{
      std::invoke(*std::forward<FUNCTION_IN_CONTEXT>(function),
            *std::forward<ARGUMENT_IN_CONTEXT>(argument))};
  }
};

template <class VALUE_TYPE>
  requires(!std::same_as<beman::optional::optional<VALUE_TYPE>,
                         std::optional<VALUE_TYPE> >)
struct BemanOptionalApplicativeMap
    : Applicative<BemanOptionalApplicativeImpl<VALUE_TYPE> > {
  using BemanOptionalApplicativeImpl<VALUE_TYPE>::apply;
  using BemanOptionalApplicativeImpl<VALUE_TYPE>::pure;
};

template <class VALUE_TYPE>
inline constexpr auto applicative_typeclass<std::optional<VALUE_TYPE> > =
    OptionalApplicativeMap<VALUE_TYPE>{};

template <class VALUE_TYPE>
  requires(!std::same_as<beman::optional::optional<VALUE_TYPE>,
                         std::optional<VALUE_TYPE> >)
inline constexpr auto applicative_typeclass<beman::optional::optional<VALUE_TYPE> > =
    BemanOptionalApplicativeMap<VALUE_TYPE>{};

}  // close namespace smd

#endif

```

## smd/typeclass/applicative.t.cpp

```cpp
#include <smd/typeclass/applicative.hpp>
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
    auto pure(this auto&&, VALUE&& value)
    {
        return smd::typeclass::test::Identity<smd::remove_cvref_t<VALUE> >{
            std::forward<VALUE>(value)};
    }

    template <class FUNCTION_IN_CONTEXT, class ARGUMENT_IN_CONTEXT>
    auto apply(this auto&&,
               const FUNCTION_IN_CONTEXT& function,
               const ARGUMENT_IN_CONTEXT& argument)
    {
        using Result = std::invoke_result_t<
            const typename smd::remove_cvref_t<FUNCTION_IN_CONTEXT>::value_type&,
            const typename smd::remove_cvref_t<ARGUMENT_IN_CONTEXT>::value_type&>;

        return smd::typeclass::test::Identity<smd::remove_cvref_t<Result> >{
            std::invoke(function.value, argument.value)};
    }

    template <class FUNCTION, class FIRST_ARGUMENT, class... REST_ARGUMENTS>
    auto invoke(this auto&& self,
                FUNCTION&& function,
                const FIRST_ARGUMENT& first_argument,
                const REST_ARGUMENTS&... rest_arguments)
    {
        return self.pure(std::invoke(std::forward<FUNCTION>(function),
                                     first_argument.value,
                                     rest_arguments.value...));
    }
};

template <class VALUE_TYPE>
struct DirectInvokeIdentityApplicativeMap
    : smd::Applicative<DirectInvokeIdentityApplicativeImpl<VALUE_TYPE> > {
    using DirectInvokeIdentityApplicativeImpl<VALUE_TYPE>::apply;
    using DirectInvokeIdentityApplicativeImpl<VALUE_TYPE>::invoke;
    using DirectInvokeIdentityApplicativeImpl<VALUE_TYPE>::pure;
};

inline constexpr DirectInvokeIdentityApplicativeMap<int> direct_invoke_map{};

template <class A, class B, class C>
void run_bare_identity_matrix_case(A a, B b, C c)
{
    using BareA = smd::typeclass::test::BareIdentity<A>;
    const auto& applicative = smd::applicative_typeclass<BareA>;

    auto summed = applicative.invoke(
        [](const A& x, const B& y, const C& z) {
            return static_cast<long double>(x) + static_cast<long double>(y)
                + static_cast<long double>(z);
        },
        BareA{a},
        smd::typeclass::test::BareIdentity<B>{b},
        smd::typeclass::test::BareIdentity<C>{c});
    auto expected = static_cast<long double>(a) + static_cast<long double>(b)
        + static_cast<long double>(c);
    CHECK(std::abs(summed.value - expected) < 1e-9L);

    auto mapped = applicative.map(
        [](const A& x) { return std::to_string(static_cast<long double>(x)); },
        BareA{a});
    CHECK_FALSE(mapped.value.empty());

    auto applied = applicative.ap(
        smd::typeclass::test::BareIdentity<std::string (*)(A)>{
            +[](A x) { return std::to_string(static_cast<long double>(x + x)); }},
        BareA{a});
    CHECK_FALSE(applied.value.empty());
}

}  // namespace

TEST_CASE("ApplicativeTypeclassTest - PureOptional")
{
    const auto& applicative = smd::applicative_typeclass<std::optional<int> >;
    auto lifted = applicative.pure(7);
    REQUIRE(lifted.has_value());
    CHECK(*lifted == 7);
}

TEST_CASE("ApplicativeTypeclassTest - ApplyOptional")
{
    std::optional<int (*)(int)> function{+[](int x) { return x + 3; }};
    std::optional<int> argument{4};
    const auto& applicative = smd::applicative_typeclass<std::optional<int (*)(int)> >;

    auto result = applicative.apply(function, argument);
    REQUIRE(result.has_value());
    CHECK(*result == 7);
}

TEST_CASE("ApplicativeTypeclassTest - InvokeOptional")
{
    // f6c2b5e1-9a3d-4f8c-b2e6-1d9c5b3f7a02
    std::optional<int> ax{10};
    std::optional<int> ay{5};
    const auto& applicative = smd::applicative_typeclass<std::optional<int> >;

    auto result = applicative.invoke([](int a, int b) { return a - b; }, ax, ay);
    REQUIRE(result.has_value());
    CHECK(*result == 5);
    // f6c2b5e1-9a3d-4f8c-b2e6-1d9c5b3f7a02 end
}

TEST_CASE("ApplicativeTypeclassTest - InvokeOptionalTernaryUsesPartialApplication")
{
    std::optional<int> ax{2};
    std::optional<int> ay{3};
    std::optional<int> az{4};
    const auto& applicative = smd::applicative_typeclass<std::optional<int> >;

    auto result = applicative.invoke(
        [](int a, int b, int c) { return a * b + c; },
        ax,
        ay,
        az);
    REQUIRE(result.has_value());
    CHECK(*result == 10);
}

TEST_CASE("ApplicativeTypeclassTest - ApplyPureOptionalTernary")
{
    // 6e8bde7b-a9f1-4c98-8f1a-807d9ee0a93b
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
    // 6e8bde7b-a9f1-4c98-8f1a-807d9ee0a93b end
}

TEST_CASE("ApplicativeTypeclassTest - MapOptional")
{
    std::optional<int> value{21};
    const auto& applicative = smd::applicative_typeclass<std::optional<int> >;

    auto result = applicative.map([](int x) { return x * 2; }, value);
    REQUIRE(result.has_value());
    CHECK(*result == 42);
}

TEST_CASE("ApplicativeTypeclassTest - InvokeWithExplicitMap")
{
    std::optional<int> ax{10};
    std::optional<int> ay{5};
    const auto& default_applicative = smd::applicative_typeclass<std::optional<int> >;
    const auto& optional_applicative = smd::applicative_typeclass<std::optional<int> >;

    auto result = default_applicative.invoke_with(
        optional_applicative,
        [](int a, int b) { return a + b; },
        ax,
        ay);
    REQUIRE(result.has_value());
    CHECK(*result == 15);
}

TEST_CASE("ApplicativeTypeclassTest - OptionalEmptyPaths")
{
    const auto& applicative = smd::applicative_typeclass<std::optional<int> >;

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
    auto invoke_result = applicative.invoke([](int a, int b) { return a + b; }, ax, ay);
    CHECK_FALSE(invoke_result.has_value());
    // b4a8c2f7-6d3e-4c1b-9f5a-7e2d4b8a6c09 end
}

TEST_CASE("ApplicativeTypeclassTest - DerivedOperations")
{
    const auto& applicative = smd::applicative_typeclass<std::optional<int> >;

    auto lifted = applicative.lift(9);
    REQUIRE(lifted.has_value());
    CHECK(*lifted == 9);

    std::optional<int (*)(int)> function{+[](int x) { return x * 3; }};
    auto applied = applicative.ap(function, std::optional<int>{7});
    REQUIRE(applied.has_value());
    CHECK(*applied == 21);

    auto zipped = applicative.zip_with(
        [](int a, int b) { return a * b; },
        std::optional<int>{6},
        std::optional<int>{5});
    REQUIRE(zipped.has_value());
    CHECK(*zipped == 30);

    auto keep_right = applicative.discard_first(std::optional<int>{1}, std::optional<int>{2});
    REQUIRE(keep_right.has_value());
    CHECK(*keep_right == 2);

    auto keep_left = applicative.discard_second(std::optional<int>{1}, std::optional<int>{2});
    REQUIRE(keep_left.has_value());
    CHECK(*keep_left == 1);
}

TEST_CASE("ApplicativeTypeclassTest - InvokeWithNttpMap")
{
    const auto& default_applicative = smd::applicative_typeclass<std::optional<int> >;

    auto result = default_applicative.invoke_with<
        smd::applicative_typeclass<std::optional<int> >>(
        [](int a, int b, int c) { return a + b + c; },
        std::optional<int>{1},
        std::optional<int>{2},
        std::optional<int>{3});
    REQUIRE(result.has_value());
    CHECK(*result == 6);

    auto apply_pure_result = default_applicative.apply_pure_with<
        smd::applicative_typeclass<std::optional<int> >>(
        [](int a, int b) { return a - b; },
        std::optional<int>{8},
        std::optional<int>{5});
    REQUIRE(apply_pure_result.has_value());
    CHECK(*apply_pure_result == 3);
}

TEST_CASE("ApplicativeTypeclassTest - BemanOptional")
{
    using BemanOptional = beman::optional::optional<int>;
    const auto& applicative = smd::applicative_typeclass<BemanOptional>;

    auto lifted = applicative.pure(11);
    REQUIRE(lifted.has_value());
    CHECK(*lifted == 11);

    beman::optional::optional<int (*)(int)> function{+[](int x) { return x + 5; }};
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

    auto invoked = applicative.invoke(
        [](int a, int b) { return a * b; },
        BemanOptional{3},
        BemanOptional{4});
    REQUIRE(invoked.has_value());
    CHECK(*invoked == 12);

    auto empty_invoked = applicative.invoke(
        [](int a, int b) { return a * b; },
        BemanOptional{},
        BemanOptional{4});
    CHECK_FALSE(empty_invoked.has_value());
}

TEST_CASE("ApplicativeTypeclassTest - ApplyPureWithExplicitMap")
{
    const auto& default_applicative = smd::applicative_typeclass<std::optional<int> >;
    const auto& optional_applicative = smd::applicative_typeclass<std::optional<int> >;

    auto result = default_applicative.apply_pure_with(
        optional_applicative,
        [](int a, int b, int c) { return a + b + c; },
        std::optional<int>{4},
        std::optional<int>{5},
        std::optional<int>{6});
    REQUIRE(result.has_value());
    CHECK(*result == 15);
}

TEST_CASE("ApplicativeTypeclassTest - TerminatingPartialExtendsAndInvokes")
{
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

TEST_CASE("ApplicativeTypeclassTest - IdentityMapUsesDerivedInvokePath")
{
    using Identity = smd::typeclass::test::Identity<int>;
    const auto& applicative = smd::applicative_typeclass<Identity>;

    auto binary = applicative.invoke(
        [](int a, int b) { return a + b; },
        Identity{2},
        Identity{3});
    CHECK(binary.value == 5);

    auto ternary = applicative.apply_pure(
        [](int a, int b, int c) { return a * 100 + b * 10 + c; },
        Identity{1},
        Identity{2},
        Identity{3});
    CHECK(ternary.value == 123);
}

TEST_CASE("ApplicativeTypeclassTest - CustomInvokeDispatchPath")
{
    const auto& default_applicative = smd::applicative_typeclass<std::optional<int> >;

    auto result = default_applicative.invoke_with(
        direct_invoke_map,
        [](int a, int b, int c) { return a + b + c; },
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

TEST_CASE("ApplicativeTypeclassTest - OptionalAndBemanVectorInstantiationPaths")
{
    const auto& optional_applicative =
        smd::applicative_typeclass<std::optional<std::vector<int> > >;

    auto lifted_vector = optional_applicative.pure(std::vector<int>{1, 2, 3});
    REQUIRE(lifted_vector.has_value());
    CHECK(lifted_vector->size() == 3);

    std::optional<std::vector<int> (*)(std::vector<int>)> append_value{
        +[](std::vector<int> v) {
            v.push_back(4);
            return v;
        }};
    auto applied_vector = optional_applicative.apply(append_value, lifted_vector);
    REQUIRE(applied_vector.has_value());
    CHECK(applied_vector->size() == 4);

    using BemanVectorOptional = beman::optional::optional<std::vector<int> >;
    const auto& beman_applicative = smd::applicative_typeclass<BemanVectorOptional>;

    auto beman_lifted = beman_applicative.pure(std::vector<int>{8, 9});
    REQUIRE(beman_lifted.has_value());
    CHECK(beman_lifted->size() == 2);

    beman::optional::optional<std::vector<int> (*)(std::vector<int>)> beman_append{
        +[](std::vector<int> v) {
            v.push_back(10);
            return v;
        }};
    auto beman_applied = beman_applicative.apply(beman_append, beman_lifted);
    REQUIRE(beman_applied.has_value());
    CHECK(beman_applied->size() == 3);
}

TEST_CASE("ApplicativeTypeclassTest - IdentityWrapperMethods")
{
    using Identity = smd::typeclass::test::Identity<int>;
    const auto& applicative = smd::applicative_typeclass<Identity>;

    auto mapped = applicative.map([](int x) { return x + 1; }, Identity{9});
    CHECK(mapped.value == 10);

    auto zipped = applicative.zip_with(
        [](int a, int b) { return a - b; },
        Identity{20},
        Identity{3});
    CHECK(zipped.value == 17);

    auto ap_result = applicative.ap(
        smd::typeclass::test::Identity<int (*)(int)>{+[](int x) { return x * 5; }},
        Identity{6});
    CHECK(ap_result.value == 30);
}

TEST_CASE("ApplicativeTypeclassTest - BareIdentityInvokeAndApplyChain")
{
    using BareIdentity = smd::typeclass::test::BareIdentity<int>;
    const auto& applicative = smd::applicative_typeclass<BareIdentity>;

    auto unary = applicative.invoke([](int x) { return x + 1; }, BareIdentity{4});
    CHECK(unary.value == 5);

    auto ternary = applicative.invoke(
        [](int a, int b, int c) { return a * b + c; },
        BareIdentity{2},
        BareIdentity{3},
        BareIdentity{4});
    CHECK(ternary.value == 10);

    auto quaternary = applicative.apply_pure(
        [](int a, int b, int c, int d) { return a + b + c + d; },
        BareIdentity{1},
        BareIdentity{2},
        BareIdentity{3},
        BareIdentity{4});
    CHECK(quaternary.value == 10);
}

TEST_CASE("ApplicativeTypeclassTest - BareIdentityWrapperCoverage")
{
    using BareIdentity = smd::typeclass::test::BareIdentity<int>;
    const auto& applicative = smd::applicative_typeclass<BareIdentity>;

    auto lifted = applicative.lift(33);
    CHECK(lifted.value == 33);

    auto mapped = applicative.map([](int x) { return x * 2; }, BareIdentity{11});
    CHECK(mapped.value == 22);

    auto applied = applicative.ap(
        smd::typeclass::test::BareIdentity<int (*)(int)>{+[](int x) { return x - 2; }},
        BareIdentity{9});
    CHECK(applied.value == 7);

    auto zipped = applicative.zip_with(
        [](int a, int b) { return a - b; },
        BareIdentity{40},
        BareIdentity{8});
    CHECK(zipped.value == 32);

    auto keep_right = applicative.discard_first(BareIdentity{5}, BareIdentity{6});
    CHECK(keep_right.value == 6);

    auto keep_left = applicative.discard_second(BareIdentity{5}, BareIdentity{6});
    CHECK(keep_left.value == 5);
}

TEST_CASE("ApplicativeTypeclassTest - BareIdentityInvokeWithMapCoverage")
{
    using BareIdentity = smd::typeclass::test::BareIdentity<int>;
    const auto& default_applicative = smd::applicative_typeclass<std::optional<int> >;
    const auto& bare_identity_applicative = smd::applicative_typeclass<BareIdentity>;

    auto explicit_map_result = default_applicative.invoke_with(
        bare_identity_applicative,
        [](int a, int b, int c) { return a + b + c; },
        BareIdentity{3},
        BareIdentity{4},
        BareIdentity{5});
    CHECK(explicit_map_result.value == 12);

    auto explicit_apply_pure_result = default_applicative.apply_pure_with(
        bare_identity_applicative,
        [](int a, int b) { return a * b; },
        BareIdentity{7},
        BareIdentity{6});
    CHECK(explicit_apply_pure_result.value == 42);

    auto nttp_map_result = default_applicative.invoke_with<bare_identity_applicative>(
        [](int a, int b) { return a - b; },
        BareIdentity{20},
        BareIdentity{9});
    CHECK(nttp_map_result.value == 11);

    auto nttp_apply_pure_result = default_applicative.apply_pure_with<bare_identity_applicative>(
        [](int a, int b, int c) { return a + b * c; },
        BareIdentity{2},
        BareIdentity{3},
        BareIdentity{4});
    CHECK(nttp_apply_pure_result.value == 14);
}

TEST_CASE("ApplicativeTypeclassTest - BareIdentityTypeMatrixCoverage")
{
    run_bare_identity_matrix_case<int, short, unsigned>(3, 4, 5U);
    run_bare_identity_matrix_case<long, int, long long>(10L, 20, 30LL);
    run_bare_identity_matrix_case<float, double, int>(1.5F, 2.25, 3);
}

TEST_CASE("ApplicativeBehaviorTest - OptionalIdentityHomomorphismAndInvoke")
{
    CHECK(smd::typeclass::test::check_applicative_identity_law(std::optional<int>{8}));
    CHECK(smd::typeclass::test::check_applicative_homomorphism_law<std::optional<int> >(
        +[](int x) { return x + 3; },
        5));
    CHECK(smd::typeclass::test::check_applicative_invoke_binary_law(
        [](int a, int b) { return a * 10 + b; },
        std::optional<int>{2},
        std::optional<int>{7}));
}

TEST_CASE("ApplicativeBehaviorTest - BareIdentityIdentityHomomorphismAndInvoke")
{
    using BareIdentity = smd::typeclass::test::BareIdentity<int>;
    CHECK(smd::typeclass::test::check_applicative_identity_law(BareIdentity{11}));
    CHECK(smd::typeclass::test::check_applicative_homomorphism_law<BareIdentity>(
        +[](int x) { return x * 4; },
        3));
    CHECK(smd::typeclass::test::check_applicative_invoke_binary_law(
        [](int a, int b) { return a - b; },
        BareIdentity{20},
        BareIdentity{6}));
}

TEST_CASE("ApplicativeBehaviorTest - BemanIdentityHomomorphismAndInvoke")
{
    using BemanOptional = beman::optional::optional<int>;

    CHECK(smd::typeclass::test::check_applicative_identity_law(BemanOptional{11}));
    CHECK(smd::typeclass::test::check_applicative_homomorphism_law<BemanOptional>(
        +[](int x) { return x * 4; },
        3));
    CHECK(smd::typeclass::test::check_applicative_invoke_binary_law(
        [](int a, int b) { return a - b; },
        BemanOptional{20},
        BemanOptional{6}));
}

TEST_CASE("ApplicativeBehaviorTest - OptionalShortCircuit")
{
    const auto& applicative = smd::applicative_typeclass<std::optional<int> >;

    std::optional<std::function<int(int)> > no_function{};
    auto no_function_result = applicative.ap(no_function, std::optional<int>{4});
    CHECK_FALSE(no_function_result.has_value());

    std::optional<std::function<int(int)> > function{
        [](int x) { return x + 1; }};
    auto no_argument_result = applicative.ap(function, std::optional<int>{});
    CHECK_FALSE(no_argument_result.has_value());

    int calls = 0;
    auto invoke_result = applicative.invoke(
        [&calls](int lhs, int rhs) {
            ++calls;
            return lhs + rhs;
        },
        std::optional<int>{3},
        std::optional<int>{});
    CHECK_FALSE(invoke_result.has_value());
    CHECK(calls == 0);
}

TEST_CASE("ApplicativeLaws - InterchangeLaw")
{
    // Interchange: ap(u, pure(y)) == ap(pure(λf. f(y)), u)
    // Ensures that applying a contextual function to a pure value is symmetric.
    using Fn = std::function<int(int)>;
    const int y = 7;

    {
        // b8e3d6a1-2c5f-4b7e-a2d8-7f6c2b3e5d15
        const auto& ap = smd::applicative_typeclass<std::optional<int> >;
        std::optional<Fn> u{[](int x) { return x * 3; }};

        auto lhs = ap.ap(u, ap.pure(y));
        auto rhs = ap.ap(ap.pure([](const Fn& fn) { return fn(y); }), u);

        REQUIRE(lhs.has_value());
        CHECK(*lhs == 21);
        CHECK(lhs == rhs);
        // b8e3d6a1-2c5f-4b7e-a2d8-7f6c2b3e5d15 end
    }
    {
        // empty function: both sides propagate the absence
        const auto& ap = smd::applicative_typeclass<std::optional<int> >;
        std::optional<Fn> empty{};
        auto lhs = ap.ap(empty, ap.pure(y));
        auto rhs = ap.ap(ap.pure([](const Fn& fn) { return fn(y); }), empty);
        CHECK_FALSE(lhs.has_value());
        CHECK(lhs == rhs);
    }
    {
        using BemanFn = beman::optional::optional<Fn>;
        const auto& ap = smd::applicative_typeclass<beman::optional::optional<int> >;
        BemanFn u{[](int x) { return x + 8; }};

        auto lhs = ap.ap(u, ap.pure(y));
        auto rhs = ap.ap(ap.pure([](const Fn& fn) { return fn(y); }), u);

        REQUIRE(lhs.has_value());
        CHECK(*lhs == 15);
        CHECK(lhs == rhs);
    }
    {
        using BI = smd::typeclass::test::BareIdentity<int>;
        using BIFn = smd::typeclass::test::BareIdentity<Fn>;
        const auto& ap = smd::applicative_typeclass<BI>;
        BIFn u{[](int x) { return x - 2; }};

        auto lhs = ap.ap(u, ap.pure(y));
        auto rhs = ap.ap(ap.pure([](const Fn& fn) { return fn(y); }), u);

        CHECK(lhs.value == 5);
        CHECK(lhs == rhs);
    }
}

TEST_CASE("ApplicativeLaws - CompositionLaw")
{
    // Composition: ap(invoke(∘, u, v), w) == ap(u, ap(v, w))
    // Composing effectful functions then applying equals sequencing the applications.
    using Fn = std::function<int(int)>;
    auto compose = [](const Fn& f, const Fn& g) {
        return Fn{[f, g](int x) { return f(g(x)); }};
    };

    {
        // e2c7f5b3-4a1d-4e8c-b3f5-9d6a5c2e3b02
        const auto& ap = smd::applicative_typeclass<std::optional<int> >;
        std::optional<Fn> u{[](int x) { return x + 10; }};
        std::optional<Fn> v{[](int x) { return x * 2; }};
        std::optional<int> w{3};

        auto lhs = ap.ap(ap.invoke(compose, u, v), w);
        auto rhs = ap.ap(u, ap.ap(v, w));

        REQUIRE(lhs.has_value());
        CHECK(*lhs == 16);  // (3 * 2) + 10
        CHECK(lhs == rhs);
        // e2c7f5b3-4a1d-4e8c-b3f5-9d6a5c2e3b02 end
    }
    {
        // empty u propagates to both sides
        const auto& ap = smd::applicative_typeclass<std::optional<int> >;
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
        const auto& ap = smd::applicative_typeclass<BI>;
        BIFn u{[](int x) { return x + 10; }};
        BIFn v{[](int x) { return x * 2; }};
        BI w{3};

        auto lhs = ap.ap(ap.invoke(compose, u, v), w);
        auto rhs = ap.ap(u, ap.ap(v, w));

        CHECK(lhs.value == 16);
        CHECK(lhs == rhs);
    }
}

TEST_CASE("ApplicativeBehaviorTest - BemanShortCircuit")
{
    using BemanOptional = beman::optional::optional<int>;
    const auto& applicative = smd::applicative_typeclass<BemanOptional>;

    beman::optional::optional<std::function<int(int)> > no_function{};
    auto no_function_result = applicative.ap(no_function, BemanOptional{5});
    CHECK_FALSE(no_function_result.has_value());

    beman::optional::optional<std::function<int(int)> > function{
        [](int x) { return x * 2; }};
    auto no_argument_result = applicative.ap(function, BemanOptional{});
    CHECK_FALSE(no_argument_result.has_value());

    int calls = 0;
    auto invoke_result = applicative.invoke(
        [&calls](int lhs, int rhs) {
            ++calls;
            return lhs - rhs;
        },
        BemanOptional{9},
        BemanOptional{});
    CHECK_FALSE(invoke_result.has_value());
    CHECK(calls == 0);
}

TEST_CASE("ApplicativeBehaviorTest - InvokeDispatchThroughBaseAndDerivedPaths")
{
    DirectInvokeIdentityApplicativeMap<int> custom_map{};
    auto& custom_base =
        static_cast<smd::Applicative<DirectInvokeIdentityApplicativeImpl<int> >&>(
            custom_map);

    auto custom_dispatched = custom_base.invoke(
        [](int a, int b, int c) { return a + b + c; },
        smd::typeclass::test::Identity<int>{1},
        smd::typeclass::test::Identity<int>{2},
        smd::typeclass::test::Identity<int>{3});
    CHECK(custom_dispatched.value == 6);

    smd::BareIdentityApplicativeMap<int> bare_map{};
    auto& bare_base =
        static_cast<smd::Applicative<smd::BareIdentityApplicativeImpl<int> >&>(bare_map);

    auto derived_dispatched = bare_base.invoke(
        [](int a, int b, int c) { return a * 100 + b * 10 + c; },
        smd::typeclass::test::BareIdentity<int>{4},
        smd::typeclass::test::BareIdentity<int>{5},
        smd::typeclass::test::BareIdentity<int>{6});
    CHECK(derived_dispatched.value == 456);
}

TEST_CASE("ApplicativeBehaviorTest - BareIdentityConstAndNonConstInvokeApMap")
{
    smd::BareIdentityApplicativeMap<int> mutable_map{};
    auto& mutable_base =
        static_cast<smd::Applicative<smd::BareIdentityApplicativeImpl<int> >&>(mutable_map);

    auto non_const_invoke = mutable_base.invoke(
        [](int a, int b) { return a + b; },
        smd::typeclass::test::BareIdentity<int>{10},
        smd::typeclass::test::BareIdentity<int>{4});
    CHECK(non_const_invoke.value == 14);

    auto non_const_map = mutable_base.map(
        [](int x) { return x * 3; },
        smd::typeclass::test::BareIdentity<int>{7});
    CHECK(non_const_map.value == 21);

    auto non_const_ap = mutable_base.ap(
        smd::typeclass::test::BareIdentity<std::function<int(int)> >{
            [](int x) { return x - 5; }},
        smd::typeclass::test::BareIdentity<int>{12});
    CHECK(non_const_ap.value == 7);

    const smd::BareIdentityApplicativeMap<int> const_map{};
    const auto& const_base =
        static_cast<const smd::Applicative<smd::BareIdentityApplicativeImpl<int> >&>(
            const_map);

    auto const_invoke = const_base.invoke(
        [](int a, int b, int c) { return a * b + c; },
        smd::typeclass::test::BareIdentity<int>{3},
        smd::typeclass::test::BareIdentity<int>{5},
        smd::typeclass::test::BareIdentity<int>{2});
    CHECK(const_invoke.value == 17);

    auto const_map_result = const_base.map(
        [](int x) { return x + 8; },
        smd::typeclass::test::BareIdentity<int>{1});
    CHECK(const_map_result.value == 9);

    auto const_ap_result = const_base.ap(
        smd::typeclass::test::BareIdentity<std::function<int(int)> >{
            [](int x) { return x * x; }},
        smd::typeclass::test::BareIdentity<int>{6});
    CHECK(const_ap_result.value == 36);
}

```

## smd/typeclass/examples/applicative_bad.cpp

```cpp
#include <smd/typeclass/examples/examples.hpp>

#include <smd/tree/fix_tree.hpp>

#include <cstddef>
#include <utility>

namespace smd::typeclass::examples {

template <class LEFT, class RIGHT>
auto cartesian_product(const smd::tree::FixTree<LEFT>&,
                       const smd::tree::FixTree<RIGHT>&)
    -> smd::tree::FixTree<std::pair<LEFT, RIGHT> >
{
    using PairTree = smd::tree::FixTree<std::pair<LEFT, RIGHT> >;
    return PairTree::leaf(std::pair<LEFT, RIGHT>{});
}

auto bad_applicative_example() -> std::size_t
{
    using IntTree = smd::tree::FixTree<int>;
    auto tx = IntTree::branch(IntTree::leaf(1), IntTree::leaf(2));
    auto ty = IntTree::leaf(3);

    // d2e7a1c9-0f3b-4b2e-9d55-1a8e7c4b2f90
    // Hypothetical: expands structure instead of preserving shape.
    auto bad = cartesian_product(tx, ty);
    // d2e7a1c9-0f3b-4b2e-9d55-1a8e7c4b2f90 end

    return bad.is_leaf() ? 1U : 0U;
}

}  // close namespace smd::typeclass::examples

```

## smd/typeclass/examples/applicative_examples.cpp

```cpp
#include <smd/typeclass/examples/examples.hpp>

#include <smd/typeclass/applicative.hpp>

#include <beman/optional/optional.hpp>

namespace smd::typeclass::examples {

auto applicative_invoke_example() -> beman::optional::optional<int>
{
    using beman::optional::optional;
    const auto& applicative = smd::applicative_typeclass<optional<int> >;

    optional<int> ax = 1;
    optional<int> ay = 2;
    optional<int> az = 3;

    // 3f0c8d0e-9a6b-4a3e-9c2a-0c1e9c3d4f11
    auto sum = applicative.invoke(
        [](int a, int b, int c) { return a + b + c; },
        ax,
        ay,
        az);
    // 3f0c8d0e-9a6b-4a3e-9c2a-0c1e9c3d4f11 end

    return sum;
}

}  // close namespace smd::typeclass::examples

```

## smd/typeclass/examples/examples.t.cpp

```cpp
#include <smd/typeclass/examples/examples.hpp>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("TypeclassExamples - SlideExamplesRemainExecutable")
{
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

#include <smd/tree/fix_tree.hpp>
#include <smd/tree/fix_tree_foldable.hpp>
#include <smd/tree/fringe_tree.hpp>
#include <smd/tree/fringe_tree_foldable.hpp>
#include <smd/tree/binary_tree.hpp>
#include <smd/tree/binary_tree_foldable.hpp>
#include <smd/typeclass/foldable.hpp>

namespace smd::typeclass::examples {

using IntTree = smd::tree::FixTree<int>;

auto make_sample_tree() -> IntTree
{
    return IntTree::branch(IntTree::leaf(1),
                           IntTree::branch(IntTree::leaf(2), IntTree::leaf(3)));
}

auto generic_length_example() -> std::size_t
{
    auto tree = make_sample_tree();
    const auto& foldable = smd::foldable_typeclass<IntTree>;

    // 9a1c4e2b-2c7e-4b1a-9f55-8b6a4d2e91aa
    auto n = foldable.length(tree);
    // 9a1c4e2b-2c7e-4b1a-9f55-8b6a4d2e91aa end

    return n;
}

auto generic_length_binary_tree_example() -> std::size_t
{
    using IntBinaryTree = smd::tree::BinaryTree<int>;
    auto tree = IntBinaryTree::from_children_ptrs(
      2,
      IntBinaryTree::make_ptr(IntBinaryTree::leaf(1)),
      IntBinaryTree::make_ptr(IntBinaryTree::from_children_ptrs(
        3,
        {},
        IntBinaryTree::make_ptr(IntBinaryTree::leaf(4)))));

    const auto& foldable = smd::foldable_typeclass<IntBinaryTree>;

    // 53b9f5b4-3b3a-4e18-9b3c-07b7e2c980f4
    auto n = foldable.length(tree);
    // 53b9f5b4-3b3a-4e18-9b3c-07b7e2c980f4 end

    return n;
}

auto generic_length_fringe_tree_example() -> std::size_t
{
        using Fringe = smd::tree::FringeTree<int>;
        auto tree = Fringe::branch(
            Fringe::branch(Fringe::leaf(1), Fringe::leaf(2)),
            Fringe::leaf(3));

        const auto& foldable = smd::foldable_typeclass<Fringe>;

        // 7c2f11d9-ef09-45e2-80da-9229f3c8d82c
        auto n = foldable.length(tree);
        // 7c2f11d9-ef09-45e2-80da-9229f3c8d82c end

        return n;
}

auto foldable_flattens_shape_example() -> bool
{
        using Tree = smd::tree::FixTree<int>;
        auto left_heavy = Tree::branch(
            Tree::leaf(1),
            Tree::branch(Tree::leaf(2), Tree::leaf(3)));
        auto right_heavy = Tree::branch(
            Tree::branch(Tree::leaf(1), Tree::leaf(2)),
            Tree::leaf(3));

        const auto& foldable = smd::foldable_typeclass<Tree>;

        // b1fd4b92-b060-4c47-8c08-97328ec02329
        auto left_flat = foldable.to_vector(left_heavy);
        auto right_flat = foldable.to_vector(right_heavy);
        // b1fd4b92-b060-4c47-8c08-97328ec02329 end

        return left_flat == right_flat;
}

}  // close namespace smd::typeclass::examples

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
    auto fmap(F&& function, const std::optional<T>& value) const
    {
        const auto& functor = smd::functor_typeclass<std::optional<T> >;
        return functor.fmap(std::forward<F>(function), value);
    }
};

template <class CONTEXT>
inline constexpr auto functor_typeclass = std::false_type{};

template <class T>
inline constexpr auto functor_typeclass<std::optional<T> > =
    OptionalFunctorObject{};

template <class CONTEXT, const auto& FUNCTOR = functor_typeclass<CONTEXT> >
auto fmap_plus_one_nttp(const CONTEXT& value)
{
    return FUNCTOR.fmap([](int x) { return x + 1; }, value);
}

auto explicit_object_lookup_example() -> std::optional<int>
{
    OptionalFunctorObject functor;
    std::optional<int> value{41};

    return functor.fmap([](int x) { return x + 1; }, value);
}

auto nttp_object_lookup_example() -> std::optional<int>
{
    std::optional<int> value{9};
    return fmap_plus_one_nttp(value);
}

}  // close namespace smd::typeclass::examples

```

## smd/typeclass/examples/traversable_examples.cpp

```cpp
#include <smd/typeclass/examples/examples.hpp>

#include <smd/tree/fix_tree.hpp>
#include <smd/tree/fix_tree_foldable.hpp>
#include <smd/tree/fix_tree_traversable.hpp>
#include <smd/typeclass/applicative.hpp>
#include <smd/typeclass/traversable.hpp>

#include <beman/optional/optional.hpp>

namespace smd::typeclass::examples {

auto traversable_relabel_example() -> beman::optional::optional<std::size_t>
{
    using IntTree = smd::tree::FixTree<int>;
    auto tree = IntTree::branch(IntTree::leaf(1), IntTree::leaf(2));
    const auto& traversable = smd::traversable_typeclass<IntTree>;

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

    const auto& foldable = smd::foldable_typeclass<IntTree>;
    return foldable.length(*relabelled);
}

auto traversable_preserves_shape_example() -> bool
{
        using IntTree = smd::tree::FixTree<int>;
        using beman::optional::optional;

        auto tree = IntTree::branch(
            IntTree::leaf(1),
            IntTree::branch(IntTree::leaf(2), IntTree::leaf(3)));
        const auto& traversable = smd::traversable_typeclass<IntTree>;

        // d804ec63-77d1-4fa0-99a6-9effce6f741b
        auto mapped = smd::traverse(
            [](int x) -> optional<int> { return optional<int>{x + 10}; },
            tree);
        // d804ec63-77d1-4fa0-99a6-9effce6f741b end

        if (!mapped || mapped->is_leaf()) {
                return false;
        }

        return mapped->left().is_leaf() &&
                     mapped->left().value() == 11 &&
                     !mapped->right().is_leaf() &&
                     mapped->right().left().is_leaf() &&
                     mapped->right().left().value() == 12 &&
                     mapped->right().right().is_leaf() &&
                     mapped->right().right().value() == 13;
}

}  // close namespace smd::typeclass::examples

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
  auto operator()(STATE s) const -> STATE
  {
    return d_f2(d_f1(std::move(s)));
  }
};

// Left fold composition (same order as f1 then f2)
template <class STATE>
struct LeftFoldProgram {
  std::function<STATE(STATE)> d_run;

  auto operator()(STATE state) const -> STATE
  {
    return d_run(std::move(state));
  }
};

// Template-based version that avoids type erasure - used internally
template <class F>
struct LeftFoldProgramT {
  F d_run;

  template <class STATE>
  auto operator()(STATE state) const -> STATE
  {
    return d_run(std::move(state));
  }
};

template <class STATE>
struct RightFoldProgram {
  std::function<STATE(STATE)> d_run;

  auto operator()(STATE state) const -> STATE
  {
    return d_run(std::move(state));
  }
};

// Template-based version that avoids type erasure - used internally
template <class F>
struct RightFoldProgramT {
  F d_run;

  template <class STATE>
  auto operator()(STATE state) const -> STATE
  {
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

}  // close namespace smd::detail

namespace smd::typeclass {

template <class STATE>
struct Monoid<smd::detail::LeftFoldProgram<STATE> > {
  auto identity() const -> smd::detail::LeftFoldProgram<STATE>
  {
    return smd::detail::LeftFoldProgram<STATE>{
      [](STATE s) { return s; }};
  }

  auto combine(const smd::detail::LeftFoldProgram<STATE>& lhs,
         const smd::detail::LeftFoldProgram<STATE>& rhs) const
    -> smd::detail::LeftFoldProgram<STATE>
  {
    return smd::detail::LeftFoldProgram<STATE>{
      [lhs, rhs](STATE s) { return rhs(lhs(std::move(s))); }};
  }
};

// Template specialization for LeftFoldProgramT - avoids type erasure
template <class F>
struct Monoid<smd::detail::LeftFoldProgramT<F> > {
  auto identity() const -> smd::detail::LeftFoldProgramT<smd::detail::IdentityFoldFunc<int> >
  {
    return smd::detail::LeftFoldProgramT<smd::detail::IdentityFoldFunc<int> >{
      smd::detail::IdentityFoldFunc<int>{}};
  }

  template <class G>
  auto combine(const smd::detail::LeftFoldProgramT<F>& lhs,
         const smd::detail::LeftFoldProgramT<G>& rhs) const
    -> smd::detail::LeftFoldProgramT<smd::detail::ComposedFoldFunc<F, G> >
  {
    return smd::detail::LeftFoldProgramT<smd::detail::ComposedFoldFunc<F, G> >{
      smd::detail::ComposedFoldFunc<F, G>{lhs.d_run, rhs.d_run}};
  }
};

template <class STATE>
struct Monoid<smd::detail::RightFoldProgram<STATE> > {
  auto identity() const -> smd::detail::RightFoldProgram<STATE>
  {
    return smd::detail::RightFoldProgram<STATE>{
      [](STATE s) { return s; }};
  }

  auto combine(const smd::detail::RightFoldProgram<STATE>& lhs,
         const smd::detail::RightFoldProgram<STATE>& rhs) const
    -> smd::detail::RightFoldProgram<STATE>
  {
    return smd::detail::RightFoldProgram<STATE>{
      [lhs, rhs](STATE s) { return lhs(rhs(std::move(s))); }};
  }
};

// Template specialization for RightFoldProgramT - avoids type erasure
template <class F>
struct Monoid<smd::detail::RightFoldProgramT<F> > {
  auto identity() const -> smd::detail::RightFoldProgramT<smd::detail::IdentityFoldFunc<int> >
  {
    return smd::detail::RightFoldProgramT<smd::detail::IdentityFoldFunc<int> >{
      smd::detail::IdentityFoldFunc<int>{}};
  }

  template <class G>
  auto combine(const smd::detail::RightFoldProgramT<F>& lhs,
         const smd::detail::RightFoldProgramT<G>& rhs) const
    -> smd::detail::RightFoldProgramT<smd::detail::ComposedFoldFunc<F, G> >
  {
    return smd::detail::RightFoldProgramT<smd::detail::ComposedFoldFunc<F, G> >{
      smd::detail::ComposedFoldFunc<F, G>{lhs.d_run, rhs.d_run}};
  }
};

template <>
struct Monoid<smd::detail::Any> {
  constexpr auto identity() const -> smd::detail::Any { return {false}; }

  constexpr auto combine(smd::detail::Any lhs, smd::detail::Any rhs) const
    -> smd::detail::Any
  {
    return {lhs.d_value || rhs.d_value};
  }
};

template <>
struct Monoid<smd::detail::All> {
  constexpr auto identity() const -> smd::detail::All { return {true}; }

  constexpr auto combine(smd::detail::All lhs, smd::detail::All rhs) const
    -> smd::detail::All
  {
    return {lhs.d_value && rhs.d_value};
  }
};

template <class VALUE_TYPE>
struct Monoid<smd::detail::First<VALUE_TYPE> > {
  auto identity() const -> smd::detail::First<VALUE_TYPE> { return {{}}; }

  auto combine(const smd::detail::First<VALUE_TYPE>& lhs,
         const smd::detail::First<VALUE_TYPE>& rhs) const
    -> smd::detail::First<VALUE_TYPE>
  {
    if (lhs.d_value) {
      return lhs;
    }
    return rhs;
  }
};

}  // close namespace smd::typeclass

namespace smd {

// Foldable pattern invariants:
// - Generic entry point is fold_map(F, T) via foldable_typeclass<T>.
// - Instances provide an object with fold_map(F, T) and specialize
//   foldable_typeclass<Concrete>.
// - Derived APIs (length, fold_left, fold_right, combine_all, any_of, all_of,
//   empty, to_vector, find_first) live on the same looked-up object.
// - Traversal order is instance-defined but must be coherent per instance.

template <class Impl>
struct Foldable : protected Impl {
  using Impl::fold_map;

  // e3a1b1a2-6adf-4cb9-8c85-c0e39a7b98f2

  // c1e5b4a7-4d3f-4c2b-a7e1-7f9d4c6b3e08
  template <class T>
  auto length(this auto&& self, T&& value) -> std::size_t
  {
    const auto count = self.fold_map(
      [](const auto&) { return typeclass::Count{1}; },
      std::forward<T>(value));
    return count.d_value;
  }
  // c1e5b4a7-4d3f-4c2b-a7e1-7f9d4c6b3e08 end

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

  // a6d2c8f3-1e7b-4a5d-b9f4-3c8e2a7d1b09
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
  // a6d2c8f3-1e7b-4a5d-b9f4-3c8e2a7d1b09 end
  // e3a1b1a2-6adf-4cb9-8c85-c0e39a7b98f2 end

  template <class T, class PREDICATE>
  auto find_first(this auto&& self, T&& value, PREDICATE&& predicate)
  {
    const auto result = self.fold_map(
      [&predicate](const auto& x) {
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

template <class T>
inline constexpr auto foldable_typeclass = std::false_type{};

}  // close namespace smd

#endif

```

## smd/typeclass/foldable.t.cpp

```cpp
#include <smd/typeclass/foldable.hpp>
#include <smd/typeclass/test/test_support.hpp>

#include <catch2/catch_test_macros.hpp>

#include <vector>

namespace {

// a7c3e1f5-8d4b-4a2c-9e7f-3b5d1c8a4e06
template <class STRUCTURE,
          const auto& FOLDABLE = smd::foldable_typeclass<STRUCTURE> >
auto sum_with_nttp_lookup(const STRUCTURE& structure)
{
    return FOLDABLE.fold_map([](int x) { return x; }, structure);
}
// a7c3e1f5-8d4b-4a2c-9e7f-3b5d1c8a4e06 end

template <class STRUCTURE,
          const auto& FOLDABLE = smd::foldable_typeclass<STRUCTURE> >
auto fold_left_with_nttp_lookup(const STRUCTURE& structure)
{
    return FOLDABLE.fold_left(structure, 0, [](int acc, int x) {
        return acc * 10 + x;
    });
}

template <class STRUCTURE,
          const auto& FOLDABLE = smd::foldable_typeclass<STRUCTURE> >
auto fold_right_with_nttp_lookup(const STRUCTURE& structure)
{
    return FOLDABLE.fold_right(structure, 0, [](int x, int acc) {
        return x * 10 + acc;
    });
}

}  // namespace

TEST_CASE("FoldableTypeclassTest - LengthOnSequence")
{
    using Sequence = smd::typeclass::test::Sequence<int>;
    auto sequence = Sequence{{1, 2, 3}};

    const auto& foldable = smd::foldable_typeclass<Sequence>;
    CHECK(foldable.length(sequence) == 3U);
}

TEST_CASE("FoldableTypeclassTest - FoldMapSumOnSequence")
{
    using Sequence = smd::typeclass::test::Sequence<int>;
    auto sequence = Sequence{{1, 2, 3}};

    const auto& foldable = smd::foldable_typeclass<Sequence>;
    const auto sum = foldable.fold_map([](int x) { return x; }, sequence);
    CHECK(sum == 6);
}

TEST_CASE("FoldableTypeclassTest - FoldMapWithExplicitObject")
{
    using Sequence = smd::typeclass::test::Sequence<int>;
    auto sequence = Sequence{{1, 2, 3}};

    const auto& foldable = smd::foldable_typeclass<Sequence>;
    const auto sum = foldable.fold_map([](int x) { return x; }, sequence);
    CHECK(sum == 6);
}

TEST_CASE("FoldableTypeclassTest - FoldMapWithNttpLookup")
{
    using Sequence = smd::typeclass::test::Sequence<int>;
    auto sequence = Sequence{{1, 2, 3}};

    CHECK(sum_with_nttp_lookup(sequence) == 6);
}

TEST_CASE("FoldableTypeclassTest - FoldLeftAndRight")
{
    using Sequence = smd::typeclass::test::Sequence<int>;
    auto sequence = Sequence{{1, 2, 3}};
    const auto& foldable = smd::foldable_typeclass<Sequence>;

    const auto left = foldable.fold_left(sequence, 0, [](int acc, int x) {
        return acc * 10 + x;
    });
    const auto right = foldable.fold_right(sequence, 0, [](int x, int acc) {
        return x * 10 + acc;
    });

    CHECK(left == 123);
    CHECK(right == 60);
}

TEST_CASE("FoldableTypeclassTest - FoldLeftRightWithExplicitObject")
{
    using Sequence = smd::typeclass::test::Sequence<int>;
    auto sequence = Sequence{{1, 2, 3}};

    const auto& foldable = smd::foldable_typeclass<Sequence>;
    const auto left = foldable.fold_left(sequence, 0, [](int acc, int x) {
        return acc * 10 + x;
    });
    const auto right = foldable.fold_right(sequence, 0, [](int x, int acc) {
        return x * 10 + acc;
    });

    CHECK(left == 123);
    CHECK(right == 60);
}

TEST_CASE("FoldableTypeclassTest - FoldLeftRightWithNttpLookup")
{
    using Sequence = smd::typeclass::test::Sequence<int>;
    auto sequence = Sequence{{1, 2, 3}};

    CHECK(fold_left_with_nttp_lookup(sequence) == 123);
    CHECK(fold_right_with_nttp_lookup(sequence) == 60);
}

TEST_CASE("FoldableTypeclassTest - PredicatesAndFind")
{
    using Sequence = smd::typeclass::test::Sequence<int>;
    auto sequence = Sequence{{1, 2, 3}};
    const auto& foldable = smd::foldable_typeclass<Sequence>;

    CHECK(foldable.any_of(sequence, [](int x) { return x == 2; }));
    CHECK(foldable.all_of(sequence, [](int x) { return x > 0; }));
    CHECK_FALSE(foldable.empty(sequence));

    auto found = foldable.find_first(sequence, [](int x) { return x > 1; });
    REQUIRE(found.has_value());
    CHECK(*found == 2);
}

TEST_CASE("FoldableTypeclassTest - ToVectorAndCombineAll")
{
    // 4c8a5f77-8a62-4f1b-a9cf-95452c4b8ea4
    // a3d5c9e1-6b2f-4a4d-c8e3-5b1d3a7f2c46
    using IntSequence = smd::typeclass::test::Sequence<int>;
    auto sequence = IntSequence{{1, 2, 3}};
    const auto& int_foldable = smd::foldable_typeclass<IntSequence>;

    const auto as_vector = int_foldable.to_vector(sequence);
    CHECK(as_vector == (std::vector<int>{1, 2, 3}));
    // a3d5c9e1-6b2f-4a4d-c8e3-5b1d3a7f2c46 end

    using VectorSequence = smd::typeclass::test::Sequence<std::vector<int> >;
    auto vectors = VectorSequence{{{1, 2}, {3}}};
    const auto& vector_foldable = smd::foldable_typeclass<VectorSequence>;
    const auto combined = vector_foldable.combine_all(vectors);
    CHECK(combined == (std::vector<int>{1, 2, 3}));

    const auto folded = vector_foldable.fold(vectors);
    CHECK(folded == (std::vector<int>{1, 2, 3}));
    // 4c8a5f77-8a62-4f1b-a9cf-95452c4b8ea4 end
}

TEST_CASE("FoldableTypeclassTest - AllOfAndFindFirstEdgeCases")
{
    using Sequence = smd::typeclass::test::Sequence<int>;
    const auto& foldable = smd::foldable_typeclass<Sequence>;

    auto mixed = Sequence{{2, -1, 4}};
    CHECK_FALSE(foldable.all_of(mixed, [](int x) { return x > 0; }));

    auto found_even = foldable.find_first(mixed, [](int x) { return x % 2 == 0; });
    REQUIRE(found_even.has_value());
    CHECK(*found_even == 2);

    auto found_large = foldable.find_first(mixed, [](int x) { return x > 100; });
    CHECK_FALSE(found_large.has_value());
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

template <class Impl>
struct Functor : protected Impl {
    using Impl::fmap;

    // e4c7a3f1-8b2d-4e1a-b6f4-1c8d7a5e3b02
    template <class T, class U>
    auto replace(this auto&& self, T&& value, U&& replacement)
    {
        return self.fmap(
            [replacement = std::forward<U>(replacement)](const auto&) {
                return replacement;
            },
            std::forward<T>(value));
    }
    // e4c7a3f1-8b2d-4e1a-b6f4-1c8d7a5e3b02 end
};

template <class T>
inline constexpr auto functor_typeclass = std::false_type{};

template <class VALUE_TYPE>
struct OptionalFunctorImpl {
    template <class F>
    auto fmap(this auto&&,
              F&& function,
              const std::optional<VALUE_TYPE>& value)
    {
        using Result = std::invoke_result_t<F, const VALUE_TYPE&>;
        if (!value) {
            return std::optional<remove_cvref_t<Result> >{};
        }
        return std::optional<remove_cvref_t<Result> >{
            std::invoke(std::forward<F>(function), *value)};
    }
};

template <class VALUE_TYPE>
    requires(!std::same_as<beman::optional::optional<VALUE_TYPE>, std::optional<VALUE_TYPE> >)
struct BemanOptionalFunctorImpl {
    template <class F>
    auto fmap(this auto&&,
              F&& function,
              const beman::optional::optional<VALUE_TYPE>& value)
    {
        using Result = std::invoke_result_t<F, const VALUE_TYPE&>;
        if (!value) {
            return beman::optional::optional<remove_cvref_t<Result> >{};
        }
        return beman::optional::optional<remove_cvref_t<Result> >{
            std::invoke(std::forward<F>(function), *value)};
    }
};

template <class VALUE_TYPE>
struct VectorFunctorImpl {
    template <class F>
    auto fmap(this auto&&,
              F&& function,
              const std::vector<VALUE_TYPE>& values)
    {
        using Result = std::invoke_result_t<F, const VALUE_TYPE&>;
        std::vector<remove_cvref_t<Result> > output;
        output.reserve(values.size());

        std::ranges::transform(values, std::back_inserter(output),
            [&function](const VALUE_TYPE& v) { return std::invoke(function, v); });

        return output;
    }
};

template <class VALUE_TYPE>
struct OptionalFunctorMap : Functor<OptionalFunctorImpl<VALUE_TYPE> > {
    using OptionalFunctorImpl<VALUE_TYPE>::fmap;
};

template <class VALUE_TYPE>
    requires(!std::same_as<beman::optional::optional<VALUE_TYPE>, std::optional<VALUE_TYPE> >)
struct BemanOptionalFunctorMap : Functor<BemanOptionalFunctorImpl<VALUE_TYPE> > {
    using BemanOptionalFunctorImpl<VALUE_TYPE>::fmap;
};

template <class VALUE_TYPE>
struct VectorFunctorMap : Functor<VectorFunctorImpl<VALUE_TYPE> > {
    using VectorFunctorImpl<VALUE_TYPE>::fmap;
};

template <class VALUE_TYPE>
inline constexpr auto functor_typeclass<std::optional<VALUE_TYPE> > =
    OptionalFunctorMap<VALUE_TYPE>{};

template <class VALUE_TYPE>
    requires(!std::same_as<beman::optional::optional<VALUE_TYPE>, std::optional<VALUE_TYPE> >)
inline constexpr auto functor_typeclass<beman::optional::optional<VALUE_TYPE> > =
    BemanOptionalFunctorMap<VALUE_TYPE>{};

template <class VALUE_TYPE>
inline constexpr auto functor_typeclass<std::vector<VALUE_TYPE> > =
    VectorFunctorMap<VALUE_TYPE>{};

}  // close namespace smd

#endif  // INCLUDED_SMD_TYPECLASS_FUNCTOR

```

## smd/typeclass/functor.t.cpp

```cpp
#include <smd/typeclass/functor.hpp>

#include <catch2/catch_test_macros.hpp>

#include <beman/optional/optional.hpp>

#include <optional>
#include <vector>

TEST_CASE("FunctorTypeclassTest - OptionalBreathing")
{
    std::optional<int> value{5};
    const auto& functor = smd::functor_typeclass<std::optional<int> >;
    auto mapped = functor.fmap([](int x) { return x + 1; }, value);

    REQUIRE(mapped.has_value());
    CHECK(*mapped == 6);
}

TEST_CASE("FunctorTypeclassTest - ReplaceVector")
{
    std::vector<int> input{1, 2, 3};
    const auto& functor = smd::functor_typeclass<std::vector<int> >;
    auto replaced = functor.replace(input, 9);

    CHECK(replaced == (std::vector<int>{9, 9, 9}));
}

TEST_CASE("FunctorTypeclassTest - OptionalFmapShortCircuit")
{
    std::optional<int> empty{};
    const auto& functor = smd::functor_typeclass<std::optional<int> >;

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

TEST_CASE("FunctorTypeclassTest - VectorFmapMapsAndPreservesEmpty")
{
    const auto& functor = smd::functor_typeclass<std::vector<int> >;

    std::vector<int> input{1, 2, 3};
    auto mapped = functor.fmap([](int x) { return x * x; }, input);
    CHECK(mapped == (std::vector<int>{1, 4, 9}));

    std::vector<int> empty_input{};
    auto empty_mapped = functor.fmap([](int x) { return x + 1; }, empty_input);
    CHECK(empty_mapped.empty());
}

TEST_CASE("FunctorTypeclassTest - BemanOptionalBreathing")
{
    beman::optional::optional<int> value{5};
    const auto& functor = smd::functor_typeclass<beman::optional::optional<int> >;
    auto mapped = functor.fmap([](int x) { return x + 2; }, value);

    REQUIRE(mapped.has_value());
    CHECK(*mapped == 7);
}

TEST_CASE("FunctorTypeclassTest - BemanOptionalFmapShortCircuit")
{
    beman::optional::optional<int> empty{};
    const auto& functor = smd::functor_typeclass<beman::optional::optional<int> >;

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

TEST_CASE("FunctorTypeclassTest - ReplaceOptionalAndBemanOptional")
{
    const auto& optional_functor = smd::functor_typeclass<std::optional<int> >;
    auto replaced_present = optional_functor.replace(std::optional<int>{1}, 42);
    REQUIRE(replaced_present.has_value());
    CHECK(*replaced_present == 42);

    auto replaced_empty = optional_functor.replace(std::optional<int>{}, 42);
    CHECK_FALSE(replaced_empty.has_value());

    const auto& beman_functor = smd::functor_typeclass<beman::optional::optional<int> >;
    auto beman_replaced_present = beman_functor.replace(beman::optional::optional<int>{2}, 99);
    REQUIRE(beman_replaced_present.has_value());
    CHECK(*beman_replaced_present == 99);

    auto beman_replaced_empty = beman_functor.replace(beman::optional::optional<int>{}, 99);
    CHECK_FALSE(beman_replaced_empty.has_value());
}

TEST_CASE("FunctorLaws - IdentityLaw")
{
    // fmap(id, x) == x for all instances and shapes
    auto id = [](int x) { return x; };

    // d8b6e1f2-7a3c-4d5e-b2a8-3f4c1d9e5b65
    {
        const auto& functor = smd::functor_typeclass<std::optional<int> >;
        CHECK(functor.fmap(id, std::optional<int>{42}) == std::optional<int>{42});
        CHECK(functor.fmap(id, std::optional<int>{}) == std::optional<int>{});
    }
    // d8b6e1f2-7a3c-4d5e-b2a8-3f4c1d9e5b65 end
    {
        const auto& functor = smd::functor_typeclass<beman::optional::optional<int> >;
        const beman::optional::optional<int> present{7};
        const beman::optional::optional<int> empty{};
        CHECK(functor.fmap(id, present) == present);
        CHECK(functor.fmap(id, empty) == empty);
    }
    {
        const auto& functor = smd::functor_typeclass<std::vector<int> >;
        const std::vector<int> v{1, 2, 3};
        CHECK(functor.fmap(id, v) == v);
        CHECK(functor.fmap(id, std::vector<int>{}) == std::vector<int>{});
    }
}

TEST_CASE("FunctorLaws - CompositionLaw")
{
    // fmap(f ∘ g, x) == fmap(f, fmap(g, x))
    auto g = [](int x) { return x + 1; };
    auto f = [](int x) { return x * 2; };
    auto fog = [](int x) { return (x + 1) * 2; };

    {
        const auto& functor = smd::functor_typeclass<std::optional<int> >;
        const std::optional<int> present{5};
        const std::optional<int> empty{};
        CHECK(functor.fmap(fog, present) == functor.fmap(f, functor.fmap(g, present)));
        CHECK(functor.fmap(fog, empty) == functor.fmap(f, functor.fmap(g, empty)));
    }
    {
        const auto& functor = smd::functor_typeclass<beman::optional::optional<int> >;
        const beman::optional::optional<int> present{5};
        const beman::optional::optional<int> empty{};
        CHECK(functor.fmap(fog, present) == functor.fmap(f, functor.fmap(g, present)));
        CHECK(functor.fmap(fog, empty) == functor.fmap(f, functor.fmap(g, empty)));
    }
    {
        const auto& functor = smd::functor_typeclass<std::vector<int> >;
        const std::vector<int> v{1, 2, 3};
        CHECK(functor.fmap(fog, v) == functor.fmap(f, functor.fmap(g, v)));
        CHECK(functor.fmap(fog, std::vector<int>{}) ==
              functor.fmap(f, functor.fmap(g, std::vector<int>{})));
    }
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
// - identity and combine must stay coherent for a single associative law domain.
// - Prefer adding new Monoid<T> specializations over ad hoc free functions.

template <class VALUE_TYPE>
struct Monoid;

template <class VALUE_TYPE>
inline constexpr Monoid<VALUE_TYPE> monoid_v = Monoid<VALUE_TYPE>{};

struct Count {
    std::size_t d_value;

    friend constexpr bool operator==(const Count& lhs, const Count& rhs) = default;
};

// c3a1e0f8-6b5d-4c2a-a8e3-3d7b9f4a1c06
template <>
struct Monoid<Count> {
    constexpr auto identity() const -> Count { return Count{0}; }

    constexpr auto combine(const Count& lhs, const Count& rhs) const -> Count
    {
        return Count{lhs.d_value + rhs.d_value};
    }
};
// c3a1e0f8-6b5d-4c2a-a8e3-3d7b9f4a1c06 end

template <>
struct Monoid<int> {
    constexpr auto identity() const -> int { return 0; }

    constexpr auto combine(int lhs, int rhs) const -> int { return lhs + rhs; }
};

template <>
struct Monoid<long> {
    constexpr auto identity() const -> long { return 0L; }

    constexpr auto combine(long lhs, long rhs) const -> long
    {
        return lhs + rhs;
    }
};

template <>
struct Monoid<std::size_t> {
    constexpr auto identity() const -> std::size_t { return 0U; }

    constexpr auto combine(std::size_t lhs, std::size_t rhs) const -> std::size_t
    {
        return lhs + rhs;
    }
};

template <>
struct Monoid<std::string> {
    auto identity() const -> std::string { return {}; }

    auto combine(const std::string& lhs, const std::string& rhs) const
        -> std::string
    {
        return lhs + rhs;
    }
};

template <class VALUE_TYPE>
struct Monoid<std::vector<VALUE_TYPE> > {
    auto identity() const -> std::vector<VALUE_TYPE> { return {}; }

    auto combine(std::vector<VALUE_TYPE> lhs,
                 const std::vector<VALUE_TYPE>& rhs) const
        -> std::vector<VALUE_TYPE>
    {
        lhs.insert(lhs.end(), rhs.begin(), rhs.end());
        return lhs;
    }
};

}  // close namespace smd::typeclass

namespace smd {

// b5f3d1a9-7c4e-4b2f-9a5d-6e3c7b8d4f02
template <class VALUE_TYPE>
auto monoid_identity() -> VALUE_TYPE
{
    return typeclass::monoid_v<VALUE_TYPE>.identity();
}

template <class VALUE_TYPE>
auto monoid_combine(const VALUE_TYPE& lhs, const VALUE_TYPE& rhs) -> VALUE_TYPE
{
    return typeclass::monoid_v<VALUE_TYPE>.combine(lhs, rhs);
}
// b5f3d1a9-7c4e-4b2f-9a5d-6e3c7b8d4f02 end

}  // close namespace smd

#endif  // INCLUDED_SMD_TYPECLASS_MONOID

```

## smd/typeclass/monoid.t.cpp

```cpp
#include <smd/typeclass/monoid.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <vector>

TEST_CASE("MonoidTypeclassTest - CountBreathing")
{
    // a1d6e3f7-3c2b-4a8e-b4f1-7c5d3a9e6b84
    const smd::typeclass::Count one{1};
    const smd::typeclass::Count two{2};

    const auto result = smd::monoid_combine(one, two);
    CHECK(result.d_value == 3U);
    // a1d6e3f7-3c2b-4a8e-b4f1-7c5d3a9e6b84 end
}

TEST_CASE("MonoidTypeclassTest - StringCombine")
{
    const auto joined = smd::monoid_combine(std::string{"hello"}, std::string{" world"});
    CHECK(joined == "hello world");
}

TEST_CASE("MonoidTypeclassTest - VectorCombine")
{
    const auto joined = smd::monoid_combine(std::vector<int>{1, 2}, std::vector<int>{3});
    CHECK(joined == (std::vector<int>{1, 2, 3}));
}

TEST_CASE("MonoidLaws - IdentityElement")
{
    // f3b4e6a2-1c7d-4e5b-8a3f-2d9c5b8e3f36
    {
        const auto& m = smd::typeclass::monoid_v<int>;
        CHECK(m.combine(m.identity(), 42) == 42);
        CHECK(m.combine(42, m.identity()) == 42);
    }
    // f3b4e6a2-1c7d-4e5b-8a3f-2d9c5b8e3f36 end
    {
        const auto& m = smd::typeclass::monoid_v<std::string>;
        CHECK(m.combine(m.identity(), std::string{"hello"}) == "hello");
        CHECK(m.combine(std::string{"hello"}, m.identity()) == "hello");
    }
    {
        const auto& m = smd::typeclass::monoid_v<std::vector<int>>;
        const std::vector<int> v{1, 2, 3};
        CHECK(m.combine(m.identity(), v) == v);
        CHECK(m.combine(v, m.identity()) == v);
    }
    {
        const auto& m = smd::typeclass::monoid_v<smd::typeclass::Count>;
        CHECK(m.combine(m.identity(), smd::typeclass::Count{5}) == smd::typeclass::Count{5});
        CHECK(m.combine(smd::typeclass::Count{5}, m.identity()) == smd::typeclass::Count{5});
    }
}

TEST_CASE("MonoidLaws - Associativity")
{
    {
        const auto& m = smd::typeclass::monoid_v<int>;
        CHECK(m.combine(m.combine(1, 2), 3) == m.combine(1, m.combine(2, 3)));
        CHECK(m.combine(m.combine(-5, 10), -3) == m.combine(-5, m.combine(10, -3)));
    }
    {
        const auto& m = smd::typeclass::monoid_v<std::string>;
        CHECK(m.combine(m.combine(std::string{"foo"}, std::string{"bar"}), std::string{"baz"}) ==
              m.combine(std::string{"foo"}, m.combine(std::string{"bar"}, std::string{"baz"})));
    }
    {
        const auto& m = smd::typeclass::monoid_v<std::vector<int>>;
        const std::vector<int> a{1, 2};
        const std::vector<int> b{3, 4};
        const std::vector<int> c{5, 6};
        CHECK(m.combine(m.combine(a, b), c) == m.combine(a, m.combine(b, c)));
    }
}

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
// - Dispatch happens through a provided object or traversable_typeclass<Concrete>.
// - Traversal must preserve container shape while sequencing effects.

template <class Impl>
struct Traversable : protected Impl {
    using Impl::traverse;
    using element_type = typename Impl::element_type;

    // 8f1d5c4a-1a7e-4b9e-8cb4-908f4ab0ca11

    // d5a2c1f8-7e3b-4d1a-c6b2-2f9e5d7a1c46
    template <class T, class F>
    auto for_each(this auto&& self, T&& value, F&& function)
    {
        using Context = remove_cvref_t<std::invoke_result_t<F, const element_type&>>;
        const auto& applicative = smd::applicative_typeclass<Context>;
        return self.traverse(applicative, std::forward<F>(function),
                             std::forward<T>(value));
    }
    // d5a2c1f8-7e3b-4d1a-c6b2-2f9e5d7a1c46 end

    // c1f8e7a2-9b6d-4c4f-a5e3-1b2d9c8f6a79
    template <class T>
    auto sequence(this auto&& self, T&& value)
    {
        using Context = element_type;
        const auto& applicative = smd::applicative_typeclass<Context>;
        return self.traverse(
            applicative,
            [](auto&& x) { return std::forward<decltype(x)>(x); },
            std::forward<T>(value));
    }
    // c1f8e7a2-9b6d-4c4f-a5e3-1b2d9c8f6a79 end

    template <class TRAVERSABLE_MAP, class T, class F>
    auto traverse_with(this auto&&,
                       const TRAVERSABLE_MAP& traversable_map,
                       F&& function,
                       T&& value)
    {
        using Context = remove_cvref_t<std::invoke_result_t<
            F, const typename remove_cvref_t<TRAVERSABLE_MAP>::element_type&>>;
        const auto& applicative = smd::applicative_typeclass<Context>;
        return traversable_map.traverse(
            applicative, std::forward<F>(function), std::forward<T>(value));
    }

    template <class TRAVERSABLE_MAP, class APPLICATIVE_MAP, class T, class F>
    auto traverse_with(this auto&&,
                       const TRAVERSABLE_MAP& traversable_map,
                       const APPLICATIVE_MAP& applicative_map,
                       F&& function,
                       T&& value)
    {
        return traversable_map.traverse(
            applicative_map, std::forward<F>(function), std::forward<T>(value));
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
    // 8f1d5c4a-1a7e-4b9e-8cb4-908f4ab0ca11 end
};

template <class T>
inline constexpr auto traversable_typeclass = std::false_type{};

template <class F, class T>
auto traverse(F&& function, T&& value)
{
    const auto& map = traversable_typeclass<remove_cvref_t<T>>;
    using element_type = typename remove_cvref_t<decltype(map)>::element_type;
    using Context     = remove_cvref_t<std::invoke_result_t<F, const element_type&>>;
    const auto& applicative = applicative_typeclass<Context>;
    return map.traverse(
        applicative, std::forward<F>(function), std::forward<T>(value));
}

}  // close namespace smd

#endif

```

## smd/typeclass/traversable.t.cpp

```cpp
#include <smd/typeclass/test/test_support.hpp>
#include <smd/typeclass/traversable.hpp>

#include <catch2/catch_test_macros.hpp>

#include <beman/optional/optional.hpp>

#include <optional>

TEST_CASE("TraversableTypeclassTest - TraverseOptionalSuccess")
{
    using Identity = smd::typeclass::test::Identity<int>;
    auto identity = Identity{1};
    const auto& traversable = smd::traversable_typeclass<Identity>;

    auto traversed = smd::traverse(
        [](int x) -> std::optional<int> {
            return x >= 0 ? std::optional<int>{x + 1} : std::optional<int>{};
        },
        identity);

    REQUIRE(traversed.has_value());
    CHECK(traversed->value == 2);
}

TEST_CASE("TraversableTypeclassTest - TraverseOptionalFailure")
{
    using Identity = smd::typeclass::test::Identity<int>;
    auto identity = Identity{-2};
    const auto& traversable = smd::traversable_typeclass<Identity>;

    auto traversed = smd::traverse(
        [](int x) -> std::optional<int> {
            return x >= 0 ? std::optional<int>{x + 1} : std::optional<int>{};
        },
        identity);

    CHECK_FALSE(traversed.has_value());
}

TEST_CASE("TraversableTypeclassTest - ForEachOptionalSuccess")
{
    using Identity = smd::typeclass::test::Identity<int>;
    auto identity = Identity{3};
    const auto& traversable = smd::traversable_typeclass<Identity>;

    auto traversed = traversable.for_each(identity, [](int x) -> std::optional<int> {
        return std::optional<int>{x * 2};
    });

    REQUIRE(traversed.has_value());
    CHECK(traversed->value == 6);
}

TEST_CASE("TraversableTypeclassTest - SequenceAndSequenceWith")
{
    // f1de12e0-2287-4568-98c7-75be4f6f7446
    // e7b4a1f9-3c8d-4e2a-b5f7-1d9c3e5a7b28
    using IdentityOpt = smd::typeclass::test::Identity<std::optional<int> >;
    auto identity = IdentityOpt{std::optional<int>{1}};
    const auto& traversable = smd::traversable_typeclass<IdentityOpt>;

    auto sequenced = traversable.sequence(identity);
    REQUIRE(sequenced.has_value());
    CHECK(sequenced->value == 1);
    // e7b4a1f9-3c8d-4e2a-b5f7-1d9c3e5a7b28 end

    auto sequenced_with = traversable.sequence_with(traversable, identity);
    REQUIRE(sequenced_with.has_value());
    CHECK(sequenced_with->value == 1);
    // f1de12e0-2287-4568-98c7-75be4f6f7446 end
}

TEST_CASE("TraversableTypeclassTest - ForEachMatchesTraverse")
{
    using Identity = smd::typeclass::test::Identity<int>;
    auto identity = Identity{4};
    const auto& traversable = smd::traversable_typeclass<Identity>;

    auto via_traverse = smd::traverse(
        [](int x) -> std::optional<int> { return std::optional<int>{x + 7}; },
        identity);
    auto via_for_each = traversable.for_each(
        identity,
        [](int x) -> std::optional<int> { return std::optional<int>{x + 7}; });

    CHECK(via_traverse == via_for_each);
}

TEST_CASE("TraversableTypeclassTest - SequenceMatchesTraverseIdentity")
{
    using IdentityOpt = smd::typeclass::test::Identity<std::optional<int> >;
    auto identity = IdentityOpt{std::optional<int>{5}};
    const auto& traversable = smd::traversable_typeclass<IdentityOpt>;

    auto via_sequence = traversable.sequence(identity);
    auto via_traverse_identity = smd::traverse(
        [](auto&& x) { return std::forward<decltype(x)>(x); },
        identity);

    CHECK(via_sequence == via_traverse_identity);
}

TEST_CASE("TraversableTypeclassTest - IdentityLawWithIdentityApplicative")
{
    using Identity = smd::typeclass::test::Identity<int>;
    const auto& traversable = smd::traversable_typeclass<Identity>;
    const auto& applicative = smd::applicative_typeclass<Identity>;

    auto value = Identity{42};

    auto lhs = smd::traverse(
        [](int x) { return applicative.pure(x); },
        value);
    auto rhs = applicative.pure(value);

    CHECK(lhs == rhs);
}

TEST_CASE("TraversableTypeclassTest - TraverseMapCoherence")
{
    using Identity = smd::typeclass::test::Identity<int>;
    const auto& traversable = smd::traversable_typeclass<Identity>;

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
    auto mapped = std::optional<smd::typeclass::test::Identity<int> >{
        smd::typeclass::test::Identity<int>{via_traverse->value * 3}};

    CHECK(mapped == via_mapped_traverse);
}

TEST_CASE("TraversableTypeclassTest - CompositionLawViaNestedOptional")
{
    using Identity = smd::typeclass::test::Identity<int>;
    const auto& traversable = smd::traversable_typeclass<Identity>;

    auto value = Identity{9};

    auto f = [](int x) -> std::optional<int> {
        return x >= 0 ? std::optional<int>{x + 2} : std::optional<int>{};
    };
    auto g = [](int x) -> std::optional<int> {
        return x % 2 == 0 ? std::optional<int>{x / 2} : std::optional<int>{};
    };

    auto lhs = smd::traverse(
        [&](int x) -> std::optional<std::optional<int> > {
            auto fx = f(x);
            if (!fx.has_value()) {
                return std::optional<std::optional<int> >{std::optional<int>{}};
            }
            return std::optional<std::optional<int> >{g(*fx)};
        },
        value);

    auto rhs = [&]() -> std::optional<std::optional<Identity> > {
        auto traversed_once = smd::traverse(f, value);
        if (!traversed_once.has_value()) {
            return std::optional<std::optional<Identity> >{std::optional<Identity>{}};
        }

        auto traversed_twice = smd::traverse(g, *traversed_once);
        return std::optional<std::optional<Identity> >{traversed_twice};
    }();

    auto unwrap_identity = [](const std::optional<std::optional<Identity> >& nested)
        -> std::optional<std::optional<int> > {
        if (!nested.has_value()) {
            return std::optional<std::optional<int> >{};
        }
        if (!nested->has_value()) {
            return std::optional<std::optional<int> >{std::optional<int>{}};
        }
        return std::optional<std::optional<int> >{std::optional<int>{nested->value().value}};
    };

    auto unwrap_traversed = [](const std::optional<smd::typeclass::test::Identity<std::optional<int> > >& traversed)
        -> std::optional<std::optional<int> > {
        if (!traversed.has_value()) {
            return std::optional<std::optional<int> >{};
        }
        return std::optional<std::optional<int> >{traversed->value};
    };

    CHECK(unwrap_traversed(lhs) == unwrap_identity(rhs));
}

TEST_CASE("TraversableTypeclassTest - NaturalityLawWithOptional")
{
    using Identity = smd::typeclass::test::Identity<int>;
    const auto& traversable = smd::traversable_typeclass<Identity>;

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

TEST_CASE("TraversableLaws - NaturalityLaw")
{
    // Naturality law: an applicative morphism commutes with traverse.
    // to_beman: std::optional<B> → beman::optional<B> is one such morphism.
    // Law: to_beman(traverse f t) == traverse (f_returning_beman) t
    using Identity = smd::typeclass::test::Identity<int>;
    const auto& traversable = smd::traversable_typeclass<Identity>;

    auto f = [](int x) -> std::optional<int> {
        return x > 0 ? std::optional<int>{x * 2} : std::optional<int>{};
    };
    auto f_returning_beman = [](int x) -> beman::optional::optional<int> {
        return x > 0 ? beman::optional::optional<int>{x * 2}
                     : beman::optional::optional<int>{};
    };
    auto to_beman = [](std::optional<Identity> o) -> beman::optional::optional<Identity> {
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

template <class T, class = void>
struct applicative_value;

template <class T>
struct applicative_value<T, std::void_t<typename remove_cvref_t<T>::value_type> > {
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

template <class T>
using applicative_value_t = typename applicative_value<remove_cvref_t<T> >::type;

}  // close namespace smd

#endif  // INCLUDED_SMD_TYPECLASS_TYPECLASS_BASE

```

## smd/typeclass/typeclass_base.t.cpp

```cpp
#include <smd/typeclass/typeclass_base.hpp>

#include <catch2/catch_test_macros.hpp>

#include <type_traits>

TEST_CASE("TypeclassBaseTest - RemoveCvrefAlias")
{
    static_assert(std::is_same_v<smd::remove_cvref_t<const int&>, int>);
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
auto zip_list_finite_length(const zip_list<T>& list) -> std::optional<std::size_t>
{
  if (list.is_repeating()) {
    return std::nullopt;
  }
  return list.finite_size();
}

template <class T>
auto zip_list_value_at(const zip_list<T>& list, std::size_t index) -> const T&
{
  if (list.is_repeating()) {
    return *list.repeated;
  }
  return list.data[index];
}

template <class FIRST, class... REST>
auto zip_list_result_size(const FIRST& first, const REST&... rest)
  -> std::optional<std::size_t>
{
  auto count = zip_list_finite_length(first);
  ((count = count
         ? std::optional<std::size_t>{std::min(*count, zip_list_finite_length(rest).value_or(*count))}
         : zip_list_finite_length(rest)),
   ...);
  return count;
}

}  // namespace detail

template <class T>
struct ZipListApplicativeImpl {
  template <class VALUE>
  auto pure(this auto&&, VALUE&& value)
  {
    using U = remove_cvref_t<VALUE>;
    return zip_list<U>::repeat(U(std::forward<VALUE>(value)));
  }

  template <class F, class A>
  auto apply(this auto&&, const zip_list<F>& functions, const zip_list<A>& arguments)
  {
    using Result = std::invoke_result_t<const F&, const A&>;
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

  template <class FUNCTION, class FIRST, class... REST>
  auto invoke(this auto&&,
              FUNCTION&& function,
              const FIRST& first,
              const REST&... rest)
  {
    using Result = std::invoke_result_t<
      FUNCTION,
      const typename FIRST::value_type&,
      const typename REST::value_type&...>;

    using U = remove_cvref_t<Result>;
    auto callable = std::forward<FUNCTION>(function);
    const auto count = detail::zip_list_result_size(first, rest...);

    if (!count.has_value()) {
      return zip_list<U>::repeat(
        std::invoke(callable,
                    detail::zip_list_value_at(first, 0),
                    detail::zip_list_value_at(rest, 0)...));
    }

    zip_list<U> result;
    result.data.reserve(*count);

    for (std::size_t index = 0; index < *count; ++index) {
      result.data.push_back(
        std::invoke(callable,
                    detail::zip_list_value_at(first, index),
                    detail::zip_list_value_at(rest, index)...));
    }

    return result;
  }
};

template <class T>
struct ZipListApplicativeMap : Applicative<ZipListApplicativeImpl<T> > {
  using ZipListApplicativeImpl<T>::apply;
  using ZipListApplicativeImpl<T>::pure;
};

template <class T>
inline constexpr auto applicative_typeclass<zip_list<T> > =
  ZipListApplicativeMap<T>{};

}  // close namespace smd

#endif

```

## smd/ziplist/zip_list_applicative.t.cpp

```cpp
#include <smd/typeclass/applicative.hpp>
#include <smd/typeclass/test/test_support.hpp>
#include <smd/ziplist/zip_list.hpp>
#include <smd/ziplist/zip_list_applicative.hpp>

#include <catch2/catch_test_macros.hpp>

#include <functional>
#include <vector>

TEST_CASE("ZipListApplicativeTest - PureBreathing")
{
    const auto& applicative = smd::applicative_typeclass<smd::zip_list<int> >;
    auto lifted = applicative.pure(9);

    CHECK(lifted.is_repeating());
    REQUIRE(lifted.repeated.has_value());
    CHECK(*lifted.repeated == 9);
}

TEST_CASE("ZipListApplicativeTest - ApplyZips")
{
    smd::zip_list<int (*)(int)> functions{{
        +[](int x) { return x + 1; },
        +[](int x) { return x * 2; },
        +[](int x) { return x - 3; },
    }};
    smd::zip_list<int> arguments{{10, 10}};
    const auto& applicative = smd::applicative_typeclass<smd::zip_list<int (*)(int)> >;

    auto result = applicative.apply(functions, arguments);
    CHECK(result.data == (std::vector<int>{11, 20}));
}

TEST_CASE("ZipListApplicativeTest - PureBroadcastsAcrossFiniteInput")
{
    const auto& applicative = smd::applicative_typeclass<smd::zip_list<int> >;
    smd::zip_list<int> xs{{1, 2, 3}};

    auto result = applicative.ap(
        applicative.pure(+[](int x) { return x + 10; }),
        xs);

    CHECK(result.data == (std::vector<int>{11, 12, 13}));
}

TEST_CASE("ZipListApplicativeTest - IdentityLawOnFiniteInput")
{
    const auto& applicative = smd::applicative_typeclass<smd::zip_list<int> >;
    smd::zip_list<int> xs{{4, 5, 6}};

    auto result = applicative.ap(
        applicative.pure(+[](int x) { return x; }),
        xs);

    CHECK(result.data == xs.data);
}

TEST_CASE("ZipListApplicativeTest - BothPureProducesRepeatingResult")
{
    const auto& applicative = smd::applicative_typeclass<smd::zip_list<int> >;

    auto result = applicative.ap(
        applicative.pure(+[](int x) { return x * 2; }),
        applicative.pure(7));

    CHECK(result.is_repeating());
    REQUIRE(result.repeated.has_value());
    CHECK(*result.repeated == 14);
}

TEST_CASE("ZipListApplicativeTest - InvokeZipsMultipleArguments")
{
    smd::zip_list<int> xs{{1, 2, 3}};
    smd::zip_list<int> ys{{10, 20}};
    smd::zip_list<int> zs{{100, 200, 300, 400}};
    const auto& applicative = smd::applicative_typeclass<smd::zip_list<int> >;

    auto result = applicative.invoke(
        [](int x, int y, int z) { return x + y + z; },
        xs,
        ys,
        zs);

    CHECK(result.data == (std::vector<int>{111, 222}));
}

TEST_CASE("ZipListApplicativeTest - InvokeWithPureAndFiniteArguments")
{
    const auto& applicative = smd::applicative_typeclass<smd::zip_list<int> >;
    smd::zip_list<int> ys{{10, 20, 30}};

    auto result = applicative.invoke(
        [](int x, int y, int z) { return x + y + z; },
        applicative.pure(1),
        ys,
        applicative.pure(100));

    CHECK(result.data == (std::vector<int>{111, 121, 131}));
}

TEST_CASE("ZipListApplicativeTest - InterchangeLaw")
{
    const auto& applicative = smd::applicative_typeclass<smd::zip_list<int> >;

    smd::zip_list<std::function<int(int)> > functions{{
        [](int x) { return x + 1; },
        [](int x) { return x * 3; },
        [](int x) { return x - 2; },
    }};
    const int value = 7;

    auto lhs = applicative.ap(functions, applicative.pure(value));
    auto rhs = applicative.ap(
        applicative.pure([](const std::function<int(int)>& function) {
            return function(value);
        }),
        functions);

    CHECK(lhs.data == rhs.data);
}

TEST_CASE("ZipListApplicativeTest - CompositionLaw")
{
    const auto& applicative = smd::applicative_typeclass<smd::zip_list<int> >;

    smd::zip_list<std::function<int(int)> > u{{
        [](int x) { return x + 10; },
        [](int x) { return x * 2; },
    }};
    smd::zip_list<std::function<int(int)> > v{{
        [](int x) { return x - 3; },
        [](int x) { return x + 4; },
    }};
    smd::zip_list<int> w{{5, 6, 7}};

    auto compose = [](const std::function<int(int)>& f) {
        return [f](const std::function<int(int)>& g) {
            return [f, g](int x) { return f(g(x)); };
        };
    };

    auto lhs = applicative.ap(
        applicative.ap(
            applicative.ap(applicative.pure(compose), u),
            v),
        w);

    auto rhs = applicative.ap(u, applicative.ap(v, w));

    CHECK(lhs.data == rhs.data);
}

TEST_CASE("ZipListApplicativeTest - IdentityHomomorphismAndInvokeViaHarness")
{
    CHECK(smd::typeclass::test::check_applicative_identity_law(
        smd::zip_list<int>{{4, 5, 6}}));
    CHECK(smd::typeclass::test::check_applicative_homomorphism_law<smd::zip_list<int> >(
        +[](int x) { return x + 9; },
        3));
    CHECK(smd::typeclass::test::check_applicative_invoke_binary_law(
        [](int a, int b) { return a * 10 + b; },
        smd::zip_list<int>{{1, 2, 3}},
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

template <class T>
struct zip_list {
  using value_type = T;

  // Invariant: when repeated has a value, this zip_list models an infinite
  // repetition of that value and data is ignored.
  std::vector<T> data;
  std::optional<T> repeated{};

  static auto repeat(T value) -> zip_list
  {
    return zip_list{{}, std::move(value)};
  }

  auto is_repeating() const -> bool { return repeated.has_value(); }

  auto finite_size() const -> std::size_t { return data.size(); }

  friend auto operator==(const zip_list& left, const zip_list& right) -> bool
  {
    if (left.is_repeating() || right.is_repeating()) {
      return left.repeated == right.repeated;
    }
    return left.data == right.data;
  }
};

}  // close namespace smd

#endif

```

