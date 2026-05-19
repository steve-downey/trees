// src/smd/tree/finger_tree_interval_index.t.cpp                      -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <smd/tree/finger_tree_interval_index.hpp>
#include <smd/tree/finger_tree_interval_index.hpp> // Re-inclusion check

#include <smd/typeclass/foldable.hpp>
#include <smd/typeclass/traversable.hpp>

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cstddef>
#include <optional>
#include <string>
#include <vector>

using Index = smd::tree::FingerTreeIntervalIndex<std::string>;
using Entry = smd::tree::Interval<std::string>;

TEST_CASE("IntervalIndex - HeaderIsIdempotent") { REQUIRE(true); }

TEST_CASE("IntervalIndex - Empty") {
    Index idx;
    CHECK(idx.query_point(0U).empty());
    CHECK(idx.query_point(100U).empty());
    CHECK(idx.query_overlap(0U, 100U).empty());
}

TEST_CASE("IntervalIndex - SingleInterval") {
    auto idx = Index::from_intervals({Entry{5U, 10U, "A"}});
    CHECK(idx.query_point(4U).empty());
    CHECK(idx.query_point(5U) == std::vector<std::string>{"A"});
    CHECK(idx.query_point(9U) == std::vector<std::string>{"A"});
    CHECK(idx.query_point(10U).empty());
}

TEST_CASE("IntervalIndex - ThreeOverlapping") {
    auto idx = Index::from_intervals(
        {Entry{0U, 5U, "A"}, Entry{3U, 10U, "B"}, Entry{8U, 12U, "C"}});

    CHECK(idx.query_point(2U) == std::vector<std::string>{"A"});
    CHECK(idx.query_point(4U) == std::vector<std::string>{"A", "B"});
    CHECK(idx.query_point(6U) == std::vector<std::string>{"B"});
    CHECK(idx.query_point(9U) == std::vector<std::string>{"B", "C"});
    CHECK(idx.query_point(11U) == std::vector<std::string>{"C"});
    CHECK(idx.query_point(12U).empty());
}

TEST_CASE("IntervalIndex - OverlapQuery") {
    auto idx = Index::from_intervals(
        {Entry{0U, 5U, "A"}, Entry{3U, 10U, "B"}, Entry{8U, 12U, "C"}});

    CHECK(idx.query_overlap(9U, 11U) == std::vector<std::string>{"B", "C"});
    CHECK(idx.query_overlap(0U, 1U) == std::vector<std::string>{"A"});
    CHECK(idx.query_overlap(12U, 20U).empty());
    CHECK(idx.query_overlap(0U, 12U) ==
          std::vector<std::string>{"A", "B", "C"});
}

TEST_CASE("IntervalIndex - BoundaryConditions") {
    // Half-open intervals: [start, end)
    auto idx = Index::from_intervals({Entry{0U, 5U, "A"}, Entry{5U, 10U, "B"}});

    // Point 5 is in B (start of B) but not in A (end of A)
    CHECK(idx.query_point(5U) == std::vector<std::string>{"B"});
    CHECK(idx.query_point(4U) == std::vector<std::string>{"A"});

    // Overlap [4,6) should hit both
    CHECK(idx.query_overlap(4U, 6U) == std::vector<std::string>{"A", "B"});
    // Overlap [5,6) should hit only B
    CHECK(idx.query_overlap(5U, 6U) == std::vector<std::string>{"B"});
}

TEST_CASE("IntervalIndex - IncrementalInsert") {
    auto idx = Index();
    idx = idx.insert(Entry{0U, 10U, "first"});
    CHECK(idx.query_point(5U) == std::vector<std::string>{"first"});

    idx = idx.insert(Entry{5U, 15U, "second"});
    CHECK(idx.query_point(7U) == std::vector<std::string>{"first", "second"});

    idx = idx.insert(Entry{20U, 30U, "third"});
    CHECK(idx.query_point(25U) == std::vector<std::string>{"third"});
    CHECK(idx.query_point(7U) == std::vector<std::string>{"first", "second"});
}

TEST_CASE("IntervalIndex - SpineTransition") {
    // Build 20 non-overlapping intervals → spine structure
    std::vector<Entry> intervals;
    for (int i = 0; i < 20; ++i)
        intervals.push_back(Entry{static_cast<std::size_t>(i * 10),
                                  static_cast<std::size_t>(i * 10 + 5),
                                  "I" + std::to_string(i)});

    auto idx = Index::from_intervals(intervals);

    for (int i = 0; i < 20; ++i) {
        auto pt = static_cast<std::size_t>(i * 10 + 2);
        auto result = idx.query_point(pt);
        REQUIRE(result.size() == 1U);
        CHECK(result[0] == "I" + std::to_string(i));
    }

    // Query gaps between intervals
    CHECK(idx.query_point(7U).empty());
    CHECK(idx.query_point(17U).empty());
}

TEST_CASE("IntervalIndex - ManyOverlapsAtSamePoint") {
    // 15 intervals all covering point 50
    std::vector<Entry> intervals;
    for (int i = 0; i < 15; ++i)
        intervals.push_back(Entry{static_cast<std::size_t>(50 - i),
                                  static_cast<std::size_t>(50 + i + 1),
                                  "V" + std::to_string(i)});

    auto idx = Index::from_intervals(intervals);
    auto result = idx.query_point(50U);
    CHECK(result.size() == 15U);
}

TEST_CASE("IntervalIndex - LargeVsBruteForce") {
    // Build 200 intervals, verify queries match brute-force
    std::vector<Entry> intervals;
    for (int i = 0; i < 200; ++i)
        intervals.push_back(Entry{static_cast<std::size_t>(i * 3),
                                  static_cast<std::size_t>(i * 3 + 10),
                                  "E" + std::to_string(i)});

    auto idx = Index::from_intervals(intervals);

    // Test a selection of query points
    for (std::size_t pt : {0U, 50U, 100U, 200U, 300U, 500U, 598U}) {
        auto tree_result = idx.query_point(pt);
        std::vector<std::string> brute;
        for (auto &e : intervals) {
            if (pt >= e.d_start && pt < e.d_end)
                brute.push_back(e.d_payload);
        }
        std::sort(tree_result.begin(), tree_result.end());
        std::sort(brute.begin(), brute.end());
        CHECK(tree_result == brute);
    }
}

TEST_CASE("IntervalIndex - Persistence") {
    auto idx1 = Index::from_intervals({Entry{0U, 10U, "A"}});
    auto idx2 = idx1.insert(Entry{5U, 15U, "B"});

    CHECK(idx1.query_point(7U) == std::vector<std::string>{"A"});
    CHECK(idx2.query_point(7U) == std::vector<std::string>{"A", "B"});
}

TEST_CASE("IntervalIndex - FoldableTypeclass") {
    auto idx = Index::from_intervals(
        {Entry{0U, 5U, "A"}, Entry{3U, 10U, "B"}, Entry{8U, 12U, "C"}});
    const auto &foldable = smd::foldable_typeclass<Index>;

    CHECK(foldable.fold_map([](const std::string &p) { return p; }, idx) ==
          "ABC");
    CHECK(foldable.length(idx) == 3U);
}

TEST_CASE("IntervalIndex - TraversableTypeclass") {
    auto idx = Index::from_intervals(
        {Entry{0U, 5U, "A"}, Entry{3U, 10U, "B"}, Entry{8U, 12U, "C"}});

    auto success = smd::traverse(
        [](const std::string &p) -> std::optional<std::string> {
            return p + "!";
        },
        idx);
    REQUIRE(success.has_value());
    CHECK(success->query_point(4U) == std::vector<std::string>{"A!", "B!"});

    auto failure = smd::traverse(
        [](const std::string &p) -> std::optional<std::string> {
            return p == "B" ? std::nullopt : std::optional{p};
        },
        idx);
    CHECK_FALSE(failure.has_value());
}
