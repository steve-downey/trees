#include <smd/typeclass/test/test_support.hpp>
#include <smd/typeclass/traversable.hpp>

#include <catch2/catch_test_macros.hpp>

#include <optional>

TEST_CASE("TraversableTypeclassTest - TraverseOptionalSuccess")
{
    using Identity = smd::typeclass::test::Identity<int>;
    auto identity = Identity{1};
    const auto& traversable = smd::traversable_typeclass<Identity>;

    auto traversed = traversable.traverse(
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

    auto traversed = traversable.traverse(
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
    using IdentityOpt = smd::typeclass::test::Identity<std::optional<int> >;
    auto identity = IdentityOpt{std::optional<int>{1}};
    const auto& traversable = smd::traversable_typeclass<IdentityOpt>;

    auto sequenced = traversable.sequence(identity);
    REQUIRE(sequenced.has_value());
    CHECK(sequenced->value == 1);

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

    auto via_traverse = traversable.traverse(
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
    auto via_traverse_identity = traversable.traverse(
        [](auto&& x) { return std::forward<decltype(x)>(x); },
        identity);

    CHECK(via_sequence == via_traverse_identity);
}
