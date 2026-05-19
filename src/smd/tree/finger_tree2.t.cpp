// src/smd/tree/finger_tree2.t.cpp                                    -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <smd/tree/finger_tree2.hpp>
#include <smd/tree/finger_tree2.hpp> // Re-inclusion verification

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <optional>
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

TEST_CASE("FingerTree2 - HeaderIsIdempotent") { REQUIRE(true); }

TEST_CASE("FingerTree2 - Empty") {
    using FT = smd::tree::FingerTree2<int>;

    auto t = FT::empty();
    CHECK(t.is_empty());
    CHECK_FALSE(t.is_leaf());
    CHECK_FALSE(t.is_branch());
    CHECK(t.measure() == 0U);
    CHECK(t.flatten().empty());
}

TEST_CASE("FingerTree2 - Leaf") {
    using FT = smd::tree::FingerTree2<int>;

    auto t = FT::leaf(42);
    CHECK_FALSE(t.is_empty());
    CHECK(t.is_leaf());
    CHECK_FALSE(t.is_branch());
    CHECK(t.measure() == 1U);
    CHECK(t.value() == 42);
    CHECK(t.flatten() == std::vector<int>{42});
}

TEST_CASE("FingerTree2 - ConsSnoc") {
    using FT = smd::tree::FingerTree2<int>;

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

TEST_CASE("FingerTree2 - ConsOverflow") {
    using FT = smd::tree::FingerTree2<int>;

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

TEST_CASE("FingerTree2 - SnocLarge") {
    using FT = smd::tree::FingerTree2<int>;

    auto t = FT::empty();
    for (int i = 0; i < 200; ++i)
        t = t.snoc(i);

    CHECK(t.measure() == 200U);
    auto v = t.flatten();
    REQUIRE(v.size() == 200U);
    for (int i = 0; i < 200; ++i)
        CHECK(v[static_cast<std::size_t>(i)] == i);
}

TEST_CASE("FingerTree2 - ViewL") {
    using FT = smd::tree::FingerTree2<int>;

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

TEST_CASE("FingerTree2 - ViewR") {
    using FT = smd::tree::FingerTree2<int>;

    auto t5 = FT::from_sequence({10, 20, 30, 40, 50});
    auto v = t5.view_r();
    REQUIRE(v.has_value());
    CHECK(v->d_value == 50);
    CHECK(v->d_rest.measure() == 4U);
    CHECK(v->d_rest.last() == 40);
}

TEST_CASE("FingerTree2 - HeadTailLastInit") {
    using FT = smd::tree::FingerTree2<int>;

    auto t = FT::from_sequence({1, 2, 3, 4, 5});
    CHECK(t.head() == 1);
    CHECK(t.last() == 5);
    CHECK(t.tail().head() == 2);
    CHECK(t.init().last() == 4);
    CHECK(t.tail().measure() == 4U);
    CHECK(t.init().measure() == 4U);
}

TEST_CASE("FingerTree2 - Flatten") {
    using FT = smd::tree::FingerTree2<int>;

    auto t = FT::empty();
    for (int i = 0; i < 100; ++i)
        t = t.snoc(i);

    auto v = t.flatten();
    REQUIRE(v.size() == 100U);
    for (int i = 0; i < 100; ++i)
        CHECK(v[static_cast<std::size_t>(i)] == i);
}

TEST_CASE("FingerTree2 - ForEach") {
    using FT = smd::tree::FingerTree2<int>;

    auto t = FT::from_sequence({1, 2, 3, 4, 5});
    std::vector<int> collected;
    t.for_each([&](int x) { collected.push_back(x); });
    CHECK(collected == std::vector<int>{1, 2, 3, 4, 5});
}

TEST_CASE("FingerTree2 - Append") {
    using FT = smd::tree::FingerTree2<int>;

    auto left = FT::from_sequence({1, 2, 3, 4, 5});
    auto right = FT::from_sequence({6, 7, 8, 9, 10});
    auto combined = left.append(right);

    CHECK(combined.measure() == 10U);
    auto v = combined.flatten();
    REQUIRE(v.size() == 10U);
    for (int i = 0; i < 10; ++i)
        CHECK(v[static_cast<std::size_t>(i)] == i + 1);
}

TEST_CASE("FingerTree2 - AppendLarge") {
    using FT = smd::tree::FingerTree2<int>;

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

TEST_CASE("FingerTree2 - AppendEmpty") {
    using FT = smd::tree::FingerTree2<int>;

    auto t = FT::from_sequence({1, 2, 3});
    auto e = FT::empty();

    CHECK(t.append(e).flatten() == std::vector<int>{1, 2, 3});
    CHECK(e.append(t).flatten() == std::vector<int>{1, 2, 3});
}

TEST_CASE("FingerTree2 - Split") {
    using FT = smd::tree::FingerTree2<int>;

    auto t = FT::empty();
    for (int i = 0; i < 100; ++i)
        t = t.snoc(i);

    // prefix > 25 triggers at element 25 (prefix = 26)
    auto sp = t.split([](std::size_t p) { return p > 25U; });
    REQUIRE(sp.has_value());
    CHECK(sp->d_pivot == 25);
    CHECK(sp->d_left.measure() == 25U);
    CHECK(sp->d_right.measure() == 74U);
}

TEST_CASE("FingerTree2 - SplitAtMeasure") {
    using FT = smd::tree::FingerTree2<int>;

    auto t = FT::empty();
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

TEST_CASE("FingerTree2 - SplitEmpty") {
    using FT = smd::tree::FingerTree2<int>;

    auto t = FT::empty();
    auto sp = t.split([](std::size_t) { return true; });
    CHECK_FALSE(sp.has_value());
}

TEST_CASE("FingerTree2 - SplitSingle") {
    using FT = smd::tree::FingerTree2<int>;

    auto t = FT::leaf(42);
    auto sp = t.split([](std::size_t p) { return p >= 1U; });
    REQUIRE(sp.has_value());
    CHECK(sp->d_pivot == 42);
    CHECK(sp->d_left.is_empty());
    CHECK(sp->d_right.is_empty());
}

TEST_CASE("FingerTree2 - Search") {
    using FT = smd::tree::FingerTree2<int>;

    auto t = FT::from_sequence({10, 20, 30, 40, 50});

    auto found = t.search([](std::size_t p) { return p >= 3U; });
    REQUIRE(found.has_value());
    CHECK(*found == 30);
}

TEST_CASE("FingerTree2 - SplitReconstructsCorrectly") {
    using FT = smd::tree::FingerTree2<int>;

    auto t = FT::empty();
    for (int i = 0; i < 50; ++i)
        t = t.snoc(i);

    for (std::size_t k = 1; k <= 50; ++k) {
        auto sp = t.split([k](std::size_t p) { return p >= k; });
        REQUIRE(sp.has_value());
        auto reconstructed =
            sp->d_left.append(FT::leaf(sp->d_pivot).append(sp->d_right));
        CHECK(reconstructed.flatten() == t.flatten());
    }
}

TEST_CASE("FingerTree2 - FromSequence") {
    using FT = smd::tree::FingerTree2<int>;

    auto t = FT::from_sequence({});
    CHECK(t.is_empty());

    t = FT::from_sequence({42});
    CHECK(t.is_leaf());
    CHECK(t.value() == 42);

    t = FT::from_sequence({1, 2, 3, 4, 5, 6, 7, 8, 9, 10});
    CHECK(t.measure() == 10U);
    CHECK(t.flatten() == std::vector<int>{1, 2, 3, 4, 5, 6, 7, 8, 9, 10});
}

TEST_CASE("FingerTree2 - Persistence") {
    using FT = smd::tree::FingerTree2<int>;

    auto t1 = FT::from_sequence({1, 2, 3});
    auto t2 = t1.cons(0);
    auto t3 = t1.snoc(4);

    CHECK(t1.flatten() == std::vector<int>{1, 2, 3});
    CHECK(t2.flatten() == std::vector<int>{0, 1, 2, 3});
    CHECK(t3.flatten() == std::vector<int>{1, 2, 3, 4});
}

TEST_CASE("FingerTree2 - WeightedMeasure") {
    using FT = smd::tree::FingerTree2<int, Weighted, WeightedMeasure>;

    auto t = FT::empty();
    for (int i = 1; i <= 5; ++i)
        t = t.snoc(i);

    // measure = 10 + 20 + 30 + 40 + 50 = 150
    CHECK(t.measure() == Weighted{150U});

    auto sp = t.split_at_measure(Weighted{60U});
    // prefix >= 60: 10, 30, 60 → triggers at element 3 (value = 3)
    // split_at puts pivot in d_right
    CHECK(sp.d_left.measure() == Weighted{30U});
    CHECK(sp.d_right.measure() == Weighted{120U});
}

TEST_CASE("FingerTree2 - StringElements") {
    using FT = smd::tree::FingerTree2<std::string>;

    auto t = FT::empty();
    t = t.snoc("hello");
    t = t.snoc("world");
    t = t.snoc("foo");

    CHECK(t.measure() == 3U);
    CHECK(t.head() == "hello");
    CHECK(t.last() == "foo");
    CHECK(t.flatten() == std::vector<std::string>{"hello", "world", "foo"});
}

TEST_CASE("FingerTree2 - SpineBorrowingViewL") {
    using FT = smd::tree::FingerTree2<int>;

    auto tree = FT::from_sequence({1, 2, 3, 4, 5, 6, 7, 8, 9, 10});
    auto current = tree;
    std::vector<int> collected;
    while (!current.is_empty()) {
        auto vl = current.view_l();
        REQUIRE(vl.has_value());
        collected.push_back(vl->d_value);
        current = std::move(vl->d_rest);
    }
    CHECK(collected == std::vector<int>{1, 2, 3, 4, 5, 6, 7, 8, 9, 10});
}

TEST_CASE("FingerTree2 - SpineBorrowingViewR") {
    using FT = smd::tree::FingerTree2<int>;

    auto tree = FT::from_sequence({1, 2, 3, 4, 5, 6, 7, 8, 9, 10});
    auto current = tree;
    std::vector<int> collected;
    while (!current.is_empty()) {
        auto vr = current.view_r();
        REQUIRE(vr.has_value());
        collected.push_back(vr->d_value);
        current = std::move(vr->d_rest);
    }
    CHECK(collected == std::vector<int>{10, 9, 8, 7, 6, 5, 4, 3, 2, 1});
}

TEST_CASE("FingerTree2 - RepeatedTailDrainsTree") {
    using FT = smd::tree::FingerTree2<int>;

    auto tree = FT::from_sequence({1, 2, 3, 4, 5, 6, 7, 8});
    auto expected = tree.flatten();

    std::vector<int> collected;
    auto current = tree;
    while (!current.is_empty()) {
        collected.push_back(current.head());
        current = current.tail();
    }
    CHECK(collected == expected);
    CHECK(current.is_empty());
}

TEST_CASE("FingerTree2 - LargeTreeSplitAndConcat") {
    using FT = smd::tree::FingerTree2<int>;
    constexpr std::size_t kN = 256U;

    auto tree = FT::empty();
    for (std::size_t i = 0; i < kN; ++i)
        tree = tree.snoc(static_cast<int>(i));

    CHECK(tree.measure() == kN);
    CHECK(tree.head() == 0);
    CHECK(tree.last() == 255);

    auto mid = tree.split_at_measure(std::size_t{kN / 2U + 1U});
    auto rebuilt = mid.d_left.append(mid.d_right);
    CHECK(rebuilt.flatten() == tree.flatten());

    auto other = FT::empty();
    for (std::size_t i = kN; i < 2U * kN; ++i)
        other = other.snoc(static_cast<int>(i));
    auto big = tree.append(other);
    CHECK(big.measure() == 2U * kN);
    CHECK(big.head() == 0);
    CHECK(big.last() == 511);

    auto found = big.search([](std::size_t p) { return p >= 300U; });
    REQUIRE(found.has_value());
    CHECK(*found == 299);
}

TEST_CASE("FingerTree2 - ConcatEdgeCases") {
    using FT = smd::tree::FingerTree2<int>;

    auto empty = FT::empty();
    auto single = FT::leaf(42);
    auto multi = FT::from_sequence({1, 2, 3});

    CHECK(empty.append(empty).is_empty());
    CHECK(empty.append(single).flatten() == std::vector<int>{42});
    CHECK(single.append(empty).flatten() == std::vector<int>{42});
    CHECK(single.append(single).flatten() == std::vector<int>{42, 42});
    CHECK(single.append(multi).flatten() == std::vector<int>{42, 1, 2, 3});
    CHECK(multi.append(single).flatten() == std::vector<int>{1, 2, 3, 42});
    CHECK(multi.append(multi).flatten() == std::vector<int>{1, 2, 3, 1, 2, 3});
}

TEST_CASE("FingerTree2 - PersistenceUnderSplitAndAppend") {
    using FT = smd::tree::FingerTree2<int>;

    auto base = FT::from_sequence({1, 2, 3, 4, 5, 6});
    auto appended = base.append(FT::from_sequence({7, 8, 9}));
    auto split = appended.split([](std::size_t p) { return p >= 7U; });
    REQUIRE(split.has_value());

    CHECK(split->d_left.flatten() == std::vector<int>{1, 2, 3, 4, 5, 6});
    CHECK(split->d_pivot == 7);
    CHECK(split->d_right.flatten() == std::vector<int>{8, 9});

    CHECK(base.flatten() == std::vector<int>{1, 2, 3, 4, 5, 6});
    CHECK(appended.flatten() == std::vector<int>{1, 2, 3, 4, 5, 6, 7, 8, 9});
}

TEST_CASE("FingerTree2 - WeightedSplitLarger") {
    using FT = smd::tree::FingerTree2<int, Weighted, WeightedMeasure>;

    auto tree = FT::empty();
    for (int i = 1; i <= 20; ++i)
        tree = tree.snoc(i);

    CHECK(tree.measure() == Weighted{2100U});

    auto found = tree.search([](Weighted p) { return p.d_total >= 550U; });
    REQUIRE(found.has_value());
    CHECK(*found == 10);

    auto split = tree.split([](Weighted p) { return p.d_total >= 550U; });
    REQUIRE(split.has_value());
    CHECK(split->d_left.measure() == Weighted{450U});
    CHECK(split->d_pivot == 10);
    CHECK(split->d_right.measure() == Weighted{1550U});
}
