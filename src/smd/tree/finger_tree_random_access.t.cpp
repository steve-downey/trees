// src/smd/tree/finger_tree_random_access.t.cpp                       -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <smd/tree/finger_tree_random_access.hpp>
#include <smd/tree/finger_tree_random_access.hpp> // Re-inclusion check

#include <smd/typeclass/foldable.hpp>
#include <smd/typeclass/traversable.hpp>

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cstddef>
#include <numeric>
#include <optional>
#include <vector>

using Seq = smd::tree::FingerTreeRandomAccess<int>;

TEST_CASE("RandomAccess - HeaderIsIdempotent") { REQUIRE(true); }

TEST_CASE("RandomAccess - Empty") {
    auto seq = Seq();
    CHECK(seq.empty());
    CHECK(seq.size() == 0U);
    CHECK(seq.to_vector().empty());
    CHECK_FALSE(seq.at(0).has_value());
}

TEST_CASE("RandomAccess - PushBackSweep") {
    // Sweep through spine-transition sizes: 1-4 (digit only), 5-8 (Deep
    // empty spine), 9-12 (first spine node), 50+ (spine itself Deep), 200+
    auto seq = Seq();
    for (int i = 0; i < 300; ++i) {
        seq = seq.push_back(i);
        REQUIRE(seq.size() == static_cast<std::size_t>(i + 1));
        REQUIRE(*seq.at(static_cast<std::size_t>(i)) == i);
    }
    auto v = seq.to_vector();
    REQUIRE(v.size() == 300U);
    for (int i = 0; i < 300; ++i)
        REQUIRE(v[static_cast<std::size_t>(i)] == i);
}

TEST_CASE("RandomAccess - PushFrontSweep") {
    auto seq = Seq();
    for (int i = 0; i < 300; ++i) {
        seq = seq.push_front(i);
        REQUIRE(seq.size() == static_cast<std::size_t>(i + 1));
        REQUIRE(*seq.at(0) == i);
    }
    auto v = seq.to_vector();
    REQUIRE(v.size() == 300U);
    for (int i = 0; i < 300; ++i)
        CHECK(v[static_cast<std::size_t>(i)] == 299 - i);
}

TEST_CASE("RandomAccess - AtOutOfBounds") {
    auto seq = Seq::from_sequence({10, 20, 30});
    CHECK(*seq.at(0) == 10);
    CHECK(*seq.at(2) == 30);
    CHECK_FALSE(seq.at(3).has_value());
    CHECK_FALSE(seq.at(999).has_value());
}

TEST_CASE("RandomAccess - InsertAtMiddle") {
    // Build sequence, then insert at middle across spine transitions
    auto seq = Seq();
    for (int i = 0; i < 100; ++i)
        seq = seq.push_back(i);

    // Insert at position 50
    seq = seq.insert(50, -1);
    REQUIRE(seq.size() == 101U);
    CHECK(*seq.at(49) == 49);
    CHECK(*seq.at(50) == -1);
    CHECK(*seq.at(51) == 50);

    // Insert at various positions to stress spine
    for (int i = 0; i < 50; ++i)
        seq = seq.insert(seq.size() / 2, 1000 + i);
    REQUIRE(seq.size() == 151U);
}

TEST_CASE("RandomAccess - EraseAtMiddle") {
    std::vector<int> expected(200);
    std::iota(expected.begin(), expected.end(), 0);
    auto seq = Seq::from_sequence(expected);

    // Erase from middle repeatedly
    for (int i = 0; i < 100; ++i) {
        auto mid = seq.size() / 2;
        expected.erase(expected.begin() + static_cast<std::ptrdiff_t>(mid));
        seq = seq.erase(mid);
    }
    CHECK(seq.size() == 100U);
    CHECK(seq.to_vector() == expected);
}

TEST_CASE("RandomAccess - UpdateSweep") {
    auto seq = Seq::from_sequence({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11});
    for (std::size_t i = 0; i < 12; ++i)
        seq = seq.update(i, static_cast<int>(i * 100));

    for (std::size_t i = 0; i < 12; ++i)
        CHECK(*seq.at(i) == static_cast<int>(i * 100));
}

TEST_CASE("RandomAccess - SpineTransitionAt9") {
    // The underlying finger tree transitions to a spine node at ~9 elements.
    // Verify operations work correctly at this boundary.
    auto seq = Seq();
    for (int i = 0; i < 20; ++i)
        seq = seq.push_back(i);

    // Access all elements to verify structural integrity
    for (int i = 0; i < 20; ++i)
        REQUIRE(*seq.at(static_cast<std::size_t>(i)) == i);

    // Erase elements across the spine boundary
    for (int i = 0; i < 10; ++i)
        seq = seq.erase(0);
    CHECK(seq.size() == 10U);
    CHECK(*seq.at(0) == 10);
    CHECK(*seq.at(9) == 19);
}

TEST_CASE("RandomAccess - LargeStress") {
    // Build 1000 elements, then random insert/erase cycle
    std::vector<int> mirror;
    auto seq = Seq();
    for (int i = 0; i < 1000; ++i) {
        mirror.push_back(i);
        seq = seq.push_back(i);
    }
    REQUIRE(seq.size() == 1000U);
    REQUIRE(seq.to_vector() == mirror);

    // Insert 200 elements at various positions
    for (int i = 0; i < 200; ++i) {
        auto pos = static_cast<std::size_t>((i * 7) % (mirror.size()));
        mirror.insert(mirror.begin() + static_cast<std::ptrdiff_t>(pos),
                      5000 + i);
        seq = seq.insert(pos, 5000 + i);
    }
    REQUIRE(seq.size() == 1200U);
    CHECK(seq.to_vector() == mirror);

    // Erase 200 elements from various positions
    for (int i = 0; i < 200; ++i) {
        auto pos = static_cast<std::size_t>((i * 13) % mirror.size());
        mirror.erase(mirror.begin() + static_cast<std::ptrdiff_t>(pos));
        seq = seq.erase(pos);
    }
    REQUIRE(seq.size() == 1000U);
    CHECK(seq.to_vector() == mirror);
}

TEST_CASE("RandomAccess - Persistence") {
    auto seq1 = Seq::from_sequence({1, 2, 3, 4, 5});
    auto seq2 = seq1.push_back(6);
    auto seq3 = seq1.push_front(0);
    auto seq4 = seq1.erase(2);

    CHECK(seq1.to_vector() == std::vector<int>{1, 2, 3, 4, 5});
    CHECK(seq2.to_vector() == std::vector<int>{1, 2, 3, 4, 5, 6});
    CHECK(seq3.to_vector() == std::vector<int>{0, 1, 2, 3, 4, 5});
    CHECK(seq4.to_vector() == std::vector<int>{1, 2, 4, 5});
}

TEST_CASE("RandomAccess - FoldableTypeclass") {
    auto seq = Seq::from_sequence({1, 2, 3, 4});
    const auto &foldable = smd::foldable_typeclass<Seq>;

    CHECK(foldable.fold_map([](int v) { return v; }, seq) == 10);
    CHECK(foldable.length(seq) == 4U);
}

TEST_CASE("RandomAccess - TraversableTypeclass") {
    auto seq = Seq::from_sequence({1, 2, 3});

    auto success =
        smd::traverse([](int v) -> std::optional<int> { return v * 10; }, seq);
    REQUIRE(success.has_value());
    CHECK(success->to_vector() == std::vector<int>{10, 20, 30});

    auto failure = smd::traverse(
        [](int v) -> std::optional<int> {
            return v == 2 ? std::nullopt : std::optional{v};
        },
        seq);
    CHECK_FALSE(failure.has_value());
}
