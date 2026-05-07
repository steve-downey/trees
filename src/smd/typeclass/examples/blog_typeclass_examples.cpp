// src/smd/typeclass/examples/blog_typeclass_examples.cpp             -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <smd/typeclass/examples/examples.hpp>

#include <smd/typeclass/foldable.hpp>
#include <smd/typeclass/functor.hpp>
#include <smd/typeclass/test/test_support.hpp>

#include <limits>
#include <optional>
#include <vector>

namespace smd::typeclass::examples {

// blog-numeric-limits-parallel-begin
auto blog_numeric_limits_parallel() -> bool {
    auto max_int = std::numeric_limits<int>::max();
    if (max_int != 2147483647) {
        return false;
    }

    const auto &functor = smd::functor_typeclass<std::optional<int>>;
    auto mapped = functor.fmap([](int x) { return x + 1; },
                               std::optional<int>{41});
    if (!mapped.has_value() || *mapped != 42) {
        return false;
    }

    return true;
}
// blog-numeric-limits-parallel-end

// blog-three-lookup-modes-begin
auto blog_three_lookup_modes() -> bool {
    using Sequence = smd::typeclass::test::Sequence<int>;
    auto seq = Sequence{{1, 2, 3}};

    const auto &foldable = smd::foldable_typeclass<Sequence>;
    auto implicit_sum =
        foldable.fold_map([](int x) { return x; }, seq);
    if (implicit_sum != 6) {
        return false;
    }

    auto explicit_length = foldable.length(seq);
    if (explicit_length != 3U) {
        return false;
    }

    return true;
}
// blog-three-lookup-modes-end

// blog-derived-operations-begin
auto blog_derived_operations() -> bool {
    const auto &functor = smd::functor_typeclass<std::optional<int>>;

    auto mapped = functor.fmap([](int x) { return x * 2; },
                               std::optional<int>{5});
    if (!mapped.has_value() || *mapped != 10) {
        return false;
    }

    auto replaced = functor.replace(std::optional<int>{5}, 99);
    if (!replaced.has_value() || *replaced != 99) {
        return false;
    }

    auto replaced_empty = functor.replace(std::optional<int>{}, 99);
    if (replaced_empty.has_value()) {
        return false;
    }

    return true;
}
// blog-derived-operations-end

} // namespace smd::typeclass::examples
