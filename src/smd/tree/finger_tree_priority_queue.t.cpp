#include <smd/tree/finger_tree_priority_queue.hpp>

#include <catch2/catch_test_macros.hpp>

#include <optional>
#include <set>
#include <vector>

TEST_CASE("FingerTreePriorityQueueTest - WrapperOperations")
{
    using Queue = smd::tree::FingerTreePriorityQueue<int>;

    auto q = Queue::from_values({5, 2, 8, 2, 7});
    REQUIRE(q.min().has_value());
    REQUIRE(q.max().has_value());
    CHECK(*q.min() == 2);
    CHECK(*q.max() == 8);

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

    auto success = traversable.traverse(
      [](int value) -> std::optional<int> { return value * 10; },
      q);
    REQUIRE(success.has_value());
    CHECK(success->to_vector() == (std::vector<int>{50, 20, 80}));
    REQUIRE(success->min().has_value());
    REQUIRE(success->max().has_value());
    CHECK(*success->min() == 20);
    CHECK(*success->max() == 80);

    auto failure = traversable.traverse(
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
