#include <smd/tree/finger_tree_random_access.hpp>

#include <catch2/catch_test_macros.hpp>

#include <optional>
#include <vector>

TEST_CASE("FingerTreeRandomAccessTest - WrapperOperations")
{
    // f2a6c9b3-8d1e-4f7a-c5b9-4e2d7a3c6f01
    using Seq = smd::tree::FingerTreeRandomAccess<int>;

    auto seq = Seq::from_sequence({1, 2, 3});
    REQUIRE(seq.at(0).has_value());
    CHECK(*seq.at(0) == 1);
    CHECK_FALSE(seq.at(99).has_value());

    auto edited = seq.push_back(4).push_front(0).insert(2, 9).update(3, 7).erase(1);
    CHECK(edited.to_vector() == (std::vector<int>{0, 9, 7, 3, 4}));
    // f2a6c9b3-8d1e-4f7a-c5b9-4e2d7a3c6f01 end
}

TEST_CASE("FingerTreeRandomAccessTest - FoldableTypeclass")
{
    using Seq = smd::tree::FingerTreeRandomAccess<int>;

    auto seq = Seq::from_sequence({1, 2, 3, 4});
    const auto& foldable = smd::foldable_typeclass<Seq>;

    CHECK(foldable.fold_map([](int value) { return value; }, seq) == 10);
    CHECK(foldable.length(seq) == 4U);
}

TEST_CASE("FingerTreeRandomAccessTest - TraversableTypeclass")
{
    using Seq = smd::tree::FingerTreeRandomAccess<int>;

    auto seq = Seq::from_sequence({1, 2, 3});
    const auto& traversable = smd::traversable_typeclass<Seq>;

    auto success = traversable.traverse(
      [](int value) -> std::optional<int> { return value * 10; },
      seq);
    REQUIRE(success.has_value());
    CHECK(success->to_vector() == (std::vector<int>{10, 20, 30}));

    auto failure = traversable.traverse(
      [](int value) -> std::optional<int> {
          if (value == 2) {
              return std::nullopt;
          }
          return value;
      },
      seq);
    CHECK_FALSE(failure.has_value());
}
