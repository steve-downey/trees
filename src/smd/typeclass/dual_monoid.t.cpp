// src/smd/typeclass/dual_monoid.t.cpp                                -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <smd/typeclass/dual_monoid.hpp>
#include <smd/typeclass/dual_monoid.hpp> // Re-inclusion verification

#include <catch2/catch_test_macros.hpp>

#include <string>

using smd::typeclass::DualMonoid;
using smd::typeclass::monoid_v;

TEST_CASE("DualMonoid - IdentityIsIdentity") {
    // combine(identity, x) == x  and  combine(x, identity) == x
    const auto &m = monoid_v<DualMonoid<std::string>>;
    DualMonoid<std::string> id = m.identity();
    DualMonoid<std::string> val = {"hello"};

    CHECK(m.combine(id, val) == val);
    CHECK(m.combine(val, id) == val);
}

TEST_CASE("DualMonoid - FlipsArguments") {
    // DualMonoid<string> reverses the concatenation order.
    const auto &m = monoid_v<DualMonoid<std::string>>;
    DualMonoid<std::string> a = {"ab"};
    DualMonoid<std::string> b = {"cd"};

    // Normal string monoid: combine("ab","cd") == "abcd"
    CHECK(monoid_v<std::string>.combine("ab", "cd") == "abcd");

    // Dual: combine(a, b).value == "cd" + "ab" == "cdab"
    CHECK(m.combine(a, b).value == "cdab");
}

TEST_CASE("DualMonoid - AssociativityHolds") {
    // Even though combine is flipped, associativity is preserved because
    // M itself is associative.
    const auto &m = monoid_v<DualMonoid<std::string>>;
    DualMonoid<std::string> a = {"a"};
    DualMonoid<std::string> b = {"b"};
    DualMonoid<std::string> c = {"c"};

    auto lhs = m.combine(m.combine(a, b), c);
    auto rhs = m.combine(a, m.combine(b, c));
    CHECK(lhs == rhs);
}

TEST_CASE("DualMonoid - DoubleDualMatchesOriginal") {
    // DualMonoid<DualMonoid<M>>.combine is operationally equivalent to
    // M.combine: the two flips cancel out.
    const auto &m = monoid_v<std::string>;
    const auto &dm = monoid_v<DualMonoid<std::string>>;
    const auto &ddm = monoid_v<DualMonoid<DualMonoid<std::string>>>;

    std::string a = "ab";
    std::string b = "cd";
    DualMonoid<DualMonoid<std::string>> dda = {DualMonoid<std::string>{a}};
    DualMonoid<DualMonoid<std::string>> ddb = {DualMonoid<std::string>{b}};

    // Original: "ab" + "cd" = "abcd"
    CHECK(m.combine(a, b) == "abcd");

    // DualMonoid: "cd" + "ab" = "cdab"
    CHECK(dm.combine(DualMonoid<std::string>{a}, DualMonoid<std::string>{b})
              .value == "cdab");

    // DoubleDual: flips back — "ab" + "cd" = "abcd"
    CHECK(ddm.combine(dda, ddb).value.value == "abcd");
}

TEST_CASE("DualMonoid - ForwardsSizeGeq") {
    // DualMonoid<std::size_t> forwards operator>= from std::size_t.
    DualMonoid<std::size_t> big = {10U};
    DualMonoid<std::size_t> small_ = {3U};
    CHECK(big >= small_);
    CHECK(!(small_ >= big));
}
