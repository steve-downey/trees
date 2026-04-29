#include <smd/typeclass/applicative.hpp>
#include <smd/typeclass/zip_list.hpp>
#include <smd/typeclass/zip_list_applicative.hpp>

#include <gtest/gtest.h>

#include <vector>

TEST(ZipListApplicativeTest, PureBreathing)
{
    auto lifted = smd::pure<smd::zip_list<int> >(9);
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

    auto result = smd::apply(functions, arguments);
    EXPECT_EQ(result.data, (std::vector<int>{11, 20}));
}
