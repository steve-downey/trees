#include <smd/ranges/range_functor.hpp>

#include <catch2/catch_test_macros.hpp>

#include <ranges>
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

TEST_CASE("RangeFunctorTest - FmapUsesLazyRangeSemantics")
{
    auto values = smd::ranges::all(std::views::iota(1, 5));
    const auto& functor = smd::functor_typeclass<decltype(values)>;

    auto mapped = functor.fmap([](int value) { return value * 10; }, values);

    CHECK(collect(mapped) == (std::vector<int>{10, 20, 30, 40}));
}

TEST_CASE("RangeFunctorTest - ReplaceKeepsRangeShape")
{
    auto values = smd::ranges::all(std::views::iota(0, 3));
    const auto& functor = smd::functor_typeclass<decltype(values)>;

    auto replaced = functor.replace(values, 9);

    CHECK(collect(replaced) == (std::vector<int>{9, 9, 9}));
}
