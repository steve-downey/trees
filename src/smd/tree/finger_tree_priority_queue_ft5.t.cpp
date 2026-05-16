// src/smd/tree/finger_tree_priority_queue_ft5.t.cpp                  -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Exercises FingerTreePriorityQueue with an explicitly FT5-backed tree and
// cross-checks FT2-backed vs FT5-backed output for semantic equivalence.

#include <smd/tree/finger_tree_priority_queue.hpp>
#include <smd/tree/finger_tree2.hpp>

#include <catch2/catch_test_macros.hpp>

#include <optional>
#include <vector>

using FT5PQ = smd::tree::FingerTreePriorityQueue<
    int, smd::tree::FingerTree5<int, smd::tree::PriorityTag<int>,
                                smd::tree::PriorityMeasure<int>>>;

using FT2PQ = smd::tree::FingerTreePriorityQueue<
    int, smd::tree::FingerTree2<int, smd::tree::PriorityTag<int>,
                                smd::tree::PriorityMeasure<int>>>;

TEST_CASE("PriorityQueueFT5 - PushAndMinMax")
{
    auto pq = FT5PQ{};
    for (int v : {5, 1, 3, 9, 2})
        pq = pq.push(v);

    CHECK(pq.min() == std::optional{1});
    CHECK(pq.max() == std::optional{9});
}

TEST_CASE("PriorityQueueFT5 - PopMin")
{
    auto pq = FT5PQ::from_values({3, 1, 4, 1, 5, 9, 2, 6});

    auto r1 = pq.pop_min();
    REQUIRE(r1.has_value());
    CHECK(r1->first == 1);

    auto r2 = r1->second.pop_min();
    REQUIRE(r2.has_value());
    CHECK(r2->first == 1);
}

TEST_CASE("PriorityQueueFT5 - PopMax")
{
    auto pq = FT5PQ::from_values({3, 1, 4, 1, 5, 9, 2, 6});

    auto r = pq.pop_max();
    REQUIRE(r.has_value());
    CHECK(r->first == 9);
}

TEST_CASE("PriorityQueueFT5 - CrossCheckWithFT2")
{
    std::vector<int> vals = {7, 2, 9, 1, 5, 3, 8, 4, 6};

    auto ft5 = FT5PQ::from_values(vals);
    auto ft2 = FT2PQ::from_values(vals);

    CHECK(ft5.min() == ft2.min());
    CHECK(ft5.max() == ft2.max());
    CHECK(ft5.to_vector() == ft2.to_vector());

    auto ft5_r = ft5.pop_min();
    auto ft2_r = ft2.pop_min();
    REQUIRE(ft5_r.has_value());
    REQUIRE(ft2_r.has_value());
    CHECK(ft5_r->first == ft2_r->first);
}
