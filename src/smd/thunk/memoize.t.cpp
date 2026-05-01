// src/smd/thunk/memoize.t.cpp                                        -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#include <smd/thunk/memoize.hpp>
#include <smd/thunk/memoize.hpp>  // Re-inclusion check

#include <catch2/catch_test_macros.hpp>

#include <string>

TEST_CASE("memoize - evaluates callable exactly once")
{
    int call_count = 0;
    auto m = smd::thunk::memoize([&call_count]() {
        ++call_count;
        return 42;
    });
    CHECK(m() == 42);
    CHECK(m() == 42);
    CHECK(m() == 42);
    CHECK(call_count == 1);
}

TEST_CASE("memoize - captures arguments")
{
    auto m = smd::thunk::memoize([](int x, int y) { return x * y; }, 6, 7);
    CHECK(m() == 42);
    CHECK(m() == 42);
}

TEST_CASE("memoize - copies share cached value")
{
    int call_count = 0;
    auto m1 = smd::thunk::memoize([&call_count]() {
        ++call_count;
        return std::string("hello");
    });
    auto m2 = m1;
    CHECK(m1() == "hello");
    CHECK(m2() == "hello");
    CHECK(call_count == 1);
}

TEST_CASE("erased_thunk - wraps and invokes callable")
{
    int call_count = 0;
    auto inner = smd::thunk::memoize([&call_count]() {
        ++call_count;
        return 99;
    });
    smd::thunk::erased_thunk<int> e{std::move(inner)};
    CHECK(e() == 99);
    CHECK(e() == 99);
    CHECK(call_count == 1);
}

TEST_CASE("measured_memoize - provides measure and deferred value")
{
    auto m = smd::thunk::measured_memoize(
        std::string("my-measure"),
        []() { return 123; });
    CHECK(m.cached_measure() == "my-measure");
    CHECK(m.force() == 123);
    CHECK(m.force() == 123);
}
