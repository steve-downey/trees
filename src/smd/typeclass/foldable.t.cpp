#include <smd/typeclass/foldable.hpp>
#include <smd/typeclass/foldable.hpp> // Re-inclusion check
#include <smd/typeclass/test/test_support.hpp>

#include <catch2/catch_test_macros.hpp>

#include <vector>

namespace {

// a7c3e1f5-8d4b-4a2c-9e7f-3b5d1c8a4e06
template <class STRUCTURE,
          const auto &FOLDABLE = smd::foldable_typeclass<STRUCTURE>>
auto sum_with_nttp_lookup(const STRUCTURE &structure) {
    return FOLDABLE.fold_map([](int x) { return x; }, structure);
}
// a7c3e1f5-8d4b-4a2c-9e7f-3b5d1c8a4e06 end

template <class STRUCTURE,
          const auto &FOLDABLE = smd::foldable_typeclass<STRUCTURE>>
auto fold_left_with_nttp_lookup(const STRUCTURE &structure) {
    return FOLDABLE.fold_left(structure, 0,
                              [](int acc, int x) { return acc * 10 + x; });
}

template <class STRUCTURE,
          const auto &FOLDABLE = smd::foldable_typeclass<STRUCTURE>>
auto fold_right_with_nttp_lookup(const STRUCTURE &structure) {
    return FOLDABLE.fold_right(structure, 0,
                               [](int x, int acc) { return x * 10 + acc; });
}

} // namespace

TEST_CASE("FoldableTypeclassTest - LengthOnSequence") {
    using Sequence = smd::typeclass::test::Sequence<int>;
    auto sequence = Sequence{{1, 2, 3}};

    const auto &foldable = smd::foldable_typeclass<Sequence>;
    CHECK(foldable.length(sequence) == 3U);
}

TEST_CASE("FoldableTypeclassTest - FoldMapSumOnSequence") {
    using Sequence = smd::typeclass::test::Sequence<int>;
    auto sequence = Sequence{{1, 2, 3}};

    const auto &foldable = smd::foldable_typeclass<Sequence>;
    const auto sum = foldable.fold_map([](int x) { return x; }, sequence);
    CHECK(sum == 6);
}

TEST_CASE("FoldableTypeclassTest - FoldMapWithExplicitObject") {
    using Sequence = smd::typeclass::test::Sequence<int>;
    auto sequence = Sequence{{1, 2, 3}};

    const auto &foldable = smd::foldable_typeclass<Sequence>;
    const auto sum = foldable.fold_map([](int x) { return x; }, sequence);
    CHECK(sum == 6);
}

TEST_CASE("FoldableTypeclassTest - FoldMapWithNttpLookup") {
    using Sequence = smd::typeclass::test::Sequence<int>;
    auto sequence = Sequence{{1, 2, 3}};

    CHECK(sum_with_nttp_lookup(sequence) == 6);
}

TEST_CASE("FoldableTypeclassTest - FoldLeftAndRight") {
    using Sequence = smd::typeclass::test::Sequence<int>;
    auto sequence = Sequence{{1, 2, 3}};
    const auto &foldable = smd::foldable_typeclass<Sequence>;

    const auto left = foldable.fold_left(
        sequence, 0, [](int acc, int x) { return acc * 10 + x; });
    const auto right = foldable.fold_right(
        sequence, 0, [](int x, int acc) { return x * 10 + acc; });

    CHECK(left == 123);
    CHECK(right == 60);
}

TEST_CASE("FoldableTypeclassTest - FoldLeftRightWithExplicitObject") {
    using Sequence = smd::typeclass::test::Sequence<int>;
    auto sequence = Sequence{{1, 2, 3}};

    const auto &foldable = smd::foldable_typeclass<Sequence>;
    const auto left = foldable.fold_left(
        sequence, 0, [](int acc, int x) { return acc * 10 + x; });
    const auto right = foldable.fold_right(
        sequence, 0, [](int x, int acc) { return x * 10 + acc; });

    CHECK(left == 123);
    CHECK(right == 60);
}

TEST_CASE("FoldableTypeclassTest - FoldLeftRightWithNttpLookup") {
    using Sequence = smd::typeclass::test::Sequence<int>;
    auto sequence = Sequence{{1, 2, 3}};

    CHECK(fold_left_with_nttp_lookup(sequence) == 123);
    CHECK(fold_right_with_nttp_lookup(sequence) == 60);
}

TEST_CASE("FoldableTypeclassTest - PredicatesAndFind") {
    using Sequence = smd::typeclass::test::Sequence<int>;
    auto sequence = Sequence{{1, 2, 3}};
    const auto &foldable = smd::foldable_typeclass<Sequence>;

    CHECK(foldable.any_of(sequence, [](int x) { return x == 2; }));
    CHECK(foldable.all_of(sequence, [](int x) { return x > 0; }));
    CHECK_FALSE(foldable.empty(sequence));

    auto found = foldable.find_first(sequence, [](int x) { return x > 1; });
    REQUIRE(found.has_value());
    CHECK(*found == 2);
}

TEST_CASE("FoldableTypeclassTest - ToVectorAndCombineAll") {
    // 4c8a5f77-8a62-4f1b-a9cf-95452c4b8ea4
    // a3d5c9e1-6b2f-4a4d-c8e3-5b1d3a7f2c46
    using IntSequence = smd::typeclass::test::Sequence<int>;
    auto sequence = IntSequence{{1, 2, 3}};
    const auto &int_foldable = smd::foldable_typeclass<IntSequence>;

    const auto as_vector = int_foldable.to_vector(sequence);
    CHECK(as_vector == (std::vector<int>{1, 2, 3}));
    // a3d5c9e1-6b2f-4a4d-c8e3-5b1d3a7f2c46 end

    using VectorSequence = smd::typeclass::test::Sequence<std::vector<int>>;
    auto vectors = VectorSequence{{{1, 2}, {3}}};
    const auto &vector_foldable = smd::foldable_typeclass<VectorSequence>;
    const auto combined = vector_foldable.combine_all(vectors);
    CHECK(combined == (std::vector<int>{1, 2, 3}));

    const auto folded = vector_foldable.fold(vectors);
    CHECK(folded == (std::vector<int>{1, 2, 3}));
    // 4c8a5f77-8a62-4f1b-a9cf-95452c4b8ea4 end
}

TEST_CASE("FoldableTypeclassTest - AllOfAndFindFirstEdgeCases") {
    using Sequence = smd::typeclass::test::Sequence<int>;
    const auto &foldable = smd::foldable_typeclass<Sequence>;

    auto mixed = Sequence{{2, -1, 4}};
    CHECK_FALSE(foldable.all_of(mixed, [](int x) { return x > 0; }));

    auto found_even =
        foldable.find_first(mixed, [](int x) { return x % 2 == 0; });
    REQUIRE(found_even.has_value());
    CHECK(*found_even == 2);

    auto found_large =
        foldable.find_first(mixed, [](int x) { return x > 100; });
    CHECK_FALSE(found_large.has_value());
}

// -- Alternate core: fold_right as primitive --

namespace {

// A Foldable instance where fold_right is the primitive, not fold_map.
// Demonstrates the alternate-core pattern: same Sequence type, different core.
template <class VALUE_TYPE>
struct RightFoldSequenceImpl {
    using element_type = VALUE_TYPE;

    template <class STATE, class F>
    auto fold_right(this auto &&,
                    const smd::typeclass::test::Sequence<VALUE_TYPE> &seq,
                    STATE initial, F &&step) {
        auto acc = std::move(initial);
        for (auto it = seq.values.rbegin(); it != seq.values.rend(); ++it) {
            acc = std::invoke(step, *it, std::move(acc));
        }
        return acc;
    }
};

// Alternate-core: using Impl::fold_right selects fold_right as primitive.
// The base Foldable<Impl> derives fold_map from fold_right automatically.
template <class VALUE_TYPE>
struct RightFoldSequenceMap : smd::Foldable<RightFoldSequenceImpl<VALUE_TYPE>> {
    using RightFoldSequenceImpl<VALUE_TYPE>::fold_right;
};

template <class VALUE_TYPE>
constexpr auto right_fold_sequence_map = RightFoldSequenceMap<VALUE_TYPE>{};

} // namespace

TEST_CASE("FoldableAlternateCore - FoldRightPrimitiveDerivesFoldMap") {
    using Sequence = smd::typeclass::test::Sequence<int>;
    auto seq = Sequence{{1, 2, 3}};
    const auto &foldable = right_fold_sequence_map<int>;

    const auto sum = foldable.fold_map([](int x) { return x; }, seq);
    CHECK(sum == 6);
}

TEST_CASE("FoldableAlternateCore - FoldRightPrimitiveDerivesLength") {
    using Sequence = smd::typeclass::test::Sequence<int>;
    auto seq = Sequence{{1, 2, 3}};
    const auto &foldable = right_fold_sequence_map<int>;

    CHECK(foldable.length(seq) == 3U);
}

TEST_CASE("FoldableAlternateCore - FoldRightPrimitiveDerivesToVector") {
    using Sequence = smd::typeclass::test::Sequence<int>;
    auto seq = Sequence{{1, 2, 3}};
    const auto &foldable = right_fold_sequence_map<int>;

    CHECK(foldable.to_vector(seq) == (std::vector<int>{1, 2, 3}));
}

TEST_CASE("FoldableAlternateCore - FoldRightPrimitiveDerivesFoldLeft") {
    using Sequence = smd::typeclass::test::Sequence<int>;
    auto seq = Sequence{{1, 2, 3}};
    const auto &foldable = right_fold_sequence_map<int>;

    const auto result =
        foldable.fold_left(seq, 0, [](int acc, int x) { return acc * 10 + x; });
    CHECK(result == 123);
}

TEST_CASE("FoldableAlternateCore - FoldRightPrimitiveDerivesPredicates") {
    using Sequence = smd::typeclass::test::Sequence<int>;
    auto seq = Sequence{{1, 2, 3}};
    const auto &foldable = right_fold_sequence_map<int>;

    CHECK(foldable.any_of(seq, [](int x) { return x == 2; }));
    CHECK(foldable.all_of(seq, [](int x) { return x > 0; }));
    CHECK_FALSE(foldable.empty(seq));
}

TEST_CASE("FoldableAlternateCore - MatchesFoldMapPrimitiveResults") {
    using Sequence = smd::typeclass::test::Sequence<int>;
    auto seq = Sequence{{4, 1, 7, 2}};

    const auto &fold_map_foldable = smd::foldable_typeclass<Sequence>;
    const auto &fold_right_foldable = right_fold_sequence_map<int>;

    CHECK(fold_map_foldable.length(seq) == fold_right_foldable.length(seq));
    CHECK(fold_map_foldable.to_vector(seq) ==
          fold_right_foldable.to_vector(seq));
    CHECK(fold_map_foldable.fold_map([](int x) { return x; }, seq) ==
          fold_right_foldable.fold_map([](int x) { return x; }, seq));
}
