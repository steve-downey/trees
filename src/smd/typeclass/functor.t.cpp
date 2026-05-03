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
