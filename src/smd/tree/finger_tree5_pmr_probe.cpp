// src/smd/tree/finger_tree5_pmr_probe.cpp                            -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Standalone PMR allocation probe for FingerTree5.
//
// Purpose:
//   Verify that FingerTree5 operations with a PMR allocator produce zero
//   global heap allocations.  This is a standalone executable — no Catch2 —
//   so the global operator new/delete replacement captures only allocations
//   from FingerTree5 and the C++ runtime, not from a test framework.
//
// Two complementary checks per operation:
//   1. null_memory_resource upstream: the monotonic buffer has no fallback.
//      Any allocation that tries to use the upstream (buffer exhaustion or
//      wrong-resource path through the PMR system) throws std::bad_alloc
//      immediately, aborting the probe.  No analysis needed — the probe
//      crashes on the first violation.
//
//   2. Global new counter: replaces operator new/delete with counting
//      wrappers.  Allocations that bypass the PMR system entirely (e.g. a
//      direct ::new call inside FingerTree5) are caught here.  The counter
//      is only tracked inside each check() scope; setup allocations are
//      excluded.
//
// AddressSanitizer note:
//   Asan wraps operator new/delete and may account for its own internal
//   allocations differently.  Under Asan the global-new counter may show
//   non-zero even for correct code.  We therefore run this probe in
//   RelWithDebInfo only, where the counter is trustworthy.  The
//   null_memory_resource check is sanitizer-agnostic and always valid.
//
// Build (RelWithDebInfo config, no sanitizer):
//   uv run cmake --build .build/build-gcc-16
//       --config RelWithDebInfo --target smd_tree_pmr_probe
//   .build/build-gcc-16/src/smd/tree/RelWithDebInfo/smd_tree_pmr_probe
//
// Exit code: 0 = all checks pass, 1 = one or more unexpected allocations.

#include <smd/tree/finger_tree5_pmr.hpp>

#include <array>
#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory_resource>
#include <vector>

// ============================================================================
//                    Global allocation counting wrappers
// ============================================================================

namespace {

std::atomic<std::size_t> g_new_count{0};
bool                     g_tracking{false};

// Pod result store — avoids std::vector (which would itself allocate).
struct Result {
    char        name[80];
    std::size_t new_count;
    bool        pass;
};

constexpr int k_max_results = 64;
Result        g_results[k_max_results];
int           g_n_results = 0;

} // namespace

// GCC -Wmismatched-new-delete fires when it sees operator delete calling
// free() on a pointer it thinks came from operator new (which here calls
// malloc).  These ARE matched — malloc/free is exactly the right pair when
// replacing the global operators.  Suppress the false positive.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmismatched-new-delete"

void* operator new(std::size_t n) {
    if (g_tracking) ++g_new_count;
    void* p = std::malloc(n);
    if (!p) throw std::bad_alloc{};
    return p;
}
void* operator new[](std::size_t n) {
    if (g_tracking) ++g_new_count;
    void* p = std::malloc(n);
    if (!p) throw std::bad_alloc{};
    return p;
}
void operator delete(void* p) noexcept { std::free(p); }
void operator delete[](void* p) noexcept { std::free(p); }
void operator delete(void* p, std::size_t) noexcept { std::free(p); }
void operator delete[](void* p, std::size_t) noexcept { std::free(p); }

#pragma GCC diagnostic pop

// ============================================================================
//                        Check harness
// ============================================================================

namespace {

// Execute fn() while counting global new calls.  The expected count is 0.
// The null_memory_resource upstream will throw bad_alloc if fn() triggers
// any PMR upstream access, making the probe self-terminating on violations.
template <typename F>
void check(const char* name, F&& fn) {
    auto before    = g_new_count.load();
    g_tracking     = true;
    fn();
    g_tracking     = false;
    auto count     = g_new_count.load() - before;

    assert(g_n_results < k_max_results);
    auto& r = g_results[g_n_results++];
    std::snprintf(r.name, sizeof(r.name), "%s", name);
    r.new_count = count;
    r.pass      = (count == 0);
}

} // namespace

// ============================================================================
//                              Probe cases
// ============================================================================

int main()
{
    using FT = smd::tree::pmr::FingerTree5<int>;

    // 4 MB arena; null_memory_resource upstream so any overflow is fatal.
    std::array<std::byte, 4 << 20> buf{};
    std::pmr::monotonic_buffer_resource mr(buf.data(), buf.size(),
                                           std::pmr::null_memory_resource());

    // --- Pre-build input data (outside tracking) ----------------------------

    // Input vector for from_sequence checks (global heap, before tracking).
    std::vector<int> v50(50), v100(100), v500(500);
    for (int i = 0; i < 50;  ++i) v50[i]  = i;
    for (int i = 0; i < 100; ++i) v100[i] = i;
    for (int i = 0; i < 500; ++i) v500[i] = i;

    // Two pre-built trees (outside tracking).
    auto base50  = FT::from_sequence(v50,  &mr);
    auto base100 = FT::from_sequence(v100, &mr);

    // -----------------------------------------------------------------------
    // Bulk construction
    // -----------------------------------------------------------------------

    check("from_sequence(50, pmr)", [&] {
        auto t = FT::from_sequence(v50, &mr);
        volatile auto s = t.size(); (void)s;
    });

    check("from_sequence(500, pmr)", [&] {
        auto t = FT::from_sequence(v500, &mr);
        volatile auto s = t.size(); (void)s;
    });

    // -----------------------------------------------------------------------
    // Element insertion
    // -----------------------------------------------------------------------

    check("snoc on 100-elem tree", [&] {
        auto t = base100.snoc(999);
        volatile auto s = t.size(); (void)s;
    });

    check("cons on 100-elem tree", [&] {
        auto t = base100.cons(-1);
        volatile auto s = t.size(); (void)s;
    });

    // Build a longer tree incrementally (exercises spine overflow).
    {
        auto t0 = FT::from_sequence(v100, &mr);
        check("50x snoc (spine overflow path)", [&] {
            auto t = t0;
            for (int i = 0; i < 50; ++i) t = t.snoc(i);
            volatile auto s = t.size(); (void)s;
        });
    }

    // -----------------------------------------------------------------------
    // View / decomposition
    // -----------------------------------------------------------------------

    check("view_l on 100-elem tree", [&] {
        auto v = base100.view_l();
        volatile bool b = v.has_value(); (void)b;
        if (v) {
            volatile auto s = v->d_rest.size(); (void)s;
        }
    });

    check("tail on 100-elem tree", [&] {
        auto t = base100.tail();
        volatile auto s = t.size(); (void)s;
    });

    check("init on 100-elem tree", [&] {
        auto t = base100.init();
        volatile auto s = t.size(); (void)s;
    });

    {
        // Build outside tracking — from_sequence scaffolding vectors use
        // the PMR alloc, so the creation is zero-global-alloc, but we
        // keep this separate to test only the tail() call.
        auto single = FT::from_sequence(std::vector<int>{42}, &mr);
        check("tail on 1-elem tree (empty result)", [&] {
            auto t = single.tail();
            volatile bool e = t.empty(); (void)e;
        });
    }

    // -----------------------------------------------------------------------
    // Concatenation
    // -----------------------------------------------------------------------

    check("append same-resource (50+50)", [&] {
        auto t = base50.append(base50);
        volatile auto s = t.size(); (void)s;
    });

    check("append same-resource (100+100)", [&] {
        auto t = base100.append(base100);
        volatile auto s = t.size(); (void)s;
    });

    // -----------------------------------------------------------------------
    // Split
    // -----------------------------------------------------------------------

    check("split at midpoint (100 elems)", [&] {
        auto sp = base100.split([](std::size_t p) { return p >= 50; });
        volatile bool b = sp.has_value(); (void)b;
    });

    check("split_at (predicate never fires)", [&] {
        auto sa = base100.split_at([](std::size_t) { return false; });
        volatile auto sl = sa.d_left.size();
        volatile bool er = sa.d_right.empty();
        (void)sl; (void)er;
    });

    check("split_at (predicate always fires)", [&] {
        auto sa = base100.split_at([](std::size_t) { return true; });
        volatile bool el = sa.d_left.empty();
        volatile auto sr = sa.d_right.size();
        (void)el; (void)sr;
    });

    // -----------------------------------------------------------------------
    // Measure / search
    // -----------------------------------------------------------------------

    check("measure() on 100-elem tree", [&] {
        volatile auto m = base100.measure(); (void)m;
    });

    check("head_ref() / last_ref()", [&] {
        volatile int h = base100.head_ref();
        volatile int l = base100.last_ref();
        (void)h; (void)l;
    });

    // -----------------------------------------------------------------------
    // Copy / assignment (same resource)
    // -----------------------------------------------------------------------

    check("copy construction (same resource)", [&] {
        FT copy = base100;         // same resource: shares d_repr, O(1)
        volatile auto s = copy.size(); (void)s;
    });

    {
        // Source tree built outside tracking; only the assignment is timed.
        auto src = FT::from_sequence(v50, &mr);
        check("move assignment (same resource)", [&] {
            FT t(&mr);
            t = std::move(src);
            volatile auto s = t.size(); (void)s;
        });
    }

    // -----------------------------------------------------------------------
    // Print results
    // -----------------------------------------------------------------------

    bool all_pass = true;
    std::fprintf(stdout, "\nFingerTree5 PMR allocation probe\n");
    std::fprintf(stdout, "%-50s  %s\n", "Operation", "Global new() calls");
    std::fprintf(stdout, "%s\n", std::string(70, '-').c_str());

    for (int i = 0; i < g_n_results; ++i) {
        const auto& r = g_results[i];
        if (r.pass) {
            std::fprintf(stdout, "  PASS  %-48s  %zu\n",
                         r.name, r.new_count);
        } else {
            std::fprintf(stdout, "  FAIL  %-48s  %zu  (expected 0)\n",
                         r.name, r.new_count);
            all_pass = false;
        }
    }
    std::fprintf(stdout, "%s\n", std::string(70, '-').c_str());
    std::fprintf(stdout, "%s\n", all_pass ? "ALL PASS" : "FAILURES DETECTED");

    return all_pass ? 0 : 1;
}
