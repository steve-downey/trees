// src/smd/tree/finger_tree5.t.cpp                                    -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <smd/tree/finger_tree5.hpp>
#include <smd/tree/finger_tree5.hpp> // Re-inclusion verification

#include <catch2/catch_test_macros.hpp>

#include <cstddef>

TEST_CASE("FingerTree5 - HeaderIsIdempotent")
{
    REQUIRE(true);
}

TEST_CASE("FingerTree5 - Empty")
{
    using FT = smd::tree::FingerTree5<int>;

    auto t = FT::empty();
    CHECK(t.is_empty());
    CHECK_FALSE(t.is_leaf());
    CHECK_FALSE(t.is_branch());
    CHECK(t.measure() == 0U);
}

TEST_CASE("FingerTree5 - Leaf")
{
    using FT = smd::tree::FingerTree5<int>;

    auto t = FT::leaf(42);
    CHECK_FALSE(t.is_empty());
    CHECK(t.is_leaf());
    CHECK_FALSE(t.is_branch());
    CHECK(t.measure() == 1U);
    CHECK(t.value() == 42);
}

TEST_CASE("FingerTree5 - DefaultTagIsSizeT")
{
    using FT = smd::tree::FingerTree5<int>;

    static_assert(std::is_same_v<FT::value_type, int>);
    static_assert(std::is_same_v<FT::tag_type, std::size_t>);
}

TEST_CASE("FingerTree5 - PersistenceOfEmptyAndLeaf")
{
    using FT = smd::tree::FingerTree5<int>;

    auto e = FT::empty();
    auto l = FT::leaf(7);
    CHECK(e.is_empty());
    CHECK(l.is_leaf());
    CHECK(l.value() == 7);

    auto e2 = e; // copy
    auto l2 = l;
    CHECK(e2.is_empty());
    CHECK(l2.value() == 7);
    CHECK(l.value() == 7); // original unchanged
}
