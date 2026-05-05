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

// Monad<Impl>
//
// Minimal hooks: pure + bind.
// Derived: apply (synthesized from bind + pure), join, kleisli, bind_with.
//
// Monad does not inherit from Applicative. The Impl delegates pure to an
// applicative typeclass object when one exists, or defines it directly when
// none is available. The base class synthesizes apply from bind + pure, so
// a Monad instance provides Applicative-equivalent operations automatically.
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

template <class VALUE_TYPE>
    requires(!std::same_as<beman::optional::optional<VALUE_TYPE>,
                           std::optional<VALUE_TYPE>>)
inline constexpr auto
    monad_typeclass<beman::optional::optional<VALUE_TYPE>> =
        BemanOptionalMonadMap<VALUE_TYPE>{};

// -- Free-function API --

template <class MA, class F>
auto mbind(MA &&ma, F &&f) {
    const auto &map = monad_typeclass<remove_cvref_t<MA>>;
    return map.bind(std::forward<MA>(ma), std::forward<F>(f));
}

template <class MMA>
auto join(MMA &&mma) {
    const auto &map = monad_typeclass<remove_cvref_t<MMA>>;
    return map.join(std::forward<MMA>(mma));
}

} // namespace smd

#endif
