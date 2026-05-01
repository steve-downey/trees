#include <smd/typeclass/test/test_support.hpp>
#include <smd/typeclass/traversable.hpp>
#include <smd/typeclass/traversable.hpp>  // Re-inclusion check

#include <catch2/catch_test_macros.hpp>

#include <beman/optional/optional.hpp>

#include <optional>

TEST_CASE("TraversableTypeclassTest - TraverseOptionalSuccess")
{
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

TEST_CASE("TraversableTypeclassTest - TraverseOptionalFailure")
{
    using Identity = smd::typeclass::test::Identity<int>;
    auto identity = Identity{-2};

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
