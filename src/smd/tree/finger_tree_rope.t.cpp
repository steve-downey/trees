// src/smd/tree/finger_tree_rope.t.cpp                                -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <smd/tree/finger_tree_rope.hpp>
#include <smd/tree/finger_tree_rope.hpp> // Re-inclusion check

#include <smd/typeclass/foldable.hpp>
#include <smd/typeclass/traversable.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

using Rope = smd::tree::FingerTreeRope;

TEST_CASE("Rope - HeaderIsIdempotent")
{
    REQUIRE(true);
}

TEST_CASE("Rope - Empty")
{
    Rope r;
    CHECK(r.size_bytes() == 0U);
    CHECK(r.to_string().empty());
    CHECK(r.chunks().empty());
}

TEST_CASE("Rope - FromTextSmall")
{
    auto r = Rope::from_text("hello world", 4);
    CHECK(r.size_bytes() == 11U);
    CHECK(r.to_string() == "hello world");
    auto c = r.chunks();
    CHECK(c.size() == 3U);
    CHECK(c[0] == "hell");
    CHECK(c[1] == "o wo");
    CHECK(c[2] == "rld");
}

TEST_CASE("Rope - FromTextChunkSizes")
{
    std::string text = "abcdefghijklmnopqrstuvwxyz";

    auto r1 = Rope::from_text(text, 1);
    CHECK(r1.size_bytes() == 26U);
    CHECK(r1.to_string() == text);
    CHECK(r1.chunks().size() == 26U);

    auto r26 = Rope::from_text(text, 26);
    CHECK(r26.size_bytes() == 26U);
    CHECK(r26.to_string() == text);
    CHECK(r26.chunks().size() == 1U);

    auto rbig = Rope::from_text(text, 1024);
    CHECK(rbig.size_bytes() == 26U);
    CHECK(rbig.to_string() == text);
}

TEST_CASE("Rope - InsertAtFront")
{
    auto r = Rope::from_text("world", 4);
    auto r2 = r.insert(0, "hello ");
    CHECK(r2.size_bytes() == 11U);
    CHECK(r2.to_string() == "hello world");
    CHECK(r.to_string() == "world");
}

TEST_CASE("Rope - InsertAtEnd")
{
    auto r = Rope::from_text("hello", 4);
    auto r2 = r.insert(r.size_bytes(), " world");
    CHECK(r2.size_bytes() == 11U);
    CHECK(r2.to_string() == "hello world");
}

TEST_CASE("Rope - InsertAtMiddle")
{
    auto r = Rope::from_text("helloworld", 4);
    auto r2 = r.insert(5, " ");
    CHECK(r2.to_string() == "hello world");
}

TEST_CASE("Rope - EraseFromFront")
{
    auto r = Rope::from_text("hello world", 4);
    auto r2 = r.erase(0, 6);
    CHECK(r2.to_string() == "world");
}

TEST_CASE("Rope - EraseFromEnd")
{
    auto r = Rope::from_text("hello world", 4);
    auto r2 = r.erase(5, 6);
    CHECK(r2.to_string() == "hello");
}

TEST_CASE("Rope - EraseFromMiddle")
{
    auto r = Rope::from_text("hello world", 4);
    auto r2 = r.erase(5, 1);
    CHECK(r2.to_string() == "helloworld");
}

TEST_CASE("Rope - Replace")
{
    auto r = Rope::from_text("hello world", 4);
    auto r2 = r.replace(6, 5, "earth");
    CHECK(r2.to_string() == "hello earth");

    auto r3 = r.replace(0, 5, "HELLO");
    CHECK(r3.to_string() == "HELLO world");
}

TEST_CASE("Rope - SpineTransition")
{
    std::string text = "01234567890123456789";
    auto r = Rope::from_text(text, 1);
    REQUIRE(r.size_bytes() == 20U);
    REQUIRE(r.to_string() == text);

    auto r2 = r.insert(10, "XXXXX");
    CHECK(r2.size_bytes() == 25U);
    CHECK(r2.to_string() == "0123456789XXXXX0123456789");

    auto r3 = r2.erase(10, 5);
    CHECK(r3.to_string() == text);
}

TEST_CASE("Rope - RepeatedInsertAtEndGrows")
{
    auto r = Rope::from_text("", 16);
    std::string expected;

    for (int i = 0; i < 100; ++i) {
        std::string chunk = "chunk" + std::to_string(i) + "_";
        expected += chunk;
        r = r.insert(r.size_bytes(), chunk);
    }
    REQUIRE(r.size_bytes() == expected.size());
    CHECK(r.to_string() == expected);
}

TEST_CASE("Rope - RepeatedInsertAtFrontGrows")
{
    auto r = Rope::from_text("", 16);
    std::string expected;

    for (int i = 0; i < 100; ++i) {
        std::string chunk = std::to_string(i) + "_";
        expected = chunk + expected;
        r = r.insert(0, chunk);
    }
    REQUIRE(r.size_bytes() == expected.size());
    CHECK(r.to_string() == expected);
}

TEST_CASE("Rope - LargeText")
{
    std::string text(5000, 'x');
    for (std::size_t i = 0; i < text.size(); ++i)
        text[i] = static_cast<char>('a' + (i % 26));

    auto r = Rope::from_text(text, 64);
    REQUIRE(r.size_bytes() == 5000U);
    REQUIRE(r.to_string() == text);

    auto r2 = r.insert(2500, "INSERTED");
    std::string exp2 = text.substr(0, 2500) + "INSERTED" + text.substr(2500);
    CHECK(r2.size_bytes() == exp2.size());
    CHECK(r2.to_string() == exp2);

    auto r3 = r.erase(1000, 500);
    std::string exp3 = text.substr(0, 1000) + text.substr(1500);
    CHECK(r3.size_bytes() == exp3.size());
    CHECK(r3.to_string() == exp3);
}

TEST_CASE("Rope - InsertEraseRoundtrip")
{
    std::string text = "The quick brown fox jumps over the lazy dog";
    auto r = Rope::from_text(text, 8);

    auto r2 = r.insert(10, "EXTRA");
    auto r3 = r2.erase(10, 5);
    CHECK(r3.to_string() == text);
}

TEST_CASE("Rope - Persistence")
{
    auto r = Rope::from_text("hello", 4);
    auto r2 = r.insert(5, " world");
    auto r3 = r.erase(0, 5);

    CHECK(r.to_string() == "hello");
    CHECK(r2.to_string() == "hello world");
    CHECK(r3.to_string() == "");
}

TEST_CASE("Rope - FoldableTypeclass")
{
    auto r = Rope::from_text("abCDxy", 2);
    const auto &foldable = smd::foldable_typeclass<Rope>;
    CHECK(foldable.length(r) == 3U);
}

TEST_CASE("Rope - TraversableTypeclass")
{
    auto r = Rope::from_text("abcd", 2);
    auto result = smd::traverse(
        [](const std::string &chunk) -> std::optional<std::string> {
            std::string upper;
            for (char c : chunk)
                upper += static_cast<char>(c - 32);
            return upper;
        },
        r);
    REQUIRE(result.has_value());
    CHECK(result->to_string() == "ABCD");
}
