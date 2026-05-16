// src/smd/tree/finger_tree_wrappers.t.cpp                            -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <smd/tree/finger_tree_wrappers.hpp>
#include <smd/tree/finger_tree_wrappers.hpp> // Re-inclusion check

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <vector>

TEST_CASE("FingerTreeWrappersTest - RandomAccessBasicOps") {
    auto ra = smd::tree::FingerTreeRandomAccess<int>{};
    ra = ra.push_back(1).push_back(2).push_back(3);
    CHECK(ra.size() == 3);
    CHECK(ra.at(0) == std::optional{1});
    CHECK(ra.at(2) == std::optional{3});
    CHECK(ra.to_vector() == (std::vector<int>{1, 2, 3}));
}

TEST_CASE("FingerTreeWrappersTest - PriorityQueueMinMax") {
    auto pq = smd::tree::FingerTreePriorityQueue<int>{};
    pq = pq.push(3).push(1).push(2);
    CHECK(pq.size() == 3);
    CHECK(pq.min() == 1);
    CHECK(pq.max() == 3);
}

TEST_CASE("FingerTreeWrappersTest - RopeConcat") {
    auto r = smd::tree::FingerTreeRope<>::from_chunks({"hello", " ", "world"});
    CHECK(r.to_string() == "hello world");
    CHECK(r.size_bytes() == 11);
}

TEST_CASE("FingerTreeWrappersTest - IntervalIndexQuery") {
    using Idx = smd::tree::FingerTreeIntervalIndex<int>;
    using Entry = smd::tree::Interval<int>;
    auto idx = Idx{};
    idx = idx.insert(Entry{0, 10, 42}).insert(Entry{5, 15, 99});
    auto hits = idx.query_point(7);
    CHECK(hits.size() == 2);
}
