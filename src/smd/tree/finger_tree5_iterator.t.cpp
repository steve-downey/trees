// src/smd/tree/finger_tree5_iterator.t.cpp                           -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// The iterator header is pulled in transitively by finger_tree5.hpp, but
// double-including it here verifies self-containment.
#include <smd/tree/finger_tree5_iterator.hpp>
#include <smd/tree/finger_tree5_iterator.hpp> // Re-inclusion verification

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <numeric>
#include <ranges>
#include <string>
#include <vector>

namespace {

using FT = smd::tree::FingerTree5<int>;

auto make_tree(int n) -> FT {
    auto t = FT{};
    for (int i = 0; i < n; ++i)
        t = t.snoc(i);
    return t;
}

} // namespace

TEST_CASE("FingerTree5Iterator - HeaderIsIdempotent")
{
    REQUIRE(true);
}

TEST_CASE("FingerTree5Iterator - IteratorCategoryIsBidirectional")
{
    using Iter = smd::tree::FingerTree5Iterator<int, std::size_t,
                                               smd::tree::UnitMeasure5<int, std::size_t>>;
    static_assert(std::bidirectional_iterator<Iter>);
    static_assert(!std::random_access_iterator<Iter>);
    static_assert(std::is_same_v<typename Iter::value_type, int>);
    REQUIRE(true);
}

TEST_CASE("FingerTree5Iterator - EmptyTreeBeginEqualsEnd")
{
    auto t  = FT{};
    auto b  = begin(t);
    auto e  = end(t);
    CHECK(b == e);
}

TEST_CASE("FingerTree5Iterator - SingleLeafIteration")
{
    auto t = FT::leaf(42);
    auto b = begin(t);
    auto e = end(t);

    REQUIRE(b != e);
    CHECK(*b == 42);
    ++b;
    CHECK(b == e);
}

TEST_CASE("FingerTree5Iterator - ForwardIterationMatchesFlatten")
{
    // Covers small N (no spine), medium N (spine exists), and N crossing
    // digit-overflow boundaries.
    for (int n : {1, 2, 4, 5, 8, 9, 12, 20, 50, 100, 256}) {
        auto t = make_tree(n);
        auto v = t.flatten();

        std::vector<int> got;
        got.reserve(static_cast<std::size_t>(n));
        for (auto x : t)
            got.push_back(x);

        CHECK(got == v);
    }
}

TEST_CASE("FingerTree5Iterator - ReverseIterationMatchesReversedFlatten")
{
    for (int n : {1, 2, 4, 5, 9, 50, 100}) {
        auto t = make_tree(n);
        auto v = t.flatten();
        std::reverse(v.begin(), v.end());

        std::vector<int> got;
        got.reserve(static_cast<std::size_t>(n));
        auto it = end(t);
        while (it != begin(t)) {
            --it;
            got.push_back(*it);
        }

        CHECK(got == v);
    }
}

TEST_CASE("FingerTree5Iterator - BidirectionalWalkAndBack")
{
    // Step forward k times, then back k times — must land on the same element.
    auto t = make_tree(50);

    for (int k : {1, 5, 20, 49}) {
        auto it = begin(t);
        int  start_val = *it;

        for (int i = 0; i < k; ++i) ++it;
        int mid_val = *it;
        CHECK(mid_val == k); // make_tree produces 0,1,...,n-1

        for (int i = 0; i < k; ++i) --it;
        CHECK(*it == start_val);
    }
}

TEST_CASE("FingerTree5Iterator - DistanceMatchesSize")
{
    for (int n : {0, 1, 5, 100, 256}) {
        auto t = make_tree(n);
        auto d = std::distance(begin(t), end(t));
        CHECK(d == n);
    }
}

TEST_CASE("FingerTree5Iterator - RangeBasedFor")
{
    auto t = make_tree(10);

    int sum = 0;
    for (auto x : t)
        sum += x;

    CHECK(sum == 10 * 9 / 2); // 0+1+...+9 = 45
}

TEST_CASE("FingerTree5Iterator - RangesAlgorithms")
{
    auto t = make_tree(20); // 0..19

    // std::ranges::find
    auto it = std::ranges::find(t, 7);
    REQUIRE(it != end(t));
    CHECK(*it == 7);

    auto miss = std::ranges::find(t, 999);
    CHECK(miss == end(t));

    // std::ranges::any_of
    CHECK(std::ranges::any_of(t, [](int x) { return x > 15; }));
    CHECK(!std::ranges::any_of(t, [](int x) { return x > 100; }));
}

TEST_CASE("FingerTree5Iterator - DigitOverflowBoundary")
{
    // 256 elements forces the spine to hold multiple Node3s at multiple depths.
    // Verifies no elements are skipped or duplicated.
    constexpr int kN = 256;
    auto t = make_tree(kN);

    std::vector<int> got;
    got.reserve(kN);
    for (auto x : t)
        got.push_back(x);

    REQUIRE(static_cast<int>(got.size()) == kN);
    for (int i = 0; i < kN; ++i)
        CHECK(got[static_cast<std::size_t>(i)] == i);
}

TEST_CASE("FingerTree5Iterator - StringElements")
{
    // Verify the iterator works for non-int value types.
    using FTS = smd::tree::FingerTree5<std::string>;
    auto t = FTS::from_sequence({"alpha", "beta", "gamma", "delta"});

    std::vector<std::string> got;
    for (const auto& s : t)
        got.push_back(s);

    CHECK(got == (std::vector<std::string>{"alpha", "beta", "gamma", "delta"}));
}

TEST_CASE("FingerTree5Iterator - PostfixIncrementDecrement")
{
    auto t  = make_tree(5);
    auto it = begin(t);

    auto old = it++;   // post-increment
    CHECK(*old == 0);
    CHECK(*it  == 1);

    auto old2 = it--; // post-decrement
    CHECK(*old2 == 1);
    CHECK(*it   == 0);
}

TEST_CASE("FingerTree5Iterator - EndMinusMinus")
{
    // operator-- on end() must land on the last element.
    for (int n : {1, 2, 10, 50, 256}) {
        auto t   = make_tree(n);
        auto it  = end(t);
        --it;
        CHECK(*it == n - 1);
    }
}
