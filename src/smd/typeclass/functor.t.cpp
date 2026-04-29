#include <smd/typeclass/functor.hpp>

#include <catch2/catch_test_macros.hpp>

#include <optional>
#include <vector>

TEST_CASE("FunctorTypeclassTest - OptionalBreathing")
{
    std::optional<int> value{5};
    const auto& functor = smd::functor_typeclass<std::optional<int> >;
    auto mapped = functor.fmap([](int x) { return x + 1; }, value);

    REQUIRE(mapped.has_value());
    CHECK(*mapped == 6);
}

TEST_CASE("FunctorTypeclassTest - ReplaceVector")
{
    std::vector<int> input{1, 2, 3};
    const auto& functor = smd::functor_typeclass<std::vector<int> >;
    auto replaced = functor.replace(input, 9);

    CHECK(replaced == (std::vector<int>{9, 9, 9}));
}
