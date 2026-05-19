// src/smd/tree/finger_tree_compare.bench.cpp                         -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Micro-benchmark comparison across FingerTree2/3/4/5.
// Build with -DTREE_ENABLE_BENCHMARKS=ON, then run the binary directly in
// RelWithDebInfo (Asan adds ~2x overhead and masks digit-representation
// differences between FT4 and FT5):
//
//   cd .build/build-clang
//   ninja smd_tree_benchmarks
//   ./RelWithDebInfo/smd_tree_benchmarks --benchmark-samples 100
//   --benchmark-warmup-time 1
//
// To run a single section:
//   ./RelWithDebInfo/smd_tree_benchmarks "Bench snoc" --benchmark-samples 50
//
// Design notes:
//   - Category A (snoc/cons): construction IS the operation; use BENCHMARK.
//   - Categories B/C (drain/append/split): tree pre-built outside meter.measure
//     so only the target operation is timed; use BENCHMARK_ADVANCED.
//   - Category D (persistent cons): FT3's lazy spine means K cons calls from
//     the same base share one memoized spine force; FT2/4/5 pay O(log N) per
//     call.  All four trees are compared here.
//
//   - FT2 kMaxDepth: FingerTree2 silently drops elements in release mode once
//     the spine depth exceeds kMaxDepth=5 (~1000-4000 elements depending on
//     insertion order).  At N >= 300'000 the FT2 measure() return values will
//     be less than N, reflecting a corrupt tree.  The benchmarks expose this
//     rather than hiding it; FT2 appearing anomalously fast at large N is
//     operating on a truncated structure.
//
// - FT3 O(N) thunk chain: FingerTree3's lazy spine thunks form an O(N/4)-deep
//     linked list during sequential construction.  Any operation that FORCES
//     the spine (drain, flatten, for_each, split_at_measure) must recurse
//     through this list.  Each recursive frame is large enough that the 8MB
//     default stack is exhausted at approximately N=100'000 for forcing
//     operations, and N~500'000–1'000'000 for destruction alone.
//     Exception: app3 (append) stays lazy — it creates a new lazy spine without
//     forcing, so FT3 append benchmarks run safely even at N=300'000+.
//     FT2 is strict (no thunks); FT4/FT5 destructor chains are O(log N).
//     FT3's persistent-amortization advantage exists only for small trees or
//     incremental (interleaved read/write) usage patterns.
//     Entries that crash are isolated in "Bench * FT3 large-N crash" test
//     cases so they can be run (or skipped) without aborting the other trees.

#include <smd/tree/finger_tree2.hpp>
#include <smd/tree/finger_tree3.hpp>
#include <smd/tree/finger_tree4.hpp>
#include <smd/tree/finger_tree5.hpp>

#include <catch2/benchmark/catch_benchmark_all.hpp>
#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <vector>

// ============================================================================
//                         SHARED HELPERS
// ============================================================================

namespace {

using FT2 = smd::tree::FingerTree2<int>;
using FT3 = smd::tree::FingerTree3<int>;
using FT4 = smd::tree::FingerTree4<int>;
using FT5 = smd::tree::FingerTree5<int>;

template <typename Tree>
auto build_snoc(int n) -> Tree {
    auto t = Tree{};
    for (int i = 0; i < n; ++i)
        t = t.snoc(i);
    return t;
}

template <typename Tree>
auto build_cons(int n) -> Tree {
    auto t = Tree{};
    for (int i = 0; i < n; ++i)
        t = t.cons(i);
    return t;
}

template <typename Tree>
auto drain_l(Tree t) -> int {
    int sum = 0;
    while (auto v = t.view_l()) {
        sum += v->d_value;
        t = std::move(v->d_rest);
    }
    return sum;
}

template <typename Tree>
auto drain_r(Tree t) -> int {
    int sum = 0;
    while (auto v = t.view_r()) {
        sum += v->d_value;
        t = std::move(v->d_rest);
    }
    return sum;
}

template <typename Tree>
auto sum_flatten(const Tree &t) -> int {
    int sum = 0;
    for (int x : t.flatten())
        sum += x;
    return sum;
}

template <typename Tree>
auto sum_for_each(const Tree &t) -> int {
    int sum = 0;
    t.for_each([&](int x) { sum += x; });
    return sum;
}

} // namespace

// ============================================================================
//                 A. BUILD — sequential insert
//
// All four trees; sizes 100, 1K, 10K, 100K.
// FT2/FT3 store nodes by value inside One/Two/Three/Four variants.
// FT4/FT5 allocate an Elem<T,Tag> via make_shared for every tree node.
// Expected: FT2/FT3 faster at small N; difference narrows at large N.
// FT5 should match or beat FT4 (inplace_vector vs four-way variant digit).
// ============================================================================

TEST_CASE("Bench A snoc") {
    BENCHMARK("FT2 100") { return build_snoc<FT2>(100).measure(); };
    BENCHMARK("FT3 100") { return build_snoc<FT3>(100).measure(); };
    BENCHMARK("FT4 100") { return build_snoc<FT4>(100).measure(); };
    BENCHMARK("FT5 100") { return build_snoc<FT5>(100).measure(); };

    BENCHMARK("FT2 1000") { return build_snoc<FT2>(1000).measure(); };
    BENCHMARK("FT3 1000") { return build_snoc<FT3>(1000).measure(); };
    BENCHMARK("FT4 1000") { return build_snoc<FT4>(1000).measure(); };
    BENCHMARK("FT5 1000") { return build_snoc<FT5>(1000).measure(); };

    BENCHMARK("FT2 10000") { return build_snoc<FT2>(10000).measure(); };
    BENCHMARK("FT3 10000") { return build_snoc<FT3>(10000).measure(); };
    BENCHMARK("FT4 10000") { return build_snoc<FT4>(10000).measure(); };
    BENCHMARK("FT5 10000") { return build_snoc<FT5>(10000).measure(); };

    BENCHMARK("FT2 100000") { return build_snoc<FT2>(100000).measure(); };
    BENCHMARK("FT3 100000") { return build_snoc<FT3>(100000).measure(); };
    BENCHMARK("FT4 100000") { return build_snoc<FT4>(100000).measure(); };
    BENCHMARK("FT5 100000") { return build_snoc<FT5>(100000).measure(); };

    BENCHMARK("FT2 300000") { return build_snoc<FT2>(300000).measure(); };
    BENCHMARK("FT3 300000") { return build_snoc<FT3>(300000).measure(); };
    BENCHMARK("FT4 300000") { return build_snoc<FT4>(300000).measure(); };
    BENCHMARK("FT5 300000") { return build_snoc<FT5>(300000).measure(); };

    // FT3 omitted at 1M/3M: O(N) thunk destructor chain → SIGSEGV.
    // See "Bench A snoc FT3 large-N crash" below.
    BENCHMARK("FT2 1000000") { return build_snoc<FT2>(1000000).measure(); };
    BENCHMARK("FT4 1000000") { return build_snoc<FT4>(1000000).measure(); };
    BENCHMARK("FT5 1000000") { return build_snoc<FT5>(1000000).measure(); };

    BENCHMARK("FT2 3000000") { return build_snoc<FT2>(3000000).measure(); };
    BENCHMARK("FT4 3000000") { return build_snoc<FT4>(3000000).measure(); };
    BENCHMARK("FT5 3000000") { return build_snoc<FT5>(3000000).measure(); };
}

// FT3-only test: isolated so that the SIGSEGV does not abort the other trees.
// Expected to fail (stack overflow) above ~500'000 elements.
// Documents the O(N) sequential-construction destructor hazard.
TEST_CASE("Bench A snoc FT3 large-N crash") {
    BENCHMARK("FT3 1000000") { return build_snoc<FT3>(1000000).measure(); };
    BENCHMARK("FT3 3000000") { return build_snoc<FT3>(3000000).measure(); };
}

TEST_CASE("Bench A cons") {
    BENCHMARK("FT2 100") { return build_cons<FT2>(100).measure(); };
    BENCHMARK("FT3 100") { return build_cons<FT3>(100).measure(); };
    BENCHMARK("FT4 100") { return build_cons<FT4>(100).measure(); };
    BENCHMARK("FT5 100") { return build_cons<FT5>(100).measure(); };

    BENCHMARK("FT2 1000") { return build_cons<FT2>(1000).measure(); };
    BENCHMARK("FT3 1000") { return build_cons<FT3>(1000).measure(); };
    BENCHMARK("FT4 1000") { return build_cons<FT4>(1000).measure(); };
    BENCHMARK("FT5 1000") { return build_cons<FT5>(1000).measure(); };

    BENCHMARK("FT2 10000") { return build_cons<FT2>(10000).measure(); };
    BENCHMARK("FT3 10000") { return build_cons<FT3>(10000).measure(); };
    BENCHMARK("FT4 10000") { return build_cons<FT4>(10000).measure(); };
    BENCHMARK("FT5 10000") { return build_cons<FT5>(10000).measure(); };

    BENCHMARK("FT2 100000") { return build_cons<FT2>(100000).measure(); };
    BENCHMARK("FT3 100000") { return build_cons<FT3>(100000).measure(); };
    BENCHMARK("FT4 100000") { return build_cons<FT4>(100000).measure(); };
    BENCHMARK("FT5 100000") { return build_cons<FT5>(100000).measure(); };

    BENCHMARK("FT2 300000") { return build_cons<FT2>(300000).measure(); };
    BENCHMARK("FT3 300000") { return build_cons<FT3>(300000).measure(); };
    BENCHMARK("FT4 300000") { return build_cons<FT4>(300000).measure(); };
    BENCHMARK("FT5 300000") { return build_cons<FT5>(300000).measure(); };

    // FT3 omitted at 1M/3M: O(N) thunk destructor chain → SIGSEGV.
    // See "Bench A cons FT3 large-N crash" below.
    BENCHMARK("FT2 1000000") { return build_cons<FT2>(1000000).measure(); };
    BENCHMARK("FT4 1000000") { return build_cons<FT4>(1000000).measure(); };
    BENCHMARK("FT5 1000000") { return build_cons<FT5>(1000000).measure(); };

    BENCHMARK("FT2 3000000") { return build_cons<FT2>(3000000).measure(); };
    BENCHMARK("FT4 3000000") { return build_cons<FT4>(3000000).measure(); };
    BENCHMARK("FT5 3000000") { return build_cons<FT5>(3000000).measure(); };
}

TEST_CASE("Bench A cons FT3 large-N crash") {
    BENCHMARK("FT3 1000000") { return build_cons<FT3>(1000000).measure(); };
    BENCHMARK("FT3 3000000") { return build_cons<FT3>(3000000).measure(); };
}

// ============================================================================
//                 B. DRAIN / TRAVERSE — sequential access
//
// Tree pre-built once; meter.measure sees only the traversal.
// drain_l and drain_r exercise view_l/view_r and spine-borrowing rebalancing.
// flatten/for_each are single-pass and bypass view entirely.
// ============================================================================

// FT3-only: forcing the spine during view_l/drain triggers the O(N) thunk
// chain. The crash threshold on an 8MB stack is ~N=100K (25K recursive forcing
// frames). Isolate so this does not abort the other trees.
TEST_CASE("Bench B drain FT3 large-N crash") {
    BENCHMARK_ADVANCED("FT3 drain_l 100000")(
        Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT3>(100000);
        meter.measure([&] { return drain_l(t); });
    };
    BENCHMARK_ADVANCED("FT3 flatten 100000")(
        Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT3>(100000);
        meter.measure([&] { return sum_flatten(t); });
    };
}

TEST_CASE("Bench B view_l drain") {
    BENCHMARK_ADVANCED("FT2 100")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT2>(100);
        meter.measure([&] { return drain_l(t); });
    };
    BENCHMARK_ADVANCED("FT3 100")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT3>(100);
        meter.measure([&] { return drain_l(t); });
    };
    BENCHMARK_ADVANCED("FT4 100")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT4>(100);
        meter.measure([&] { return drain_l(t); });
    };
    BENCHMARK_ADVANCED("FT5 100")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT5>(100);
        meter.measure([&] { return drain_l(t); });
    };

    BENCHMARK_ADVANCED("FT2 1000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT2>(1000);
        meter.measure([&] { return drain_l(t); });
    };
    BENCHMARK_ADVANCED("FT3 1000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT3>(1000);
        meter.measure([&] { return drain_l(t); });
    };
    BENCHMARK_ADVANCED("FT4 1000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT4>(1000);
        meter.measure([&] { return drain_l(t); });
    };
    BENCHMARK_ADVANCED("FT5 1000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT5>(1000);
        meter.measure([&] { return drain_l(t); });
    };

    BENCHMARK_ADVANCED("FT2 10000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT2>(10000);
        meter.measure([&] { return drain_l(t); });
    };
    BENCHMARK_ADVANCED("FT3 10000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT3>(10000);
        meter.measure([&] { return drain_l(t); });
    };
    BENCHMARK_ADVANCED("FT4 10000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT4>(10000);
        meter.measure([&] { return drain_l(t); });
    };
    BENCHMARK_ADVANCED("FT5 10000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT5>(10000);
        meter.measure([&] { return drain_l(t); });
    };

    BENCHMARK_ADVANCED("FT2 100000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT2>(100000);
        meter.measure([&] { return drain_l(t); });
    };
    // FT3 omitted: view_l forces the O(N) spine thunk chain → SIGSEGV at ~100K.
    BENCHMARK_ADVANCED("FT4 100000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT4>(100000);
        meter.measure([&] { return drain_l(t); });
    };
    BENCHMARK_ADVANCED("FT5 100000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT5>(100000);
        meter.measure([&] { return drain_l(t); });
    };

    BENCHMARK_ADVANCED("FT2 300000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT2>(300000);
        meter.measure([&] { return drain_l(t); });
    };
    // FT3 omitted (see above).
    BENCHMARK_ADVANCED("FT4 300000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT4>(300000);
        meter.measure([&] { return drain_l(t); });
    };
    BENCHMARK_ADVANCED("FT5 300000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT5>(300000);
        meter.measure([&] { return drain_l(t); });
    };

    BENCHMARK_ADVANCED("FT2 1000000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT2>(1000000);
        meter.measure([&] { return drain_l(t); });
    };
    // FT3 omitted: crashes on tree destruction (O(N) thunk chain).
    BENCHMARK_ADVANCED("FT4 1000000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT4>(1000000);
        meter.measure([&] { return drain_l(t); });
    };
    BENCHMARK_ADVANCED("FT5 1000000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT5>(1000000);
        meter.measure([&] { return drain_l(t); });
    };

    BENCHMARK_ADVANCED("FT2 3000000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT2>(3000000);
        meter.measure([&] { return drain_l(t); });
    };
    // FT3 omitted: crashes on tree destruction (O(N) thunk chain).
    BENCHMARK_ADVANCED("FT4 3000000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT4>(3000000);
        meter.measure([&] { return drain_l(t); });
    };
    BENCHMARK_ADVANCED("FT5 3000000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT5>(3000000);
        meter.measure([&] { return drain_l(t); });
    };
}

TEST_CASE("Bench B view_r drain") {
    BENCHMARK_ADVANCED("FT2 100")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT2>(100);
        meter.measure([&] { return drain_r(t); });
    };
    BENCHMARK_ADVANCED("FT3 100")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT3>(100);
        meter.measure([&] { return drain_r(t); });
    };
    BENCHMARK_ADVANCED("FT4 100")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT4>(100);
        meter.measure([&] { return drain_r(t); });
    };
    BENCHMARK_ADVANCED("FT5 100")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT5>(100);
        meter.measure([&] { return drain_r(t); });
    };

    BENCHMARK_ADVANCED("FT2 1000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT2>(1000);
        meter.measure([&] { return drain_r(t); });
    };
    BENCHMARK_ADVANCED("FT3 1000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT3>(1000);
        meter.measure([&] { return drain_r(t); });
    };
    BENCHMARK_ADVANCED("FT4 1000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT4>(1000);
        meter.measure([&] { return drain_r(t); });
    };
    BENCHMARK_ADVANCED("FT5 1000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT5>(1000);
        meter.measure([&] { return drain_r(t); });
    };

    BENCHMARK_ADVANCED("FT2 10000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT2>(10000);
        meter.measure([&] { return drain_r(t); });
    };
    BENCHMARK_ADVANCED("FT3 10000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT3>(10000);
        meter.measure([&] { return drain_r(t); });
    };
    BENCHMARK_ADVANCED("FT4 10000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT4>(10000);
        meter.measure([&] { return drain_r(t); });
    };
    BENCHMARK_ADVANCED("FT5 10000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT5>(10000);
        meter.measure([&] { return drain_r(t); });
    };

    BENCHMARK_ADVANCED("FT2 100000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT2>(100000);
        meter.measure([&] { return drain_r(t); });
    };
    // FT3 omitted: view_r forces the O(N) spine thunk chain → SIGSEGV at ~100K.
    BENCHMARK_ADVANCED("FT4 100000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT4>(100000);
        meter.measure([&] { return drain_r(t); });
    };
    BENCHMARK_ADVANCED("FT5 100000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT5>(100000);
        meter.measure([&] { return drain_r(t); });
    };

    BENCHMARK_ADVANCED("FT2 300000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT2>(300000);
        meter.measure([&] { return drain_r(t); });
    };
    // FT3 omitted (see above).
    BENCHMARK_ADVANCED("FT4 300000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT4>(300000);
        meter.measure([&] { return drain_r(t); });
    };
    BENCHMARK_ADVANCED("FT5 300000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT5>(300000);
        meter.measure([&] { return drain_r(t); });
    };

    BENCHMARK_ADVANCED("FT2 1000000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT2>(1000000);
        meter.measure([&] { return drain_r(t); });
    };
    // FT3 omitted: crashes on tree destruction (O(N) thunk chain).
    BENCHMARK_ADVANCED("FT4 1000000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT4>(1000000);
        meter.measure([&] { return drain_r(t); });
    };
    BENCHMARK_ADVANCED("FT5 1000000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT5>(1000000);
        meter.measure([&] { return drain_r(t); });
    };

    BENCHMARK_ADVANCED("FT2 3000000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT2>(3000000);
        meter.measure([&] { return drain_r(t); });
    };
    // FT3 omitted: crashes on tree destruction (O(N) thunk chain).
    BENCHMARK_ADVANCED("FT4 3000000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT4>(3000000);
        meter.measure([&] { return drain_r(t); });
    };
    BENCHMARK_ADVANCED("FT5 3000000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT5>(3000000);
        meter.measure([&] { return drain_r(t); });
    };
}

TEST_CASE("Bench B flatten") {
    BENCHMARK_ADVANCED("FT2 100")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT2>(100);
        meter.measure([&] { return sum_flatten(t); });
    };
    BENCHMARK_ADVANCED("FT3 100")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT3>(100);
        meter.measure([&] { return sum_flatten(t); });
    };
    BENCHMARK_ADVANCED("FT4 100")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT4>(100);
        meter.measure([&] { return sum_flatten(t); });
    };
    BENCHMARK_ADVANCED("FT5 100")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT5>(100);
        meter.measure([&] { return sum_flatten(t); });
    };

    BENCHMARK_ADVANCED("FT2 1000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT2>(1000);
        meter.measure([&] { return sum_flatten(t); });
    };
    BENCHMARK_ADVANCED("FT3 1000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT3>(1000);
        meter.measure([&] { return sum_flatten(t); });
    };
    BENCHMARK_ADVANCED("FT4 1000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT4>(1000);
        meter.measure([&] { return sum_flatten(t); });
    };
    BENCHMARK_ADVANCED("FT5 1000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT5>(1000);
        meter.measure([&] { return sum_flatten(t); });
    };

    BENCHMARK_ADVANCED("FT2 10000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT2>(10000);
        meter.measure([&] { return sum_flatten(t); });
    };
    BENCHMARK_ADVANCED("FT3 10000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT3>(10000);
        meter.measure([&] { return sum_flatten(t); });
    };
    BENCHMARK_ADVANCED("FT4 10000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT4>(10000);
        meter.measure([&] { return sum_flatten(t); });
    };
    BENCHMARK_ADVANCED("FT5 10000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT5>(10000);
        meter.measure([&] { return sum_flatten(t); });
    };

    BENCHMARK_ADVANCED("FT2 100000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT2>(100000);
        meter.measure([&] { return sum_flatten(t); });
    };
    // FT3 omitted: flatten forces the O(N) spine thunk chain → SIGSEGV at
    // ~100K.
    BENCHMARK_ADVANCED("FT4 100000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT4>(100000);
        meter.measure([&] { return sum_flatten(t); });
    };
    BENCHMARK_ADVANCED("FT5 100000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT5>(100000);
        meter.measure([&] { return sum_flatten(t); });
    };

    BENCHMARK_ADVANCED("FT2 300000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT2>(300000);
        meter.measure([&] { return sum_flatten(t); });
    };
    // FT3 omitted (see above).
    BENCHMARK_ADVANCED("FT4 300000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT4>(300000);
        meter.measure([&] { return sum_flatten(t); });
    };
    BENCHMARK_ADVANCED("FT5 300000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT5>(300000);
        meter.measure([&] { return sum_flatten(t); });
    };

    BENCHMARK_ADVANCED("FT2 1000000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT2>(1000000);
        meter.measure([&] { return sum_flatten(t); });
    };
    // FT3 omitted: crashes on tree destruction (O(N) thunk chain).
    BENCHMARK_ADVANCED("FT4 1000000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT4>(1000000);
        meter.measure([&] { return sum_flatten(t); });
    };
    BENCHMARK_ADVANCED("FT5 1000000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT5>(1000000);
        meter.measure([&] { return sum_flatten(t); });
    };

    BENCHMARK_ADVANCED("FT2 3000000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT2>(3000000);
        meter.measure([&] { return sum_flatten(t); });
    };
    // FT3 omitted: crashes on tree destruction (O(N) thunk chain).
    BENCHMARK_ADVANCED("FT4 3000000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT4>(3000000);
        meter.measure([&] { return sum_flatten(t); });
    };
    BENCHMARK_ADVANCED("FT5 3000000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT5>(3000000);
        meter.measure([&] { return sum_flatten(t); });
    };
}

TEST_CASE("Bench B for_each") {
    BENCHMARK_ADVANCED("FT2 100")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT2>(100);
        meter.measure([&] { return sum_for_each(t); });
    };
    BENCHMARK_ADVANCED("FT3 100")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT3>(100);
        meter.measure([&] { return sum_for_each(t); });
    };
    BENCHMARK_ADVANCED("FT4 100")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT4>(100);
        meter.measure([&] { return sum_for_each(t); });
    };
    BENCHMARK_ADVANCED("FT5 100")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT5>(100);
        meter.measure([&] { return sum_for_each(t); });
    };

    BENCHMARK_ADVANCED("FT2 1000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT2>(1000);
        meter.measure([&] { return sum_for_each(t); });
    };
    BENCHMARK_ADVANCED("FT3 1000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT3>(1000);
        meter.measure([&] { return sum_for_each(t); });
    };
    BENCHMARK_ADVANCED("FT4 1000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT4>(1000);
        meter.measure([&] { return sum_for_each(t); });
    };
    BENCHMARK_ADVANCED("FT5 1000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT5>(1000);
        meter.measure([&] { return sum_for_each(t); });
    };

    BENCHMARK_ADVANCED("FT2 10000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT2>(10000);
        meter.measure([&] { return sum_for_each(t); });
    };
    BENCHMARK_ADVANCED("FT3 10000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT3>(10000);
        meter.measure([&] { return sum_for_each(t); });
    };
    BENCHMARK_ADVANCED("FT4 10000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT4>(10000);
        meter.measure([&] { return sum_for_each(t); });
    };
    BENCHMARK_ADVANCED("FT5 10000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT5>(10000);
        meter.measure([&] { return sum_for_each(t); });
    };

    BENCHMARK_ADVANCED("FT2 100000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT2>(100000);
        meter.measure([&] { return sum_for_each(t); });
    };
    // FT3 omitted: for_each forces the O(N) spine thunk chain → SIGSEGV at
    // ~100K.
    BENCHMARK_ADVANCED("FT4 100000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT4>(100000);
        meter.measure([&] { return sum_for_each(t); });
    };
    BENCHMARK_ADVANCED("FT5 100000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT5>(100000);
        meter.measure([&] { return sum_for_each(t); });
    };

    BENCHMARK_ADVANCED("FT2 300000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT2>(300000);
        meter.measure([&] { return sum_for_each(t); });
    };
    // FT3 omitted (see above).
    BENCHMARK_ADVANCED("FT4 300000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT4>(300000);
        meter.measure([&] { return sum_for_each(t); });
    };
    BENCHMARK_ADVANCED("FT5 300000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT5>(300000);
        meter.measure([&] { return sum_for_each(t); });
    };

    BENCHMARK_ADVANCED("FT2 1000000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT2>(1000000);
        meter.measure([&] { return sum_for_each(t); });
    };
    // FT3 omitted: crashes on tree destruction (O(N) thunk chain).
    BENCHMARK_ADVANCED("FT4 1000000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT4>(1000000);
        meter.measure([&] { return sum_for_each(t); });
    };
    BENCHMARK_ADVANCED("FT5 1000000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT5>(1000000);
        meter.measure([&] { return sum_for_each(t); });
    };

    BENCHMARK_ADVANCED("FT2 3000000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT2>(3000000);
        meter.measure([&] { return sum_for_each(t); });
    };
    // FT3 omitted: crashes on tree destruction (O(N) thunk chain).
    BENCHMARK_ADVANCED("FT4 3000000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT4>(3000000);
        meter.measure([&] { return sum_for_each(t); });
    };
    BENCHMARK_ADVANCED("FT5 3000000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT5>(3000000);
        meter.measure([&] { return sum_for_each(t); });
    };
}

// ============================================================================
//                 C. STRUCTURAL — O(log N) operations
//
// append: Hinze-Paterson app3.  split: threshold search down the spine.
// split_and_concat: exercises both paths back-to-back; total work O(log N).
// Sizes 100, 1K, 10K (100K exceeds FT3 thunk-forcing budget for this run).
// ============================================================================

TEST_CASE("Bench C append") {
    BENCHMARK_ADVANCED("FT2 100+100")(Catch::Benchmark::Chronometer meter) {
        auto a = build_snoc<FT2>(100);
        auto b = build_snoc<FT2>(100);
        meter.measure([&] { return a.append(b).measure(); });
    };
    BENCHMARK_ADVANCED("FT3 100+100")(Catch::Benchmark::Chronometer meter) {
        auto a = build_snoc<FT3>(100);
        auto b = build_snoc<FT3>(100);
        meter.measure([&] { return a.append(b).measure(); });
    };
    BENCHMARK_ADVANCED("FT4 100+100")(Catch::Benchmark::Chronometer meter) {
        auto a = build_snoc<FT4>(100);
        auto b = build_snoc<FT4>(100);
        meter.measure([&] { return a.append(b).measure(); });
    };
    BENCHMARK_ADVANCED("FT5 100+100")(Catch::Benchmark::Chronometer meter) {
        auto a = build_snoc<FT5>(100);
        auto b = build_snoc<FT5>(100);
        meter.measure([&] { return a.append(b).measure(); });
    };

    BENCHMARK_ADVANCED("FT2 1000+1000")(Catch::Benchmark::Chronometer meter) {
        auto a = build_snoc<FT2>(1000);
        auto b = build_snoc<FT2>(1000);
        meter.measure([&] { return a.append(b).measure(); });
    };
    BENCHMARK_ADVANCED("FT3 1000+1000")(Catch::Benchmark::Chronometer meter) {
        auto a = build_snoc<FT3>(1000);
        auto b = build_snoc<FT3>(1000);
        meter.measure([&] { return a.append(b).measure(); });
    };
    BENCHMARK_ADVANCED("FT4 1000+1000")(Catch::Benchmark::Chronometer meter) {
        auto a = build_snoc<FT4>(1000);
        auto b = build_snoc<FT4>(1000);
        meter.measure([&] { return a.append(b).measure(); });
    };
    BENCHMARK_ADVANCED("FT5 1000+1000")(Catch::Benchmark::Chronometer meter) {
        auto a = build_snoc<FT5>(1000);
        auto b = build_snoc<FT5>(1000);
        meter.measure([&] { return a.append(b).measure(); });
    };

    BENCHMARK_ADVANCED("FT2 10000+10000")(Catch::Benchmark::Chronometer meter) {
        auto a = build_snoc<FT2>(10000);
        auto b = build_snoc<FT2>(10000);
        meter.measure([&] { return a.append(b).measure(); });
    };
    BENCHMARK_ADVANCED("FT3 10000+10000")(Catch::Benchmark::Chronometer meter) {
        auto a = build_snoc<FT3>(10000);
        auto b = build_snoc<FT3>(10000);
        meter.measure([&] { return a.append(b).measure(); });
    };
    BENCHMARK_ADVANCED("FT4 10000+10000")(Catch::Benchmark::Chronometer meter) {
        auto a = build_snoc<FT4>(10000);
        auto b = build_snoc<FT4>(10000);
        meter.measure([&] { return a.append(b).measure(); });
    };
    BENCHMARK_ADVANCED("FT5 10000+10000")(Catch::Benchmark::Chronometer meter) {
        auto a = build_snoc<FT5>(10000);
        auto b = build_snoc<FT5>(10000);
        meter.measure([&] { return a.append(b).measure(); });
    };

    BENCHMARK_ADVANCED("FT2 30000+30000")(Catch::Benchmark::Chronometer meter) {
        auto a = build_snoc<FT2>(30000);
        auto b = build_snoc<FT2>(30000);
        meter.measure([&] { return a.append(b).measure(); });
    };
    BENCHMARK_ADVANCED("FT3 30000+30000")(Catch::Benchmark::Chronometer meter) {
        auto a = build_snoc<FT3>(30000);
        auto b = build_snoc<FT3>(30000);
        meter.measure([&] { return a.append(b).measure(); });
    };
    BENCHMARK_ADVANCED("FT4 30000+30000")(Catch::Benchmark::Chronometer meter) {
        auto a = build_snoc<FT4>(30000);
        auto b = build_snoc<FT4>(30000);
        meter.measure([&] { return a.append(b).measure(); });
    };
    BENCHMARK_ADVANCED("FT5 30000+30000")(Catch::Benchmark::Chronometer meter) {
        auto a = build_snoc<FT5>(30000);
        auto b = build_snoc<FT5>(30000);
        meter.measure([&] { return a.append(b).measure(); });
    };

    BENCHMARK_ADVANCED("FT2 100000+100000")(
        Catch::Benchmark::Chronometer meter) {
        auto a = build_snoc<FT2>(100000);
        auto b = build_snoc<FT2>(100000);
        meter.measure([&] { return a.append(b).measure(); });
    };
    // FT3 included: app3 is lazy — no spine thunks are forced during append.
    // The result's measure() reads cached values only.
    BENCHMARK_ADVANCED("FT3 100000+100000")(
        Catch::Benchmark::Chronometer meter) {
        auto a = build_snoc<FT3>(100000);
        auto b = build_snoc<FT3>(100000);
        meter.measure([&] { return a.append(b).measure(); });
    };
    BENCHMARK_ADVANCED("FT4 100000+100000")(
        Catch::Benchmark::Chronometer meter) {
        auto a = build_snoc<FT4>(100000);
        auto b = build_snoc<FT4>(100000);
        meter.measure([&] { return a.append(b).measure(); });
    };
    BENCHMARK_ADVANCED("FT5 100000+100000")(
        Catch::Benchmark::Chronometer meter) {
        auto a = build_snoc<FT5>(100000);
        auto b = build_snoc<FT5>(100000);
        meter.measure([&] { return a.append(b).measure(); });
    };

    BENCHMARK_ADVANCED("FT2 300000+300000")(
        Catch::Benchmark::Chronometer meter) {
        auto a = build_snoc<FT2>(300000);
        auto b = build_snoc<FT2>(300000);
        meter.measure([&] { return a.append(b).measure(); });
    };
    // FT3 included: lazy app3 creates a new lazy spine (no forcing, no crash).
    BENCHMARK_ADVANCED("FT3 300000+300000")(
        Catch::Benchmark::Chronometer meter) {
        auto a = build_snoc<FT3>(300000);
        auto b = build_snoc<FT3>(300000);
        meter.measure([&] { return a.append(b).measure(); });
    };
    BENCHMARK_ADVANCED("FT4 300000+300000")(
        Catch::Benchmark::Chronometer meter) {
        auto a = build_snoc<FT4>(300000);
        auto b = build_snoc<FT4>(300000);
        meter.measure([&] { return a.append(b).measure(); });
    };
    BENCHMARK_ADVANCED("FT5 300000+300000")(
        Catch::Benchmark::Chronometer meter) {
        auto a = build_snoc<FT5>(300000);
        auto b = build_snoc<FT5>(300000);
        meter.measure([&] { return a.append(b).measure(); });
    };
}

TEST_CASE("Bench C split at middle") {
    BENCHMARK_ADVANCED("FT2 100")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT2>(100);
        meter.measure([&] {
            auto sa = t.split_at_measure(std::size_t{50});
            return sa.d_left.measure();
        });
    };
    BENCHMARK_ADVANCED("FT3 100")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT3>(100);
        meter.measure([&] {
            auto sa = t.split_at_measure(std::size_t{50});
            return sa.d_left.measure();
        });
    };
    BENCHMARK_ADVANCED("FT4 100")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT4>(100);
        meter.measure([&] {
            auto sa = t.split_at_measure(std::size_t{50});
            return sa.d_left.measure();
        });
    };
    BENCHMARK_ADVANCED("FT5 100")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT5>(100);
        meter.measure([&] {
            auto sa = t.split_at_measure(std::size_t{50});
            return sa.d_left.measure();
        });
    };

    BENCHMARK_ADVANCED("FT2 1000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT2>(1000);
        meter.measure([&] {
            auto sa = t.split_at_measure(std::size_t{500});
            return sa.d_left.measure();
        });
    };
    BENCHMARK_ADVANCED("FT3 1000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT3>(1000);
        meter.measure([&] {
            auto sa = t.split_at_measure(std::size_t{500});
            return sa.d_left.measure();
        });
    };
    BENCHMARK_ADVANCED("FT4 1000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT4>(1000);
        meter.measure([&] {
            auto sa = t.split_at_measure(std::size_t{500});
            return sa.d_left.measure();
        });
    };
    BENCHMARK_ADVANCED("FT5 1000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT5>(1000);
        meter.measure([&] {
            auto sa = t.split_at_measure(std::size_t{500});
            return sa.d_left.measure();
        });
    };

    BENCHMARK_ADVANCED("FT2 10000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT2>(10000);
        meter.measure([&] {
            auto sa = t.split_at_measure(std::size_t{5000});
            return sa.d_left.measure();
        });
    };
    BENCHMARK_ADVANCED("FT3 10000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT3>(10000);
        meter.measure([&] {
            auto sa = t.split_at_measure(std::size_t{5000});
            return sa.d_left.measure();
        });
    };
    BENCHMARK_ADVANCED("FT4 10000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT4>(10000);
        meter.measure([&] {
            auto sa = t.split_at_measure(std::size_t{5000});
            return sa.d_left.measure();
        });
    };
    BENCHMARK_ADVANCED("FT5 10000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT5>(10000);
        meter.measure([&] {
            auto sa = t.split_at_measure(std::size_t{5000});
            return sa.d_left.measure();
        });
    };

    BENCHMARK_ADVANCED("FT2 30000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT2>(30000);
        meter.measure([&] {
            auto sa = t.split_at_measure(std::size_t{15000});
            return sa.d_left.measure();
        });
    };
    BENCHMARK_ADVANCED("FT3 30000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT3>(30000);
        meter.measure([&] {
            auto sa = t.split_at_measure(std::size_t{15000});
            return sa.d_left.measure();
        });
    };
    BENCHMARK_ADVANCED("FT4 30000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT4>(30000);
        meter.measure([&] {
            auto sa = t.split_at_measure(std::size_t{15000});
            return sa.d_left.measure();
        });
    };
    BENCHMARK_ADVANCED("FT5 30000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT5>(30000);
        meter.measure([&] {
            auto sa = t.split_at_measure(std::size_t{15000});
            return sa.d_left.measure();
        });
    };

    BENCHMARK_ADVANCED("FT2 100000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT2>(100000);
        meter.measure([&] {
            auto sa = t.split_at_measure(std::size_t{50000});
            return sa.d_left.measure();
        });
    };
    // FT3 omitted at 100K+: split_impl forces the O(N) thunk chain → SIGSEGV.
    // See "Bench C split FT3 large-N crash" below.
    BENCHMARK_ADVANCED("FT4 100000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT4>(100000);
        meter.measure([&] {
            auto sa = t.split_at_measure(std::size_t{50000});
            return sa.d_left.measure();
        });
    };
    BENCHMARK_ADVANCED("FT5 100000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT5>(100000);
        meter.measure([&] {
            auto sa = t.split_at_measure(std::size_t{50000});
            return sa.d_left.measure();
        });
    };

    BENCHMARK_ADVANCED("FT2 300000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT2>(300000);
        meter.measure([&] {
            auto sa = t.split_at_measure(std::size_t{150000});
            return sa.d_left.measure();
        });
    };
    // FT3 omitted: O(N) thunk chain forced during split_impl.
    BENCHMARK_ADVANCED("FT4 300000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT4>(300000);
        meter.measure([&] {
            auto sa = t.split_at_measure(std::size_t{150000});
            return sa.d_left.measure();
        });
    };
    BENCHMARK_ADVANCED("FT5 300000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT5>(300000);
        meter.measure([&] {
            auto sa = t.split_at_measure(std::size_t{150000});
            return sa.d_left.measure();
        });
    };
}

// FT3-only: split forces all spine thunks in one O(N) recursive chain.
// These are expected to crash at N >= ~100K.
TEST_CASE("Bench C split FT3 large-N crash") {
    BENCHMARK_ADVANCED("FT3 100000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT3>(100000);
        meter.measure([&] {
            auto sa = t.split_at_measure(std::size_t{50000});
            return sa.d_left.measure();
        });
    };
    BENCHMARK_ADVANCED("FT3 300000")(Catch::Benchmark::Chronometer meter) {
        auto t = build_snoc<FT3>(300000);
        meter.measure([&] {
            auto sa = t.split_at_measure(std::size_t{150000});
            return sa.d_left.measure();
        });
    };
}

// ============================================================================
//                 D. PERSISTENT FAN-OUT — FT3's advantage scenario
//
// K = 100 persistent cons operations from the same base snapshot.
//
// FT2/FT4/FT5: each cons is strict — the spine push (if the left digit is
// full) executes immediately and independently for each of the K callers.
//
// FT3: each cons creates a lazy thunk; the first drain that forces the thunk
// memoizes the result.  Subsequent drains get the cached result for free.
// Here we only measure cons (not drain), so the thunks are never forced and
// we directly see the cons-allocation cost difference.
//
// For the talk: run with --benchmark-samples 200 and compare "persistent cons"
// for small K (advantage is muted) vs large K (FT3 advantage clear).
// ============================================================================

TEST_CASE("Bench D persistent cons K=100 N=1000") {
    BENCHMARK_ADVANCED("FT2")(Catch::Benchmark::Chronometer meter) {
        auto base = build_snoc<FT2>(1000);
        meter.measure([&] {
            int sum = 0;
            for (int k = 0; k < 100; ++k)
                sum += static_cast<int>(base.cons(k).measure());
            return sum;
        });
    };
    BENCHMARK_ADVANCED("FT3")(Catch::Benchmark::Chronometer meter) {
        auto base = build_snoc<FT3>(1000);
        meter.measure([&] {
            int sum = 0;
            for (int k = 0; k < 100; ++k)
                sum += static_cast<int>(base.cons(k).measure());
            return sum;
        });
    };
    BENCHMARK_ADVANCED("FT4")(Catch::Benchmark::Chronometer meter) {
        auto base = build_snoc<FT4>(1000);
        meter.measure([&] {
            int sum = 0;
            for (int k = 0; k < 100; ++k)
                sum += static_cast<int>(base.cons(k).measure());
            return sum;
        });
    };
    BENCHMARK_ADVANCED("FT5")(Catch::Benchmark::Chronometer meter) {
        auto base = build_snoc<FT5>(1000);
        meter.measure([&] {
            int sum = 0;
            for (int k = 0; k < 100; ++k)
                sum += static_cast<int>(base.cons(k).measure());
            return sum;
        });
    };
}

TEST_CASE("Bench D persistent drain K=100 N=1000") {
    // FT3's memoization pays off when we also flatten each derived tree:
    // the spine thunk forced by the first drain is reused by the other 99.
    // FT2/FT4/FT5 have no such sharing — each flatten independently traverses
    // the full structure (though structural sharing via shared_ptr still helps
    // for FT4/FT5 at the node level).
    BENCHMARK_ADVANCED("FT2")(Catch::Benchmark::Chronometer meter) {
        auto base = build_snoc<FT2>(1000);
        auto derived = std::vector<FT2>{};
        derived.reserve(100);
        for (int k = 0; k < 100; ++k)
            derived.push_back(base.cons(k));
        meter.measure([&] {
            int sum = 0;
            for (const auto &t : derived)
                sum += sum_flatten(t);
            return sum;
        });
    };
    BENCHMARK_ADVANCED("FT3")(Catch::Benchmark::Chronometer meter) {
        auto base = build_snoc<FT3>(1000);
        auto derived = std::vector<FT3>{};
        derived.reserve(100);
        for (int k = 0; k < 100; ++k)
            derived.push_back(base.cons(k));
        meter.measure([&] {
            int sum = 0;
            for (const auto &t : derived)
                sum += sum_flatten(t);
            return sum;
        });
    };
    BENCHMARK_ADVANCED("FT4")(Catch::Benchmark::Chronometer meter) {
        auto base = build_snoc<FT4>(1000);
        auto derived = std::vector<FT4>{};
        derived.reserve(100);
        for (int k = 0; k < 100; ++k)
            derived.push_back(base.cons(k));
        meter.measure([&] {
            int sum = 0;
            for (const auto &t : derived)
                sum += sum_flatten(t);
            return sum;
        });
    };
    BENCHMARK_ADVANCED("FT5")(Catch::Benchmark::Chronometer meter) {
        auto base = build_snoc<FT5>(1000);
        auto derived = std::vector<FT5>{};
        derived.reserve(100);
        for (int k = 0; k < 100; ++k)
            derived.push_back(base.cons(k));
        meter.measure([&] {
            int sum = 0;
            for (const auto &t : derived)
                sum += sum_flatten(t);
            return sum;
        });
    };
}

TEST_CASE("Bench D persistent cons K=100 N=3000") {
    BENCHMARK_ADVANCED("FT2")(Catch::Benchmark::Chronometer meter) {
        auto base = build_snoc<FT2>(3000);
        meter.measure([&] {
            int sum = 0;
            for (int k = 0; k < 100; ++k)
                sum += static_cast<int>(base.cons(k).measure());
            return sum;
        });
    };
    BENCHMARK_ADVANCED("FT3")(Catch::Benchmark::Chronometer meter) {
        auto base = build_snoc<FT3>(3000);
        meter.measure([&] {
            int sum = 0;
            for (int k = 0; k < 100; ++k)
                sum += static_cast<int>(base.cons(k).measure());
            return sum;
        });
    };
    BENCHMARK_ADVANCED("FT4")(Catch::Benchmark::Chronometer meter) {
        auto base = build_snoc<FT4>(3000);
        meter.measure([&] {
            int sum = 0;
            for (int k = 0; k < 100; ++k)
                sum += static_cast<int>(base.cons(k).measure());
            return sum;
        });
    };
    BENCHMARK_ADVANCED("FT5")(Catch::Benchmark::Chronometer meter) {
        auto base = build_snoc<FT5>(3000);
        meter.measure([&] {
            int sum = 0;
            for (int k = 0; k < 100; ++k)
                sum += static_cast<int>(base.cons(k).measure());
            return sum;
        });
    };
}

TEST_CASE("Bench D persistent cons K=100 N=10000") {
    BENCHMARK_ADVANCED("FT2")(Catch::Benchmark::Chronometer meter) {
        auto base = build_snoc<FT2>(10000);
        meter.measure([&] {
            int sum = 0;
            for (int k = 0; k < 100; ++k)
                sum += static_cast<int>(base.cons(k).measure());
            return sum;
        });
    };
    BENCHMARK_ADVANCED("FT3")(Catch::Benchmark::Chronometer meter) {
        auto base = build_snoc<FT3>(10000);
        meter.measure([&] {
            int sum = 0;
            for (int k = 0; k < 100; ++k)
                sum += static_cast<int>(base.cons(k).measure());
            return sum;
        });
    };
    BENCHMARK_ADVANCED("FT4")(Catch::Benchmark::Chronometer meter) {
        auto base = build_snoc<FT4>(10000);
        meter.measure([&] {
            int sum = 0;
            for (int k = 0; k < 100; ++k)
                sum += static_cast<int>(base.cons(k).measure());
            return sum;
        });
    };
    BENCHMARK_ADVANCED("FT5")(Catch::Benchmark::Chronometer meter) {
        auto base = build_snoc<FT5>(10000);
        meter.measure([&] {
            int sum = 0;
            for (int k = 0; k < 100; ++k)
                sum += static_cast<int>(base.cons(k).measure());
            return sum;
        });
    };
}

TEST_CASE("Bench D persistent cons K=100 N=30000") {
    BENCHMARK_ADVANCED("FT2")(Catch::Benchmark::Chronometer meter) {
        auto base = build_snoc<FT2>(30000);
        meter.measure([&] {
            int sum = 0;
            for (int k = 0; k < 100; ++k)
                sum += static_cast<int>(base.cons(k).measure());
            return sum;
        });
    };
    BENCHMARK_ADVANCED("FT3")(Catch::Benchmark::Chronometer meter) {
        auto base = build_snoc<FT3>(30000);
        meter.measure([&] {
            int sum = 0;
            for (int k = 0; k < 100; ++k)
                sum += static_cast<int>(base.cons(k).measure());
            return sum;
        });
    };
    BENCHMARK_ADVANCED("FT4")(Catch::Benchmark::Chronometer meter) {
        auto base = build_snoc<FT4>(30000);
        meter.measure([&] {
            int sum = 0;
            for (int k = 0; k < 100; ++k)
                sum += static_cast<int>(base.cons(k).measure());
            return sum;
        });
    };
    BENCHMARK_ADVANCED("FT5")(Catch::Benchmark::Chronometer meter) {
        auto base = build_snoc<FT5>(30000);
        meter.measure([&] {
            int sum = 0;
            for (int k = 0; k < 100; ++k)
                sum += static_cast<int>(base.cons(k).measure());
            return sum;
        });
    };
}

TEST_CASE("Bench D persistent drain K=100 N=3000") {
    BENCHMARK_ADVANCED("FT2")(Catch::Benchmark::Chronometer meter) {
        auto base = build_snoc<FT2>(3000);
        auto derived = std::vector<FT2>{};
        derived.reserve(100);
        for (int k = 0; k < 100; ++k)
            derived.push_back(base.cons(k));
        meter.measure([&] {
            int sum = 0;
            for (const auto &t : derived)
                sum += sum_flatten(t);
            return sum;
        });
    };
    BENCHMARK_ADVANCED("FT3")(Catch::Benchmark::Chronometer meter) {
        auto base = build_snoc<FT3>(3000);
        auto derived = std::vector<FT3>{};
        derived.reserve(100);
        for (int k = 0; k < 100; ++k)
            derived.push_back(base.cons(k));
        meter.measure([&] {
            int sum = 0;
            for (const auto &t : derived)
                sum += sum_flatten(t);
            return sum;
        });
    };
    BENCHMARK_ADVANCED("FT4")(Catch::Benchmark::Chronometer meter) {
        auto base = build_snoc<FT4>(3000);
        auto derived = std::vector<FT4>{};
        derived.reserve(100);
        for (int k = 0; k < 100; ++k)
            derived.push_back(base.cons(k));
        meter.measure([&] {
            int sum = 0;
            for (const auto &t : derived)
                sum += sum_flatten(t);
            return sum;
        });
    };
    BENCHMARK_ADVANCED("FT5")(Catch::Benchmark::Chronometer meter) {
        auto base = build_snoc<FT5>(3000);
        auto derived = std::vector<FT5>{};
        derived.reserve(100);
        for (int k = 0; k < 100; ++k)
            derived.push_back(base.cons(k));
        meter.measure([&] {
            int sum = 0;
            for (const auto &t : derived)
                sum += sum_flatten(t);
            return sum;
        });
    };
}

TEST_CASE("Bench D persistent drain K=100 N=10000") {
    BENCHMARK_ADVANCED("FT2")(Catch::Benchmark::Chronometer meter) {
        auto base = build_snoc<FT2>(10000);
        auto derived = std::vector<FT2>{};
        derived.reserve(100);
        for (int k = 0; k < 100; ++k)
            derived.push_back(base.cons(k));
        meter.measure([&] {
            int sum = 0;
            for (const auto &t : derived)
                sum += sum_flatten(t);
            return sum;
        });
    };
    BENCHMARK_ADVANCED("FT3")(Catch::Benchmark::Chronometer meter) {
        auto base = build_snoc<FT3>(10000);
        auto derived = std::vector<FT3>{};
        derived.reserve(100);
        for (int k = 0; k < 100; ++k)
            derived.push_back(base.cons(k));
        meter.measure([&] {
            int sum = 0;
            for (const auto &t : derived)
                sum += sum_flatten(t);
            return sum;
        });
    };
    BENCHMARK_ADVANCED("FT4")(Catch::Benchmark::Chronometer meter) {
        auto base = build_snoc<FT4>(10000);
        auto derived = std::vector<FT4>{};
        derived.reserve(100);
        for (int k = 0; k < 100; ++k)
            derived.push_back(base.cons(k));
        meter.measure([&] {
            int sum = 0;
            for (const auto &t : derived)
                sum += sum_flatten(t);
            return sum;
        });
    };
    BENCHMARK_ADVANCED("FT5")(Catch::Benchmark::Chronometer meter) {
        auto base = build_snoc<FT5>(10000);
        auto derived = std::vector<FT5>{};
        derived.reserve(100);
        for (int k = 0; k < 100; ++k)
            derived.push_back(base.cons(k));
        meter.measure([&] {
            int sum = 0;
            for (const auto &t : derived)
                sum += sum_flatten(t);
            return sum;
        });
    };
}

TEST_CASE("Bench D persistent drain K=100 N=30000") {
    BENCHMARK_ADVANCED("FT2")(Catch::Benchmark::Chronometer meter) {
        auto base = build_snoc<FT2>(30000);
        auto derived = std::vector<FT2>{};
        derived.reserve(100);
        for (int k = 0; k < 100; ++k)
            derived.push_back(base.cons(k));
        meter.measure([&] {
            int sum = 0;
            for (const auto &t : derived)
                sum += sum_flatten(t);
            return sum;
        });
    };
    BENCHMARK_ADVANCED("FT3")(Catch::Benchmark::Chronometer meter) {
        auto base = build_snoc<FT3>(30000);
        auto derived = std::vector<FT3>{};
        derived.reserve(100);
        for (int k = 0; k < 100; ++k)
            derived.push_back(base.cons(k));
        meter.measure([&] {
            int sum = 0;
            for (const auto &t : derived)
                sum += sum_flatten(t);
            return sum;
        });
    };
    BENCHMARK_ADVANCED("FT4")(Catch::Benchmark::Chronometer meter) {
        auto base = build_snoc<FT4>(30000);
        auto derived = std::vector<FT4>{};
        derived.reserve(100);
        for (int k = 0; k < 100; ++k)
            derived.push_back(base.cons(k));
        meter.measure([&] {
            int sum = 0;
            for (const auto &t : derived)
                sum += sum_flatten(t);
            return sum;
        });
    };
    BENCHMARK_ADVANCED("FT5")(Catch::Benchmark::Chronometer meter) {
        auto base = build_snoc<FT5>(30000);
        auto derived = std::vector<FT5>{};
        derived.reserve(100);
        for (int k = 0; k < 100; ++k)
            derived.push_back(base.cons(k));
        meter.measure([&] {
            int sum = 0;
            for (const auto &t : derived)
                sum += sum_flatten(t);
            return sum;
        });
    };
}
