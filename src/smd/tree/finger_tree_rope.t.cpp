#include <smd/tree/finger_tree_rope.hpp>
#include <smd/tree/finger_tree_rope.hpp>  // Re-inclusion check

#include <catch2/catch_test_macros.hpp>

#include <optional>
#include <string>

TEST_CASE("FingerTreeRopeTest - WrapperOperations")
{
    using Rope = smd::tree::FingerTreeRope;

    // c8e3b5f9-2a7d-4c1e-b9f3-5a4d2b8c7e06
    auto rope = Rope::from_text("abCDxy", 2)
                    .insert(2, "--")
                    .erase(5, 2)
                    .replace(0, 2, "AB");

    CHECK(rope.to_string() == "AB--Cy");
    CHECK(rope.size_bytes() == 6U);
    // c8e3b5f9-2a7d-4c1e-b9f3-5a4d2b8c7e06 end
}

TEST_CASE("FingerTreeRopeTest - FoldableTypeclass")
{
    using Rope = smd::tree::FingerTreeRope;

    auto rope = Rope::from_text("abcdefgh", 2);
    const auto& foldable = smd::foldable_typeclass<Rope>;

    CHECK(
      foldable.fold_map(
        [](const std::string& chunk) { return chunk.size(); },
        rope) ==
      8U);
    CHECK(foldable.length(rope) == 4U);
}

TEST_CASE("FingerTreeRopeTest - TraversableTypeclass")
{
    using Rope = smd::tree::FingerTreeRope;

    auto rope = Rope::from_text("abcd", 2);

    auto success = smd::traverse(
      [](const std::string& chunk) -> std::optional<std::string> {
          return chunk + "!";
      },
      rope);
    REQUIRE(success.has_value());
    CHECK(success->to_string() == "ab!cd!");

    auto failure = smd::traverse(
      [](const std::string& chunk) -> std::optional<std::string> {
          if (chunk == "cd") {
              return std::nullopt;
          }
          return chunk;
      },
      rope);
    CHECK_FALSE(failure.has_value());
}
