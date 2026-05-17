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

// ============================================================================
//              Allocator coherency tests (Lakos rule)
// ============================================================================

TEST_CASE("pmr::FingerTree5 - extended constructor: same resource shares data")
{
    std::pmr::monotonic_buffer_resource mr;
    auto t = FT::from_sequence({1, 2, 3, 4, 5}, &mr);
    CHECK(t.get_allocator().resource() == &mr);

    // Uses-allocator extended constructor: same resource → fast share (O(1))
    FT t2(std::move(t), &mr);
    CHECK(t2.get_allocator().resource() == &mr);
    CHECK(t2.flatten() == (std::vector<int>{1, 2, 3, 4, 5}));
}

TEST_CASE("pmr::FingerTree5 - extended constructor: different resource rebuilds")
{
    std::pmr::monotonic_buffer_resource mr1;
    std::pmr::monotonic_buffer_resource mr2;

    auto t = FT::from_sequence({10, 20, 30}, &mr1);
    CHECK(t.get_allocator().resource() == &mr1);

    // Different resource: extended constructor rebuilds (O(N)) — no mixing
    FT t2(std::move(t), &mr2);
    CHECK(t2.get_allocator().resource() == &mr2);
    CHECK(t2.flatten() == (std::vector<int>{10, 20, 30}));
}

TEST_CASE("pmr::FingerTree5 - append: same resource uses structural sharing")
{
    std::pmr::monotonic_buffer_resource mr;
    auto a = FT::from_sequence({1, 2, 3}, &mr);
    auto b = FT::from_sequence({4, 5, 6}, &mr);

    auto c = a.append(b);
    CHECK(c.get_allocator().resource() == &mr);
    CHECK(c.flatten() == (std::vector<int>{1, 2, 3, 4, 5, 6}));
}

TEST_CASE("pmr::FingerTree5 - append: different resources rebuilds right side")
{
    std::pmr::monotonic_buffer_resource mr1;
    std::pmr::monotonic_buffer_resource mr2;

    auto a = FT::from_sequence({1, 2, 3}, &mr1);
    auto b = FT::from_sequence({4, 5, 6}, &mr2);

    // b is rebuilt using mr1 before concatenation — result is coherent
    auto c = a.append(b);
    CHECK(c.get_allocator().resource() == &mr1);
    CHECK(c.flatten() == (std::vector<int>{1, 2, 3, 4, 5, 6}));
}

TEST_CASE("pmr::FingerTree5 - move assignment: same resource fast path")
{
    std::pmr::monotonic_buffer_resource mr;
    FT t(&mr);
    auto src = FT::from_sequence({7, 8, 9}, &mr);

    t = std::move(src);
    CHECK(t.get_allocator().resource() == &mr);
    CHECK(t.flatten() == (std::vector<int>{7, 8, 9}));
}

TEST_CASE("pmr::FingerTree5 - copy assignment: different resource rebuilds")
{
    std::pmr::monotonic_buffer_resource mr1;
    std::pmr::monotonic_buffer_resource mr2;

    FT t(&mr1);
    auto src = FT::from_sequence({11, 22, 33}, &mr2);

    // Copy into t: mr2 elements rebuilt into mr1 — no cross-resource sharing
    t = src;
    CHECK(t.get_allocator().resource() == &mr1);
    CHECK(t.flatten() == (std::vector<int>{11, 22, 33}));
}
