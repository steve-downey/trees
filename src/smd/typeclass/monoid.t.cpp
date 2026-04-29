#include <smd/typeclass/monoid.hpp>

#include <gtest/gtest.h>

#include <string>
#include <vector>

TEST(MonoidTypeclassTest, CountBreathing)
{
    const smd::typeclass::Count one{1};
    const smd::typeclass::Count two{2};

    const auto result = smd::monoid_combine(one, two);
    EXPECT_EQ(result.d_value, 3U);
}

TEST(MonoidTypeclassTest, StringCombine)
{
    const auto joined = smd::monoid_combine(std::string{"hello"}, std::string{" world"});
    EXPECT_EQ(joined, "hello world");
}

TEST(MonoidTypeclassTest, VectorCombine)
{
    const auto joined = smd::monoid_combine(std::vector<int>{1, 2}, std::vector<int>{3});
    EXPECT_EQ(joined, (std::vector<int>{1, 2, 3}));
}
