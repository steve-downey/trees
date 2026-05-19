// src/smd/tree/finger_tree_rope_ft5.t.cpp                            -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Exercises FingerTreeRope with an explicitly FT5-backed tree and cross-checks
// FT2-backed vs FT5-backed output for semantic equivalence.

#include <smd/tree/finger_tree2.hpp>
#include <smd/tree/finger_tree_rope.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <vector>

using FT5Rope = smd::tree::FingerTreeRope<smd::tree::FingerTree5<
    std::string, std::size_t, smd::tree::RopeChunkMeasure>>;

using FT2Rope = smd::tree::FingerTreeRope<smd::tree::FingerTree2<
    std::string, std::size_t, smd::tree::RopeChunkMeasure>>;

TEST_CASE("RopeFT5 - FromTextAndToString") {
    auto r = FT5Rope::from_text("hello world", 4);
    CHECK(r.to_string() == "hello world");
    CHECK(r.size_bytes() == 11U);
}

TEST_CASE("RopeFT5 - InsertAndErase") {
    auto r = FT5Rope::from_text("hello world");
    auto ins = r.insert(5, ", dear");
    CHECK(ins.to_string() == "hello, dear world");

    auto era = ins.erase(5, 6);
    CHECK(era.to_string() == "hello world");
}

TEST_CASE("RopeFT5 - Replace") {
    auto r = FT5Rope::from_text("foo bar baz");
    auto rep = r.replace(4, 3, "qux");
    CHECK(rep.to_string() == "foo qux baz");
}

TEST_CASE("RopeFT5 - CrossCheckWithFT2") {
    std::string text = "The quick brown fox jumps over the lazy dog";

    auto ft5 = FT5Rope::from_text(text, 8);
    auto ft2 = FT2Rope::from_text(text, 8);

    CHECK(ft5.to_string() == ft2.to_string());
    CHECK(ft5.size_bytes() == ft2.size_bytes());

    auto ft5_ins = ft5.insert(10, "very ");
    auto ft2_ins = ft2.insert(10, "very ");
    CHECK(ft5_ins.to_string() == ft2_ins.to_string());

    auto ft5_era = ft5.erase(4, 6);
    auto ft2_era = ft2.erase(4, 6);
    CHECK(ft5_era.to_string() == ft2_era.to_string());
}
