// src/smd/tree/finger_tree3_compile_probe.cpp                        -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Compile-time regression probe for FingerTree3.
//
// FingerTree3 has the same DEPTH NTTP template explosion as FingerTree2
// (kMaxDepth+1 = 6 distinct class bodies per <T,Tag,Measure>) plus the
// lazy-spine machinery: erased_thunk, memoize, SpineSusp.  Expected compile
// cost is similar to or slightly higher than FingerTree2.
//
// Note: FingerTree3's app3 compares the accumulated spine measure against
// tag_id<Tag>() using !=, so the Tag type must provide operator==
// (operator!= is synthesized from it in C++20).
//
// Measuring compile time (from .build/build-gcc-16):
//
//   touch ../../src/smd/tree/finger_tree3_compile_probe.cpp
//   time ninja -j1
//   src/smd/tree/CMakeFiles/smd_tree_compile_probes.dir/Debug/finger_tree3_compile_probe.cpp.o
//
// Counting DWARF class-type entries:
//
//   readelf --debug-dump=info
//   src/smd/tree/CMakeFiles/smd_tree_compile_probes.dir/Debug/finger_tree3_compile_probe.cpp.o
//   | grep -c DW_TAG_class_type
//
// GCC per-phase time breakdown (add temporarily; produces verbose output):
//   target_compile_options(smd_tree_compile_probes PRIVATE -ftime-report)

#include <smd/tree/finger_tree3.hpp>

#include <cstddef>

namespace {

struct WeightTag {
    int value;
    friend auto operator==(const WeightTag &, const WeightTag &)
        -> bool = default;
    friend auto operator>=(WeightTag a, WeightTag b) -> bool {
        return a.value >= b.value;
    }
};

struct WeightMeasure {
    auto operator()(int x) const -> WeightTag { return WeightTag{x}; }
};

} // namespace

namespace smd::typeclass {
template <>
struct Monoid<WeightTag> {
    constexpr auto identity() const -> WeightTag { return WeightTag{0}; }
    constexpr auto combine(WeightTag a, WeightTag b) const -> WeightTag {
        return WeightTag{a.value + b.value};
    }
};
} // namespace smd::typeclass

void finger_tree3_probe_unit_measure() {
    using FT = smd::tree::FingerTree3<int>;

    auto t = FT::empty();
    for (int i = 0; i < 16; ++i)
        t = t.snoc(i);
    for (int i = 0; i < 4; ++i)
        t = t.cons(-i);

    auto m = t.measure();
    auto v = t.flatten();
    auto h = t.head();
    auto la = t.last();
    auto vl = t.view_l();
    auto vr = t.view_r();
    auto ta = t.tail();
    auto in = t.init();
    auto ap = t.append(t);
    auto sa = t.split_at_measure(std::size_t{10});
    auto sp = t.split([](std::size_t p) { return p > 5; });
    auto sr = t.search([](std::size_t p) { return p > 5; });
    int sum = 0;
    t.for_each([&](int x) { sum += x; });

    (void)m;
    (void)v;
    (void)h;
    (void)la;
    (void)vl;
    (void)vr;
    (void)ta;
    (void)in;
    (void)ap;
    (void)sa;
    (void)sp;
    (void)sr;
    (void)sum;
}

void finger_tree3_probe_custom_measure() {
    using FT = smd::tree::FingerTree3<int, WeightTag, WeightMeasure>;

    auto t = FT::empty();
    for (int i = 1; i <= 8; ++i)
        t = t.snoc(i);

    auto sa = t.split_at_measure(WeightTag{15});
    auto ap = t.append(t);
    auto v = t.flatten();
    auto sp = t.split([](WeightTag p) { return p.value > 10; });

    (void)sa;
    (void)ap;
    (void)v;
    (void)sp;
}
