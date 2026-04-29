#include <smd/typeclass/foldable.hpp>
#include <smd/typeclass/test/test_support.hpp>

#include <catch2/catch_test_macros.hpp>

#include <vector>

namespace {

template <class STRUCTURE,
          const auto& FOLDABLE = smd::foldable_typeclass<STRUCTURE> >
auto sum_with_nttp_lookup(const STRUCTURE& structure)
{
    return FOLDABLE.fold_map([](int x) { return x; }, structure);
}

template <class STRUCTURE,
          const auto& FOLDABLE = smd::foldable_typeclass<STRUCTURE> >
auto fold_left_with_nttp_lookup(const STRUCTURE& structure)
{
    return FOLDABLE.fold_left(structure, 0, [](int acc, int x) {
        return acc * 10 + x;
    });
}

template <class STRUCTURE,
          const auto& FOLDABLE = smd::foldable_typeclass<STRUCTURE> >
auto fold_right_with_nttp_lookup(const STRUCTURE& structure)
{
    return FOLDABLE.fold_right(structure, 0, [](int x, int acc) {
        return x * 10 + acc;
    });
}

}  // namespace

TEST_CASE("FoldableTypeclassTest - LengthOnSequence")
{
    using Sequence = smd::typeclass::test::Sequence<int>;
    auto sequence = Sequence{{1, 2, 3}};

    const auto& foldable = smd::foldable_typeclass<Sequence>;
    CHECK(foldable.length(sequence) == 3U);
}

TEST_CASE("FoldableTypeclassTest - FoldMapSumOnSequence")
{
    using Sequence = smd::typeclass::test::Sequence<int>;
    auto sequence = Sequence{{1, 2, 3}};

    const auto& foldable = smd::foldable_typeclass<Sequence>;
    const auto sum = foldable.fold_map([](int x) { return x; }, sequence);
    CHECK(sum == 6);
}

TEST_CASE("FoldableTypeclassTest - FoldMapWithExplicitObject")
{
    using Sequence = smd::typeclass::test::Sequence<int>;
    auto sequence = Sequence{{1, 2, 3}};

    const auto& foldable = smd::foldable_typeclass<Sequence>;
    const auto sum = foldable.fold_map([](int x) { return x; }, sequence);
    CHECK(sum == 6);
}

TEST_CASE("FoldableTypeclassTest - FoldMapWithNttpLookup")
{
    using Sequence = smd::typeclass::test::Sequence<int>;
    auto sequence = Sequence{{1, 2, 3}};

    CHECK(sum_with_nttp_lookup(sequence) == 6);
}

TEST_CASE("FoldableTypeclassTest - FoldLeftAndRight")
{
    using Sequence = smd::typeclass::test::Sequence<int>;
    auto sequence = Sequence{{1, 2, 3}};
    const auto& foldable = smd::foldable_typeclass<Sequence>;

    const auto left = foldable.fold_left(sequence, 0, [](int acc, int x) {
        return acc * 10 + x;
    });
    const auto right = foldable.fold_right(sequence, 0, [](int x, int acc) {
        return x * 10 + acc;
    });

    CHECK(left == 123);
    CHECK(right == 60);
}

TEST_CASE("FoldableTypeclassTest - FoldLeftRightWithExplicitObject")
{
    using Sequence = smd::typeclass::test::Sequence<int>;
    auto sequence = Sequence{{1, 2, 3}};

    const auto& foldable = smd::foldable_typeclass<Sequence>;
    const auto left = foldable.fold_left(sequence, 0, [](int acc, int x) {
        return acc * 10 + x;
    });
    const auto right = foldable.fold_right(sequence, 0, [](int x, int acc) {
        return x * 10 + acc;
    });

    CHECK(left == 123);
    CHECK(right == 60);
}

TEST_CASE("FoldableTypeclassTest - FoldLeftRightWithNttpLookup")
{
    using Sequence = smd::typeclass::test::Sequence<int>;
    auto sequence = Sequence{{1, 2, 3}};

    CHECK(fold_left_with_nttp_lookup(sequence) == 123);
    CHECK(fold_right_with_nttp_lookup(sequence) == 60);
}

TEST_CASE("FoldableTypeclassTest - PredicatesAndFind")
{
    using Sequence = smd::typeclass::test::Sequence<int>;
    auto sequence = Sequence{{1, 2, 3}};
    const auto& foldable = smd::foldable_typeclass<Sequence>;

    CHECK(foldable.any_of(sequence, [](int x) { return x == 2; }));
    CHECK(foldable.all_of(sequence, [](int x) { return x > 0; }));
    CHECK_FALSE(foldable.empty(sequence));

    auto found = foldable.find_first(sequence, [](int x) { return x > 1; });
    REQUIRE(found.has_value());
    CHECK(*found == 2);
}

TEST_CASE("FoldableTypeclassTest - ToVectorAndCombineAll")
{
    // 4c8a5f77-8a62-4f1b-a9cf-95452c4b8ea4
    using IntSequence = smd::typeclass::test::Sequence<int>;
    auto sequence = IntSequence{{1, 2, 3}};
    const auto& int_foldable = smd::foldable_typeclass<IntSequence>;

    const auto as_vector = int_foldable.to_vector(sequence);
    CHECK(as_vector == (std::vector<int>{1, 2, 3}));

    using VectorSequence = smd::typeclass::test::Sequence<std::vector<int> >;
    auto vectors = VectorSequence{{{1, 2}, {3}}};
    const auto& vector_foldable = smd::foldable_typeclass<VectorSequence>;
    const auto combined = vector_foldable.combine_all(vectors);
    CHECK(combined == (std::vector<int>{1, 2, 3}));

    const auto folded = vector_foldable.fold(vectors);
    CHECK(folded == (std::vector<int>{1, 2, 3}));
    // 4c8a5f77-8a62-4f1b-a9cf-95452c4b8ea4 end
}
