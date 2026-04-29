#include <smd/typeclass/applicative.hpp>
#include <smd/typeclass/zip_list.hpp>
#include <smd/typeclass/zip_list_applicative.hpp>

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
