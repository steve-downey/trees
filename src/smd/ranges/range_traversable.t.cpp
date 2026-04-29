#include <smd/ranges/range_traversable.hpp>

#include <gtest/gtest.h>

#include <optional>
#include <sstream>
#include <ranges>
#include <type_traits>
#include <vector>

namespace {

template <std::ranges::input_range RANGE>
auto collect(RANGE&& range)
{
    using Value = std::ranges::range_value_t<RANGE>;
    std::vector<Value> values;

    for (auto&& value : range) {
        values.emplace_back(value);
    }

    return values;
}

template <std::ranges::input_range OUTER_RANGE>
auto collect_nested(OUTER_RANGE&& outer_range)
{
    std::vector<std::vector<std::ranges::range_value_t<std::ranges::range_value_t<OUTER_RANGE> > > > collected;

    for (auto&& inner_range : outer_range) {
        collected.push_back(collect(inner_range));
    }

    return collected;
}

}  // namespace

TEST(RangeTraversableTest, TraverseOptionalSuccess)
{
    auto values = smd::ranges::from_vector(std::vector<int>{1, 2, 3});
    const auto& traversable = smd::traversable_typeclass<decltype(values)>;

    auto traversed = traversable.traverse(
        [](int value) -> std::optional<int> {
            return std::optional<int>{value + 1};
        },
        values);

    ASSERT_TRUE(traversed.has_value());
    EXPECT_EQ(collect(*traversed), (std::vector<int>{2, 3, 4}));
}

TEST(RangeTraversableTest, TraverseOptionalFailure)
{
    auto values = smd::ranges::from_vector(std::vector<int>{1, -2, 3});
    const auto& traversable = smd::traversable_typeclass<decltype(values)>;

    auto traversed = traversable.traverse(
        [](int value) -> std::optional<int> {
            return value >= 0 ? std::optional<int>{value + 1}
                              : std::optional<int>{};
        },
        values);

    EXPECT_FALSE(traversed.has_value());
}

TEST(RangeTraversableTest, TraverseWithRangeApplicativeEnumeratesChoices)
{
    auto values = smd::ranges::from_vector(std::vector<int>{1, 2});
    const auto& traversable = smd::traversable_typeclass<decltype(values)>;

    auto traversed = traversable.traverse(
        [](int value) {
            return smd::ranges::from_vector(std::vector<int>{value, value + 10});
        },
        values);

    EXPECT_EQ(
        collect_nested(traversed),
        (std::vector<std::vector<int> >{{1, 2}, {1, 12}, {11, 2}, {11, 12}}));
}

TEST(RangeTraversableTest, TraversableIsNotDefinedForInputOnlyRanges)
{
    using InputView = std::ranges::basic_istream_view<int, char>;
    using InputList = smd::ranges::list_range<InputView>;

    static_assert(std::is_same_v<
        decltype(smd::traversable_typeclass<InputList>),
        const std::false_type>);
}
