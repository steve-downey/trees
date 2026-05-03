// src/smd/ranges/range_list.t.cpp                                    -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <smd/ranges/range_list.hpp>
#include <smd/ranges/range_list.hpp> // Re-inclusion check

#include <catch2/catch_test_macros.hpp>

#include <ranges>
#include <vector>

TEST_CASE("RangeListTest - FromVectorIterates") {
    auto rl = smd::ranges::from_vector(std::vector<int>{1, 2, 3});
    std::vector<int> got;
    for (auto x : rl) {
        got.push_back(x);
    }
    CHECK(got == (std::vector<int>{1, 2, 3}));
}

TEST_CASE("RangeListTest - SingleElementView") {
    auto rl = smd::ranges::single(42);
    CHECK(std::ranges::distance(rl) == 1);
    CHECK(*std::ranges::begin(rl) == 42);
}

TEST_CASE("RangeListTest - AllWrapsExistingVector") {
    std::vector<int> v{10, 20, 30};
    auto rl = smd::ranges::all(v);
    CHECK(std::ranges::distance(rl) == 3);
    CHECK(*std::ranges::begin(rl) == 10);
}
