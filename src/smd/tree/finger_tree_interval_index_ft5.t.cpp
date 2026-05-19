// src/smd/tree/finger_tree_interval_index_ft5.t.cpp                  -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Exercises FingerTreeIntervalIndex with an explicitly FT5-backed tree and
// cross-checks FT2-backed vs FT5-backed output for semantic equivalence.

#include <smd/tree/finger_tree2.hpp>
#include <smd/tree/finger_tree_interval_index.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <vector>

using Entry = smd::tree::Interval<std::string>;
using Tag = smd::tree::IntervalMaxEndTag<std::string>;
using Measure = smd::tree::IntervalMeasure<std::string>;

using FT5Idx = smd::tree::FingerTreeIntervalIndex<
    std::string, smd::tree::FingerTree5<Entry, Tag, Measure>>;

using FT2Idx = smd::tree::FingerTreeIntervalIndex<
    std::string, smd::tree::FingerTree2<Entry, Tag, Measure>>;

TEST_CASE("IntervalIndexFT5 - QueryPoint") {
    auto idx = FT5Idx{};
    idx = idx.insert({0, 10, "A"});
    idx = idx.insert({5, 15, "B"});
    idx = idx.insert({12, 20, "C"});

    auto r7 = idx.query_point(7);
    REQUIRE(r7.size() == 2U);
    CHECK(r7[0] == "A");
    CHECK(r7[1] == "B");

    auto r14 = idx.query_point(14);
    REQUIRE(r14.size() == 2U);
    CHECK(r14[0] == "B");
    CHECK(r14[1] == "C");
}

TEST_CASE("IntervalIndexFT5 - QueryOverlap") {
    auto idx = FT5Idx{};
    idx = idx.insert({0, 5, "A"});
    idx = idx.insert({3, 8, "B"});
    idx = idx.insert({7, 12, "C"});

    // A=[0,5): 0<9 ∧ 4<5 → overlaps; B=[3,8): 3<9 ∧ 4<8 → overlaps;
    // C=[7,12): 7<9 ∧ 4<12 → overlaps.  All three match [4,9).
    auto r = idx.query_overlap(4, 9);
    REQUIRE(r.size() == 3U);
    CHECK(r[0] == "A");
    CHECK(r[1] == "B");
    CHECK(r[2] == "C");
}

TEST_CASE("IntervalIndexFT5 - CrossCheckWithFT2") {
    std::vector<Entry> intervals = {
        {0, 10, "A"}, {5, 15, "B"}, {3, 8, "C"}, {12, 20, "D"}, {1, 6, "E"}};

    auto ft5 = FT5Idx::from_intervals(intervals);
    auto ft2 = FT2Idx::from_intervals(intervals);

    // Entries should be stored in the same insertion order.
    auto ft5_entries = ft5.entries();
    auto ft2_entries = ft2.entries();
    REQUIRE(ft5_entries.size() == ft2_entries.size());
    for (std::size_t i = 0; i < ft5_entries.size(); ++i) {
        CHECK(ft5_entries[i].d_start == ft2_entries[i].d_start);
        CHECK(ft5_entries[i].d_end == ft2_entries[i].d_end);
        CHECK(ft5_entries[i].d_payload == ft2_entries[i].d_payload);
    }

    // Point queries at several points should agree.
    for (std::size_t pt : {0U, 4U, 7U, 11U, 15U}) {
        CHECK(ft5.query_point(pt) == ft2.query_point(pt));
    }

    // Overlap queries should agree.
    CHECK(ft5.query_overlap(2, 9) == ft2.query_overlap(2, 9));
    CHECK(ft5.query_overlap(10, 18) == ft2.query_overlap(10, 18));
}
