#include <smd/typeclass/test/test_support.hpp>
#include <smd/typeclass/traversable.hpp>

#include <gtest/gtest.h>

#include <optional>

TEST(TraversableTypeclassTest, TraverseOptionalSuccess)
{
    using Identity = smd::typeclass::test::Identity<int>;
    auto identity = Identity{1};
    const auto& traversable = smd::traversable_typeclass<Identity>;

    auto traversed = traversable.traverse(
        [](int x) -> std::optional<int> {
            return x >= 0 ? std::optional<int>{x + 1} : std::optional<int>{};
        },
        identity);

    ASSERT_TRUE(traversed.has_value());
    EXPECT_EQ(traversed->value, 2);
}

TEST(TraversableTypeclassTest, TraverseOptionalFailure)
{
    using Identity = smd::typeclass::test::Identity<int>;
    auto identity = Identity{-2};
    const auto& traversable = smd::traversable_typeclass<Identity>;

    auto traversed = traversable.traverse(
        [](int x) -> std::optional<int> {
            return x >= 0 ? std::optional<int>{x + 1} : std::optional<int>{};
        },
        identity);

    EXPECT_FALSE(traversed.has_value());
}

TEST(TraversableTypeclassTest, ForEachOptionalSuccess)
{
    using Identity = smd::typeclass::test::Identity<int>;
    auto identity = Identity{3};
    const auto& traversable = smd::traversable_typeclass<Identity>;

    auto traversed = traversable.for_each(identity, [](int x) -> std::optional<int> {
        return std::optional<int>{x * 2};
    });

    ASSERT_TRUE(traversed.has_value());
    EXPECT_EQ(traversed->value, 6);
}

TEST(TraversableTypeclassTest, SequenceAndSequenceWith)
{
    // f1de12e0-2287-4568-98c7-75be4f6f7446
    using IdentityOpt = smd::typeclass::test::Identity<std::optional<int> >;
    auto identity = IdentityOpt{std::optional<int>{1}};
    const auto& traversable = smd::traversable_typeclass<IdentityOpt>;

    auto sequenced = traversable.sequence(identity);
    ASSERT_TRUE(sequenced.has_value());
    EXPECT_EQ(sequenced->value, 1);

    auto sequenced_with = traversable.sequence_with(traversable, identity);
    ASSERT_TRUE(sequenced_with.has_value());
    EXPECT_EQ(sequenced_with->value, 1);
    // f1de12e0-2287-4568-98c7-75be4f6f7446 end
}
