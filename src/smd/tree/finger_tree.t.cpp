#include <smd/tree/finger_tree.hpp>
#include <smd/tree/finger_tree_foldable.hpp>
#include <smd/tree/memoized_thunk.hpp>
#include <smd/tree/finger_tree_traversable.hpp>

#include <catch2/catch_test_macros.hpp>

#include <atomic>
#include <cstddef>
#include <optional>
#include <vector>

namespace {

auto ceil_log2(std::size_t n) -> std::size_t
{
    if (n <= 1U) {
        return 0U;
    }

    std::size_t value = 1U;
    std::size_t bits = 0U;
    while (value < n) {
        value <<= 1U;
        ++bits;
    }
    return bits;
}

struct Weighted {
    std::size_t d_total;

    friend bool operator==(const Weighted&, const Weighted&) = default;
    friend bool operator>=(const Weighted& lhs, const Weighted& rhs)
    {
        return lhs.d_total >= rhs.d_total;
    }
};

struct WeightedMeasure {
    auto operator()(int value) const -> Weighted
    {
        return Weighted{static_cast<std::size_t>(value * 10)};
    }
};

}  // namespace

namespace smd::typeclass {

template <>
struct Monoid<Weighted> {
    constexpr auto identity() const -> Weighted { return Weighted{0U}; }

    constexpr auto combine(const Weighted& lhs, const Weighted& rhs) const
        -> Weighted
    {
        return Weighted{lhs.d_total + rhs.d_total};
    }
};

}  // namespace smd::typeclass

TEST_CASE("FingerTreeTest - EmptyLeafAndPredicates")
{
    using Tree = smd::tree::FingerTree<int>;

    auto empty = Tree::empty();
    CHECK(empty.is_empty());
    CHECK_FALSE(empty.is_leaf());
    CHECK_FALSE(empty.is_branch());
    CHECK(empty.measure() == 0U);
    CHECK(empty.breadth() == 0U);
    CHECK(empty.depth() == 0U);
    CHECK(empty.flatten() == (std::vector<int>{}));
    CHECK_FALSE(empty.view_l().has_value());
    CHECK_FALSE(empty.view_r().has_value());

    auto single = Tree::leaf(42);
    CHECK_FALSE(single.is_empty());
    CHECK(single.is_leaf());
    CHECK_FALSE(single.is_branch());
    CHECK(single.measure() == 1U);
    CHECK(single.value() == 42);
    CHECK(single.flatten() == (std::vector<int>{42}));
}

TEST_CASE("FingerTreeStrictnessTest - MemoizedThunkForcesOnce")
{
    std::atomic<int> evaluations{0};
    auto delayed = smd::tree::detail::thunk([
        &evaluations]() {
        evaluations.fetch_add(1, std::memory_order_relaxed);
        return 42;
    });

    CHECK(delayed() == 42);
    CHECK(delayed() == 42);
    CHECK(evaluations.load(std::memory_order_relaxed) == 1);
}

TEST_CASE("FingerTreeStrictnessTest - MemoizedThunkSharesAcrossCopies")
{
    std::atomic<int> evaluations{0};
    auto delayed = smd::tree::detail::thunk([
        &evaluations]() {
        evaluations.fetch_add(1, std::memory_order_relaxed);
        return 7;
    });
    auto alias = delayed;

    CHECK(delayed() == 7);
    CHECK(alias() == 7);
    CHECK(evaluations.load(std::memory_order_relaxed) == 1);
}

TEST_CASE("FingerTreeStrictnessTest - MeasuredThunkExposesCachedMeasureWithoutForce")
{
    std::atomic<int> evaluations{0};
    auto delayed = smd::tree::detail::measured_thunk(
        std::size_t{99},
        [&evaluations]() {
            evaluations.fetch_add(1, std::memory_order_relaxed);
            return 123;
        });

    CHECK(delayed.cached_measure() == 99U);
    CHECK(evaluations.load(std::memory_order_relaxed) == 0);
    CHECK(delayed.force() == 123);
    CHECK(delayed.cached_measure() == 99U);
    CHECK(evaluations.load(std::memory_order_relaxed) == 1);
}

TEST_CASE("FingerTreeTest - FromSequenceConsSnocAndMemberAppend")
{
    using Tree = smd::tree::FingerTree<int>;

    auto from = Tree::from_sequence({1, 2, 3});
    CHECK(from.flatten() == (std::vector<int>{1, 2, 3}));

    auto with_cons = from.cons(0);
    CHECK(with_cons.flatten() == (std::vector<int>{0, 1, 2, 3}));

    auto with_snoc = with_cons.snoc(4);
    CHECK(with_snoc.flatten() == (std::vector<int>{0, 1, 2, 3, 4}));

    auto appended_member = from.append(Tree::from_sequence({4, 5}));
    CHECK(appended_member.flatten() == (std::vector<int>{1, 2, 3, 4, 5}));
}

TEST_CASE("FingerTreeTest - SingletonViewsAndEmptyTailInit")
{
    using Tree = smd::tree::FingerTree<int>;

    auto single = Tree::leaf(7);
    auto left = single.view_l();
    REQUIRE(left.has_value());
    CHECK(left->d_value == 7);
    CHECK(left->d_rest.is_empty());

    auto right = single.view_r();
    REQUIRE(right.has_value());
    CHECK(right->d_value == 7);
    CHECK(right->d_rest.is_empty());

    auto empty = Tree::empty();
    CHECK(empty.tail().is_empty());
    CHECK(empty.init().is_empty());
}

TEST_CASE("FingerTreeTest - BasicMeasureDepthFlatten")
{
    using Tree = smd::tree::FingerTree<int>;

    auto tree = Tree::branch(
        Tree::branch(Tree::leaf(1), Tree::leaf(2)),
        Tree::leaf(3));

    CHECK(tree.measure() == 3U);
    CHECK(tree.breadth() == 3U);
    CHECK(tree.depth() >= 1U);
    CHECK(tree.flatten() == (std::vector<int>{1, 2, 3}));
}

TEST_CASE("FingerTreeTest - ViewsAndListOps")
{
    using Tree = smd::tree::FingerTree<int>;

    auto tree = Tree::branch(
        Tree::branch(Tree::leaf(1), Tree::leaf(2)),
        Tree::leaf(3));

    auto left_view = tree.view_l();
    REQUIRE(left_view.has_value());
    CHECK(left_view->d_value == 1);
    CHECK(left_view->d_rest.flatten() == (std::vector<int>{2, 3}));

    auto right_view = tree.view_r();
    REQUIRE(right_view.has_value());
    CHECK(right_view->d_value == 3);
    CHECK(right_view->d_rest.flatten() == (std::vector<int>{1, 2}));

    CHECK(tree.head() == 1);
    CHECK(tree.last() == 3);
    CHECK(tree.tail().flatten() == (std::vector<int>{2, 3}));
    CHECK(tree.init().flatten() == (std::vector<int>{1, 2}));
}

TEST_CASE("FingerTreeTest - PrependAppendConcat")
{
    using Tree = smd::tree::FingerTree<int>;

    auto tree = Tree::branch(Tree::leaf(1), Tree::leaf(2));

    auto prepended = Tree::prepend(0, tree);
    CHECK(prepended.flatten() == (std::vector<int>{0, 1, 2}));

    auto appended = Tree::append(tree, 3);
    CHECK(appended.flatten() == (std::vector<int>{1, 2, 3}));

    auto concatenated = Tree::concat(tree, tree);
    CHECK(concatenated.flatten() == (std::vector<int>{1, 2, 1, 2}));
}

TEST_CASE("FingerTreeTest - MonoidTaggedMeasure")
{
    using Tree = smd::tree::FingerTree<int, Weighted, WeightedMeasure>;

    auto tree = Tree::from_sequence({1, 2, 3});
    CHECK(tree.measure() == Weighted{60U});

    auto prepended = Tree::prepend(4, tree);
    CHECK(prepended.measure() == Weighted{100U});

    auto concatenated = Tree::concat(tree, Tree::leaf(5));
    CHECK(concatenated.measure() == Weighted{110U});
}

TEST_CASE("FingerTreeTest - MeasureGuidedSearchAndSplit")
{
    using Tree = smd::tree::FingerTree<int>;

    auto tree = Tree::from_sequence({1, 2, 3, 4, 5});

    auto found = tree.search([](std::size_t prefix) { return prefix >= 3U; });
    REQUIRE(found.has_value());
    CHECK(*found == 3);

    auto split = tree.split([](std::size_t prefix) { return prefix >= 3U; });
    REQUIRE(split.has_value());
    CHECK(split->d_left.flatten() == (std::vector<int>{1, 2}));
    CHECK(split->d_pivot == 3);
    CHECK(split->d_right.flatten() == (std::vector<int>{4, 5}));

    CHECK_FALSE(tree.search([](std::size_t prefix) { return prefix >= 6U; }).has_value());
    CHECK_FALSE(tree.split([](std::size_t prefix) { return prefix >= 6U; }).has_value());
}

TEST_CASE("FingerTreeTest - MeasureGuidedSearchAndSplitWithCustomTag")
{
    using Tree = smd::tree::FingerTree<int, Weighted, WeightedMeasure>;

    auto tree = Tree::from_sequence({1, 2, 3, 4});

    auto found = tree.search([](Weighted prefix) { return prefix.d_total >= 35U; });
    REQUIRE(found.has_value());
    CHECK(*found == 3);

    auto split = tree.split([](Weighted prefix) { return prefix.d_total >= 35U; });
    REQUIRE(split.has_value());
    CHECK(split->d_left.flatten() == (std::vector<int>{1, 2}));
    CHECK(split->d_left.measure() == Weighted{30U});
    CHECK(split->d_pivot == 3);
    CHECK(split->d_right.flatten() == (std::vector<int>{4}));
    CHECK(split->d_right.measure() == Weighted{40U});
}

TEST_CASE("FingerTreeTest - SplitAtCountBoundary")
{
    using Tree = smd::tree::FingerTree<int>;

    auto tree = Tree::from_sequence({1, 2, 3, 4, 5});

    auto at_three = tree.split_at([](std::size_t prefix) { return prefix >= 3U; });
    CHECK(at_three.d_left.flatten() == (std::vector<int>{1, 2}));
    CHECK(at_three.d_right.flatten() == (std::vector<int>{3, 4, 5}));

    auto at_one = tree.split_at([](std::size_t prefix) { return prefix >= 1U; });
    CHECK(at_one.d_left.is_empty());
    CHECK(at_one.d_right.flatten() == (std::vector<int>{1, 2, 3, 4, 5}));

    auto none = tree.split_at([](std::size_t prefix) { return prefix >= 6U; });
    CHECK(none.d_left.flatten() == (std::vector<int>{1, 2, 3, 4, 5}));
    CHECK(none.d_right.is_empty());
}

TEST_CASE("FingerTreeTest - SplitAtWeightedBoundary")
{
    using Tree = smd::tree::FingerTree<int, Weighted, WeightedMeasure>;

    auto tree = Tree::from_sequence({1, 2, 3, 4});

    auto split = tree.split_at([](Weighted prefix) { return prefix.d_total >= 35U; });
    CHECK(split.d_left.flatten() == (std::vector<int>{1, 2}));
    CHECK(split.d_left.measure() == Weighted{30U});
    CHECK(split.d_right.flatten() == (std::vector<int>{3, 4}));
    CHECK(split.d_right.measure() == Weighted{70U});
}

TEST_CASE("FingerTreeTest - SplitAtIndexConvenience")
{
    using Tree = smd::tree::FingerTree<int>;

    auto tree = Tree::from_sequence({1, 2, 3, 4, 5});

    auto at_zero = tree.split_at_index(0U);
    CHECK(at_zero.d_left.is_empty());
    CHECK(at_zero.d_right.flatten() == (std::vector<int>{1, 2, 3, 4, 5}));

    auto at_three = tree.split_at_index(3U);
    CHECK(at_three.d_left.flatten() == (std::vector<int>{1, 2, 3}));
    CHECK(at_three.d_right.flatten() == (std::vector<int>{4, 5}));

    auto beyond = tree.split_at_index(99U);
    CHECK(beyond.d_left.flatten() == (std::vector<int>{1, 2, 3, 4, 5}));
    CHECK(beyond.d_right.is_empty());
}

TEST_CASE("FingerTreeTest - SplitAtMeasureConvenience")
{
    using CountTree = smd::tree::FingerTree<int>;

    auto count_tree = CountTree::from_sequence({1, 2, 3, 4, 5});
    auto count_split = count_tree.split_at_measure(3U);
    CHECK(count_split.d_left.flatten() == (std::vector<int>{1, 2}));
    CHECK(count_split.d_right.flatten() == (std::vector<int>{3, 4, 5}));

    using WeightedTree = smd::tree::FingerTree<int, Weighted, WeightedMeasure>;

    auto weighted_tree = WeightedTree::from_sequence({1, 2, 3, 4});
    auto weighted_split = weighted_tree.split_at_measure(Weighted{35U});
    CHECK(weighted_split.d_left.flatten() == (std::vector<int>{1, 2}));
    CHECK(weighted_split.d_right.flatten() == (std::vector<int>{3, 4}));
}

TEST_CASE("FingerTreePersistenceTest - SharedVersionsSurviveAppendAndPops")
{
    using Tree = smd::tree::FingerTree<int>;

    auto base = Tree::from_sequence({1, 2, 3, 4});
    auto appended = base.append(Tree::from_sequence({5, 6}));
    auto left_popped = appended.tail();
    auto right_popped = appended.init();

    CHECK(base.flatten() == (std::vector<int>{1, 2, 3, 4}));
    CHECK(base.head() == 1);
    CHECK(base.last() == 4);

    CHECK(appended.flatten() == (std::vector<int>{1, 2, 3, 4, 5, 6}));
    CHECK(left_popped.flatten() == (std::vector<int>{2, 3, 4, 5, 6}));
    CHECK(right_popped.flatten() == (std::vector<int>{1, 2, 3, 4, 5}));

    CHECK(base.flatten() == (std::vector<int>{1, 2, 3, 4}));
}

TEST_CASE("FingerTreePersistenceTest - SharedVersionsSurviveSearchAndSplit")
{
    using Tree = smd::tree::FingerTree<int>;

    auto base = Tree::from_sequence({1, 2, 3, 4, 5, 6});
    auto appended = base.append(Tree::from_sequence({7, 8, 9}));
    auto split = appended.split([](std::size_t prefix) { return prefix >= 7U; });
    REQUIRE(split.has_value());

    auto count_split = appended.split_at_index(4U);
    auto found = appended.search([](std::size_t prefix) { return prefix >= 8U; });

    REQUIRE(found.has_value());
    CHECK(*found == 8);
    CHECK(split->d_left.flatten() == (std::vector<int>{1, 2, 3, 4, 5, 6}));
    CHECK(split->d_pivot == 7);
    CHECK(split->d_right.flatten() == (std::vector<int>{8, 9}));
    CHECK(count_split.d_left.flatten() == (std::vector<int>{1, 2, 3, 4}));
    CHECK(count_split.d_right.flatten() == (std::vector<int>{5, 6, 7, 8, 9}));

    CHECK(base.flatten() == (std::vector<int>{1, 2, 3, 4, 5, 6}));
    CHECK(base.search([](std::size_t prefix) { return prefix >= 4U; }) == std::optional<int>{4});
}

TEST_CASE("FingerTreePersistenceTest - WeightedSharedVersionsKeepMeasures")
{
    using Tree = smd::tree::FingerTree<int, Weighted, WeightedMeasure>;

    auto base = Tree::from_sequence({1, 2, 3});
    auto appended = base.append(Tree::from_sequence({4, 5}));
    auto split = appended.split_at_measure(Weighted{60U});

    CHECK(base.measure() == Weighted{60U});
    CHECK(base.flatten() == (std::vector<int>{1, 2, 3}));

    CHECK(appended.measure() == Weighted{150U});
    CHECK(appended.flatten() == (std::vector<int>{1, 2, 3, 4, 5}));
    CHECK(split.d_left.measure() == Weighted{30U});
    CHECK(split.d_right.measure() == Weighted{120U});

    CHECK(base.measure() == Weighted{60U});
    CHECK(base.flatten() == (std::vector<int>{1, 2, 3}));
}

TEST_CASE("FingerTreePersistenceTest - RepeatedSplitPopAcrossSharedVersions")
{
    using Tree = smd::tree::FingerTree<int>;

    auto base = Tree::from_sequence({1, 2, 3, 4, 5, 6, 7, 8});
    auto shared = base.append(Tree::from_sequence({9, 10, 11, 12}));

    const auto base_snapshot = base.flatten();
    const auto shared_snapshot = shared.flatten();

    auto current = shared;
    for (int round = 0; round < 4; ++round) {
        const auto flat = current.flatten();
        REQUIRE(flat.size() >= 4U);

        auto split = current.split_at_index(flat.size() / 2U);
        auto left_flat = split.d_left.flatten();
        auto right_flat = split.d_right.flatten();

        REQUIRE_FALSE(left_flat.empty());
        REQUIRE_FALSE(right_flat.empty());

        auto left_tail = split.d_left.tail();
        auto right_init = split.d_right.init();
        auto recombined = left_tail.append(right_init);

        auto expected = flat;
        expected.erase(expected.begin());
        expected.pop_back();
        CHECK(recombined.flatten() == expected);

        auto rebuilt = split.d_left.append(split.d_right);
        CHECK(rebuilt.flatten() == flat);

        CHECK(base.flatten() == base_snapshot);
        CHECK(shared.flatten() == shared_snapshot);

        current = rebuilt.append(Tree::leaf(100 + round)).tail();
    }
}

TEST_CASE("FingerTreeTest - DepthRemainsLogarithmic")
{
    using Tree = smd::tree::FingerTree<int>;
    constexpr std::size_t kSize = 1024U;

    auto by_snoc = Tree::empty();
    for (std::size_t i = 0; i < kSize; ++i) {
        by_snoc = by_snoc.snoc(static_cast<int>(i));
    }

    auto bound = 2U * ceil_log2(kSize + 1U) + 1U;
    CHECK(by_snoc.depth() <= bound);

    auto by_append = Tree::empty();
    for (std::size_t i = 0; i < kSize; ++i) {
        by_append = by_append.append(Tree::leaf(static_cast<int>(i)));
    }
    CHECK(by_append.depth() <= bound);
}

TEST_CASE("FingerTreeFoldableTest - FoldMapAndDerivedOperations")
{
    using Tree = smd::tree::FingerTree<int>;

    auto tree = Tree::from_sequence({1, 2, 3, 4});
    const auto& foldable = smd::foldable_typeclass<Tree>;

    CHECK(foldable.length(tree) == 4U);
    CHECK(foldable.fold_map([](int x) { return x; }, tree) == 10);
    CHECK(foldable.to_vector(tree) == (std::vector<int>{1, 2, 3, 4}));

    const auto left = foldable.fold_left(tree, 0, [](int acc, int x) {
        return acc * 10 + x;
    });
    CHECK(left == 1234);
}

TEST_CASE("FingerTreeTraversableTest - TraverseOptionalSuccess")
{
    using Tree = smd::tree::FingerTree<int>;

    auto tree = Tree::from_sequence({1, 2, 3});
    const auto& traversable = smd::traversable_typeclass<Tree>;

    auto traversed = smd::traverse(
        [](int x) -> std::optional<int> {
            return x > 0 ? std::optional<int>{x * 10} : std::optional<int>{};
        },
        tree);

    REQUIRE(traversed.has_value());
    CHECK(traversed->flatten() == (std::vector<int>{10, 20, 30}));
}

TEST_CASE("FingerTreeTraversableTest - TraverseOptionalFailure")
{
    using Tree = smd::tree::FingerTree<int>;

    auto tree = Tree::from_sequence({1, -2, 3});
    const auto& traversable = smd::traversable_typeclass<Tree>;

    auto traversed = smd::traverse(
        [](int x) -> std::optional<int> {
            return x > 0 ? std::optional<int>{x * 10} : std::optional<int>{};
        },
        tree);

    CHECK_FALSE(traversed.has_value());
}
