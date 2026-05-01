// src/smd/fixpoint/cata.t.cpp                                        -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <smd/fixpoint/cata.hpp>
#include <smd/fixpoint/cata.hpp>  // Re-inclusion check

#include <smd/fixpoint/box.hpp>
#include <smd/fixpoint/overloaded.hpp>

#include <catch2/catch_test_macros.hpp>

#include <functional>
#include <variant>

using smd::fixpoint::Box;
using smd::fixpoint::Fix;
using smd::fixpoint::cata;
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

auto make_zero() -> Nat
{
    return wrap<NatF>(NatF<Nat>{Zero{}});
}

auto make_succ(Nat n) -> Nat
{
    return wrap<NatF>(NatF<Nat>{Succ<Nat>{make_box<Nat>(std::move(n))}});
}

template <typename A, typename F>
auto fmap_nat(F&& f, const NatF<A>& nat)
{
    using B = std::invoke_result_t<F, const A&>;
    return std::visit(
        overloaded{
            [](const Zero&) -> NatF<B> { return Zero{}; },
            [&f](const Succ<A>& s) -> NatF<B> {
                return Succ<B>{make_box<B>(std::invoke(f, *s.pred))};
            },
        },
        nat);
}

auto fmap_nat_fn = [](auto&& f, const auto& nat) {
    return fmap_nat(std::forward<decltype(f)>(f), nat);
};

auto count_algebra = [](const NatF<int>& n) -> int {
    return std::visit(
        overloaded{
            [](const Zero&) { return 0; },
            [](const Succ<int>& s) { return *s.pred + 1; },
        },
        n);
};

}  // namespace

TEST_CASE("Cata - NatZero")
{
    auto zero = make_zero();
    CHECK(cata<int>(count_algebra, fmap_nat_fn, zero) == 0);
}

TEST_CASE("Cata - NatTwo")
{
    auto two = make_succ(make_succ(make_zero()));
    CHECK(cata<int>(count_algebra, fmap_nat_fn, two) == 2);
}

TEST_CASE("Cata - NatFive")
{
    auto n = make_zero();
    for (int i = 0; i < 5; ++i) {
        n = make_succ(std::move(n));
    }
    CHECK(cata<int>(count_algebra, fmap_nat_fn, n) == 5);
}

TEST_CASE("Cata - NatCustomAlgebra")
{
    auto three = make_succ(make_succ(make_succ(make_zero())));

    auto bool_algebra = [](const NatF<bool>& n) -> bool {
        return std::visit(
            overloaded{
                [](const Zero&) { return true; },
                [](const Succ<bool>& s) { return !*s.pred; },
            },
            n);
    };

    CHECK(cata<bool>(bool_algebra, fmap_nat_fn, three) == false);
}
