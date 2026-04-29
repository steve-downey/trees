#include <smd/typeclass/monoid.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <vector>

TEST_CASE("MonoidTypeclassTest - CountBreathing")
{
    const smd::typeclass::Count one{1};
    const smd::typeclass::Count two{2};

    const auto result = smd::monoid_combine(one, two);
    CHECK(result.d_value == 3U);
}

TEST_CASE("MonoidTypeclassTest - StringCombine")
{
    const auto joined = smd::monoid_combine(std::string{"hello"}, std::string{" world"});
    CHECK(joined == "hello world");
}

TEST_CASE("MonoidTypeclassTest - VectorCombine")
{
    const auto joined = smd::monoid_combine(std::vector<int>{1, 2}, std::vector<int>{3});
    CHECK(joined == (std::vector<int>{1, 2, 3}));
}
