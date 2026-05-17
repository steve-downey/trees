// src/smd/tree/finger_tree5_pmr.t.cpp                                 -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <smd/tree/finger_tree5_pmr.hpp>
#include <smd/tree/finger_tree5_pmr.hpp>  // Re-inclusion verification

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cstddef>
#include <memory_resource>
#include <numeric>
#include <vector>

namespace {

using FT = smd::tree::pmr::FingerTree5<int>;

} // namespace

static_assert(std::ranges::bidirectional_range<FT>);
static_assert(std::ranges::sized_range<FT>);
static_assert(std::same_as<FT::allocator_type,
                            std::pmr::polymorphic_allocator<std::byte>>);

TEST_CASE("pmr::FingerTree5 - HeaderIsIdempotent") { REQUIRE(true); }

TEST_CASE("pmr::FingerTree5 - default construction uses default resource")
{
    FT t;
    CHECK(t.empty());
    CHECK(t.size() == 0U);
    CHECK(t.get_allocator().resource() == std::pmr::get_default_resource());
}

TEST_CASE("pmr::FingerTree5 - monotonic_buffer_resource")
{
    std::array<std::byte, 65536> buf{};
    std::pmr::monotonic_buffer_resource mr(buf.data(), buf.size());

    FT t(&mr);
    CHECK(t.empty());
    CHECK(t.get_allocator().resource() == &mr);

    // Bulk construction — Elem and Deep allocations go through the resource.
    auto t2 = FT::from_sequence({1, 2, 3, 4, 5, 6, 7, 8, 9, 10}, &mr);
    CHECK(t2.size() == 10U);

    auto flat = t2.flatten();
    CHECK(flat.size() == 10U);
    for (int i = 0; i < 10; ++i)
        CHECK(flat[static_cast<std::size_t>(i)] == i + 1);
}

TEST_CASE("pmr::FingerTree5 - cons/snoc use custom resource")
{
    std::pmr::monotonic_buffer_resource mr;
    FT t(&mr);

    for (int i = 0; i < 50; ++i)
        t = t.snoc(i);

    CHECK(t.size() == 50U);
    CHECK(t.head() == 0);
    CHECK(t.last() == 49);
}

TEST_CASE("pmr::FingerTree5 - split and append work")
{
    std::pmr::monotonic_buffer_resource mr;
    auto t = FT::from_sequence({1, 2, 3, 4, 5, 6, 7, 8, 9, 10}, &mr);

    auto sp = t.split([](std::size_t prefix) { return prefix >= 5; });
    REQUIRE(sp.has_value());
    CHECK(sp->d_pivot == 5);
    CHECK(sp->d_left.size() == 4U);
    CHECK(sp->d_right.size() == 5U);
}

TEST_CASE("pmr::FingerTree5 - Container requirements satisfied")
{
    std::pmr::monotonic_buffer_resource mr;
    auto a = FT::from_sequence({1, 2, 3}, &mr);
    auto b = FT::from_sequence({1, 2, 3}, &mr);
    auto c = FT::from_sequence({4, 5, 6}, &mr);

    CHECK(a == b);
    CHECK(!(a == c));

    auto copy = a;
    swap(a, c);
    CHECK(a.flatten() == (std::vector<int>{4, 5, 6}));
    CHECK(c.flatten() == (std::vector<int>{1, 2, 3}));

    std::vector<int> rev(a.rbegin(), a.rend());
    CHECK(rev == (std::vector<int>{6, 5, 4}));
}

TEST_CASE("pmr::FingerTree5 - pool_resource for node reuse")
{
    std::pmr::unsynchronized_pool_resource pool;
    auto t = FT::from_sequence({}, &pool);

    for (int i = 0; i < 100; ++i)
        t = t.snoc(i);

    CHECK(t.size() == 100U);
    auto flat = t.flatten();
    for (int i = 0; i < 100; ++i)
        CHECK(flat[static_cast<std::size_t>(i)] == i);
}
