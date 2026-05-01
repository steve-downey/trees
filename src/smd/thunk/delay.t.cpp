// src/smd/thunk/delay.t.cpp                                          -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#include <smd/thunk/delay.hpp>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("delay - invokes callable with captured arguments")
{
    auto d = smd::thunk::delay([](int x, int y) { return x + y; }, 3, 4);
    CHECK(d() == 7);
}

TEST_CASE("delay - re-evaluates on each call")
{
    int count = 0;
    auto d = smd::thunk::delay([&count]() { return ++count; });
    CHECK(d() == 1);
    CHECK(d() == 2);
    CHECK(d() == 3);
}

TEST_CASE("delay - captures args by value")
{
    int x = 10;
    auto d = smd::thunk::delay([](int v) { return v * 2; }, x);
    x = 99;
    CHECK(d() == 20);
}
