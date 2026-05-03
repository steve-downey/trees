#include <smd/tree/finger_tree_priority_queue.hpp>
#include <smd/tree/finger_tree_priority_queue.hpp> // Re-inclusion check

#include <catch2/catch_test_macros.hpp>

#include <optional>
#include <set>
#include <vector>

TEST_CASE("FingerTreePriorityQueueTest - WrapperOperations") {
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

TEST_CASE("FingerTreePriorityQueueTest - FoldableTypeclass") {
    using Queue = smd::tree::FingerTreePriorityQueue<int>;

    auto q = Queue::from_values({5, 2, 8, 2, 7});
    const auto &foldable = smd::foldable_typeclass<Queue>;

    CHECK(foldable.fold_map([](int value) { return value; }, q) == 24);
    CHECK(foldable.length(q) == 5U);
}

TEST_CASE("FingerTreePriorityQueueTest - TraversableTypeclass") {
    using Queue = smd::tree::FingerTreePriorityQueue<int>;

    auto q = Queue::from_values({5, 2, 8});

    auto success = smd::traverse(
        [](int value) -> std::optional<int> { return value * 10; }, q);
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

TEST_CASE("FingerTreePriorityQueueTest - RepeatedPushPopMatchesMultiset") {
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

        CHECK(q.size() == expected.size());
        CHECK(q.size() == q.to_vector().size());

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

TEST_CASE("FingerTreePriorityQueueTest - PopMinWithDuplicates") {
    using Queue = smd::tree::FingerTreePriorityQueue<int>;

    auto q = Queue::from_values({5, 2, 8, 2, 7});
    REQUIRE(q.min().has_value());
    CHECK(*q.min() == 2);

    auto pop1 = q.pop_min();
    REQUIRE(pop1.has_value());
    CHECK(pop1->first == 2);

    auto q2 = pop1->second;
    REQUIRE(q2.min().has_value());
    REQUIRE(q2.max().has_value());
    CHECK(*q2.min() == 2);
    CHECK(*q2.max() == 8);

    auto pop2 = q2.pop_min();
    REQUIRE(pop2.has_value());
    CHECK(pop2->first == 2);

    auto q3 = pop2->second;
    REQUIRE(q3.min().has_value());
    REQUIRE(q3.max().has_value());
    CHECK(*q3.min() == 5);
    CHECK(*q3.max() == 8);
}

TEST_CASE("FingerTreePriorityQueueTest - PopMaxWithDuplicates") {
    using Queue = smd::tree::FingerTreePriorityQueue<int>;

    auto q = Queue::from_values({5, 8, 2, 8, 7});
    REQUIRE(q.max().has_value());
    CHECK(*q.max() == 8);

    auto pop1 = q.pop_max();
    REQUIRE(pop1.has_value());
    CHECK(pop1->first == 8);

    auto q2 = pop1->second;
    REQUIRE(q2.min().has_value());
    REQUIRE(q2.max().has_value());
    CHECK(*q2.max() == 8);
    CHECK(*q2.min() == 2);

    auto pop2 = q2.pop_max();
    REQUIRE(pop2.has_value());
    CHECK(pop2->first == 8);

    auto q3 = pop2->second;
    REQUIRE(q3.min().has_value());
    REQUIRE(q3.max().has_value());
    CHECK(*q3.max() == 7);
    CHECK(*q3.min() == 2);
}
