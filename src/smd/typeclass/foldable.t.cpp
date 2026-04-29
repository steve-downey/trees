#include <smd/typeclass/foldable.hpp>
#include <smd/typeclass/test/test_support.hpp>

#include <gtest/gtest.h>

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

TEST(FoldableTypeclassTest, LengthOnSequence)
{
    using Sequence = smd::typeclass::test::Sequence<int>;
    auto sequence = Sequence{{1, 2, 3}};

    const auto& foldable = smd::foldable_typeclass<Sequence>;
    EXPECT_EQ(foldable.length(sequence), 3U);
}

TEST(FoldableTypeclassTest, FoldMapSumOnSequence)
{
    using Sequence = smd::typeclass::test::Sequence<int>;
    auto sequence = Sequence{{1, 2, 3}};

    const auto& foldable = smd::foldable_typeclass<Sequence>;
    const auto sum = foldable.fold_map([](int x) { return x; }, sequence);
    EXPECT_EQ(sum, 6);
}

TEST(FoldableTypeclassTest, FoldMapWithExplicitObject)
{
    using Sequence = smd::typeclass::test::Sequence<int>;
    auto sequence = Sequence{{1, 2, 3}};

    const auto& foldable = smd::foldable_typeclass<Sequence>;
    const auto sum = foldable.fold_map([](int x) { return x; }, sequence);
    EXPECT_EQ(sum, 6);
}

TEST(FoldableTypeclassTest, FoldMapWithNttpLookup)
{
    using Sequence = smd::typeclass::test::Sequence<int>;
    auto sequence = Sequence{{1, 2, 3}};

    EXPECT_EQ(sum_with_nttp_lookup(sequence), 6);
}

TEST(FoldableTypeclassTest, FoldLeftAndRight)
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

    EXPECT_EQ(left, 123);
    EXPECT_EQ(right, 60);
}

TEST(FoldableTypeclassTest, FoldLeftRightWithExplicitObject)
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

    EXPECT_EQ(left, 123);
    EXPECT_EQ(right, 60);
}

TEST(FoldableTypeclassTest, FoldLeftRightWithNttpLookup)
{
    using Sequence = smd::typeclass::test::Sequence<int>;
    auto sequence = Sequence{{1, 2, 3}};

    EXPECT_EQ(fold_left_with_nttp_lookup(sequence), 123);
    EXPECT_EQ(fold_right_with_nttp_lookup(sequence), 60);
}

TEST(FoldableTypeclassTest, PredicatesAndFind)
{
    using Sequence = smd::typeclass::test::Sequence<int>;
    auto sequence = Sequence{{1, 2, 3}};
    const auto& foldable = smd::foldable_typeclass<Sequence>;

    EXPECT_TRUE(foldable.any_of(sequence, [](int x) { return x == 2; }));
    EXPECT_TRUE(foldable.all_of(sequence, [](int x) { return x > 0; }));
    EXPECT_FALSE(foldable.empty(sequence));

    auto found = foldable.find_first(sequence, [](int x) { return x > 1; });
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(*found, 2);
}

TEST(FoldableTypeclassTest, ToVectorAndCombineAll)
{
    // 4c8a5f77-8a62-4f1b-a9cf-95452c4b8ea4
    using IntSequence = smd::typeclass::test::Sequence<int>;
    auto sequence = IntSequence{{1, 2, 3}};
    const auto& int_foldable = smd::foldable_typeclass<IntSequence>;

    const auto as_vector = int_foldable.to_vector(sequence);
    EXPECT_EQ(as_vector, (std::vector<int>{1, 2, 3}));

    using VectorSequence = smd::typeclass::test::Sequence<std::vector<int> >;
    auto vectors = VectorSequence{{{1, 2}, {3}}};
    const auto& vector_foldable = smd::foldable_typeclass<VectorSequence>;
    const auto combined = vector_foldable.combine_all(vectors);
    EXPECT_EQ(combined, (std::vector<int>{1, 2, 3}));

    const auto folded = vector_foldable.fold(vectors);
    EXPECT_EQ(folded, (std::vector<int>{1, 2, 3}));
    // 4c8a5f77-8a62-4f1b-a9cf-95452c4b8ea4 end
}
