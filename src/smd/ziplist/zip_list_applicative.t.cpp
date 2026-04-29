#include <smd/typeclass/applicative.hpp>
#include <smd/ziplist/zip_list.hpp>
#include <smd/ziplist/zip_list_applicative.hpp>

#include <gtest/gtest.h>

#include <vector>

TEST(ZipListApplicativeTest, PureBreathing)
{
    const auto& applicative = smd::applicative_typeclass<smd::zip_list<int> >;
    auto lifted = applicative.pure(9);
    EXPECT_EQ(lifted.data, (std::vector<int>{9}));
}

TEST(ZipListApplicativeTest, ApplyZips)
{
    smd::zip_list<int (*)(int)> functions{{
        +[](int x) { return x + 1; },
        +[](int x) { return x * 2; },
        +[](int x) { return x - 3; },
    }};
    smd::zip_list<int> arguments{{10, 10}};
    const auto& applicative = smd::applicative_typeclass<smd::zip_list<int (*)(int)> >;

    auto result = applicative.apply(functions, arguments);
    EXPECT_EQ(result.data, (std::vector<int>{11, 20}));
}

TEST(ZipListApplicativeTest, InvokeZipsMultipleArguments)
{
    smd::zip_list<int> xs{{1, 2, 3}};
    smd::zip_list<int> ys{{10, 20}};
    smd::zip_list<int> zs{{100, 200, 300, 400}};
    const auto& applicative = smd::applicative_typeclass<smd::zip_list<int> >;

    auto result = applicative.invoke(
        [](int x, int y, int z) { return x + y + z; },
        xs,
        ys,
        zs);

    EXPECT_EQ(result.data, (std::vector<int>{111, 222}));
}
