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
    auto result = monad.bind(std::optional<int>{5},
                             [](int x) { return std::optional<int>{x * 2}; });
    REQUIRE(result.has_value());
    CHECK(*result == 10);
}

TEST_CASE("MonadTypeclassTest - BindOptionalAbsent") {
    const auto &monad = smd::monad_typeclass<std::optional<int>>;
    auto result = monad.bind(std::optional<int>{},
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
    auto f = [](int x) {
        return std::optional<std::string>{std::to_string(x)};
    };
    int a = 42;

    auto lhs = monad.bind(monad.pure(a), f);
    auto rhs = f(a);
    CHECK(lhs == rhs);
}

TEST_CASE("MonadTypeclassTest - RightIdentityLaw") {
    // bind(ma, pure) == ma
    const auto &monad = smd::monad_typeclass<std::optional<int>>;
    std::optional<int> ma{99};

    auto result = monad.bind(ma, [](int x) { return std::optional<int>{x}; });
    CHECK(result == ma);
}

TEST_CASE("MonadTypeclassTest - RightIdentityLawAbsent") {
    const auto &monad = smd::monad_typeclass<std::optional<int>>;
    std::optional<int> ma{};

    auto result = monad.bind(ma, [](int x) { return std::optional<int>{x}; });
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
        MonadOnly<int>{5}, [](int x) { return MonadOnly<int>{x * 3}; });
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
    auto result =
        monad_only_int.bind(ma, [](int x) { return MonadOnly<int>{x}; });
    CHECK(result == ma);
}

TEST_CASE("MonadOnlyTest - AssociativityLaw") {
    MonadOnly<int> ma{3};
    auto f = [](int x) { return MonadOnly<int>{x + 10}; };
    auto g = [](int x) { return MonadOnly<int>{x * 2}; };

    auto lhs = monad_only_int.bind(monad_only_int.bind(ma, f), g);
    auto rhs = monad_only_int.bind(
        ma, [&](int a) { return monad_only_int.bind(f(a), g); });
    CHECK(lhs == rhs);
}

// -- Kleisli composition --

TEST_CASE("MonadTypeclassTest - KleisliComposition") {
    const auto &monad = smd::monad_typeclass<std::optional<int>>;
    auto f = [](int x) { return std::optional<int>{x + 1}; };
    auto g = [](int x) {
        return std::optional<std::string>{std::to_string(x)};
    };

    auto composed = monad.kleisli(f, g);
    auto result = composed(9);
    REQUIRE(result.has_value());
    CHECK(*result == "10");
}

TEST_CASE("MonadTypeclassTest - KleisliCompositionShortCircuit") {
    const auto &monad = smd::monad_typeclass<std::optional<int>>;
    auto f = [](int) { return std::optional<int>{}; };
    auto g = [](int x) {
        return std::optional<std::string>{std::to_string(x)};
    };

    auto composed = monad.kleisli(f, g);
    auto result = composed(9);
    CHECK_FALSE(result.has_value());
}

// -- Free-function API --

TEST_CASE("MonadFreeFunctionTest - MBind") {
    auto result = smd::mbind(std::optional<int>{5},
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
    auto result = monad.bind_with(monad, std::optional<int>{5}, [](int x) {
        return std::optional<int>{x + 1};
    });
    REQUIRE(result.has_value());
    CHECK(*result == 6);
}
