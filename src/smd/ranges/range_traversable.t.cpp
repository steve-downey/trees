#include <smd/ranges/range_traversable.hpp>
#include <smd/ranges/range_traversable.hpp> // Re-inclusion check
#include <smd/ziplist/zip_list_applicative.hpp>

#include <catch2/catch_test_macros.hpp>

#include <optional>
#include <ranges>
#include <sstream>
#include <type_traits>
#include <vector>

#include <algorithm>

namespace {

template <std::ranges::input_range RANGE>
auto collect(RANGE &&range) {
    using Value = std::ranges::range_value_t<RANGE>;
    std::vector<Value> values;

    for (auto &&value : range) {
        values.emplace_back(value);
    }

    return values;
}

template <std::ranges::input_range OUTER_RANGE>
auto collect_nested(OUTER_RANGE &&outer_range) {
    std::vector<std::vector<
        std::ranges::range_value_t<std::ranges::range_value_t<OUTER_RANGE>>>>
        collected;

    for (auto &&inner_range : outer_range) {
        collected.push_back(collect(inner_range));
    }

    return collected;
}

auto to_vector_of_ziplists(
    const smd::zip_list<std::vector<int>> &zip_of_vectors)
    -> std::vector<smd::zip_list<int>> {
    std::vector<smd::zip_list<int>> rows;
    if (zip_of_vectors.data.empty()) {
        return rows;
    }

    std::size_t row_count = zip_of_vectors.data.front().size();
    for (const auto &column : zip_of_vectors.data) {
        row_count = std::min(row_count, column.size());
    }

    rows.assign(row_count, smd::zip_list<int>{});
    for (auto &row : rows) {
        row.data.reserve(zip_of_vectors.data.size());
    }

    for (std::size_t index = 0; index < row_count; ++index) {
        for (const auto &column : zip_of_vectors.data) {
            rows[index].data.push_back(column[index]);
        }
    }

    return rows;
}

} // namespace

TEST_CASE("RangeTraversableTest - TraverseOptionalSuccess") {
    // c7f3a1e8-2b5d-4f9c-a4e7-1b3d6c8a5f02
    auto values = smd::ranges::from_vector(std::vector<int>{1, 2, 3});

    auto traversed = smd::traverse(
        [](int value) -> std::optional<int> {
            return std::optional<int>{value + 1};
        },
        values);

    REQUIRE(traversed.has_value());
    CHECK(collect(*traversed) == (std::vector<int>{2, 3, 4}));
    // c7f3a1e8-2b5d-4f9c-a4e7-1b3d6c8a5f02 end
}

TEST_CASE("RangeTraversableTest - TraverseOptionalFailure") {
    // e9b1d4f2-7c3a-4e8b-f6c2-5d1a9e3b7f04
    auto values = smd::ranges::from_vector(std::vector<int>{1, -2, 3});

    auto traversed = smd::traverse(
        [](int value) -> std::optional<int> {
            return value >= 0 ? std::optional<int>{value + 1}
                              : std::optional<int>{};
        },
        values);

    CHECK_FALSE(traversed.has_value());
    // e9b1d4f2-7c3a-4e8b-f6c2-5d1a9e3b7f04 end
}

TEST_CASE(
    "RangeTraversableTest - TraverseWithRangeApplicativeEnumeratesChoices") {
    auto values = smd::ranges::from_vector(std::vector<int>{1, 2});

    auto traversed = smd::traverse(
        [](int value) {
            return smd::ranges::from_vector(
                std::vector<int>{value, value + 10});
        },
        values);

    CHECK(collect_nested(traversed) ==
          (std::vector<std::vector<int>>{{1, 2}, {1, 12}, {11, 2}, {11, 12}}));
}

TEST_CASE("RangeTraversableTest - TraversableIsNotDefinedForInputOnlyRanges") {
    using InputView = std::ranges::basic_istream_view<int, char>;
    using InputList = smd::ranges::list_range<InputView>;

    static_assert(
        std::is_same_v<decltype(smd::traversable_typeclass<InputList>),
                       const std::false_type>);
}

TEST_CASE(
    "RangeTraversableTest - TransposeConvertsRangeOfZiplistsToZiplistOfRanges") {
    // 0e9a7d13-9082-4b9e-b93f-86ef0e0ba20a
    using Zip = smd::zip_list<int>;
    auto values = smd::ranges::from_vector(std::vector<Zip>{
        Zip{{1, 2, 3}}, Zip{{10, 20}}, Zip{{100, 200, 300, 400}}});

    const auto &traversable = smd::traversable_typeclass<decltype(values)>;
    // d4f9b1e3-8c2a-4d7f-b6e1-3a5c9d2b7f48
    auto transposed = traversable.transpose(values);

    REQUIRE(transposed.data.size() == 2U);
    CHECK(collect(transposed.data[0]) == (std::vector<int>{1, 10, 100}));
    CHECK(collect(transposed.data[1]) == (std::vector<int>{2, 20, 200}));
    // d4f9b1e3-8c2a-4d7f-b6e1-3a5c9d2b7f48 end
    // 0e9a7d13-9082-4b9e-b93f-86ef0e0ba20a end
}

TEST_CASE("RangeTraversableTest - "
          "TransposeConvertsRangeOfZiplistsToZiplistOfRangesLengthFive") {
    using Zip = smd::zip_list<int>;
    auto values = smd::ranges::from_vector(
        std::vector<Zip>{Zip{{1, 2, 3, 4, 5}}, Zip{{10, 20, 30, 40, 50}},
                         Zip{{100, 200, 300, 400, 500}}});

    const auto &traversable = smd::traversable_typeclass<decltype(values)>;
    auto transposed = traversable.transpose(values);

    REQUIRE(transposed.data.size() == 5U);
    CHECK(collect(transposed.data[0]) == (std::vector<int>{1, 10, 100}));
    CHECK(collect(transposed.data[1]) == (std::vector<int>{2, 20, 200}));
    CHECK(collect(transposed.data[2]) == (std::vector<int>{3, 30, 300}));
    CHECK(collect(transposed.data[3]) == (std::vector<int>{4, 40, 400}));
    CHECK(collect(transposed.data[4]) == (std::vector<int>{5, 50, 500}));
}

TEST_CASE("RangeTraversableTest - ConvertZiplistOfVectorsToVectorOfZiplists") {
    // 4be89584-35cc-4933-b3de-6d524d54371d
    smd::zip_list<std::vector<int>> zip_of_vectors{
        {{1, 10, 100}, {2, 20, 200}}};

    auto as_rows = to_vector_of_ziplists(zip_of_vectors);

    REQUIRE(as_rows.size() == 3U);
    CHECK(as_rows[0].data == (std::vector<int>{1, 2}));
    CHECK(as_rows[1].data == (std::vector<int>{10, 20}));
    CHECK(as_rows[2].data == (std::vector<int>{100, 200}));
    // 4be89584-35cc-4933-b3de-6d524d54371d end
}

TEST_CASE("RangeTraversableTest - "
          "ConvertZiplistOfVectorsToVectorOfZiplistsLengthFive") {
    smd::zip_list<std::vector<int>> zip_of_vectors{
        {{1, 10, 100, 1000, 10000}, {2, 20, 200, 2000, 20000}}};

    auto as_rows = to_vector_of_ziplists(zip_of_vectors);

    REQUIRE(as_rows.size() == 5U);
    CHECK(as_rows[0].data == (std::vector<int>{1, 2}));
    CHECK(as_rows[1].data == (std::vector<int>{10, 20}));
    CHECK(as_rows[2].data == (std::vector<int>{100, 200}));
    CHECK(as_rows[3].data == (std::vector<int>{1000, 2000}));
    CHECK(as_rows[4].data == (std::vector<int>{10000, 20000}));
}
