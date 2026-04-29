#include <smd/tree/finger_tree_rope.hpp>

#include <gtest/gtest.h>

#include <optional>
#include <string>

TEST(FingerTreeRopeTest, WrapperOperations)
{
    using Rope = smd::tree::FingerTreeRope;

    auto rope = Rope::from_text("abCDxy", 2)
                    .insert(2, "--")
                    .erase(5, 2)
                    .replace(0, 2, "AB");

    EXPECT_EQ(rope.to_string(), "AB--Cy");
    EXPECT_EQ(rope.size_bytes(), 6U);
}

TEST(FingerTreeRopeTest, FoldableTypeclass)
{
    using Rope = smd::tree::FingerTreeRope;

    auto rope = Rope::from_text("abcdefgh", 2);
    const auto& foldable = smd::foldable_typeclass<Rope>;

    EXPECT_EQ(
      foldable.fold_map(
        [](const std::string& chunk) { return chunk.size(); },
        rope),
      8U);
    EXPECT_EQ(foldable.length(rope), 4U);
}

TEST(FingerTreeRopeTest, TraversableTypeclass)
{
    using Rope = smd::tree::FingerTreeRope;

    auto rope = Rope::from_text("abcd", 2);
    const auto& traversable = smd::traversable_typeclass<Rope>;

    auto success = traversable.traverse(
      [](const std::string& chunk) -> std::optional<std::string> {
          return chunk + "!";
      },
      rope);
    ASSERT_TRUE(success.has_value());
    EXPECT_EQ(success->to_string(), "ab!cd!");

    auto failure = traversable.traverse(
      [](const std::string& chunk) -> std::optional<std::string> {
          if (chunk == "cd") {
              return std::nullopt;
          }
          return chunk;
      },
      rope);
    EXPECT_FALSE(failure.has_value());
}
