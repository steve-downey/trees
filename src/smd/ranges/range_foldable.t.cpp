#include <smd/ranges/range_foldable.hpp>
#include <smd/ranges/range_foldable.hpp> // Re-inclusion check

#include <catch2/catch_test_macros.hpp>

#include <ranges>
#include <vector>

TEST_CASE("RangeFoldableTest - LengthAndFoldLeftFollowRangeOrder") {
    auto values = smd::ranges::all(std::views::iota(1, 5));
    const auto &foldable = smd::foldable_typeclass<decltype(values)>;

    CHECK(foldable.length(values) == 4U);

    const auto folded = foldable.fold_left(
        values, 0, [](int acc, int value) { return acc * 10 + value; });
    CHECK(folded == 1234);
}

TEST_CASE("RangeFoldableTest - ToVectorMaterializesValues") {
    auto values = smd::ranges::from_vector(std::vector<int>{3, 1, 4});
    const auto &foldable = smd::foldable_typeclass<decltype(values)>;

    CHECK(foldable.to_vector(values) == (std::vector<int>{3, 1, 4}));
}

TEST_CASE("RangeFoldableTest - PredicatesAndFindUseRangeSemantics") {
    auto values = smd::ranges::all(std::views::iota(1, 6));
    const auto &foldable = smd::foldable_typeclass<decltype(values)>;

    CHECK(foldable.any_of(values, [](int value) { return value == 4; }));
    CHECK_FALSE(foldable.all_of(values, [](int value) { return value < 5; }));
    CHECK_FALSE(foldable.empty(values));

    auto found =
        foldable.find_first(values, [](int value) { return value > 3; });
    REQUIRE(found.has_value());
    CHECK(*found == 4);
}
