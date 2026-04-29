#include <smd/ranges/range_applicative.hpp>

#include <gtest/gtest.h>

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

}  // namespace

TEST(RangeApplicativeTest, PureCreatesSingletonRange)
{
    const auto& applicative =
        smd::applicative_typeclass<decltype(smd::ranges::single(1))>;

    auto singleton = applicative.pure(7);

    EXPECT_EQ(collect(singleton), (std::vector<int>{7}));
}

TEST(RangeApplicativeTest, InvokeUsesListNondeterminism)
{
    auto lhs = smd::ranges::from_vector(std::vector<int>{1, 2});
    auto rhs = smd::ranges::from_vector(std::vector<int>{10, 20});
    const auto& applicative = smd::applicative_typeclass<decltype(lhs)>;

    auto summed = applicative.invoke(
        [](int left, int right) { return left + right; },
        lhs,
        rhs);

    EXPECT_EQ(collect(summed), (std::vector<int>{11, 21, 12, 22}));
}
