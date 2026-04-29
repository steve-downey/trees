#include <smd/tree/finger_tree_interval_index.hpp>

#include <gtest/gtest.h>

#include <optional>
#include <string>
#include <vector>

TEST(FingerTreeIntervalIndexTest, WrapperOperations)
{
    using Index = smd::tree::FingerTreeIntervalIndex<std::string>;
    using Entry = smd::tree::Interval<std::string>;

    auto idx = Index::from_intervals({
        Entry{0U, 5U, "A"},
        Entry{3U, 10U, "B"},
        Entry{8U, 12U, "C"}
    });

    EXPECT_EQ(idx.query_point(2U), (std::vector<std::string>{"A"}));
    EXPECT_EQ(idx.query_point(4U), (std::vector<std::string>{"A", "B"}));
    EXPECT_EQ(idx.query_overlap(9U, 11U), (std::vector<std::string>{"B", "C"}));
}

TEST(FingerTreeIntervalIndexTest, FoldableTypeclass)
{
    using Index = smd::tree::FingerTreeIntervalIndex<std::string>;
    using Entry = smd::tree::Interval<std::string>;

    auto idx = Index::from_intervals({
      Entry{0U, 5U, "A"},
      Entry{3U, 10U, "B"},
      Entry{8U, 12U, "C"}
    });
    const auto& foldable = smd::foldable_typeclass<Index>;

    EXPECT_EQ(
      foldable.fold_map([](const std::string& payload) { return payload; }, idx),
      "ABC");
    EXPECT_EQ(foldable.length(idx), 3U);
}

TEST(FingerTreeIntervalIndexTest, TraversableTypeclass)
{
    using Index = smd::tree::FingerTreeIntervalIndex<std::string>;
    using Entry = smd::tree::Interval<std::string>;

    auto idx = Index::from_intervals({
      Entry{0U, 5U, "A"},
      Entry{3U, 10U, "B"},
      Entry{8U, 12U, "C"}
    });
    const auto& traversable = smd::traversable_typeclass<Index>;

    auto success = traversable.traverse(
      [](const std::string& payload) -> std::optional<std::string> {
          return payload + "!";
      },
      idx);
    ASSERT_TRUE(success.has_value());
    EXPECT_EQ(success->query_point(4U), (std::vector<std::string>{"A!", "B!"}));
    EXPECT_EQ(success->query_overlap(9U, 11U),
              (std::vector<std::string>{"B!", "C!"}));

    auto failure = traversable.traverse(
      [](const std::string& payload) -> std::optional<std::string> {
          if (payload == "B") {
              return std::nullopt;
          }
          return payload;
      },
      idx);
    EXPECT_FALSE(failure.has_value());
}
