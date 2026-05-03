// src/smd/ziplist/zip_list.t.cpp                                     -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <smd/ziplist/zip_list.hpp>
#include <smd/ziplist/zip_list.hpp> // Re-inclusion check

#include <catch2/catch_test_macros.hpp>

TEST_CASE("ZipListTest - FiniteListConstruction") {
    smd::zip_list<int> zl{{1, 2, 3}};
    CHECK_FALSE(zl.is_repeating());
    CHECK(zl.finite_size() == 3);
    CHECK(zl.data == (std::vector<int>{1, 2, 3}));
}

TEST_CASE("ZipListTest - InfiniteRepeat") {
    auto zl = smd::zip_list<int>::repeat(42);
    CHECK(zl.is_repeating());
    CHECK(zl.repeated == std::optional{42});
}

TEST_CASE("ZipListTest - EqualityFinite") {
    smd::zip_list<int> a{{1, 2, 3}};
    smd::zip_list<int> b{{1, 2, 3}};
    smd::zip_list<int> c{{1, 2}};
    CHECK(a == b);
    CHECK_FALSE(a == c);
}

TEST_CASE("ZipListTest - EqualityRepeating") {
    auto r1 = smd::zip_list<int>::repeat(7);
    auto r2 = smd::zip_list<int>::repeat(7);
    auto r3 = smd::zip_list<int>::repeat(9);
    CHECK(r1 == r2);
    CHECK_FALSE(r1 == r3);
}
