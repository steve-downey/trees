// smd/conceptmap/functor.h                                           -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_CONCEPTMAP_FUNCTOR
#define INCLUDED_SMD_CONCEPTMAP_FUNCTOR

#include <algorithm>
#include <beman/optional/optional.hpp>
#include <concepts>
#include <ranges>
#include <type_traits>

namespace smd {
namespace conceptmap {

/** CRTP base providing `map` and the derived `replace` operation.
 * @tparam Impl  implementation class supplying the primitive `map(c, g)`
 * @tparam C     the container type being mapped over
 */
template <template <typename> typename Impl, typename C>
struct Functor : protected Impl<C> {
    /** Apply @p g to every element of @p c, returning a new container. */
    auto map(this auto &&self, C const &c, auto g) {
        std::puts("Functor::map");
        return self.map(c, g);
    }
    /** Replace every element of @p c with the constant @p u. */
    auto replace(this auto &&self, C const &c, auto u) {
        std::puts("Functor::replace");
        return self.map(c, [u]() { return u; });
    }
};

/** Concept-map lookup object for the Functor typeclass (older conceptmap surface).
 * Defaults to `std::false_type`; specialize for each supported container type.
 */
template <typename C>
auto functor_concept_map = std::false_type{};

/** Functor implementation for types that expose a `.transform(g)` member. */
template <typename C>
class Transform {
  public:
    using value_type = C::value_type;
    /** Delegate to `c.transform(g)`. */
    auto map(this auto && /*self*/, C const &c, auto g) {
        std::puts("Transform::map()");
        return c.transform(g);
    }
};

/** Functor concept-map record for `Transform`-based containers. */
template <typename T>
struct TransformFunctorMap : public Functor<Transform, T> {
    using Transform<T>::map;
};

/** Functor instance for `std::optional<T>` via `.transform()`. */
template <typename T>
inline constexpr auto functor_concept_map<std::optional<T>> =
    TransformFunctorMap<std::optional<T>>{};

/** Functor instance for `beman::optional::optional<T>` via `.transform()`. */
template <typename T>
inline constexpr auto functor_concept_map<beman::optional::optional<T>> =
    TransformFunctorMap<beman::optional::optional<T>>{};

/** Functor implementation for range types via `std::views::transform`. */
template <typename C>
    requires std::ranges::range<C>
class RangeTransform {
  public:
    using value_type = C::value_type;
    /** Return a lazy transform view of @p c applying @p g to each element. */
    auto map(this auto && /*self*/, C const &c, auto g) {
        std::puts("RangeTransform::map()");
        return std::views::transform(c, g);
    }
};

/** Functor concept-map record for `RangeTransform`-based containers. */
template <typename T>
struct RangeTransformFunctorMap : public Functor<RangeTransform, T> {
    using RangeTransform<T>::map;
};

/** Functor instance for any range type `R<T>` via `std::views::transform`. */
template <template <typename> typename R, typename T>
    requires std::ranges::range<R<T>>
inline constexpr auto functor_concept_map<R<T>> =
    RangeTransformFunctorMap<R<T>>{};

} // namespace conceptmap
} // namespace smd

#endif
