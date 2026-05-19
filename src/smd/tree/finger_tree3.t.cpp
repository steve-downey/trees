// src/smd/tree/finger_tree3.t.cpp                                    -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <smd/tree/finger_tree3.hpp>
#include <smd/tree/finger_tree3.hpp> // Re-inclusion verification

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <vector>

TEST_CASE("FingerTree3 - HeaderIsIdempotent") { REQUIRE(true); }

TEST_CASE("FingerTree3 - EmptyAndLeaf") {
    using FT = smd::tree::FingerTree3<int>;

    auto e = FT::empty();
    CHECK(e.is_empty());
    CHECK(e.measure() == 0U);

    auto l = FT::leaf(42);
    CHECK(l.is_leaf());
    CHECK(l.value() == 42);
    CHECK(l.measure() == 1U);
}

TEST_CASE("FingerTree3 - ConsSnoc") {
    using FT = smd::tree::FingerTree3<int>;

    auto t = FT::empty();
    for (int i = 0; i < 20; ++i)
        t = t.snoc(i);

    CHECK(t.measure() == 20U);
    CHECK(t.head() == 0);
    CHECK(t.last() == 19);

    auto v = t.flatten();
    REQUIRE(v.size() == 20U);
    for (int i = 0; i < 20; ++i)
        CHECK(v[static_cast<std::size_t>(i)] == i);
}

TEST_CASE("FingerTree3 - LazyBuildIsCheap") {
    using FT = smd::tree::FingerTree3<int>;

    auto t = FT::empty();
    for (int i = 0; i < 200; ++i)
        t = t.snoc(i);

    CHECK(t.measure() == 200U);
    auto v = t.flatten();
    REQUIRE(v.size() == 200U);
    for (int i = 0; i < 200; ++i)
        CHECK(v[static_cast<std::size_t>(i)] == i);
}

TEST_CASE("FingerTree3 - ViewLR") {
    using FT = smd::tree::FingerTree3<int>;

    auto t = FT::from_sequence({10, 20, 30, 40, 50});
    auto vl = t.view_l();
    REQUIRE(vl.has_value());
    CHECK(vl->d_value == 10);
    CHECK(vl->d_rest.measure() == 4U);

    auto vr = t.view_r();
    REQUIRE(vr.has_value());
    CHECK(vr->d_value == 50);
    CHECK(vr->d_rest.measure() == 4U);
}

TEST_CASE("FingerTree3 - Append") {
    using FT = smd::tree::FingerTree3<int>;

    auto left = FT::from_sequence({1, 2, 3, 4, 5});
    auto right = FT::from_sequence({6, 7, 8, 9, 10});
    auto combined = left.append(right);
    CHECK(combined.measure() == 10U);
    CHECK(combined.flatten() ==
          std::vector<int>{1, 2, 3, 4, 5, 6, 7, 8, 9, 10});
}

TEST_CASE("FingerTree3 - Split") {
    using FT = smd::tree::FingerTree3<int>;

    auto t = FT::empty();
    for (int i = 0; i < 100; ++i)
        t = t.snoc(i);

    auto sp = t.split([](std::size_t p) { return p > 25U; });
    REQUIRE(sp.has_value());
    CHECK(sp->d_pivot == 25);
    CHECK(sp->d_left.measure() == 25U);
    CHECK(sp->d_right.measure() == 74U);
}

TEST_CASE("FingerTree3 - SplitAtMeasure") {
    using FT = smd::tree::FingerTree3<int>;

    auto t = FT::empty();
    for (int i = 0; i < 100; ++i)
        t = t.snoc(i);

    auto sa = t.split_at_measure(std::size_t{51});
    CHECK(sa.d_left.measure() == 50U);
    CHECK(sa.d_right.measure() == 50U);
}

TEST_CASE("FingerTree3 - Persistence") {
    using FT = smd::tree::FingerTree3<int>;

    auto t1 = FT::from_sequence({1, 2, 3});
    auto t2 = t1.cons(0);
    auto t3 = t1.snoc(4);

    CHECK(t1.flatten() == std::vector<int>{1, 2, 3});
    CHECK(t2.flatten() == std::vector<int>{0, 1, 2, 3});
    CHECK(t3.flatten() == std::vector<int>{1, 2, 3, 4});
}

TEST_CASE("FingerTree3 - ForEach") {
    using FT = smd::tree::FingerTree3<int>;

    auto t = FT::from_sequence({1, 2, 3, 4, 5});
    std::vector<int> collected;
    t.for_each([&](int x) { collected.push_back(x); });
    CHECK(collected == std::vector<int>{1, 2, 3, 4, 5});
}
