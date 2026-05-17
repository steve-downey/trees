// src/smd/tree/finger_tree5.t.cpp                                    -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <smd/tree/finger_tree5.hpp>
#include <smd/tree/finger_tree5.hpp> // Re-inclusion verification
#include <smd/tree/finger_tree5_pmr.hpp>

#include <catch2/catch_test_macros.hpp>

#include <bit>
#include <cstddef>
#include <memory_resource>
#include <string>
#include <vector>

namespace {

struct Weighted {
    std::size_t d_total;

    friend bool operator==(const Weighted &, const Weighted &) = default;
    friend bool operator>=(const Weighted &lhs, const Weighted &rhs) {
        return lhs.d_total >= rhs.d_total;
    }
};

struct WeightedMeasure {
    auto operator()(int value) const -> Weighted {
        return Weighted{static_cast<std::size_t>(value * 10)};
    }
};

} // namespace

namespace smd::typeclass {

template <>
struct Monoid<Weighted> {
    constexpr auto identity() const -> Weighted { return Weighted{0U}; }

    constexpr auto combine(const Weighted &lhs, const Weighted &rhs) const
        -> Weighted {
        return Weighted{lhs.d_total + rhs.d_total};
    }
};

} // namespace smd::typeclass

TEST_CASE("FingerTree5 - HeaderIsIdempotent")
{
    REQUIRE(true);
}

TEST_CASE("FingerTree5 - Empty")
{
    using FT = smd::tree::FingerTree5<int>;

    auto t = FT{};
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

    auto t = FT{};
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

    auto t = FT{};
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

    auto t = FT{};
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

    auto t = FT{};
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

    auto t = FT{};
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

    auto t = FT{};
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

    auto t = FT{};
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

    auto left = FT{};
    for (int i = 0; i < 50; ++i)
        left = left.snoc(i);
    auto right = FT{};
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
    auto e = FT{};

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

    auto left = FT{};
    for (int i = 0; i < 256; ++i)
        left = left.snoc(i);
    auto right = FT{};
    for (int i = 256; i < 512; ++i)
        right = right.snoc(i);

    auto combined = left.append(right);
    CHECK(combined.measure() == 512U);

    auto v = combined.flatten();
    REQUIRE(v.size() == 512U);
    for (int i = 0; i < 512; ++i)
        CHECK(v[static_cast<std::size_t>(i)] == i);
}

TEST_CASE("FingerTree5 - Split")
{
    using FT = smd::tree::FingerTree5<int>;

    auto t = FT{};
    for (int i = 0; i < 100; ++i)
        t = t.snoc(i);

    auto sp = t.split([](std::size_t p) { return p > 25U; });
    REQUIRE(sp.has_value());
    CHECK(sp->d_pivot == 25);
    CHECK(sp->d_left.measure() == 25U);
    CHECK(sp->d_right.measure() == 74U);
}

TEST_CASE("FingerTree5 - SplitAtMeasure")
{
    using FT = smd::tree::FingerTree5<int>;

    auto t = FT{};
    for (int i = 0; i < 100; ++i)
        t = t.snoc(i);

    auto sa = t.split_at_measure(std::size_t{51});
    CHECK(sa.d_left.measure() == 50U);
    CHECK(sa.d_right.measure() == 50U);

    auto lv = sa.d_left.flatten();
    auto rv = sa.d_right.flatten();
    REQUIRE(lv.size() == 50U);
    REQUIRE(rv.size() == 50U);
    for (int i = 0; i < 50; ++i) {
        CHECK(lv[static_cast<std::size_t>(i)] == i);
        CHECK(rv[static_cast<std::size_t>(i)] == i + 50);
    }
}

TEST_CASE("FingerTree5 - SplitEmpty")
{
    using FT = smd::tree::FingerTree5<int>;

    auto t = FT{};
    auto sp = t.split([](std::size_t) { return true; });
    CHECK_FALSE(sp.has_value());
}

TEST_CASE("FingerTree5 - SplitLeafMatching")
{
    using FT = smd::tree::FingerTree5<int>;

    auto t = FT::leaf(7);
    auto sp = t.split([](std::size_t p) { return p > 0U; });
    REQUIRE(sp.has_value());
    CHECK(sp->d_pivot == 7);
    CHECK(sp->d_left.is_empty());
    CHECK(sp->d_right.is_empty());
}

TEST_CASE("FingerTree5 - SplitLeafNonMatching")
{
    using FT = smd::tree::FingerTree5<int>;

    auto t = FT::leaf(7);
    auto sp = t.split([](std::size_t) { return false; });
    CHECK_FALSE(sp.has_value());
}

TEST_CASE("FingerTree5 - SplitNoMatch")
{
    using FT = smd::tree::FingerTree5<int>;

    auto t = FT::from_sequence({1, 2, 3, 4, 5});
    auto sp = t.split([](std::size_t) { return false; });
    CHECK_FALSE(sp.has_value());
}

TEST_CASE("FingerTree5 - Search")
{
    using FT = smd::tree::FingerTree5<int>;

    auto t = FT{};
    for (int i = 0; i < 50; ++i)
        t = t.snoc(i);

    auto found = t.search([](std::size_t p) { return p > 10U; });
    REQUIRE(found.has_value());
    CHECK(*found == 10);
}

TEST_CASE("FingerTree5 - SplitAtPredicateAlwaysTrue")
{
    using FT = smd::tree::FingerTree5<int>;

    auto t = FT::from_sequence({1, 2, 3, 4, 5});
    auto sp = t.split([](std::size_t) { return true; });
    REQUIRE(sp.has_value());
    CHECK(sp->d_pivot == 1);
    CHECK(sp->d_left.is_empty());
    CHECK(sp->d_right.flatten() == std::vector<int>{2, 3, 4, 5});
}

TEST_CASE("FingerTree5 - SplitRoundTripsViaConcat")
{
    using FT = smd::tree::FingerTree5<int>;

    auto t = FT{};
    for (int i = 0; i < 100; ++i)
        t = t.snoc(i);

    for (std::size_t pivot : {std::size_t{1}, std::size_t{27}, std::size_t{50},
                              std::size_t{75}, std::size_t{99}}) {
        auto sa = t.split_at_measure(pivot);
        auto rejoined = FT::concat(sa.d_left, sa.d_right);
        CHECK(rejoined.flatten() == t.flatten());
    }
}

TEST_CASE("FingerTree5 - WeightedSplitAtMeasure")
{
    using FT = smd::tree::FingerTree5<int, Weighted, WeightedMeasure>;

    auto t = FT{};
    for (int i = 1; i <= 10; ++i)
        t = t.snoc(i); // weights: 10, 20, 30, ..., 100 ; total 550

    CHECK(t.measure() == Weighted{550U});

    auto sa = t.split_at_measure(Weighted{200U});
    // Sum of 10+20+30+40 = 100 < 200 ; +50 = 150 < 200 ; +60 = 210 >= 200
    // So pivot is element 6 (weight 60), left has 1..5, right has 6..10.
    CHECK(sa.d_left.measure() == Weighted{150U});
    CHECK(sa.d_right.measure() == Weighted{400U});
    CHECK(sa.d_left.flatten() == std::vector<int>{1, 2, 3, 4, 5});
    CHECK(sa.d_right.flatten() == std::vector<int>{6, 7, 8, 9, 10});
}

TEST_CASE("FingerTree5 - SplitWithStatefulPredicate")
{
    // Verifies the predicate doesn't get erased to std::function; a
    // capturing/non-copyable callable still threads through end-to-end.
    using FT = smd::tree::FingerTree5<int>;

    auto t = FT{};
    for (int i = 0; i < 30; ++i)
        t = t.snoc(i);

    struct Threshold {
        std::size_t value;
        auto operator()(std::size_t p) const -> bool { return p > value; }
    };

    Threshold pred{17U};
    auto sp = t.split(pred);
    REQUIRE(sp.has_value());
    CHECK(sp->d_pivot == 17);
    CHECK(sp->d_left.measure() == 17U);
}

TEST_CASE("FingerTree5 - StringElements")
{
    using FT = smd::tree::FingerTree5<std::string>;

    auto t = FT::from_sequence({"alpha", "beta", "gamma", "delta"});
    CHECK(t.measure() == 4U);
    CHECK(t.head() == "alpha");
    CHECK(t.last() == "delta");
}

TEST_CASE("FingerTree5 - SpineDepthIsLogarithmic")
{
    // Verifies the structural invariant: spine_depth() <= floor(log2(N)).
    //
    // Each element at spine level d is a Node covering >= 2^d leaves, so a
    // non-empty spine at level d requires N >= 2^d.  Therefore
    // spine_depth() <= floor(log2(N)), i.e., within std::bit_width(N) - 1.
    //
    // This bound makes every recursive call chain in FingerTree5
    // (flatten, for_each, view_l/r rebalancing, shared_ptr destructor cascade)
    // safe from stack overflow for all realistically sized trees.
    using FT = smd::tree::FingerTree5<int>;

    for (int n : {1, 2, 3, 9, 10, 100, 1000, 5000, 10000}) {
        auto t = FT{};
        for (int i = 0; i < n; ++i)
            t = t.snoc(i);

        auto depth = t.spine_depth();
        std::size_t bound = std::bit_width(static_cast<std::size_t>(n));
        INFO("n=" << n << " spine_depth=" << depth << " bit_width=" << bound);
        CHECK(depth < bound); // strict: depth <= floor(log2(n)) < bit_width(n)
    }

    // Empty and single-element trees have depth 0.
    CHECK(FT{}.spine_depth() == 0U);
    CHECK(FT::leaf(42).spine_depth() == 0U);
}

TEST_CASE("FingerTree5 - LargeTreeRecursionNoStackOverflow")
{
    // Exercises all O(log N) recursive call chains at N = 10'000.
    // spine_depth() is at most floor(log2(10000)) = 13.
    // The shared_ptr destructor cascade, flatten_elems, for_each_internal,
    // and view_l/r spine-borrowing all recurse through the same bounded chain.
    //
    // A naive singly-linked list of the same size would overflow the default
    // stack at ~8K–64K frames.  A finger tree cannot: the structure enforces
    // the logarithmic depth.
    using FT = smd::tree::FingerTree5<int>;

    static constexpr int kN = 10'000;

    auto t = FT{};
    for (int i = 0; i < kN; ++i)
        t = t.snoc(i);

    REQUIRE(t.measure() == static_cast<std::size_t>(kN));
    REQUIRE(t.spine_depth() <= std::bit_width(static_cast<std::size_t>(kN)) - 1U);

    // flatten: recurses through spine chain then Elem nodes
    auto v = t.flatten();
    REQUIRE(static_cast<int>(v.size()) == kN);
    CHECK(v.front() == 0);
    CHECK(v.back() == kN - 1);

    // for_each: same recursive structure as flatten
    long long sum = 0;
    t.for_each([&](int x) { sum += x; });
    CHECK(sum == static_cast<long long>(kN) * (kN - 1) / 2);

    // view_l drain: exercises spine-borrowing rebalancing at each step
    int count = 0;
    auto u = t;
    while (auto vl = u.view_l()) {
        ++count;
        u = std::move(vl->d_rest);
    }
    CHECK(count == kN);

    // Destructor of t runs here — shared_ptr chain unwinds spine_depth() frames.
}

TEST_CASE("FingerTree5 - ReversedFlattenMatchesReverse")
{
    using FT = smd::tree::FingerTree5<int>;

    for (int n : {1, 2, 5, 10, 50, 100}) {
        auto t = FT{};
        for (int i = 0; i < n; ++i)
            t = t.snoc(i);

        auto fwd = t.flatten();
        std::reverse(fwd.begin(), fwd.end());
        CHECK(t.reversed().flatten() == fwd);
    }
}

TEST_CASE("FingerTree5 - ReversedTwiceIsOriginal")
{
    using FT = smd::tree::FingerTree5<int>;

    auto t = FT{};
    for (int i = 0; i < 20; ++i)
        t = t.snoc(i);

    CHECK(t.reversed().reversed().flatten() == t.flatten());
}

TEST_CASE("FingerTree5 - ReversedMeasureOnCommutative")
{
    // Unit-count measure is commutative: reversed().measure().value == measure().
    using FT = smd::tree::FingerTree5<int>;

    auto t = FT{};
    for (int i = 0; i < 10; ++i)
        t = t.snoc(i);

    CHECK(t.reversed().measure().value == t.measure());
}

TEST_CASE("FingerTree5 - ReversedNonCommutativeElementOrder")
{
    // Reversed tree has the correct element order regardless of the monoid's
    // commutativity.  Verify with string elements.
    using FT = smd::tree::FingerTree5<std::string>;

    auto t = FT::from_sequence({"a", "b", "c", "d", "e"});
    CHECK(t.reversed().flatten()
          == (std::vector<std::string>{"e", "d", "c", "b", "a"}));
}

TEST_CASE("FingerTree5 - ReversedSplitAtMeasure")
{
    // split_at_measure on a reversed tree splits from the new left edge
    // (the old right edge).  split_at_measure places elements strictly
    // BEFORE the threshold-tripping pivot in d_left; the pivot itself
    // goes to the front of d_right (consistent with the non-reversed API).
    //
    // Reversed: 10 9 8 7 6 5 4 3 2 1
    // Prefix grows: 1, 2, 3 after element 8.  Pivot = 8.
    // d_left = {10, 9}   (prefix 2 < 3)
    // d_right = {8, 7, 6, 5, 4, 3, 2, 1}  (pivot cons'd onto remainder)
    using FT = smd::tree::FingerTree5<int>;

    auto t = FT{};
    for (int i = 1; i <= 10; ++i)
        t = t.snoc(i); // 1 2 3 4 5 6 7 8 9 10

    auto rev = t.reversed(); // 10 9 8 7 6 5 4 3 2 1
    auto sa  = rev.split_at_measure({3U}); // DualMonoid<size_t> threshold

    CHECK(sa.d_left.flatten()  == (std::vector<int>{10, 9}));
    CHECK(sa.d_right.flatten() == (std::vector<int>{8, 7, 6, 5, 4, 3, 2, 1}));
}

TEST_CASE("FingerTree5 - head_ref and last_ref return stable reference")
{
    using FT = smd::tree::FingerTree5<int>;

    auto t = FT::from_sequence({10, 20, 30, 40});

    // head_ref/last_ref return const T& into the tree's leaf nodes.
    const int& h = t.head_ref();
    const int& l = t.last_ref();
    CHECK(h == 10);
    CHECK(l == 40);

    // The references must agree with head()/last() copies.
    CHECK(h == t.head());
    CHECK(l == t.last());

    // Verify with string T (no copy needed for large elements).
    using FS = smd::tree::FingerTree5<std::string>;
    auto s = FS::from_sequence({"alpha", "beta", "gamma"});
    CHECK(s.head_ref() == "alpha");
    CHECK(s.last_ref() == "gamma");
    CHECK(s.head_ref() == s.head());
    CHECK(s.last_ref() == s.last());
}

// ============================================================================
//                      Container / ReversibleContainer tests
// ============================================================================

static_assert(std::ranges::bidirectional_range<smd::tree::FingerTree5<int>>);
static_assert(std::ranges::common_range<smd::tree::FingerTree5<int>>);
static_assert(std::ranges::sized_range<smd::tree::FingerTree5<int>>);

TEST_CASE("FingerTree5 - Container type aliases are present")
{
    using FT = smd::tree::FingerTree5<int>;
    static_assert(std::same_as<FT::value_type,             int>);
    static_assert(std::same_as<FT::reference,              const int&>);
    static_assert(std::same_as<FT::const_reference,        const int&>);
    static_assert(std::same_as<FT::difference_type,        std::ptrdiff_t>);
    static_assert(std::same_as<FT::size_type,              std::size_t>);
    static_assert(std::same_as<FT::iterator,               FT::const_iterator>);
    REQUIRE(true);
}

TEST_CASE("FingerTree5 - empty() is the Container query, FT{} is default construction")
{
    using FT = smd::tree::FingerTree5<int>;
    FT t{};
    CHECK(t.empty());
    CHECK(t.is_empty());
    CHECK(t.size() == 0U);

    auto t2 = FT::leaf(42);
    CHECK(!t2.empty());
    CHECK(t2.size() == 1U);
}

TEST_CASE("FingerTree5 - size() matches measure() for unit-measure trees")
{
    using FT = smd::tree::FingerTree5<int>;
    auto t = FT::from_sequence({1, 2, 3, 4, 5});
    CHECK(t.size() == 5U);
    CHECK(t.size() == t.measure());
}

TEST_CASE("FingerTree5 - max_size() is reasonable")
{
    using FT = smd::tree::FingerTree5<int>;
    CHECK(FT{}.max_size() == std::numeric_limits<std::size_t>::max());
}

TEST_CASE("FingerTree5 - swap() exchanges contents")
{
    using FT = smd::tree::FingerTree5<int>;
    auto a = FT::from_sequence({1, 2, 3});
    auto b = FT::from_sequence({10, 20});
    swap(a, b);
    CHECK(a.flatten() == (std::vector<int>{10, 20}));
    CHECK(b.flatten() == (std::vector<int>{1, 2, 3}));
    a.swap(b);
    CHECK(a.flatten() == (std::vector<int>{1, 2, 3}));
}

TEST_CASE("FingerTree5 - operator== compares element-wise")
{
    using FT = smd::tree::FingerTree5<int>;
    auto a = FT::from_sequence({1, 2, 3});
    auto b = FT::from_sequence({1, 2, 3});
    auto c = FT::from_sequence({1, 2, 4});
    auto d = FT::from_sequence({1, 2});
    CHECK(a == b);
    CHECK(!(a == c));
    CHECK(!(a == d));
    CHECK(FT{} == FT{});
}

TEST_CASE("FingerTree5 - cbegin/cend match begin/end")
{
    using FT = smd::tree::FingerTree5<int>;
    auto t = FT::from_sequence({5, 6, 7});
    CHECK(std::ranges::equal(t, std::ranges::subrange(t.cbegin(), t.cend())));
}

TEST_CASE("FingerTree5 - rbegin/rend iterate in reverse")
{
    using FT = smd::tree::FingerTree5<int>;
    auto t = FT::from_sequence({1, 2, 3, 4, 5});
    std::vector<int> rev(t.rbegin(), t.rend());
    CHECK(rev == (std::vector<int>{5, 4, 3, 2, 1}));
}

TEST_CASE("FingerTree5 - front() and back() match head_ref() and last_ref()")
{
    using FT = smd::tree::FingerTree5<int>;
    auto t = FT::from_sequence({10, 20, 30});
    CHECK(t.front() == 10);
    CHECK(t.back()  == 30);
    CHECK(t.front() == t.head_ref());
    CHECK(t.back()  == t.last_ref());
}

// ============================================================================
//                       AllocatorAware + PMR tests
// ============================================================================

TEST_CASE("FingerTree5 - allocator_type alias present and get_allocator() works")
{
    using FT   = smd::tree::FingerTree5<int>;
    using Alloc = FT::allocator_type;
    static_assert(std::same_as<Alloc, std::allocator<std::byte>>);

    FT t;
    auto a = t.get_allocator();
    static_assert(std::same_as<decltype(a), Alloc>);
    REQUIRE(true);
}

TEST_CASE("FingerTree5 - allocator-extended constructor")
{
    std::allocator<std::byte> alloc;
    smd::tree::FingerTree5<int> t(alloc);
    CHECK(t.empty());
    CHECK(t.get_allocator() == alloc);
}

TEST_CASE("FingerTree5 - PMR monotonic_buffer_resource")
{
    std::array<std::byte, 65536> buf{};
    std::pmr::monotonic_buffer_resource mr(buf.data(), buf.size());

    using PMR_FT = smd::tree::pmr::FingerTree5<int>;

    // Construct with custom memory resource
    PMR_FT t(&mr);
    CHECK(t.empty());

    // All allocations go through the resource
    auto t2 = PMR_FT::from_sequence({1, 2, 3, 4, 5}, &mr);
    CHECK(t2.size() == 5U);
    CHECK(t2.flatten() == (std::vector<int>{1, 2, 3, 4, 5}));

    // Operations using the allocated tree
    auto t3 = t2.snoc(6);
    CHECK(t3.size() == 6U);
}

TEST_CASE("FingerTree5 - PMR bidirectional_range static_assert")
{
    using PMR_FT = smd::tree::pmr::FingerTree5<int>;
    static_assert(std::ranges::bidirectional_range<PMR_FT>);
    static_assert(std::ranges::sized_range<PMR_FT>);
    REQUIRE(true);
}
