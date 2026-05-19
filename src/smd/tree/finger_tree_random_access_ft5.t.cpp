// src/smd/tree/finger_tree_random_access_ft5.t.cpp                   -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Exercises FingerTreeRandomAccess with an explicitly FT5-backed tree,
// and cross-checks FT2-backed vs FT5-backed output for semantic equivalence.

#include <smd/tree/finger_tree2.hpp> // for cross-check test
#include <smd/tree/finger_tree_random_access.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <optional>
#include <vector>

// Explicit FT5-backed type alias.
using FT5Seq = smd::tree::FingerTreeRandomAccess<
    int, smd::tree::FingerTree5<int, std::size_t,
                                smd::tree::UnitMeasure5<int, std::size_t>>>;

// FT2-backed type alias for cross-check.
using FT2Seq = smd::tree::FingerTreeRandomAccess<
    int, smd::tree::FingerTree2<int, std::size_t,
                                smd::tree::UnitMeasure2<int, std::size_t>>>;

TEST_CASE("RandomAccessFT5 - EmptyAndSize") {
    FT5Seq seq;
    CHECK(seq.empty());
    CHECK(seq.size() == 0U);
}

TEST_CASE("RandomAccessFT5 - PushBackAndAt") {
    auto seq = FT5Seq{};
    for (int i = 0; i < 10; ++i)
        seq = seq.push_back(i);

    CHECK(seq.size() == 10U);
    for (int i = 0; i < 10; ++i)
        CHECK(seq.at(static_cast<std::size_t>(i)) == std::optional{i});

    CHECK(!seq.at(10).has_value());
}

TEST_CASE("RandomAccessFT5 - PushFront") {
    auto seq = FT5Seq{};
    for (int i = 0; i < 5; ++i)
        seq = seq.push_front(i); // 4 3 2 1 0

    CHECK(seq.to_vector() == (std::vector<int>{4, 3, 2, 1, 0}));
}

TEST_CASE("RandomAccessFT5 - InsertAndErase") {
    auto seq = FT5Seq::from_sequence({1, 2, 4, 5});
    auto ins = seq.insert(2, 3); // {1, 2, 3, 4, 5}
    CHECK(ins.to_vector() == (std::vector<int>{1, 2, 3, 4, 5}));

    auto era = ins.erase(2); // {1, 2, 4, 5}
    CHECK(era.to_vector() == (std::vector<int>{1, 2, 4, 5}));
}

TEST_CASE("RandomAccessFT5 - Update") {
    auto seq = FT5Seq::from_sequence({1, 2, 3, 4, 5});
    auto upd = seq.update(2, 99);
    CHECK(upd.to_vector() == (std::vector<int>{1, 2, 99, 4, 5}));
}

TEST_CASE("RandomAccessFT5 - LargeTree") {
    // Exercises spine structure (> 4 elements forces a non-trivial spine).
    auto seq = FT5Seq{};
    for (int i = 0; i < 300; ++i)
        seq = seq.push_back(i);

    CHECK(seq.size() == 300U);
    CHECK(seq.at(0) == std::optional{0});
    CHECK(seq.at(150) == std::optional{150});
    CHECK(seq.at(299) == std::optional{299});
}

TEST_CASE("RandomAccessFT5 - CrossCheckWithFT2") {
    // Build the same content with FT5-backed and FT2-backed wrappers,
    // then verify all operations return equivalent results.
    constexpr int kN = 50;

    FT5Seq ft5_seq = FT5Seq::from_sequence({});
    FT2Seq ft2_seq = FT2Seq::from_sequence({});

    for (int i = 0; i < kN; ++i) {
        ft5_seq = ft5_seq.push_back(i);
        ft2_seq = ft2_seq.push_back(i);
    }

    // size
    CHECK(ft5_seq.size() == ft2_seq.size());

    // to_vector
    CHECK(ft5_seq.to_vector() == ft2_seq.to_vector());

    // at — every index
    for (int i = 0; i < kN; ++i) {
        auto idx = static_cast<std::size_t>(i);
        CHECK(ft5_seq.at(idx) == ft2_seq.at(idx));
    }

    // insert at midpoint
    auto ft5_ins = ft5_seq.insert(25, 999);
    auto ft2_ins = ft2_seq.insert(25, 999);
    CHECK(ft5_ins.to_vector() == ft2_ins.to_vector());

    // erase at midpoint
    auto ft5_era = ft5_seq.erase(25);
    auto ft2_era = ft2_seq.erase(25);
    CHECK(ft5_era.to_vector() == ft2_era.to_vector());

    // update at midpoint
    auto ft5_upd = ft5_seq.update(25, -1);
    auto ft2_upd = ft2_seq.update(25, -1);
    CHECK(ft5_upd.to_vector() == ft2_upd.to_vector());
}
