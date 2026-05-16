// src/smd/tree/finger_tree_std_compare.bench.cpp                      -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Baseline benchmarks comparing FingerTree5-backed wrappers against their
// std library equivalents at N = {100, 1000, 10000, 100000}.
//
// Build with -DTREE_ENABLE_BENCHMARKS=ON in RelWithDebInfo (Asan adds ~2×
// overhead and should not be used for timing):
//
//   ninja -C .build/build-clang/RelWithDebInfo smd_tree_benchmarks
//   ./RelWithDebInfo/smd_tree_benchmarks "Bench Std: RandomAccess vs vector"
//       --benchmark-samples 20 --benchmark-warmup-time 1
//
// Design:
//   - BENCHMARK:          operation IS the construction (e.g. push N times from empty)
//   - BENCHMARK_ADVANCED: data pre-built outside meter.measure; only the
//     timed operation is measured
//   - Each lambda returns a value to prevent dead-code elimination
//   - Fixed RNG seed 42 for reproducibility
//   - Category 6 ("Sequential iteration — std wins") is included deliberately
//     as an honesty check; vector should win by a large margin

#include <smd/tree/finger_tree_interval_index.hpp>
#include <smd/tree/finger_tree_priority_queue.hpp>
#include <smd/tree/finger_tree_random_access.hpp>
#include <smd/tree/finger_tree_rope.hpp>

#include <catch2/benchmark/catch_benchmark_all.hpp>
#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <deque>
#include <functional>
#include <numeric>
#include <queue>
#include <random>
#include <string>
#include <vector>

// ============================================================================
//                              HELPERS
// ============================================================================

namespace {

using Seq  = smd::tree::FingerTreeRandomAccess<int>;
using Rope = smd::tree::FingerTreeRope<>;
using PQ   = smd::tree::FingerTreePriorityQueue<int>;
using Idx  = smd::tree::FingerTreeIntervalIndex<int>;
using Ivl  = smd::tree::Interval<int>;

// -- RandomAccess helpers ----------------------------------------------------

auto make_seq(int n) -> Seq {
    std::vector<int> v(n);
    std::iota(v.begin(), v.end(), 0);
    return Seq::from_sequence(std::move(v));
}

auto make_vec(int n) -> std::vector<int> {
    std::vector<int> v(n);
    std::iota(v.begin(), v.end(), 0);
    return v;
}

// -- Rope helpers ------------------------------------------------------------

auto make_rope(std::size_t n) -> Rope {
    return Rope::from_text(std::string(n, 'x'));
}

// -- Priority queue helpers --------------------------------------------------

auto make_pq_vec(int n) -> std::vector<int> {
    std::vector<int> v(n);
    std::mt19937 rng{42};
    std::generate(v.begin(), v.end(), [&] { return static_cast<int>(rng() % 100'000); });
    return v;
}

auto make_pq(int n) -> PQ {
    return PQ::from_values(make_pq_vec(n));
}

auto make_std_min_pq(int n) {
    std::priority_queue<int, std::vector<int>, std::greater<int>> pq;
    for (auto x : make_pq_vec(n))
        pq.push(x);
    return pq;
}

auto make_std_max_pq(int n) {
    std::priority_queue<int> pq;
    for (auto x : make_pq_vec(n))
        pq.push(x);
    return pq;
}

// -- Interval index helpers --------------------------------------------------

auto make_ivl_vec(int n) -> std::vector<Ivl> {
    std::vector<Ivl> v;
    v.reserve(n);
    std::mt19937 rng{42};
    for (int i = 0; i < n; ++i) {
        auto start = static_cast<std::size_t>(rng() % 10'000);
        auto end   = start + static_cast<std::size_t>(rng() % 200) + 1;
        v.push_back(Ivl{start, end, i});
    }
    return v;
}

auto make_idx(int n) -> Idx {
    return Idx::from_intervals(make_ivl_vec(n));
}

auto brute_query_point(const std::vector<Ivl> &intervals,
                       std::size_t             point) -> std::vector<int> {
    std::vector<int> out;
    for (const auto &iv : intervals)
        if (iv.d_start <= point && point < iv.d_end)
            out.push_back(iv.d_payload);
    return out;
}

auto brute_query_overlap(const std::vector<Ivl> &intervals,
                         std::size_t lo, std::size_t hi) -> std::vector<int> {
    std::vector<int> out;
    for (const auto &iv : intervals)
        if (iv.d_start < hi && lo < iv.d_end)
            out.push_back(iv.d_payload);
    return out;
}

} // namespace

// ============================================================================
//     CATEGORY 1: RandomAccess vs std::vector / std::deque
// ============================================================================

TEST_CASE("Bench Std: RandomAccess vs vector")
{
    // -------------------------------------------------------------------------
    // push_back — build N-element sequence from empty
    // FT5 expected ~2–4× slower than vector (make_shared per node overflow)
    // -------------------------------------------------------------------------

    BENCHMARK("FT5 push_back N=100") {
        auto s = Seq{};
        for (int i = 0; i < 100; ++i) s = s.push_back(i);
        return s.size();
    };
    BENCHMARK("vector push_back N=100") {
        std::vector<int> v;
        for (int i = 0; i < 100; ++i) v.push_back(i);
        return v.size();
    };

    BENCHMARK("FT5 push_back N=1000") {
        auto s = Seq{};
        for (int i = 0; i < 1000; ++i) s = s.push_back(i);
        return s.size();
    };
    BENCHMARK("vector push_back N=1000") {
        std::vector<int> v;
        for (int i = 0; i < 1000; ++i) v.push_back(i);
        return v.size();
    };

    BENCHMARK("FT5 push_back N=10000") {
        auto s = Seq{};
        for (int i = 0; i < 10'000; ++i) s = s.push_back(i);
        return s.size();
    };
    BENCHMARK("vector push_back N=10000") {
        std::vector<int> v;
        for (int i = 0; i < 10'000; ++i) v.push_back(i);
        return v.size();
    };

    // -------------------------------------------------------------------------
    // push_front — FT5 O(1) amort vs deque O(1) amort, vector O(N)
    // -------------------------------------------------------------------------

    BENCHMARK("FT5 push_front N=1000") {
        auto s = Seq{};
        for (int i = 0; i < 1000; ++i) s = s.push_front(i);
        return s.size();
    };
    BENCHMARK("deque push_front N=1000") {
        std::deque<int> d;
        for (int i = 0; i < 1000; ++i) d.push_front(i);
        return d.size();
    };
    BENCHMARK("vector insert_front N=1000") {
        std::vector<int> v;
        v.reserve(1000);
        for (int i = 0; i < 1000; ++i) v.insert(v.begin(), i);
        return v.size();
    };

    BENCHMARK("FT5 push_front N=10000") {
        auto s = Seq{};
        for (int i = 0; i < 10'000; ++i) s = s.push_front(i);
        return s.size();
    };
    BENCHMARK("deque push_front N=10000") {
        std::deque<int> d;
        for (int i = 0; i < 10'000; ++i) d.push_front(i);
        return d.size();
    };

    // -------------------------------------------------------------------------
    // at_random — FT5 O(log N) vs vector O(1)
    // Vector expected to win by 10–100×
    // -------------------------------------------------------------------------

    BENCHMARK_ADVANCED("FT5 at_random N=1000")(Catch::Benchmark::Chronometer meter) {
        auto s = make_seq(1000);
        std::mt19937 rng{42};
        auto idx = static_cast<std::size_t>(rng() % 1000);
        meter.measure([&] { return s.at(idx); });
    };
    BENCHMARK_ADVANCED("vector at_random N=1000")(Catch::Benchmark::Chronometer meter) {
        auto v = make_vec(1000);
        std::mt19937 rng{42};
        auto idx = static_cast<std::size_t>(rng() % 1000);
        meter.measure([&] { return v[idx]; });
    };

    BENCHMARK_ADVANCED("FT5 at_random N=100000")(Catch::Benchmark::Chronometer meter) {
        auto s = make_seq(100'000);
        std::mt19937 rng{42};
        auto idx = static_cast<std::size_t>(rng() % 100'000);
        meter.measure([&] { return s.at(idx); });
    };
    BENCHMARK_ADVANCED("vector at_random N=100000")(Catch::Benchmark::Chronometer meter) {
        auto v = make_vec(100'000);
        std::mt19937 rng{42};
        auto idx = static_cast<std::size_t>(rng() % 100'000);
        meter.measure([&] { return v[idx]; });
    };

    // -------------------------------------------------------------------------
    // insert_mid — FT5 O(log N) vs vector O(N)
    // FT5 expected to win at N ≥ ~1000
    // -------------------------------------------------------------------------

    BENCHMARK_ADVANCED("FT5 insert_mid N=100")(Catch::Benchmark::Chronometer meter) {
        auto s = make_seq(100);
        meter.measure([&] { return s.insert(50, 42).size(); });
    };
    BENCHMARK_ADVANCED("vector insert_mid N=100")(Catch::Benchmark::Chronometer meter) {
        auto v = make_vec(100);
        meter.measure([&] {
            auto w = v;
            w.insert(w.begin() + 50, 42);
            return w.size();
        });
    };

    BENCHMARK_ADVANCED("FT5 insert_mid N=10000")(Catch::Benchmark::Chronometer meter) {
        auto s = make_seq(10'000);
        meter.measure([&] { return s.insert(5000, 42).size(); });
    };
    BENCHMARK_ADVANCED("vector insert_mid N=10000")(Catch::Benchmark::Chronometer meter) {
        auto v = make_vec(10'000);
        meter.measure([&] {
            auto w = v;
            w.insert(w.begin() + 5000, 42);
            return w.size();
        });
    };

    BENCHMARK_ADVANCED("FT5 insert_mid N=100000")(Catch::Benchmark::Chronometer meter) {
        auto s = make_seq(100'000);
        meter.measure([&] { return s.insert(50'000, 42).size(); });
    };
    BENCHMARK_ADVANCED("vector insert_mid N=100000")(Catch::Benchmark::Chronometer meter) {
        auto v = make_vec(100'000);
        meter.measure([&] {
            auto w = v;
            w.insert(w.begin() + 50'000, 42);
            return w.size();
        });
    };

    // -------------------------------------------------------------------------
    // erase_mid — FT5 O(log N) vs vector O(N)
    // -------------------------------------------------------------------------

    BENCHMARK_ADVANCED("FT5 erase_mid N=10000")(Catch::Benchmark::Chronometer meter) {
        auto s = make_seq(10'000);
        meter.measure([&] { return s.erase(5000).size(); });
    };
    BENCHMARK_ADVANCED("vector erase_mid N=10000")(Catch::Benchmark::Chronometer meter) {
        auto v = make_vec(10'000);
        meter.measure([&] {
            auto w = v;
            w.erase(w.begin() + 5000);
            return w.size();
        });
    };

    BENCHMARK_ADVANCED("FT5 erase_mid N=100000")(Catch::Benchmark::Chronometer meter) {
        auto s = make_seq(100'000);
        meter.measure([&] { return s.erase(50'000).size(); });
    };
    BENCHMARK_ADVANCED("vector erase_mid N=100000")(Catch::Benchmark::Chronometer meter) {
        auto v = make_vec(100'000);
        meter.measure([&] {
            auto w = v;
            w.erase(w.begin() + 50'000);
            return w.size();
        });
    };

    // -------------------------------------------------------------------------
    // concat — FT5 O(log N) vs vector O(N) copy+insert
    // FT5 expected to win at all N
    // -------------------------------------------------------------------------

    BENCHMARK_ADVANCED("FT5 concat N=10000")(Catch::Benchmark::Chronometer meter) {
        auto a = make_seq(5000);
        auto b = make_seq(5000);
        meter.measure([&] {
            // FingerTreeRandomAccess has no direct concat; go through the tree
            auto av = a.to_vector();
            auto bv = b.to_vector();
            av.insert(av.end(), bv.begin(), bv.end());
            return Seq::from_sequence(std::move(av)).size();
            // TODO: add a concat(a, b) method to the wrapper for O(log N) concat
        });
    };
    BENCHMARK_ADVANCED("vector concat N=10000")(Catch::Benchmark::Chronometer meter) {
        auto a = make_vec(5000);
        auto b = make_vec(5000);
        meter.measure([&] {
            auto c = a;
            c.insert(c.end(), b.begin(), b.end());
            return c.size();
        });
    };

    // -------------------------------------------------------------------------
    // snapshot — FT5 O(1) copy vs vector O(N) deep copy
    // FT5 expected to win enormously at large N
    // -------------------------------------------------------------------------

    BENCHMARK_ADVANCED("FT5 snapshot N=10000")(Catch::Benchmark::Chronometer meter) {
        auto s = make_seq(10'000);
        meter.measure([&] {
            auto copy = s;  // O(1): just copies the internal shared_ptr
            return copy.size();
        });
    };
    BENCHMARK_ADVANCED("vector snapshot N=10000")(Catch::Benchmark::Chronometer meter) {
        auto v = make_vec(10'000);
        meter.measure([&] {
            auto copy = v;  // O(N)
            return copy.size();
        });
    };

    BENCHMARK_ADVANCED("FT5 snapshot N=100000")(Catch::Benchmark::Chronometer meter) {
        auto s = make_seq(100'000);
        meter.measure([&] { return (s).size(); });
    };
    BENCHMARK_ADVANCED("vector snapshot N=100000")(Catch::Benchmark::Chronometer meter) {
        auto v = make_vec(100'000);
        meter.measure([&] {
            auto copy = v;
            return copy.size();
        });
    };
}

// ============================================================================
//     CATEGORY 2: Rope vs std::string
// ============================================================================

TEST_CASE("Bench Std: Rope vs string")
{
    // -------------------------------------------------------------------------
    // insert_mid — Rope O(log N) vs string O(N)
    // -------------------------------------------------------------------------

    BENCHMARK_ADVANCED("Rope insert_mid N=10000")(Catch::Benchmark::Chronometer meter) {
        auto r = make_rope(10'000);
        meter.measure([&] { return r.insert(5000, "hello").size_bytes(); });
    };
    BENCHMARK_ADVANCED("string insert_mid N=10000")(Catch::Benchmark::Chronometer meter) {
        auto s = std::string(10'000, 'x');
        meter.measure([&] {
            auto t = s;
            t.insert(5000, "hello");
            return t.size();
        });
    };

    BENCHMARK_ADVANCED("Rope insert_mid N=100000")(Catch::Benchmark::Chronometer meter) {
        auto r = make_rope(100'000);
        meter.measure([&] { return r.insert(50'000, "hello").size_bytes(); });
    };
    BENCHMARK_ADVANCED("string insert_mid N=100000")(Catch::Benchmark::Chronometer meter) {
        auto s = std::string(100'000, 'x');
        meter.measure([&] {
            auto t = s;
            t.insert(50'000, "hello");
            return t.size();
        });
    };

    // -------------------------------------------------------------------------
    // erase_mid — Rope O(log N) vs string O(N)
    // -------------------------------------------------------------------------

    BENCHMARK_ADVANCED("Rope erase_mid N=10000")(Catch::Benchmark::Chronometer meter) {
        auto r = make_rope(10'000);
        meter.measure([&] { return r.erase(5000, 5).size_bytes(); });
    };
    BENCHMARK_ADVANCED("string erase_mid N=10000")(Catch::Benchmark::Chronometer meter) {
        auto s = std::string(10'000, 'x');
        meter.measure([&] {
            auto t = s;
            t.erase(5000, 5);
            return t.size();
        });
    };

    BENCHMARK_ADVANCED("Rope erase_mid N=100000")(Catch::Benchmark::Chronometer meter) {
        auto r = make_rope(100'000);
        meter.measure([&] { return r.erase(50'000, 5).size_bytes(); });
    };
    BENCHMARK_ADVANCED("string erase_mid N=100000")(Catch::Benchmark::Chronometer meter) {
        auto s = std::string(100'000, 'x');
        meter.measure([&] {
            auto t = s;
            t.erase(50'000, 5);
            return t.size();
        });
    };

    // -------------------------------------------------------------------------
    // concat — Rope via insert-at-end vs string operator+
    // TODO: add Rope::concat(Rope, Rope) for a true O(log N) comparison;
    //       current insert(size_bytes(), rhs.to_string()) materialises rhs, so
    //       this overstates the rope's cost.
    // -------------------------------------------------------------------------

    BENCHMARK_ADVANCED("Rope concat_via_insert N=10000")(Catch::Benchmark::Chronometer meter) {
        auto a = make_rope(5000);
        auto b = make_rope(5000);
        meter.measure([&] {
            return a.insert(a.size_bytes(), b.to_string()).size_bytes();
        });
    };
    BENCHMARK_ADVANCED("string concat N=10000")(Catch::Benchmark::Chronometer meter) {
        auto a = std::string(5000, 'x');
        auto b = std::string(5000, 'y');
        meter.measure([&] { return (a + b).size(); });
    };

    // -------------------------------------------------------------------------
    // snapshot — Rope O(1) copy vs string O(N) copy
    // -------------------------------------------------------------------------

    BENCHMARK_ADVANCED("Rope snapshot N=100000")(Catch::Benchmark::Chronometer meter) {
        auto r = make_rope(100'000);
        meter.measure([&] { return r.size_bytes(); });  // copy is O(1)
    };
    BENCHMARK_ADVANCED("string snapshot N=100000")(Catch::Benchmark::Chronometer meter) {
        auto s = std::string(100'000, 'x');
        meter.measure([&] {
            auto copy = s;
            return copy.size();
        });
    };

    // -------------------------------------------------------------------------
    // materialise — Rope to_string() vs string noop
    // String wins (already contiguous)
    // -------------------------------------------------------------------------

    BENCHMARK_ADVANCED("Rope to_string N=10000")(Catch::Benchmark::Chronometer meter) {
        auto r = make_rope(10'000);
        meter.measure([&] { return r.to_string().size(); });
    };
    BENCHMARK_ADVANCED("string noop N=10000")(Catch::Benchmark::Chronometer meter) {
        auto s = std::string(10'000, 'x');
        meter.measure([&] { return s.size(); });
    };
}

// ============================================================================
//     CATEGORY 3: PriorityQueue vs std::priority_queue
// ============================================================================

TEST_CASE("Bench Std: PQ vs std::priority_queue")
{
    // -------------------------------------------------------------------------
    // push N elements from empty
    // FT5 O(1) amort vs std O(log N) — FT5 expected faster at large N
    // -------------------------------------------------------------------------

    BENCHMARK("FT5 PQ push N=1000") {
        auto pq = PQ{};
        std::mt19937 rng{42};
        for (int i = 0; i < 1000; ++i)
            pq = pq.push(static_cast<int>(rng() % 100'000));
        return pq.min();
    };
    BENCHMARK("std min-PQ push N=1000") {
        std::priority_queue<int, std::vector<int>, std::greater<int>> pq;
        std::mt19937 rng{42};
        for (int i = 0; i < 1000; ++i)
            pq.push(static_cast<int>(rng() % 100'000));
        return pq.top();
    };

    BENCHMARK("FT5 PQ push N=10000") {
        auto pq = PQ{};
        std::mt19937 rng{42};
        for (int i = 0; i < 10'000; ++i)
            pq = pq.push(static_cast<int>(rng() % 100'000));
        return pq.min();
    };
    BENCHMARK("std min-PQ push N=10000") {
        std::priority_queue<int, std::vector<int>, std::greater<int>> pq;
        std::mt19937 rng{42};
        for (int i = 0; i < 10'000; ++i)
            pq.push(static_cast<int>(rng() % 100'000));
        return pq.top();
    };

    // -------------------------------------------------------------------------
    // pop_min — drain all N elements from pre-built PQ
    // -------------------------------------------------------------------------

    BENCHMARK_ADVANCED("FT5 PQ pop_min_all N=1000")(Catch::Benchmark::Chronometer meter) {
        auto base = make_pq(1000);
        meter.measure([&] {
            auto pq = base;  // O(1) copy
            int sum = 0;
            while (auto p = pq.pop_min()) {
                sum += p->first;
                pq = std::move(p->second);
            }
            return sum;
        });
    };
    BENCHMARK_ADVANCED("std PQ pop_min_all N=1000")(Catch::Benchmark::Chronometer meter) {
        auto base = make_std_min_pq(1000);
        meter.measure([&] {
            auto pq = base;  // O(N) copy
            int sum = 0;
            while (!pq.empty()) { sum += pq.top(); pq.pop(); }
            return sum;
        });
    };

    BENCHMARK_ADVANCED("FT5 PQ pop_min_all N=10000")(Catch::Benchmark::Chronometer meter) {
        auto base = make_pq(10'000);
        meter.measure([&] {
            auto pq = base;
            int sum = 0;
            while (auto p = pq.pop_min()) { sum += p->first; pq = std::move(p->second); }
            return sum;
        });
    };
    BENCHMARK_ADVANCED("std PQ pop_min_all N=10000")(Catch::Benchmark::Chronometer meter) {
        auto base = make_std_min_pq(10'000);
        meter.measure([&] {
            auto pq = base;
            int sum = 0;
            while (!pq.empty()) { sum += pq.top(); pq.pop(); }
            return sum;
        });
    };

    // -------------------------------------------------------------------------
    // pop_max — FT5 is double-ended (unique vs std); std max-heap comparison
    // -------------------------------------------------------------------------

    BENCHMARK_ADVANCED("FT5 PQ pop_max_all N=10000")(Catch::Benchmark::Chronometer meter) {
        auto base = make_pq(10'000);
        meter.measure([&] {
            auto pq = base;
            int sum = 0;
            while (auto p = pq.pop_max()) { sum += p->first; pq = std::move(p->second); }
            return sum;
        });
    };
    BENCHMARK_ADVANCED("std max-PQ pop_all N=10000")(Catch::Benchmark::Chronometer meter) {
        auto base = make_std_max_pq(10'000);
        meter.measure([&] {
            auto pq = base;
            int sum = 0;
            while (!pq.empty()) { sum += pq.top(); pq.pop(); }
            return sum;
        });
    };

    // -------------------------------------------------------------------------
    // push_then_pop — interleaved push2/pop1 N times
    // Realistic workload; tests amortised behaviour
    // -------------------------------------------------------------------------

    BENCHMARK("FT5 PQ push2_pop1 N=1000") {
        auto pq = PQ{};
        std::mt19937 rng{42};
        for (int i = 0; i < 1000; ++i) {
            pq = pq.push(static_cast<int>(rng() % 100'000));
            pq = pq.push(static_cast<int>(rng() % 100'000));
            if (auto p = pq.pop_min()) pq = std::move(p->second);
        }
        return pq.min();
    };
    BENCHMARK("std PQ push2_pop1 N=1000") {
        std::priority_queue<int, std::vector<int>, std::greater<int>> pq;
        std::mt19937 rng{42};
        for (int i = 0; i < 1000; ++i) {
            pq.push(static_cast<int>(rng() % 100'000));
            pq.push(static_cast<int>(rng() % 100'000));
            if (!pq.empty()) pq.pop();
        }
        return pq.top();
    };

    BENCHMARK("FT5 PQ push2_pop1 N=10000") {
        auto pq = PQ{};
        std::mt19937 rng{42};
        for (int i = 0; i < 10'000; ++i) {
            pq = pq.push(static_cast<int>(rng() % 100'000));
            pq = pq.push(static_cast<int>(rng() % 100'000));
            if (auto p = pq.pop_min()) pq = std::move(p->second);
        }
        return pq.min();
    };
    BENCHMARK("std PQ push2_pop1 N=10000") {
        std::priority_queue<int, std::vector<int>, std::greater<int>> pq;
        std::mt19937 rng{42};
        for (int i = 0; i < 10'000; ++i) {
            pq.push(static_cast<int>(rng() % 100'000));
            pq.push(static_cast<int>(rng() % 100'000));
            if (!pq.empty()) pq.pop();
        }
        return pq.top();
    };

    // TODO: PQ merge benchmark — add a merge method to FingerTreePriorityQueue
    //       (currently requires accessing d_tree directly which is private)
}

// ============================================================================
//     CATEGORY 4: IntervalIndex vs brute-force std::vector scan
// ============================================================================

TEST_CASE("Bench Std: IntervalIndex vs brute-force")
{
    // -------------------------------------------------------------------------
    // build — insert N random intervals from empty
    // -------------------------------------------------------------------------

    BENCHMARK("FT5 Idx build N=1000") {
        auto v = make_ivl_vec(1000);
        return Idx::from_intervals(std::move(v)).entries().size();
    };
    BENCHMARK("vector build N=1000") {
        auto v = make_ivl_vec(1000);
        return v.size();  // O(1): vector is the "index"
    };

    BENCHMARK("FT5 Idx build N=10000") {
        auto v = make_ivl_vec(10'000);
        return Idx::from_intervals(std::move(v)).entries().size();
    };
    BENCHMARK("vector build N=10000") {
        auto v = make_ivl_vec(10'000);
        return v.size();
    };

    // -------------------------------------------------------------------------
    // query_point — single point query on pre-built index
    // FT5 O(log N + k) vs brute-force O(N)
    // FT5 expected to win at N ≥ 1000
    // -------------------------------------------------------------------------

    BENCHMARK_ADVANCED("FT5 Idx query_point N=1000")(Catch::Benchmark::Chronometer meter) {
        auto idx = make_idx(1000);
        meter.measure([&] { return idx.query_point(5000).size(); });
    };
    BENCHMARK_ADVANCED("vector query_point N=1000")(Catch::Benchmark::Chronometer meter) {
        auto v = make_ivl_vec(1000);
        meter.measure([&] { return brute_query_point(v, 5000).size(); });
    };

    BENCHMARK_ADVANCED("FT5 Idx query_point N=10000")(Catch::Benchmark::Chronometer meter) {
        auto idx = make_idx(10'000);
        meter.measure([&] { return idx.query_point(5000).size(); });
    };
    BENCHMARK_ADVANCED("vector query_point N=10000")(Catch::Benchmark::Chronometer meter) {
        auto v = make_ivl_vec(10'000);
        meter.measure([&] { return brute_query_point(v, 5000).size(); });
    };

    BENCHMARK_ADVANCED("FT5 Idx query_point N=100000")(Catch::Benchmark::Chronometer meter) {
        auto idx = make_idx(100'000);
        meter.measure([&] { return idx.query_point(5000).size(); });
    };
    BENCHMARK_ADVANCED("vector query_point N=100000")(Catch::Benchmark::Chronometer meter) {
        auto v = make_ivl_vec(100'000);
        meter.measure([&] { return brute_query_point(v, 5000).size(); });
    };

    // -------------------------------------------------------------------------
    // query_overlap — range query on pre-built index
    // -------------------------------------------------------------------------

    BENCHMARK_ADVANCED("FT5 Idx query_overlap N=10000")(Catch::Benchmark::Chronometer meter) {
        auto idx = make_idx(10'000);
        meter.measure([&] { return idx.query_overlap(4000, 6000).size(); });
    };
    BENCHMARK_ADVANCED("vector query_overlap N=10000")(Catch::Benchmark::Chronometer meter) {
        auto v = make_ivl_vec(10'000);
        meter.measure([&] { return brute_query_overlap(v, 4000, 6000).size(); });
    };

    BENCHMARK_ADVANCED("FT5 Idx query_overlap N=100000")(Catch::Benchmark::Chronometer meter) {
        auto idx = make_idx(100'000);
        meter.measure([&] { return idx.query_overlap(4000, 6000).size(); });
    };
    BENCHMARK_ADVANCED("vector query_overlap N=100000")(Catch::Benchmark::Chronometer meter) {
        auto v = make_ivl_vec(100'000);
        meter.measure([&] { return brute_query_overlap(v, 4000, 6000).size(); });
    };
}

// ============================================================================
//     CATEGORY 5: Persistence cost
// ============================================================================
//
// Creates K snapshots from the same base structure.
// FT5: O(K × log N) total (path copying per modification)
// std: O(K × N) total (deep copy per snapshot)
// At K=100 and N=10000, the gap should be ~100× in FT5's favour.

TEST_CASE("Bench Std: Persistence cost")
{
    constexpr int N = 10'000;

    // K=10 snapshots — each derived independently from the same base
    BENCHMARK_ADVANCED("FT5 10 snapshots N=10000")(Catch::Benchmark::Chronometer meter) {
        auto base = make_seq(N);
        meter.measure([&] {
            std::size_t total = 0;
            for (int k = 0; k < 10; ++k)
                total += base.insert(N / 2, k).size();  // O(log N) each, base unchanged
            return total;
        });
    };
    BENCHMARK_ADVANCED("vector 10 snapshots N=10000")(Catch::Benchmark::Chronometer meter) {
        auto base = make_vec(N);
        meter.measure([&] {
            std::size_t total = 0;
            for (int k = 0; k < 10; ++k) {
                auto copy = base;                        // O(N) deep copy each time
                copy.insert(copy.begin() + N / 2, k);
                total += copy.size();
            }
            return total;
        });
    };

    // K=100 snapshots
    BENCHMARK_ADVANCED("FT5 100 snapshots N=10000")(Catch::Benchmark::Chronometer meter) {
        auto base = make_seq(N);
        meter.measure([&] {
            std::size_t total = 0;
            for (int k = 0; k < 100; ++k)
                total += base.insert(N / 2, k).size();
            return total;
        });
    };
    BENCHMARK_ADVANCED("vector 100 snapshots N=10000")(Catch::Benchmark::Chronometer meter) {
        auto base = make_vec(N);
        meter.measure([&] {
            std::size_t total = 0;
            for (int k = 0; k < 100; ++k) {
                auto copy = base;
                copy.insert(copy.begin() + N / 2, k);
                total += copy.size();
            }
            return total;
        });
    };
}

// ============================================================================
//     CATEGORY 6: Sequential iteration — std wins (honesty check)
// ============================================================================
//
// Vector's contiguous layout makes sequential iteration 10–50× faster.
// FingerTreeRandomAccess does not yet expose begin()/end() (that requires
// the Container compliance pass, Phase 4).  Until then the FT5 benchmarks
// include an explicit to_vector() materialisation, which makes FT5 look
// even slower — the correct direction for an honesty-check category.

TEST_CASE("Bench Std: Sequential iteration (std wins)")
{
    BENCHMARK_ADVANCED("FT5 iterate_sum N=1000")(Catch::Benchmark::Chronometer meter) {
        auto s = make_seq(1000);
        meter.measure([&] {
            // to_vector() materialises the tree (O(N)) before iterating
            int sum = 0;
            for (auto x : s.to_vector()) sum += x;
            return sum;
        });
    };
    BENCHMARK_ADVANCED("vector iterate_sum N=1000")(Catch::Benchmark::Chronometer meter) {
        auto v = make_vec(1000);
        meter.measure([&] {
            int sum = 0;
            for (auto x : v) sum += x;
            return sum;
        });
    };

    BENCHMARK_ADVANCED("FT5 iterate_sum N=10000")(Catch::Benchmark::Chronometer meter) {
        auto s = make_seq(10'000);
        meter.measure([&] {
            int sum = 0;
            for (auto x : s.to_vector()) sum += x;
            return sum;
        });
    };
    BENCHMARK_ADVANCED("vector iterate_sum N=10000")(Catch::Benchmark::Chronometer meter) {
        auto v = make_vec(10'000);
        meter.measure([&] {
            int sum = 0;
            for (auto x : v) sum += x;
            return sum;
        });
    };

    BENCHMARK_ADVANCED("FT5 iterate_sum N=100000")(Catch::Benchmark::Chronometer meter) {
        auto s = make_seq(100'000);
        meter.measure([&] {
            int sum = 0;
            for (auto x : s.to_vector()) sum += x;
            return sum;
        });
    };
    BENCHMARK_ADVANCED("vector iterate_sum N=100000")(Catch::Benchmark::Chronometer meter) {
        auto v = make_vec(100'000);
        meter.measure([&] {
            int sum = 0;
            for (auto x : v) sum += x;
            return sum;
        });
    };
}
