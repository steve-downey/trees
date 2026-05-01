#include <smd/typeclass/monoid.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <vector>

TEST_CASE("MonoidTypeclassTest - CountBreathing")
{
    // a1d6e3f7-3c2b-4a8e-b4f1-7c5d3a9e6b84
    const smd::typeclass::Count one{1};
    const smd::typeclass::Count two{2};

    const auto result = smd::monoid_combine(one, two);
    CHECK(result.d_value == 3U);
    // a1d6e3f7-3c2b-4a8e-b4f1-7c5d3a9e6b84 end
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

TEST_CASE("MonoidLaws - IdentityElement")
{
    // f3b4e6a2-1c7d-4e5b-8a3f-2d9c5b8e3f36
    {
        const auto& m = smd::typeclass::monoid_v<int>;
        CHECK(m.combine(m.identity(), 42) == 42);
        CHECK(m.combine(42, m.identity()) == 42);
    }
    // f3b4e6a2-1c7d-4e5b-8a3f-2d9c5b8e3f36 end
    {
        const auto& m = smd::typeclass::monoid_v<std::string>;
        CHECK(m.combine(m.identity(), std::string{"hello"}) == "hello");
        CHECK(m.combine(std::string{"hello"}, m.identity()) == "hello");
    }
    {
        const auto& m = smd::typeclass::monoid_v<std::vector<int>>;
        const std::vector<int> v{1, 2, 3};
        CHECK(m.combine(m.identity(), v) == v);
        CHECK(m.combine(v, m.identity()) == v);
    }
    {
        const auto& m = smd::typeclass::monoid_v<smd::typeclass::Count>;
        CHECK(m.combine(m.identity(), smd::typeclass::Count{5}) == smd::typeclass::Count{5});
        CHECK(m.combine(smd::typeclass::Count{5}, m.identity()) == smd::typeclass::Count{5});
    }
}

TEST_CASE("MonoidLaws - Associativity")
{
    {
        const auto& m = smd::typeclass::monoid_v<int>;
        CHECK(m.combine(m.combine(1, 2), 3) == m.combine(1, m.combine(2, 3)));
        CHECK(m.combine(m.combine(-5, 10), -3) == m.combine(-5, m.combine(10, -3)));
    }
    {
        const auto& m = smd::typeclass::monoid_v<std::string>;
        CHECK(m.combine(m.combine(std::string{"foo"}, std::string{"bar"}), std::string{"baz"}) ==
              m.combine(std::string{"foo"}, m.combine(std::string{"bar"}, std::string{"baz"})));
    }
    {
        const auto& m = smd::typeclass::monoid_v<std::vector<int>>;
        const std::vector<int> a{1, 2};
        const std::vector<int> b{3, 4};
        const std::vector<int> c{5, 6};
        CHECK(m.combine(m.combine(a, b), c) == m.combine(a, m.combine(b, c)));
    }
}
