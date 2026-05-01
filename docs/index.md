- [Overview](#orgb614887)
- [Active Source Files](#orgd7cfc91)
  - [examples/cpo<sub>example.cpp</sub>](#orgfca1763)
  - [examples/main.cpp](#org141faf3)
  - [examples/map<sub>example.cpp</sub>](#org62d326b)
  - [smd/ranges/range<sub>applicative.hpp</sub>](#orgad0d5e1)
  - [smd/ranges/range<sub>foldable.hpp</sub>](#org03925a3)
  - [smd/ranges/range<sub>functor.hpp</sub>](#orgafda85e)
  - [smd/ranges/range<sub>list.hpp</sub>](#org41fda00)
  - [smd/ranges/range<sub>traversable.hpp</sub>](#org9fbc0f6)
  - [smd/thunk/delay.hpp](#orga5e3dcf)
  - [smd/thunk/memoize.hpp](#org0a3c9a6)
  - [smd/tree/binary<sub>tree</sub><sub>applicative.hpp</sub>](#orge4257c0)
  - [smd/tree/binary<sub>tree</sub><sub>foldable.hpp</sub>](#org0e755ae)
  - [smd/tree/binary<sub>tree.hpp</sub>](#orge8034d2)
  - [smd/tree/binary<sub>tree</sub><sub>traversable.hpp</sub>](#org2edaacd)
  - [smd/tree/finger<sub>tree</sub><sub>foldable.hpp</sub>](#org45924aa)
  - [smd/tree/finger<sub>tree.hpp</sub>](#orgc8adb1f)
  - [smd/tree/finger<sub>tree</sub><sub>interval</sub><sub>index.hpp</sub>](#org2ffd65e)
  - [smd/tree/finger<sub>tree</sub><sub>priority</sub><sub>queue.hpp</sub>](#org0db261f)
  - [smd/tree/finger<sub>tree</sub><sub>random</sub><sub>access.hpp</sub>](#org4981d68)
  - [smd/tree/finger<sub>tree</sub><sub>rope.hpp</sub>](#org413b5a9)
  - [smd/tree/finger<sub>tree</sub><sub>traversable.hpp</sub>](#orgb3e7bbb)
  - [smd/tree/finger<sub>tree</sub><sub>wrappers.hpp</sub>](#orgf67c401)
  - [smd/tree/fix<sub>tree</sub><sub>applicative.hpp</sub>](#org5b2110f)
  - [smd/tree/fix<sub>tree</sub><sub>foldable.hpp</sub>](#org45c3846)
  - [smd/tree/fix<sub>tree.hpp</sub>](#org232c2a2)
  - [smd/tree/fix<sub>tree</sub><sub>traversable.hpp</sub>](#orgcd6a700)
  - [smd/tree/fringe<sub>tree</sub><sub>applicative.hpp</sub>](#org54ebcb5)
  - [smd/tree/fringe<sub>tree</sub><sub>foldable.hpp</sub>](#org4862bec)
  - [smd/tree/fringe<sub>tree.hpp</sub>](#org9fd9e3f)
  - [smd/tree/fringe<sub>tree</sub><sub>traversable.hpp</sub>](#orgf236ef2)
  - [smd/tree/memoized<sub>thunk.hpp</sub>](#org8b85e4c)
  - [smd/typeclass/applicative.hpp](#orgaff5802)
  - [smd/typeclass/examples/applicative<sub>bad.cpp</sub>](#org7772d2c)
  - [smd/typeclass/examples/applicative<sub>examples.cpp</sub>](#orgb401833)
  - [smd/typeclass/examples/foldable<sub>examples.cpp</sub>](#org7f4af1e)
  - [smd/typeclass/examples/lookup<sub>modes</sub><sub>examples.cpp</sub>](#org5f424cd)
  - [smd/typeclass/examples/traversable<sub>examples.cpp</sub>](#org98d8729)
  - [smd/typeclass/foldable.hpp](#org5159ca4)
  - [smd/typeclass/functor.hpp](#orge937553)
  - [smd/typeclass/monoid.hpp](#orga2c4273)
  - [smd/typeclass/traversable.hpp](#org6540d8d)
  - [smd/typeclass/typeclass<sub>base.hpp</sub>](#org11fad53)
  - [smd/ziplist/zip<sub>list</sub><sub>applicative.hpp</sub>](#org667c4de)
  - [smd/ziplist/zip<sub>list.hpp</sub>](#orgd23e2b5)
- [Regeneration](#org5c00dee)



<a id="orgb614887"></a>

# Overview

This index transcludes active files under `trees/src` that are explicitly listed in CMake `target_sources()` entries. It excludes `deadcode` and `smd/conceptmap`.


<a id="orgd7cfc91"></a>

# Active Source Files


<a id="orgfca1763"></a>

## examples/cpo<sub>example.cpp</sub>

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


<a id="org141faf3"></a>

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


<a id="org62d326b"></a>

## examples/map<sub>example.cpp</sub>

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


<a id="orgad0d5e1"></a>

## smd/ranges/range<sub>applicative.hpp</sub>

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


<a id="org03925a3"></a>

## smd/ranges/range<sub>foldable.hpp</sub>

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


<a id="orgafda85e"></a>

## smd/ranges/range<sub>functor.hpp</sub>

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


<a id="org41fda00"></a>

## smd/ranges/range<sub>list.hpp</sub>

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


<a id="org9fbc0f6"></a>

## smd/ranges/range<sub>traversable.hpp</sub>

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


<a id="orga5e3dcf"></a>

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

// delay(callable, args...) — capture a callable and its arguments into a
// closure that invokes them on demand.  The closure is not memoized; each
// call re-invokes callable.  Use smd::thunk::memoize() for call-once
// semantics.
template <typename Callable, typename... Args>
auto delay(Callable&& c, Args&&... args)
{
  using CallableT = std::remove_cvref_t<Callable>;
  using ArgsTuple = std::tuple<std::remove_cvref_t<Args>...>;

  return [callable = CallableT(std::forward<Callable>(c)),
          arguments = ArgsTuple(std::forward<Args>(args)...)]() mutable {
    return std::apply(
      [&](auto&... unpacked) { return std::invoke(callable, unpacked...); },
      arguments);
  };
}

}  // namespace smd::thunk

#endif
```


<a id="org0a3c9a6"></a>

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

// erased_thunk<Result> — type-erased wrapper for any callable returning Result.
// Copies share the same underlying callable via shared_ptr.
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

    auto invoke() -> const Result& override { return d_callable(); }
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

// memoize(callable, args...) — returns a callable that evaluates
// delay(callable, args...) exactly once and caches the result.  All copies
// of the returned callable share the same cached value via shared_ptr.
//
// Not thread-safe: concurrent first calls race on the variant mutation.
// See thread-safety note at the top of this file.
template <typename Callable, typename... Args>
auto memoize(Callable&& c, Args&&... args)
{
  using Closure = decltype(delay(std::forward<Callable>(c), std::forward<Args>(args)...));
  using Result  = std::invoke_result_t<Closure&>;
  using State   = std::variant<std::monostate, Closure, Result>;

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

// measured_memoize(measure, callable, args...) — like memoize(), but attaches
// a pre-computed Measure alongside the deferred result.  Returns a struct with
// cached_measure() and force() accessors.
template <typename Measure, typename Callable, typename... Args>
auto measured_memoize(Measure measure, Callable&& c, Args&&... args)
{
  auto deferred = memoize(std::forward<Callable>(c), std::forward<Args>(args)...);

  return [measure = std::move(measure), deferred = std::move(deferred)]() mutable {
    struct MeasuredAccess {
      Measure                   d_measure;
      mutable decltype(deferred) d_force;

      [[nodiscard]] auto cached_measure() const -> const Measure& { return d_measure; }
      [[nodiscard]] auto force() const -> decltype(auto) { return d_force(); }
    };
    return MeasuredAccess{std::move(measure), std::move(deferred)};
  }();
}

}  // namespace smd::thunk

#endif
```


<a id="orge4257c0"></a>

## smd/tree/binary<sub>tree</sub><sub>applicative.hpp</sub>

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


<a id="org0e755ae"></a>

## smd/tree/binary<sub>tree</sub><sub>foldable.hpp</sub>

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


<a id="orge8034d2"></a>

## smd/tree/binary<sub>tree.hpp</sub>

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


<a id="org2edaacd"></a>

## smd/tree/binary<sub>tree</sub><sub>traversable.hpp</sub>

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


<a id="org45924aa"></a>

## smd/tree/finger<sub>tree</sub><sub>foldable.hpp</sub>

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


<a id="orgc8adb1f"></a>

## smd/tree/finger<sub>tree.hpp</sub>

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
struct Four {
  T a;
  T b;
  T c;
  T d;
};

template <typename T>
using Digit = std::variant<One<T>, Two<T>, Three<T>, Four<T>>;

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

template <typename VALUE_TYPE, typename TAG_TYPE>
struct UnitMeasure {
  auto operator()(const VALUE_TYPE&) const -> TAG_TYPE { return TAG_TYPE{1}; }
};

template <typename NODE_T, typename TAG_TYPE>
struct NodeMeasure {
  auto operator()(const NODE_T& node) const -> TAG_TYPE
  {
    return std::visit([](const auto& n) -> TAG_TYPE { return n.d_measure; }, node);
  }
};

namespace detail {

template <typename... Ts>
struct overloaded : Ts... {
  using Ts::operator()...;
};

template <typename... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

}  // namespace detail

template <typename T,
          typename TAG_TYPE = std::size_t,
          typename MEASURE_POLICY = UnitMeasure<T, TAG_TYPE>,
          int DEPTH = 0>
class FingerTree {
  template <typename, typename, typename, int>
  friend class FingerTree;

  static_assert(std::is_default_constructible_v<MEASURE_POLICY>,
                "FingerTree measure policy must be default-constructible");

  static constexpr int kMaxDepth = 10;

  using Tag = TAG_TYPE;
  using MeasurePolicy = MEASURE_POLICY;
  using NodeT = Node<T, Tag>;
  using SpineMeasure = NodeMeasure<NodeT, Tag>;
  struct SpineTerminal {};
  using SpineTree = std::conditional_t<
    (DEPTH < kMaxDepth),
    FingerTree<NodeT, Tag, SpineMeasure, DEPTH + 1>,
    SpineTerminal>;
  using SpinePtr = std::shared_ptr<const SpineTree>;

  // -- Tag operations --

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

  static constexpr auto elem_leaf_count(
    [[maybe_unused]] const T& elem) -> std::size_t
  {
    if constexpr (DEPTH == 0) {
      return 1U;
    } else {
      return std::visit([](const auto& n) { return n.d_leaf_count; }, elem);
    }
  }

  // -- Node construction --

  static auto make_node2(T a, T b) -> NodeT
  {
    auto m = tag_combine(tag_value(a), tag_value(b));
    auto lc = elem_leaf_count(a) + elem_leaf_count(b);
    return Node2<T, Tag>{std::move(m), lc,
                         std::make_shared<const T>(std::move(a)),
                         std::make_shared<const T>(std::move(b))};
  }

  static auto make_node3(T a, T b, T c) -> NodeT
  {
    auto m = tag_combine(tag_combine(tag_value(a), tag_value(b)), tag_value(c));
    auto lc = elem_leaf_count(a) + elem_leaf_count(b) + elem_leaf_count(c);
    return Node3<T, Tag>{std::move(m), lc,
                         std::make_shared<const T>(std::move(a)),
                         std::make_shared<const T>(std::move(b)),
                         std::make_shared<const T>(std::move(c))};
  }

  static auto node_measure(const NodeT& node) -> Tag
  {
    return std::visit([](const auto& n) -> Tag { return n.d_measure; }, node);
  }

  static void node_flatten_into(const NodeT& node, std::vector<T>& out)
  {
    std::visit(detail::overloaded{
      [&](const Node2<T, Tag>& n) {
        out.push_back(*n.a);
        out.push_back(*n.b);
      },
      [&](const Node3<T, Tag>& n) {
        out.push_back(*n.a);
        out.push_back(*n.b);
        out.push_back(*n.c);
      }
    }, node);
  }

  static auto node_to_digit(const NodeT& node) -> Digit<T>
  {
    return std::visit(detail::overloaded{
      [](const Node2<T, Tag>& n) -> Digit<T> { return Two<T>{*n.a, *n.b}; },
      [](const Node3<T, Tag>& n) -> Digit<T> {
        return Three<T>{*n.a, *n.b, *n.c};
      }
    }, node);
  }

  static auto node_to_list(const NodeT& node) -> std::vector<T>
  {
    std::vector<T> result;
    node_flatten_into(node, result);
    return result;
  }

  // -- Digit helpers --

  static auto digit_measure(const Digit<T>& d) -> Tag
  {
    return std::visit(detail::overloaded{
      [](const One<T>& x) { return tag_value(x.a); },
      [](const Two<T>& x) {
        return tag_combine(tag_value(x.a), tag_value(x.b));
      },
      [](const Three<T>& x) {
        return tag_combine(tag_combine(tag_value(x.a), tag_value(x.b)),
                           tag_value(x.c));
      },
      [](const Four<T>& x) {
        return tag_combine(
          tag_combine(tag_combine(tag_value(x.a), tag_value(x.b)),
                      tag_value(x.c)),
          tag_value(x.d));
      }
    }, d);
  }

  static auto digit_leaf_count(const Digit<T>& d) -> std::size_t
  {
    return std::visit(detail::overloaded{
      [](const One<T>& x) { return elem_leaf_count(x.a); },
      [](const Two<T>& x) {
        return elem_leaf_count(x.a) + elem_leaf_count(x.b);
      },
      [](const Three<T>& x) {
        return elem_leaf_count(x.a) + elem_leaf_count(x.b) + elem_leaf_count(x.c);
      },
      [](const Four<T>& x) {
        return elem_leaf_count(x.a) + elem_leaf_count(x.b)
             + elem_leaf_count(x.c) + elem_leaf_count(x.d);
      }
    }, d);
  }

  static void digit_flatten_into(const Digit<T>& d, std::vector<T>& out)
  {
    std::visit(detail::overloaded{
      [&](const One<T>& x) { out.push_back(x.a); },
      [&](const Two<T>& x) { out.push_back(x.a); out.push_back(x.b); },
      [&](const Three<T>& x) {
        out.push_back(x.a); out.push_back(x.b); out.push_back(x.c);
      },
      [&](const Four<T>& x) {
        out.push_back(x.a); out.push_back(x.b);
        out.push_back(x.c); out.push_back(x.d);
      }
    }, d);
  }

  static auto digit_to_list(const Digit<T>& d) -> std::vector<T>
  {
    std::vector<T> result;
    digit_flatten_into(d, result);
    return result;
  }

  static auto digit_head(const Digit<T>& d) -> const T&
  {
    return std::visit([](const auto& x) -> const T& { return x.a; }, d);
  }

  static auto digit_last(const Digit<T>& d) -> const T&
  {
    return std::visit(detail::overloaded{
      [](const One<T>& x) -> const T& { return x.a; },
      [](const Two<T>& x) -> const T& { return x.b; },
      [](const Three<T>& x) -> const T& { return x.c; },
      [](const Four<T>& x) -> const T& { return x.d; }
    }, d);
  }

  static auto digit_tail(const Digit<T>& d) -> std::optional<Digit<T>>
  {
    return std::visit(detail::overloaded{
      [](const One<T>&) -> std::optional<Digit<T>> { return std::nullopt; },
      [](const Two<T>& x) -> std::optional<Digit<T>> { return One<T>{x.b}; },
      [](const Three<T>& x) -> std::optional<Digit<T>> {
        return Two<T>{x.b, x.c};
      },
      [](const Four<T>& x) -> std::optional<Digit<T>> {
        return Three<T>{x.b, x.c, x.d};
      }
    }, d);
  }

  static auto digit_init(const Digit<T>& d) -> std::optional<Digit<T>>
  {
    return std::visit(detail::overloaded{
      [](const One<T>&) -> std::optional<Digit<T>> { return std::nullopt; },
      [](const Two<T>& x) -> std::optional<Digit<T>> { return One<T>{x.a}; },
      [](const Three<T>& x) -> std::optional<Digit<T>> {
        return Two<T>{x.a, x.b};
      },
      [](const Four<T>& x) -> std::optional<Digit<T>> {
        return Three<T>{x.a, x.b, x.c};
      }
    }, d);
  }

  // -- Spine helpers --

  static auto spine_measure(const SpinePtr& sp) -> Tag
  {
    if constexpr (DEPTH < kMaxDepth) {
      if (!sp) return tag_identity();
      return sp->measure();
    } else {
      (void)sp;
      return tag_identity();
    }
  }

  static auto spine_breadth(const SpinePtr& sp) -> std::size_t
  {
    if constexpr (DEPTH < kMaxDepth) {
      if (!sp) return 0U;
      return sp->breadth();
    } else {
      (void)sp;
      return 0U;
    }
  }

  static auto spine_depth(const SpinePtr& sp) -> std::size_t
  {
    if constexpr (DEPTH < kMaxDepth) {
      if (!sp) return 0U;
      return sp->depth();
    } else {
      (void)sp;
      return 0U;
    }
  }

  static auto spine_is_empty(const SpinePtr& sp) -> bool
  {
    if constexpr (DEPTH < kMaxDepth) {
      return !sp || sp->is_empty();
    } else {
      (void)sp;
      return true;
    }
  }

  static auto spine_cons(const SpinePtr& spine, NodeT node) -> SpinePtr
  {
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

  static auto spine_snoc(const SpinePtr& spine, NodeT node) -> SpinePtr
  {
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

  explicit FingerTree(Repr repr)
    : d_repr(std::move(repr))
  {
  }

  // -- Smart constructors --

  static auto make_empty() -> FingerTree { return FingerTree(Repr{Empty{}}); }

  static auto make_single(T value) -> FingerTree
  {
    auto m = tag_value(value);
    return FingerTree(Repr{Single{std::move(m), std::move(value)}});
  }

  static auto make_deep(Digit<T> left, SpinePtr spine, Digit<T> right) -> FingerTree
  {
    auto m = tag_combine(
      tag_combine(digit_measure(left), spine_measure(spine)),
      digit_measure(right));
    auto b = digit_leaf_count(left) + spine_breadth(spine)
           + digit_leaf_count(right);
    auto d = std::size_t{1} + spine_depth(spine);
    return FingerTree(Repr{std::make_shared<const Deep>(
      Deep{std::move(m), b, d,
           std::move(left), std::move(spine), std::move(right)})});
  }

  static auto digit_to_tree(const Digit<T>& d) -> FingerTree
  {
    return std::visit(detail::overloaded{
      [](const One<T>& x) { return make_single(x.a); },
      [](const Two<T>& x) {
        return make_deep(One<T>{x.a}, nullptr, One<T>{x.b});
      },
      [](const Three<T>& x) {
        return make_deep(Two<T>{x.a, x.b}, nullptr, One<T>{x.c});
      },
      [](const Four<T>& x) {
        return make_deep(Two<T>{x.a, x.b}, nullptr, Two<T>{x.c, x.d});
      }
    }, d);
  }

  static auto deep_l(SpinePtr spine, Digit<T> right) -> FingerTree
  {
    if constexpr (DEPTH < kMaxDepth) {
      if (spine_is_empty(spine)) {
        return digit_to_tree(right);
      }
      auto vl = spine->view_l();
      assert(vl.has_value());
      auto new_left = node_to_digit(vl->d_value);
      SpinePtr new_spine;
      if (!vl->d_rest.is_empty()) {
        new_spine = std::make_shared<const SpineTree>(
          std::move(vl->d_rest));
      }
      return make_deep(std::move(new_left), std::move(new_spine),
                        std::move(right));
    } else {
      return digit_to_tree(right);
    }
  }

  static auto deep_r(Digit<T> left, SpinePtr spine) -> FingerTree
  {
    if constexpr (DEPTH < kMaxDepth) {
      if (spine_is_empty(spine)) {
        return digit_to_tree(left);
      }
      auto vr = spine->view_r();
      assert(vr.has_value());
      auto new_right = node_to_digit(vr->d_value);
      SpinePtr new_spine;
      if (!vr->d_rest.is_empty()) {
        new_spine = std::make_shared<const SpineTree>(
          std::move(vr->d_rest));
      }
      return make_deep(std::move(left), std::move(new_spine),
                        std::move(new_right));
    } else {
      return digit_to_tree(left);
    }
  }

  // -- nodes: pack elements into Node2/Node3 sequence --

  static auto nodes_from(std::vector<T> elems) -> std::vector<NodeT>
  {
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
        result.push_back(make_node2(std::move(elems[i]),
                                    std::move(elems[i + 1])));
        break;
      case 3:
        result.push_back(make_node3(std::move(elems[i]),
                                    std::move(elems[i + 1]),
                                    std::move(elems[i + 2])));
        break;
      case 4:
        result.push_back(make_node2(std::move(elems[i]),
                                    std::move(elems[i + 1])));
        result.push_back(make_node2(std::move(elems[i + 2]),
                                    std::move(elems[i + 3])));
        break;
      default:
        assert(false && "nodes_from: invalid element count");
    }
    return result;
  }

  // -- app3: Hinze-Paterson recursive concatenation --

  static auto app3(const FingerTree& left,
                   std::vector<T> middle,
                   const FingerTree& right) -> FingerTree
  {
    if (left.is_empty()) {
      auto result = right;
      for (auto it = middle.rbegin(); it != middle.rend(); ++it) {
        result = result.cons(std::move(*it));
      }
      return result;
    }
    if (right.is_empty()) {
      auto result = left;
      for (auto& elem : middle) {
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
      for (auto& elem : middle) {
        result = result.snoc(std::move(elem));
      }
      return result.snoc(std::get<Single>(right.d_repr).d_value);
    }

    if constexpr (DEPTH < kMaxDepth) {
      const auto& ld = *std::get<DeepPtr>(left.d_repr);
      const auto& rd = *std::get<DeepPtr>(right.d_repr);

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

      auto left_spine = ld.d_spine
        ? *ld.d_spine : SpineTree::empty();
      auto right_spine = rd.d_spine
        ? *rd.d_spine : SpineTree::empty();

      auto new_spine = SpineTree::app3(
        left_spine, std::move(ns), right_spine);
      SpinePtr sp;
      if (!new_spine.is_empty()) {
        sp = std::make_shared<const SpineTree>(std::move(new_spine));
      }

      return make_deep(ld.d_left, std::move(sp), rd.d_right);
    } else {
      auto result = left.flatten();
      result.insert(result.end(),
                    std::make_move_iterator(middle.begin()),
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
  static auto split_digit(const PREDICATE& predicate,
                          Tag prefix,
                          const Digit<T>& d) -> std::optional<DigitSplit>
  {
    auto elems = digit_to_list(d);
    auto running = prefix;
    for (std::size_t i = 0; i < elems.size(); ++i) {
      running = tag_combine(running, tag_value(elems[i]));
      if (predicate(running)) {
        std::optional<Digit<T>> left_d;
        std::optional<Digit<T>> right_d;
        if (i > 0) {
          std::vector<T> lv(elems.begin(),
                            elems.begin() + static_cast<std::ptrdiff_t>(i));
          left_d = list_to_digit(std::move(lv));
        }
        auto remaining = elems.size() - i - 1;
        if (remaining > 0) {
          std::vector<T> rv(
            elems.begin() + static_cast<std::ptrdiff_t>(i + 1), elems.end());
          right_d = list_to_digit(std::move(rv));
        }
        return DigitSplit{std::move(left_d), std::move(elems[i]),
                          std::move(right_d)};
      }
    }
    return std::nullopt;
  }

  static auto list_to_digit(std::vector<T> elems) -> std::optional<Digit<T>>
  {
    switch (elems.size()) {
      case 0: return std::nullopt;
      case 1: return One<T>{std::move(elems[0])};
      case 2: return Two<T>{std::move(elems[0]), std::move(elems[1])};
      case 3: return Three<T>{std::move(elems[0]), std::move(elems[1]),
                               std::move(elems[2])};
      case 4: return Four<T>{std::move(elems[0]), std::move(elems[1]),
                              std::move(elems[2]), std::move(elems[3])};
      default: assert(false); return std::nullopt;
    }
  }

  // Assemble tree from optional digit + spine + digit
  static auto assemble_left(std::optional<Digit<T>> left_d,
                            SpinePtr spine,
                            Digit<T> right) -> FingerTree
  {
    if (left_d.has_value()) {
      return make_deep(std::move(*left_d), std::move(spine),
                        std::move(right));
    }
    return deep_l(std::move(spine), std::move(right));
  }

  static auto assemble_right(Digit<T> left,
                             SpinePtr spine,
                             std::optional<Digit<T>> right_d) -> FingerTree
  {
    if (right_d.has_value()) {
      return make_deep(std::move(left), std::move(spine),
                        std::move(*right_d));
    }
    return deep_r(std::move(left), std::move(spine));
  }

  // Flatten spine nodes into elements
  static void spine_flatten_into(const SpinePtr& sp, std::vector<T>& out)
  {
    if constexpr (DEPTH < kMaxDepth) {
      if (spine_is_empty(sp)) return;
      auto spine_elems = sp->flatten();
      for (auto& node : spine_elems) {
        node_flatten_into(node, out);
      }
    }
  }

 public:
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

 private:
  // -- split_impl: measure-guided split --

  template <typename PREDICATE>
  auto split_impl(const PREDICATE& predicate,
                  Tag prefix) const -> std::optional<Split>
  {
    if (is_empty()) return std::nullopt;

    if (is_leaf()) {
      const auto& s = std::get<Single>(d_repr);
      auto p = tag_combine(prefix, s.d_measure);
      if (predicate(p)) {
        return Split{make_empty(), s.d_value, make_empty()};
      }
      return std::nullopt;
    }

    if constexpr (DEPTH < kMaxDepth) {
      const auto& d = *std::get<DeepPtr>(d_repr);

      // Check left digit
      auto vl = tag_combine(prefix, digit_measure(d.d_left));
      if (predicate(vl)) {
        auto ds = split_digit(predicate, prefix, d.d_left);
        if (!ds.has_value()) return std::nullopt;
        auto left_tree = ds->d_left.has_value()
          ? digit_to_tree(*ds->d_left) : make_empty();
        auto right_tree = assemble_left(
          std::move(ds->d_right), d.d_spine, d.d_right);
        return Split{std::move(left_tree), std::move(ds->d_pivot),
                     std::move(right_tree)};
      }

      // Check spine
      auto vm = tag_combine(vl, spine_measure(d.d_spine));
      if (predicate(vm)) {
        if (spine_is_empty(d.d_spine)) return std::nullopt;

        auto spine_split = d.d_spine->split_impl(
          [&](const Tag& t) { return predicate(tag_combine(vl, t)); },
          tag_identity());
        if (!spine_split.has_value()) return std::nullopt;

        auto node_prefix = tag_combine(vl,
          spine_split->d_left.measure());

        auto nd = node_to_digit(spine_split->d_pivot);
        auto node_split = split_digit(predicate, node_prefix, nd);
        if (!node_split.has_value()) return std::nullopt;

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

        auto left_tree = assemble_right(
          d.d_left, std::move(sl), std::move(node_split->d_left));
        auto right_tree = assemble_left(
          std::move(node_split->d_right), std::move(sr), d.d_right);
        return Split{std::move(left_tree), std::move(node_split->d_pivot),
                     std::move(right_tree)};
      }

      // Check right digit
      auto ds = split_digit(predicate, vm, d.d_right);
      if (!ds.has_value()) return std::nullopt;
      auto left_tree = assemble_right(
        d.d_left, d.d_spine, std::move(ds->d_left));
      auto right_tree = ds->d_right.has_value()
        ? digit_to_tree(*ds->d_right) : make_empty();
      return Split{std::move(left_tree), std::move(ds->d_pivot),
                   std::move(right_tree)};
    } else {
      auto vec = flatten();
      auto running = prefix;
      for (std::size_t i = 0; i < vec.size(); ++i) {
        running = tag_combine(running, tag_value(vec[i]));
        if (predicate(running)) {
          std::vector<T> lv(vec.begin(),
            vec.begin() + static_cast<std::ptrdiff_t>(i));
          std::vector<T> rv(
            vec.begin() + static_cast<std::ptrdiff_t>(i + 1), vec.end());
          return Split{from_sequence(std::move(lv)), std::move(vec[i]),
                       from_sequence(std::move(rv))};
        }
      }
      return std::nullopt;
    }
  }

 public:
  FingerTree()
    : d_repr(Empty{})
  {
  }

  static auto empty() -> FingerTree { return make_empty(); }

  static auto leaf(T value) -> FingerTree
  {
    return make_single(std::move(value));
  }

  auto is_empty() const -> bool
  {
    return std::holds_alternative<Empty>(d_repr);
  }

  auto is_leaf() const -> bool
  {
    return std::holds_alternative<Single>(d_repr);
  }

  auto is_branch() const -> bool
  {
    return std::holds_alternative<DeepPtr>(d_repr);
  }

  auto measure() const -> Tag
  {
    return std::visit(detail::overloaded{
      [](const Empty&) { return tag_identity(); },
      [](const Single& s) -> Tag { return s.d_measure; },
      [](const DeepPtr& d) -> Tag { return d->d_measure; }
    }, d_repr);
  }

  auto breadth() const -> std::size_t
  {
    return std::visit(detail::overloaded{
      [](const Empty&) -> std::size_t { return 0U; },
      [](const Single& s) -> std::size_t {
        return elem_leaf_count(s.d_value);
      },
      [](const DeepPtr& d) -> std::size_t { return d->d_breadth; }
    }, d_repr);
  }

  auto depth() const -> std::size_t
  {
    return std::visit(detail::overloaded{
      [](const Empty&) -> std::size_t { return 0U; },
      [](const Single&) -> std::size_t { return 1U; },
      [](const DeepPtr& d) -> std::size_t { return d->d_depth; }
    }, d_repr);
  }

  auto value() const -> const T&
  {
    assert(is_leaf());
    return std::get<Single>(d_repr).d_value;
  }

  auto cons(T x) const -> FingerTree
  {
    return std::visit(detail::overloaded{
      [&](const Empty&) { return make_single(std::move(x)); },
      [&](const Single& s) {
        return make_deep(One<T>{std::move(x)}, nullptr,
                          One<T>{s.d_value});
      },
      [&](const DeepPtr& d) -> FingerTree {
        return std::visit(detail::overloaded{
          [&](const One<T>& dig) {
            return make_deep(Two<T>{std::move(x), dig.a},
                              d->d_spine, d->d_right);
          },
          [&](const Two<T>& dig) {
            return make_deep(Three<T>{std::move(x), dig.a, dig.b},
                              d->d_spine, d->d_right);
          },
          [&](const Three<T>& dig) {
            return make_deep(
              Four<T>{std::move(x), dig.a, dig.b, dig.c},
              d->d_spine, d->d_right);
          },
          [&](const Four<T>& dig) -> FingerTree {
            auto node = make_node3(dig.b, dig.c, dig.d);
            auto new_spine = spine_cons(d->d_spine, std::move(node));
            return make_deep(Two<T>{std::move(x), dig.a},
                              std::move(new_spine), d->d_right);
          }
        }, d->d_left);
      }
    }, d_repr);
  }

  auto snoc(T x) const -> FingerTree
  {
    return std::visit(detail::overloaded{
      [&](const Empty&) { return make_single(std::move(x)); },
      [&](const Single& s) {
        return make_deep(One<T>{s.d_value}, nullptr,
                          One<T>{std::move(x)});
      },
      [&](const DeepPtr& d) -> FingerTree {
        return std::visit(detail::overloaded{
          [&](const One<T>& dig) {
            return make_deep(d->d_left, d->d_spine,
                              Two<T>{dig.a, std::move(x)});
          },
          [&](const Two<T>& dig) {
            return make_deep(d->d_left, d->d_spine,
                              Three<T>{dig.a, dig.b, std::move(x)});
          },
          [&](const Three<T>& dig) {
            return make_deep(
              d->d_left, d->d_spine,
              Four<T>{dig.a, dig.b, dig.c, std::move(x)});
          },
          [&](const Four<T>& dig) -> FingerTree {
            auto node = make_node3(dig.a, dig.b, dig.c);
            auto new_spine = spine_snoc(d->d_spine, std::move(node));
            return make_deep(d->d_left, std::move(new_spine),
                              Two<T>{dig.d, std::move(x)});
          }
        }, d->d_right);
      }
    }, d_repr);
  }

  auto append(const FingerTree& right) const -> FingerTree
  {
    return app3(*this, {}, right);
  }

  static auto branch(const FingerTree& left,
                     const FingerTree& right) -> FingerTree
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

  static auto concat(const FingerTree& left,
                     const FingerTree& right) -> FingerTree
  {
    return left.append(right);
  }

  auto flatten() const -> std::vector<T>
  {
    return std::visit(detail::overloaded{
      [](const Empty&) -> std::vector<T> { return {}; },
      [](const Single& s) -> std::vector<T> { return {s.d_value}; },
      [](const DeepPtr& d) -> std::vector<T> {
        std::vector<T> out;
        out.reserve(d->d_breadth);
        digit_flatten_into(d->d_left, out);
        spine_flatten_into(d->d_spine, out);
        digit_flatten_into(d->d_right, out);
        return out;
      }
    }, d_repr);
  }

  auto view_l() const -> std::optional<View>
  {
    return std::visit(detail::overloaded{
      [](const Empty&) -> std::optional<View> { return std::nullopt; },
      [](const Single& s) -> std::optional<View> {
        return View{s.d_value, make_empty()};
      },
      [](const DeepPtr& d) -> std::optional<View> {
        auto h = digit_head(d->d_left);
        auto t = digit_tail(d->d_left);
        if (t.has_value()) {
          return View{h, make_deep(std::move(*t), d->d_spine,
                                    d->d_right)};
        }
        return View{h, deep_l(d->d_spine, d->d_right)};
      }
    }, d_repr);
  }

  auto view_r() const -> std::optional<View>
  {
    return std::visit(detail::overloaded{
      [](const Empty&) -> std::optional<View> { return std::nullopt; },
      [](const Single& s) -> std::optional<View> {
        return View{s.d_value, make_empty()};
      },
      [](const DeepPtr& d) -> std::optional<View> {
        auto l = digit_last(d->d_right);
        auto i = digit_init(d->d_right);
        if (i.has_value()) {
          return View{l, make_deep(d->d_left, d->d_spine,
                                    std::move(*i))};
        }
        return View{l, deep_r(d->d_left, d->d_spine)};
      }
    }, d_repr);
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

  template <typename PREDICATE>
  auto search(PREDICATE&& predicate) const -> std::optional<T>
  {
    auto sp = split(std::forward<PREDICATE>(predicate));
    if (!sp.has_value()) return std::nullopt;
    return std::move(sp->d_pivot);
  }

  template <typename PREDICATE>
  auto split(PREDICATE&& predicate) const -> std::optional<Split>
  {
    return split_impl(predicate, tag_identity());
  }

  template <typename PREDICATE>
  auto split_at(PREDICATE&& predicate) const -> SplitAt
  {
    auto sp = split(std::forward<PREDICATE>(predicate));
    if (!sp.has_value()) {
      return SplitAt{*this, empty()};
    }
    return SplitAt{std::move(sp->d_left),
                   sp->d_right.cons(std::move(sp->d_pivot))};
  }

  auto split_at_index(std::size_t index) const -> SplitAt
  {
    if (index == 0U) {
      return SplitAt{empty(), *this};
    }
    if (index >= breadth()) {
      return SplitAt{*this, empty()};
    }
    auto vec = flatten();
    auto clamped = index > vec.size() ? vec.size() : index;
    std::vector<T> lv(vec.begin(),
      vec.begin() + static_cast<std::ptrdiff_t>(clamped));
    std::vector<T> rv(
      vec.begin() + static_cast<std::ptrdiff_t>(clamped), vec.end());
    return SplitAt{from_sequence(std::move(lv)),
                   from_sequence(std::move(rv))};
  }

  auto split_at_measure(const Tag& threshold) const -> SplitAt
    requires requires(const Tag& lhs, const Tag& rhs) {
      { lhs >= rhs } -> std::convertible_to<bool>;
    }
  {
    return split_at(
      [&threshold](const Tag& prefix) { return prefix >= threshold; });
  }

  static auto from_sequence(std::vector<T> values) -> FingerTree
  {
    auto result = empty();
    for (auto& v : values) {
      result = result.snoc(std::move(v));
    }
    return result;
  }
};

}  // namespace smd::tree

#endif
```


<a id="org2ffd65e"></a>

## smd/tree/finger<sub>tree</sub><sub>interval</sub><sub>index.hpp</sub>

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

namespace smd {

template <class PAYLOAD_TYPE>
struct FingerTreeIntervalIndexFoldableImpl {
  template <class F>
  auto fold_map(this auto&&,
                F&& function,
                const smd::tree::FingerTreeIntervalIndex<PAYLOAD_TYPE>& index)
    -> remove_cvref_t<std::invoke_result_t<F, const PAYLOAD_TYPE&>>
  {
    using Result = remove_cvref_t<std::invoke_result_t<F, const PAYLOAD_TYPE&>>;
    return std::ranges::fold_left(
        index.entries(),
        smd::typeclass::monoid_v<Result>.identity(),
        [&](Result acc, const auto& entry) {
          return smd::typeclass::monoid_v<Result>.combine(
              std::move(acc), std::invoke(function, entry.d_payload));
        });
  }
};

template <class PAYLOAD_TYPE>
struct FingerTreeIntervalIndexFoldableMap
  : Foldable<FingerTreeIntervalIndexFoldableImpl<PAYLOAD_TYPE>> {
  using FingerTreeIntervalIndexFoldableImpl<PAYLOAD_TYPE>::fold_map;
};

template <class PAYLOAD_TYPE>
inline constexpr auto foldable_typeclass<smd::tree::FingerTreeIntervalIndex<PAYLOAD_TYPE>> =
  FingerTreeIntervalIndexFoldableMap<PAYLOAD_TYPE>{};

template <class PAYLOAD_TYPE>
struct FingerTreeIntervalIndexTraversableImpl {
  using element_type = PAYLOAD_TYPE;

  template <class APPLICATIVE, class F>
  auto traverse(this auto&&,
                const APPLICATIVE& applicative,
                F&& function,
                const smd::tree::FingerTreeIntervalIndex<PAYLOAD_TYPE>& index)
  {
    using Context = remove_cvref_t<std::invoke_result_t<F, const PAYLOAD_TYPE&>>;
    using U = smd::applicative_value_t<Context>;

    auto accumulated = applicative.pure(std::vector<smd::tree::Interval<U>>{});

    for (const auto& entry : index.entries()) {
      auto lifted = std::invoke(function, entry.d_payload);
      accumulated = applicative.invoke(
        [start = entry.d_start, end = entry.d_end](
          std::vector<smd::tree::Interval<U>> values, U payload) {
          values.push_back(
            smd::tree::Interval<U>{start, end, std::move(payload)});
          return values;
        },
        std::move(accumulated),
        std::move(lifted));
    }

    return applicative.invoke(
      [](std::vector<smd::tree::Interval<U>> values) {
        return smd::tree::FingerTreeIntervalIndex<U>::from_intervals(
          std::move(values));
      },
      std::move(accumulated));
  }
};

template <class PAYLOAD_TYPE>
struct FingerTreeIntervalIndexTraversableMap
  : Traversable<FingerTreeIntervalIndexTraversableImpl<PAYLOAD_TYPE>> {
  using FingerTreeIntervalIndexTraversableImpl<PAYLOAD_TYPE>::traverse;
};

template <class PAYLOAD_TYPE>
inline constexpr auto traversable_typeclass<smd::tree::FingerTreeIntervalIndex<PAYLOAD_TYPE>> =
  FingerTreeIntervalIndexTraversableMap<PAYLOAD_TYPE>{};

}  // namespace smd

#endif
```


<a id="org0db261f"></a>

## smd/tree/finger<sub>tree</sub><sub>priority</sub><sub>queue.hpp</sub>

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

namespace smd {

template <class T>
struct FingerTreePriorityQueueFoldableImpl {
  template <class F>
  auto fold_map(this auto&&,
                F&& function,
                const smd::tree::FingerTreePriorityQueue<T>& queue)
    -> remove_cvref_t<std::invoke_result_t<F, const T&>>
  {
    using Result = remove_cvref_t<std::invoke_result_t<F, const T&>>;
    return std::ranges::fold_left(
        queue.to_vector(),
        smd::typeclass::monoid_v<Result>.identity(),
        [&](Result acc, const auto& value) {
          return smd::typeclass::monoid_v<Result>.combine(
              std::move(acc), std::invoke(function, value));
        });
  }
};

template <class T>
struct FingerTreePriorityQueueFoldableMap
  : Foldable<FingerTreePriorityQueueFoldableImpl<T>> {
  using FingerTreePriorityQueueFoldableImpl<T>::fold_map;
};

template <class T>
inline constexpr auto foldable_typeclass<smd::tree::FingerTreePriorityQueue<T>> =
  FingerTreePriorityQueueFoldableMap<T>{};

template <class T>
struct FingerTreePriorityQueueTraversableImpl {
  using element_type = T;

  template <class APPLICATIVE, class F>
  auto traverse(this auto&&,
                const APPLICATIVE& applicative,
                F&& function,
                const smd::tree::FingerTreePriorityQueue<T>& queue)
  {
    using Context = remove_cvref_t<std::invoke_result_t<F, const T&>>;
    using U = smd::applicative_value_t<Context>;

    auto accumulated = applicative.pure(std::vector<U>{});

    for (const auto& value : queue.to_vector()) {
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
        return smd::tree::FingerTreePriorityQueue<U>::from_values(
          std::move(values));
      },
      std::move(accumulated));
  }
};

template <class T>
struct FingerTreePriorityQueueTraversableMap
  : Traversable<FingerTreePriorityQueueTraversableImpl<T>> {
  using FingerTreePriorityQueueTraversableImpl<T>::traverse;
};

template <class T>
inline constexpr auto traversable_typeclass<smd::tree::FingerTreePriorityQueue<T>> =
  FingerTreePriorityQueueTraversableMap<T>{};

}  // namespace smd

#endif
```


<a id="org4981d68"></a>

## smd/tree/finger<sub>tree</sub><sub>random</sub><sub>access.hpp</sub>

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

namespace smd {

template <class T>
struct FingerTreeRandomAccessFoldableImpl {
  template <class F>
  auto fold_map(this auto&&,
                F&& function,
                const smd::tree::FingerTreeRandomAccess<T>& sequence)
    -> remove_cvref_t<std::invoke_result_t<F, const T&>>
  {
    using Result = remove_cvref_t<std::invoke_result_t<F, const T&>>;
    return std::ranges::fold_left(
        sequence.to_vector(),
        smd::typeclass::monoid_v<Result>.identity(),
        [&](Result acc, const auto& value) {
          return smd::typeclass::monoid_v<Result>.combine(
              std::move(acc), std::invoke(function, value));
        });
  }
};

template <class T>
struct FingerTreeRandomAccessFoldableMap
  : Foldable<FingerTreeRandomAccessFoldableImpl<T>> {
  using FingerTreeRandomAccessFoldableImpl<T>::fold_map;
};

template <class T>
inline constexpr auto foldable_typeclass<smd::tree::FingerTreeRandomAccess<T>> =
  FingerTreeRandomAccessFoldableMap<T>{};

template <class T>
struct FingerTreeRandomAccessTraversableImpl {
  using element_type = T;

  template <class APPLICATIVE, class F>
  auto traverse(this auto&&,
                const APPLICATIVE& applicative,
                F&& function,
                const smd::tree::FingerTreeRandomAccess<T>& sequence)
  {
    using Context = remove_cvref_t<std::invoke_result_t<F, const T&>>;
    using U = smd::applicative_value_t<Context>;

    auto accumulated = applicative.pure(std::vector<U>{});

    for (const auto& value : sequence.to_vector()) {
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
        return smd::tree::FingerTreeRandomAccess<U>::from_sequence(
          std::move(values));
      },
      std::move(accumulated));
  }
};

template <class T>
struct FingerTreeRandomAccessTraversableMap
  : Traversable<FingerTreeRandomAccessTraversableImpl<T>> {
  using FingerTreeRandomAccessTraversableImpl<T>::traverse;
};

template <class T>
inline constexpr auto traversable_typeclass<smd::tree::FingerTreeRandomAccess<T>> =
  FingerTreeRandomAccessTraversableMap<T>{};

}  // namespace smd

#endif
```


<a id="org413b5a9"></a>

## smd/tree/finger<sub>tree</sub><sub>rope.hpp</sub>

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

namespace smd {

struct FingerTreeRopeFoldableImpl {
  template <class F>
  auto fold_map(this auto&&,
                F&& function,
                const smd::tree::FingerTreeRope& rope)
    -> remove_cvref_t<std::invoke_result_t<F, const std::string&>>
  {
    using Result =
      remove_cvref_t<std::invoke_result_t<F, const std::string&>>;
    return std::ranges::fold_left(
        rope.chunks(),
        smd::typeclass::monoid_v<Result>.identity(),
        [&](Result acc, const auto& chunk) {
          return smd::typeclass::monoid_v<Result>.combine(
              std::move(acc), std::invoke(function, chunk));
        });
  }
};

struct FingerTreeRopeFoldableMap : Foldable<FingerTreeRopeFoldableImpl> {
  using FingerTreeRopeFoldableImpl::fold_map;
};

template <>
inline constexpr auto foldable_typeclass<smd::tree::FingerTreeRope> =
  FingerTreeRopeFoldableMap{};

struct FingerTreeRopeTraversableImpl {
  using element_type = std::string;

  template <class APPLICATIVE, class F>
  auto traverse(this auto&&,
                const APPLICATIVE& applicative,
                F&& function,
                const smd::tree::FingerTreeRope& rope)
  {
    using Context = remove_cvref_t<std::invoke_result_t<F, const std::string&>>;
    using U = smd::applicative_value_t<Context>;

    auto accumulated = applicative.pure(std::vector<U>{});

    for (const auto& chunk : rope.chunks()) {
      auto lifted = std::invoke(function, chunk);
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
        return smd::tree::FingerTreeRope::from_chunks(std::move(values));
      },
      std::move(accumulated));
  }
};

struct FingerTreeRopeTraversableMap : Traversable<FingerTreeRopeTraversableImpl> {
  using FingerTreeRopeTraversableImpl::traverse;
};

template <>
inline constexpr auto traversable_typeclass<smd::tree::FingerTreeRope> =
  FingerTreeRopeTraversableMap{};

}  // namespace smd

#endif
```


<a id="orgb3e7bbb"></a>

## smd/tree/finger<sub>tree</sub><sub>traversable.hpp</sub>

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


<a id="orgf67c401"></a>

## smd/tree/finger<sub>tree</sub><sub>wrappers.hpp</sub>

```cpp
// src/smd/tree/finger_tree_wrappers.hpp                              -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_FINGER_TREE_WRAPPERS
#define INCLUDED_SMD_TREE_FINGER_TREE_WRAPPERS

#include <smd/tree/finger_tree_interval_index.hpp>
#include <smd/tree/finger_tree_priority_queue.hpp>
#include <smd/tree/finger_tree_random_access.hpp>
#include <smd/tree/finger_tree_rope.hpp>

#endif
```


<a id="org5b2110f"></a>

## smd/tree/fix<sub>tree</sub><sub>applicative.hpp</sub>

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


<a id="org45c3846"></a>

## smd/tree/fix<sub>tree</sub><sub>foldable.hpp</sub>

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


<a id="org232c2a2"></a>

## smd/tree/fix<sub>tree.hpp</sub>

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


<a id="orgcd6a700"></a>

## smd/tree/fix<sub>tree</sub><sub>traversable.hpp</sub>

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


<a id="org54ebcb5"></a>

## smd/tree/fringe<sub>tree</sub><sub>applicative.hpp</sub>

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


<a id="org4862bec"></a>

## smd/tree/fringe<sub>tree</sub><sub>foldable.hpp</sub>

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


<a id="org9fd9e3f"></a>

## smd/tree/fringe<sub>tree.hpp</sub>

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


<a id="orgf236ef2"></a>

## smd/tree/fringe<sub>tree</sub><sub>traversable.hpp</sub>

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


<a id="org8b85e4c"></a>

## smd/tree/memoized<sub>thunk.hpp</sub>

```cpp
// src/smd/tree/memoized_thunk.hpp                                    -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_MEMOIZED_THUNK
#define INCLUDED_SMD_TREE_MEMOIZED_THUNK

// Forwarding header — canonical implementations now live in smd/thunk/.
// Use <smd/thunk/delay.hpp> and <smd/thunk/memoize.hpp> directly in new code.

#include <smd/thunk/delay.hpp>
#include <smd/thunk/memoize.hpp>

namespace smd::tree::detail {

// Aliases into the canonical smd::thunk namespace, preserving the old names
// used by finger_tree.t.cpp and any other existing callers.
using smd::thunk::erased_thunk;

template <typename Callable, typename... Args>
auto delay(Callable&& c, Args&&... args)
{
  return smd::thunk::delay(std::forward<Callable>(c), std::forward<Args>(args)...);
}

template <typename Callable, typename... Args>
auto thunk(Callable&& c, Args&&... args)
{
  return smd::thunk::memoize(std::forward<Callable>(c), std::forward<Args>(args)...);
}

template <typename Measure, typename Callable, typename... Args>
auto measured_thunk(Measure measure, Callable&& c, Args&&... args)
{
  return smd::thunk::measured_memoize(
    std::move(measure),
    std::forward<Callable>(c),
    std::forward<Args>(args)...);
}

}  // namespace smd::tree::detail

#endif
```


<a id="orgaff5802"></a>

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


<a id="org7772d2c"></a>

## smd/typeclass/examples/applicative<sub>bad.cpp</sub>

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


<a id="orgb401833"></a>

## smd/typeclass/examples/applicative<sub>examples.cpp</sub>

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


<a id="org7f4af1e"></a>

## smd/typeclass/examples/foldable<sub>examples.cpp</sub>

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


<a id="org5f424cd"></a>

## smd/typeclass/examples/lookup<sub>modes</sub><sub>examples.cpp</sub>

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


<a id="org98d8729"></a>

## smd/typeclass/examples/traversable<sub>examples.cpp</sub>

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


<a id="org5159ca4"></a>

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


<a id="orge937553"></a>

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


<a id="orga2c4273"></a>

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


<a id="org6540d8d"></a>

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


<a id="org11fad53"></a>

## smd/typeclass/typeclass<sub>base.hpp</sub>

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


<a id="org667c4de"></a>

## smd/ziplist/zip<sub>list</sub><sub>applicative.hpp</sub>

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


<a id="orgd23e2b5"></a>

## smd/ziplist/zip<sub>list.hpp</sub>

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


<a id="org5c00dee"></a>

# Regeneration

Run from repository root to regenerate this file:

```bash
set -euo pipefail
repo_root="$(git rev-parse --show-toplevel)"
make -C "$repo_root/trees" docs-index
```
