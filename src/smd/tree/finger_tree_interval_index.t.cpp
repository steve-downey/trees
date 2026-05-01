#include <smd/tree/finger_tree_interval_index.hpp>
#include <smd/tree/finger_tree_interval_index.hpp>  // Re-inclusion check

#include <catch2/catch_test_macros.hpp>

#include <optional>
#include <string>
#include <vector>

TEST_CASE("FingerTreeIntervalIndexTest - WrapperOperations")
{
    using Index = smd::tree::FingerTreeIntervalIndex<std::string>;
    using Entry = smd::tree::Interval<std::string>;

    auto idx = Index::from_intervals({
        Entry{0U, 5U, "A"},
        Entry{3U, 10U, "B"},
        Entry{8U, 12U, "C"}
    });

    CHECK(idx.query_point(2U) == (std::vector<std::string>{"A"}));
    CHECK(idx.query_point(4U) == (std::vector<std::string>{"A", "B"}));
    CHECK(idx.query_overlap(9U, 11U) == (std::vector<std::string>{"B", "C"}));
}

TEST_CASE("FingerTreeIntervalIndexTest - FoldableTypeclass")
{
    using Index = smd::tree::FingerTreeIntervalIndex<std::string>;
    using Entry = smd::tree::Interval<std::string>;

    auto idx = Index::from_intervals({
      Entry{0U, 5U, "A"},
      Entry{3U, 10U, "B"},
      Entry{8U, 12U, "C"}
    });
    const auto& foldable = smd::foldable_typeclass<Index>;

    CHECK(
      foldable.fold_map([](const std::string& payload) { return payload; }, idx) ==
      "ABC");
    CHECK(foldable.length(idx) == 3U);
}

TEST_CASE("FingerTreeIntervalIndexTest - TraversableTypeclass")
{
    using Index = smd::tree::FingerTreeIntervalIndex<std::string>;
    using Entry = smd::tree::Interval<std::string>;

    auto idx = Index::from_intervals({
      Entry{0U, 5U, "A"},
      Entry{3U, 10U, "B"},
      Entry{8U, 12U, "C"}
    });

    auto success = smd::traverse(
      [](const std::string& payload) -> std::optional<std::string> {
          return payload + "!";
      },
      idx);
    REQUIRE(success.has_value());
    CHECK(success->query_point(4U) == (std::vector<std::string>{"A!", "B!"}));
    CHECK(success->query_overlap(9U, 11U) ==
              (std::vector<std::string>{"B!", "C!"}));

    auto failure = smd::traverse(
      [](const std::string& payload) -> std::optional<std::string> {
          if (payload == "B") {
              return std::nullopt;
          }
          return payload;
      },
      idx);
    CHECK_FALSE(failure.has_value());
}
