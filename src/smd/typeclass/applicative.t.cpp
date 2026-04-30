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
    std::optional<int> ax{10};
    std::optional<int> ay{5};
    const auto& applicative = smd::applicative_typeclass<std::optional<int> >;

    auto result = applicative.invoke([](int a, int b) { return a - b; }, ax, ay);
    REQUIRE(result.has_value());
    CHECK(*result == 5);
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

    std::optional<int> ax{1};
    std::optional<int> ay{};
    auto invoke_result = applicative.invoke([](int a, int b) { return a + b; }, ax, ay);
    CHECK_FALSE(invoke_result.has_value());
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
    auto partial = smd::detail::make_terminating_partial(
        [](int a, int b, int c) { return a * 100 + b * 10 + c; });

    auto partial2 = partial(1);
    auto partial3 = partial2(2);
    CHECK(partial3(3) == 123);

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
    const auto& applicative = smd::applicative_typeclass<std::optional<int> >;

    std::optional<int> value{8};

    auto id_via_ap = applicative.ap(
        applicative.pure(+[](int x) { return x; }),
        value);
    CHECK(id_via_ap == value);

    auto hom_left = applicative.ap(
        applicative.pure(+[](int x) { return x + 3; }),
        applicative.pure(5));
    auto hom_right = applicative.pure(8);
    CHECK(hom_left == hom_right);

    auto invoke_binary = applicative.invoke(
        [](int a, int b) { return a * 10 + b; },
        std::optional<int>{2},
        std::optional<int>{7});

    auto pure_then_ap = applicative.ap(
        applicative.ap(
            applicative.pure([](int a) {
                return [a](int b) { return a * 10 + b; };
            }),
            std::optional<int>{2}),
        std::optional<int>{7});

    CHECK(invoke_binary == pure_then_ap);
}

TEST_CASE("ApplicativeBehaviorTest - BareIdentityIdentityHomomorphismAndInvoke")
{
    using BareIdentity = smd::typeclass::test::BareIdentity<int>;
    const auto& applicative = smd::applicative_typeclass<BareIdentity>;

    BareIdentity value{11};

    auto id_via_ap = applicative.ap(
        applicative.pure(+[](int x) { return x; }),
        value);
    CHECK(id_via_ap == value);

    auto hom_left = applicative.ap(
        applicative.pure(+[](int x) { return x * 4; }),
        applicative.pure(3));
    auto hom_right = applicative.pure(12);
    CHECK(hom_left == hom_right);

    auto invoke_binary = applicative.invoke(
        [](int a, int b) { return a - b; },
        BareIdentity{20},
        BareIdentity{6});

    auto pure_then_ap = applicative.ap(
        applicative.ap(
            applicative.pure([](int a) {
                return [a](int b) { return a - b; };
            }),
            BareIdentity{20}),
        BareIdentity{6});

    CHECK(invoke_binary == pure_then_ap);
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
