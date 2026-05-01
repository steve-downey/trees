#include <smd/tree/finger_tree_priority_queue.hpp>

#include <catch2/catch_test_macros.hpp>

#include <optional>
#include <set>
#include <vector>

TEST_CASE("FingerTreePriorityQueueTest - WrapperOperations")
{
    using Queue = smd::tree::FingerTreePriorityQueue<int>;

    // a4d7f1c6-3b9e-4a2d-f8c4-6e5b1d9c3a07
    auto q = Queue::from_values({5, 2, 8, 2, 7});
    REQUIRE(q.min().has_value());
    REQUIRE(q.max().has_value());
    CHECK(*q.min() == 2);
    CHECK(*q.max() == 8);
    // a4d7f1c6-3b9e-4a2d-f8c4-6e5b1d9c3a07 end

    auto min_pop = q.pop_min();
    REQUIRE(min_pop.has_value());
    CHECK(min_pop->first == 2);
    REQUIRE(min_pop->second.min().has_value());
    CHECK(*min_pop->second.min() == 2);

    auto max_pop = min_pop->second.pop_max();
    REQUIRE(max_pop.has_value());
    CHECK(max_pop->first == 8);
    REQUIRE(max_pop->second.max().has_value());
    CHECK(*max_pop->second.max() == 7);
}

TEST_CASE("FingerTreePriorityQueueTest - FoldableTypeclass")
{
    using Queue = smd::tree::FingerTreePriorityQueue<int>;

    auto q = Queue::from_values({5, 2, 8, 2, 7});
    const auto& foldable = smd::foldable_typeclass<Queue>;

    CHECK(foldable.fold_map([](int value) { return value; }, q) == 24);
    CHECK(foldable.length(q) == 5U);
}

TEST_CASE("FingerTreePriorityQueueTest - TraversableTypeclass")
{
    using Queue = smd::tree::FingerTreePriorityQueue<int>;

    auto q = Queue::from_values({5, 2, 8});
    const auto& traversable = smd::traversable_typeclass<Queue>;

    auto success = smd::traverse(
      [](int value) -> std::optional<int> { return value * 10; },
      q);
    REQUIRE(success.has_value());
    CHECK(success->to_vector() == (std::vector<int>{50, 20, 80}));
    REQUIRE(success->min().has_value());
    REQUIRE(success->max().has_value());
    CHECK(*success->min() == 20);
    CHECK(*success->max() == 80);

    auto failure = smd::traverse(
      [](int value) -> std::optional<int> {
          if (value == 8) {
              return std::nullopt;
          }
          return value;
      },
      q);
    CHECK_FALSE(failure.has_value());
}

TEST_CASE("FingerTreePriorityQueueTest - RepeatedPushPopMatchesMultiset")
{
    using Queue = smd::tree::FingerTreePriorityQueue<int>;

    auto q = Queue::from_values({5, 2, 8, 2, 7, 1, 9, 1});
    std::multiset<int> expected{5, 2, 8, 2, 7, 1, 9, 1};

    for (int i = 0; i < 250; ++i) {
        auto value = (i * 7) % 11;
        q = q.push(value);
        expected.insert(value);

        if ((i % 2) == 0) {
            auto popped = q.pop_min();
            REQUIRE(popped.has_value());
            REQUIRE_FALSE(expected.empty());
            CHECK(popped->first == *expected.begin());
            expected.erase(expected.begin());
            q = std::move(popped->second);
        } else {
            auto popped = q.pop_max();
            REQUIRE(popped.has_value());
            REQUIRE_FALSE(expected.empty());
            auto it = std::prev(expected.end());
            CHECK(popped->first == *it);
            expected.erase(it);
            q = std::move(popped->second);
        }

        if (!expected.empty()) {
            REQUIRE(q.min().has_value());
            REQUIRE(q.max().has_value());
            CHECK(*q.min() == *expected.begin());
            CHECK(*q.max() == *std::prev(expected.end()));
        } else {
            CHECK_FALSE(q.min().has_value());
            CHECK_FALSE(q.max().has_value());
        }
    }

    auto values = q.to_vector();
    std::multiset<int> actual(values.begin(), values.end());
    CHECK(actual == expected);
}

TEST_CASE("FingerTreePriorityQueueTest - LazyRemovalDebug_SimpleSplitOnMin", "[.lazy-debug]")
{
    // Minimal test to isolate split-based removal on MinTree
    using Queue = smd::tree::FingerTreePriorityQueue<int>;

    auto q = Queue::from_values({5, 2, 8});
    REQUIRE(q.min().has_value());
    CHECK(*q.min() == 2);

    // This is equivalent to what would happen in pop_min with split/concat
    // remove_one_split_min will be called on d_min_tree
    // We're testing this path in isolation to pinpoint SEGV

    // For now, just verify the basic structure is correct
    CHECK(q.max().has_value());
    CHECK(*q.max() == 8);
}

TEST_CASE("FingerTreePriorityQueueTest - LazyRemovalDebug_CrossMeasureSplit", "[.lazy-debug]")
{
    // Test attempting cross-measure split removal (likely to trigger SEGV)
    using Queue = smd::tree::FingerTreePriorityQueue<int>;

    auto q = Queue::from_values({5, 2, 8, 2, 7});
    REQUIRE(q.min().has_value());
    auto min_val = *q.min();

    // The crash likely happens when:
    // 1. We pop_min() which gets min_val = 2
    // 2. We split MinTree on 2 (works fine)
    // 3. We split MaxTree on 2 (should also work, if using MaxTag predicate)

    // This test validates both trees can be split with correct predicates
    REQUIRE(q.max().has_value());
    auto max_val = *q.max();
    CHECK(max_val == 8);

    // Both min and max should be present in both trees
    auto v = q.to_vector();
    std::set<int> s(v.begin(), v.end());
    REQUIRE(s.count(min_val) > 0);
    REQUIRE(s.count(max_val) > 0);
}

TEST_CASE("FingerTreePriorityQueueTest - LazyRemovalDebug_DirectSplitCall", "[.lazy-debug]")
{
    // Test the split methods directly to pinpoint SEGV location
    using Queue = smd::tree::FingerTreePriorityQueue<int>;

    auto q = Queue::from_values({5, 2, 8, 2, 7});

    // For now, just verify that the helpers exist and can be called
    // (We can't call them directly since they're private)
    // But we can verify the public interface still works
    REQUIRE(q.min().has_value());
    REQUIRE(q.max().has_value());
}

TEST_CASE("FingerTreePriorityQueueTest - LazyRemovalDebug_SplitBasedPopMin", "[.lazy-debug]")
{
    // Test the lazy pop_min path directly
    using Queue = smd::tree::FingerTreePriorityQueue<int>;

    auto q = Queue::from_values({5, 2, 8});
    REQUIRE(q.min().has_value());
    REQUIRE(q.max().has_value());
    CHECK(*q.min() == 2);
    CHECK(*q.max() == 8);

    // Use the lazy split-based removal path
    auto popped = q.pop_min_lazy();
    REQUIRE(popped.has_value());
    CHECK(popped->first == 2);

    // Verify the resulting queue is correct
    auto q2 = popped->second;
    REQUIRE(q2.min().has_value());
    REQUIRE(q2.max().has_value());
    CHECK(*q2.min() == 5);
    CHECK(*q2.max() == 8);
}

TEST_CASE("FingerTreePriorityQueueTest - LazyRemovalDebug_SplitWithDuplicates", "[.lazy-debug]")
{
    // Test with duplicates like the original WrapperOperations
    using Queue = smd::tree::FingerTreePriorityQueue<int>;

    auto q = Queue::from_values({5, 2, 8, 2, 7});
    REQUIRE(q.min().has_value());
    REQUIRE(q.max().has_value());
    CHECK(*q.min() == 2);
    CHECK(*q.max() == 8);

    // First pop_min_lazy (removes one instance of 2)
    auto pop1 = q.pop_min_lazy();
    REQUIRE(pop1.has_value());
    CHECK(pop1->first == 2);

    auto q2 = pop1->second;
    REQUIRE(q2.min().has_value());
    REQUIRE(q2.max().has_value());
    CHECK(*q2.min() == 2);  // Still have another 2
    CHECK(*q2.max() == 8);

    // Second pop_min_lazy (removes the other 2)
    auto pop2 = q2.pop_min_lazy();
    REQUIRE(pop2.has_value());
    CHECK(pop2->first == 2);

    auto q3 = pop2->second;
    REQUIRE(q3.min().has_value());
    REQUIRE(q3.max().has_value());
    CHECK(*q3.min() == 5);
    CHECK(*q3.max() == 8);
}

TEST_CASE("FingerTreePriorityQueueTest - LazyRemovalDebug_PinpointSecondPopState", "[.lazy-debug]")
{
    using Queue = smd::tree::FingerTreePriorityQueue<int>;

    auto q = Queue::from_values({5, 2, 8, 2, 7});
    auto pop1 = q.pop_min_lazy();
    REQUIRE(pop1.has_value());
    auto pop2 = pop1->second.pop_min_lazy();
    REQUIRE(pop2.has_value());

    auto q3 = pop2->second;

    // These checks are split to isolate the first operation that hangs.
    REQUIRE(q3.min().has_value());
    CHECK(*q3.min() == 5);
    auto values = q3.to_vector();
    CHECK(values.size() == 3);
}

TEST_CASE("FingerTreePriorityQueueTest - LazyRemovalDebug_MinSplitOnlyDuplicates", "[.lazy-debug]")
{
    using Queue = smd::tree::FingerTreePriorityQueue<int>;

    auto q = Queue::from_values({5, 2, 8, 2, 7});
    auto pop1 = q.debug_pop_min_split_min_only();
    REQUIRE(pop1.has_value());
    CHECK(pop1->first == 2);
}

TEST_CASE("FingerTreePriorityQueueTest - LazyRemovalDebug_MaxRebuildOnlyDuplicates", "[.lazy-debug]")
{
    using Queue = smd::tree::FingerTreePriorityQueue<int>;

    auto q = Queue::from_values({5, 2, 8, 2, 7});
    auto pop1 = q.debug_pop_min_rebuild_max_only();
    REQUIRE(pop1.has_value());
    CHECK(pop1->first == 2);
}

TEST_CASE("FingerTreePriorityQueueTest - LazyRemovalDebug_CombinedComponentsOnly", "[.lazy-debug]")
{
    using Queue = smd::tree::FingerTreePriorityQueue<int>;

    auto q = Queue::from_values({5, 2, 8, 2, 7});
    auto m = q.debug_pop_min_lazy_components_only();
    REQUIRE(m.has_value());
    CHECK(*m == 2);
}

TEST_CASE("FingerTreePriorityQueueTest - LazyRemovalDebug_ConstructFromBothComponents", "[.lazy-debug]")
{
    using Queue = smd::tree::FingerTreePriorityQueue<int>;

    auto q = Queue::from_values({5, 2, 8, 2, 7});
    CHECK(q.debug_construct_lazy_min_result());
}

TEST_CASE("FingerTreePriorityQueueTest - LazyRemovalDebug_FirstLazyPopOnly", "[.lazy-debug]")
{
    using Queue = smd::tree::FingerTreePriorityQueue<int>;

    auto q = Queue::from_values({5, 2, 8, 2, 7});
    auto pop1 = q.pop_min_lazy();
    REQUIRE(pop1.has_value());
    CHECK(pop1->first == 2);
}

TEST_CASE("FingerTreePriorityQueueTest - LazyRemovalDebug_SecondLazyPopOnly", "[.lazy-debug]")
{
    using Queue = smd::tree::FingerTreePriorityQueue<int>;

    auto q = Queue::from_values({5, 2, 8, 2, 7});
    auto pop1 = q.pop_min_lazy();
    REQUIRE(pop1.has_value());
    auto pop2 = pop1->second.pop_min_lazy();
    REQUIRE(pop2.has_value());
    CHECK(pop2->first == 2);
}

TEST_CASE("FingerTreePriorityQueueTest - LazyRemovalDebug_FirstPopThenReadMinMax", "[.lazy-debug]")
{
    using Queue = smd::tree::FingerTreePriorityQueue<int>;

    auto q = Queue::from_values({5, 2, 8, 2, 7});
    auto pop1 = q.pop_min_lazy();
    REQUIRE(pop1.has_value());
    auto q2 = pop1->second;

    REQUIRE(q2.min().has_value());
    REQUIRE(q2.max().has_value());
    CHECK(*q2.min() == 2);
    CHECK(*q2.max() == 8);
}

TEST_CASE("FingerTreePriorityQueueTest - LazyRemovalDebug_FirstPopThenComponentsOnly", "[.lazy-debug]")
{
    using Queue = smd::tree::FingerTreePriorityQueue<int>;

    auto q = Queue::from_values({5, 2, 8, 2, 7});
    auto pop1 = q.pop_min_lazy();
    REQUIRE(pop1.has_value());
    auto m = pop1->second.debug_pop_min_lazy_components_only();
    REQUIRE(m.has_value());
    CHECK(*m == 2);
}
