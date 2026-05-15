// src/smd/tree/finger_tree5.t.cpp                                    -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <smd/tree/finger_tree5.hpp>
#include <smd/tree/finger_tree5.hpp> // Re-inclusion verification

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <string>
#include <vector>

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
    CHECK(t.flatten().empty());
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
    CHECK(t.flatten() == std::vector<int>{42});
}

TEST_CASE("FingerTree5 - DefaultTagIsSizeT")
{
    using FT = smd::tree::FingerTree5<int>;

    static_assert(std::is_same_v<FT::value_type, int>);
    static_assert(std::is_same_v<FT::tag_type, std::size_t>);
}

TEST_CASE("FingerTree5 - ConsSnoc")
{
    using FT = smd::tree::FingerTree5<int>;

    auto t = FT::leaf(5);
    auto t2 = t.cons(3);
    CHECK(t2.measure() == 2U);
    CHECK(t2.head() == 3);
    CHECK(t2.last() == 5);

    auto t3 = t2.snoc(7);
    CHECK(t3.measure() == 3U);
    CHECK(t3.head() == 3);
    CHECK(t3.last() == 7);
}

TEST_CASE("FingerTree5 - DigitOverflowOnCons")
{
    // Exactly exercise the size==4 spill: cons four items, then the fifth
    // forces a Node3 to be pushed down the spine.
    using FT = smd::tree::FingerTree5<int>;

    auto t0 = FT::leaf(0); // Single
    auto t1 = t0.cons(1);  // Deep [1]|[0]
    auto t2 = t1.cons(2);  // Deep [2,1]|[0]
    auto t3 = t2.cons(3);  // Deep [3,2,1]|[0]
    auto t4 = t3.cons(4);  // Deep [4,3,2,1]|[0] — left digit now full
    CHECK(t4.is_branch());
    CHECK(t4.measure() == 5U);

    auto t5 = t4.cons(5);  // Spill: left becomes [5,4], spine gets Node3{3,2,1}
    CHECK(t5.is_branch());
    CHECK(t5.measure() == 6U);
    CHECK(t5.head() == 5);
    CHECK(t5.last() == 0);

    CHECK(t5.flatten() == std::vector<int>{5, 4, 3, 2, 1, 0});
}

TEST_CASE("FingerTree5 - DigitOverflowOnSnoc")
{
    using FT = smd::tree::FingerTree5<int>;

    auto t = FT::empty();
    for (int i = 1; i <= 5; ++i)
        t = t.snoc(i);
    CHECK(t.measure() == 5U);
    CHECK(t.head() == 1);
    CHECK(t.last() == 5);
    CHECK(t.flatten() == std::vector<int>{1, 2, 3, 4, 5});
}

TEST_CASE("FingerTree5 - ConsOverflow")
{
    using FT = smd::tree::FingerTree5<int>;

    auto t = FT::empty();
    for (int i = 0; i < 20; ++i)
        t = t.cons(i);

    CHECK(t.measure() == 20U);
    CHECK(t.head() == 19);
    CHECK(t.last() == 0);

    auto v = t.flatten();
    CHECK(v.size() == 20U);
    for (int i = 0; i < 20; ++i)
        CHECK(v[static_cast<std::size_t>(i)] == 19 - i);
}

TEST_CASE("FingerTree5 - SnocLarge")
{
    using FT = smd::tree::FingerTree5<int>;

    auto t = FT::empty();
    for (int i = 0; i < 200; ++i)
        t = t.snoc(i);

    CHECK(t.measure() == 200U);
    auto v = t.flatten();
    REQUIRE(v.size() == 200U);
    for (int i = 0; i < 200; ++i)
        CHECK(v[static_cast<std::size_t>(i)] == i);
}

TEST_CASE("FingerTree5 - ViewL")
{
    using FT = smd::tree::FingerTree5<int>;

    auto t = FT::empty();
    CHECK_FALSE(t.view_l().has_value());

    t = FT::leaf(42);
    auto v = t.view_l();
    REQUIRE(v.has_value());
    CHECK(v->d_value == 42);
    CHECK(v->d_rest.is_empty());

    auto t5 = FT::from_sequence({10, 20, 30, 40, 50});
    v = t5.view_l();
    REQUIRE(v.has_value());
    CHECK(v->d_value == 10);
    CHECK(v->d_rest.measure() == 4U);
    CHECK(v->d_rest.head() == 20);
}

TEST_CASE("FingerTree5 - ViewR")
{
    using FT = smd::tree::FingerTree5<int>;

    auto t = FT::empty();
    CHECK_FALSE(t.view_r().has_value());

    auto t5 = FT::from_sequence({10, 20, 30, 40, 50});
    auto v = t5.view_r();
    REQUIRE(v.has_value());
    CHECK(v->d_value == 50);
    CHECK(v->d_rest.measure() == 4U);
    CHECK(v->d_rest.last() == 40);
}

TEST_CASE("FingerTree5 - ViewDrainsTreeOrderPreserving")
{
    // Repeated view_l on a deeply-spined tree must drain it in insertion order.
    using FT = smd::tree::FingerTree5<int>;

    auto t = FT::empty();
    for (int i = 0; i < 50; ++i)
        t = t.snoc(i);

    std::vector<int> drained;
    while (auto v = t.view_l()) {
        drained.push_back(v->d_value);
        t = std::move(v->d_rest);
    }
    REQUIRE(drained.size() == 50U);
    for (int i = 0; i < 50; ++i)
        CHECK(drained[static_cast<std::size_t>(i)] == i);
    CHECK(t.is_empty());
}

TEST_CASE("FingerTree5 - HeadTailLastInit")
{
    using FT = smd::tree::FingerTree5<int>;

    auto t = FT::from_sequence({1, 2, 3, 4, 5});
    CHECK(t.head() == 1);
    CHECK(t.last() == 5);
    CHECK(t.tail().head() == 2);
    CHECK(t.init().last() == 4);
    CHECK(t.tail().measure() == 4U);
    CHECK(t.init().measure() == 4U);
}

TEST_CASE("FingerTree5 - Flatten")
{
    using FT = smd::tree::FingerTree5<int>;

    auto t = FT::empty();
    for (int i = 0; i < 100; ++i)
        t = t.snoc(i);

    auto v = t.flatten();
    REQUIRE(v.size() == 100U);
    for (int i = 0; i < 100; ++i)
        CHECK(v[static_cast<std::size_t>(i)] == i);
}

TEST_CASE("FingerTree5 - ForEach")
{
    using FT = smd::tree::FingerTree5<int>;

    auto t = FT::from_sequence({1, 2, 3, 4, 5});
    std::vector<int> collected;
    t.for_each([&](int x) { collected.push_back(x); });
    CHECK(collected == std::vector<int>{1, 2, 3, 4, 5});
}

TEST_CASE("FingerTree5 - FromSequenceEmpty")
{
    using FT = smd::tree::FingerTree5<int>;

    auto t = FT::from_sequence({});
    CHECK(t.is_empty());
    CHECK(t.measure() == 0U);
}

TEST_CASE("FingerTree5 - PersistenceAfterMutation")
{
    using FT = smd::tree::FingerTree5<int>;

    auto t0 = FT::from_sequence({1, 2, 3});
    auto t1 = t0.cons(0);
    auto t2 = t0.snoc(4);

    CHECK(t0.flatten() == std::vector<int>{1, 2, 3});
    CHECK(t1.flatten() == std::vector<int>{0, 1, 2, 3});
    CHECK(t2.flatten() == std::vector<int>{1, 2, 3, 4});
}

TEST_CASE("FingerTree5 - Append")
{
    using FT = smd::tree::FingerTree5<int>;

    auto left = FT::from_sequence({1, 2, 3, 4, 5});
    auto right = FT::from_sequence({6, 7, 8, 9, 10});
    auto combined = left.append(right);

    CHECK(combined.measure() == 10U);
    auto v = combined.flatten();
    REQUIRE(v.size() == 10U);
    for (int i = 0; i < 10; ++i)
        CHECK(v[static_cast<std::size_t>(i)] == i + 1);
}

TEST_CASE("FingerTree5 - AppendLarge")
{
    using FT = smd::tree::FingerTree5<int>;

    auto left = FT::empty();
    for (int i = 0; i < 50; ++i)
        left = left.snoc(i);
    auto right = FT::empty();
    for (int i = 50; i < 100; ++i)
        right = right.snoc(i);

    auto combined = left.append(right);
    CHECK(combined.measure() == 100U);

    auto v = combined.flatten();
    REQUIRE(v.size() == 100U);
    for (int i = 0; i < 100; ++i)
        CHECK(v[static_cast<std::size_t>(i)] == i);
}

TEST_CASE("FingerTree5 - AppendEmpty")
{
    using FT = smd::tree::FingerTree5<int>;

    auto t = FT::from_sequence({1, 2, 3});
    auto e = FT::empty();

    CHECK(t.append(e).flatten() == std::vector<int>{1, 2, 3});
    CHECK(e.append(t).flatten() == std::vector<int>{1, 2, 3});
    CHECK(e.append(e).is_empty());
}

TEST_CASE("FingerTree5 - AppendSingle")
{
    using FT = smd::tree::FingerTree5<int>;

    auto t = FT::from_sequence({1, 2, 3});
    auto s = FT::leaf(99);

    CHECK(t.append(s).flatten() == std::vector<int>{1, 2, 3, 99});
    CHECK(s.append(t).flatten() == std::vector<int>{99, 1, 2, 3});
}

TEST_CASE("FingerTree5 - ConcatMatchesAppend")
{
    using FT = smd::tree::FingerTree5<int>;

    auto a = FT::from_sequence({1, 2, 3});
    auto b = FT::from_sequence({4, 5, 6});

    auto via_member = a.append(b).flatten();
    auto via_static = FT::concat(a, b).flatten();
    CHECK(via_member == via_static);
}

TEST_CASE("FingerTree5 - AppendStressCrossSpine")
{
    // Cross-spine concatenation: both sides have nontrivial spines.
    using FT = smd::tree::FingerTree5<int>;

    auto left = FT::empty();
    for (int i = 0; i < 256; ++i)
        left = left.snoc(i);
    auto right = FT::empty();
    for (int i = 256; i < 512; ++i)
        right = right.snoc(i);

    auto combined = left.append(right);
    CHECK(combined.measure() == 512U);

    auto v = combined.flatten();
    REQUIRE(v.size() == 512U);
    for (int i = 0; i < 512; ++i)
        CHECK(v[static_cast<std::size_t>(i)] == i);
}

TEST_CASE("FingerTree5 - StringElements")
{
    using FT = smd::tree::FingerTree5<std::string>;

    auto t = FT::from_sequence({"alpha", "beta", "gamma", "delta"});
    CHECK(t.measure() == 4U);
    CHECK(t.head() == "alpha");
    CHECK(t.last() == "delta");
}
