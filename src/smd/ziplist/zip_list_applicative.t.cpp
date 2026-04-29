#include <smd/typeclass/applicative.hpp>
#include <smd/ziplist/zip_list.hpp>
#include <smd/ziplist/zip_list_applicative.hpp>

#include <catch2/catch_test_macros.hpp>

#include <vector>

TEST_CASE("ZipListApplicativeTest - PureBreathing")
{
    const auto& applicative = smd::applicative_typeclass<smd::zip_list<int> >;
    auto lifted = applicative.pure(9);
    CHECK(lifted.data == (std::vector<int>{9}));
}

TEST_CASE("ZipListApplicativeTest - ApplyZips")
{
    smd::zip_list<int (*)(int)> functions{{
        +[](int x) { return x + 1; },
        +[](int x) { return x * 2; },
        +[](int x) { return x - 3; },
    }};
    smd::zip_list<int> arguments{{10, 10}};
    const auto& applicative = smd::applicative_typeclass<smd::zip_list<int (*)(int)> >;

    auto result = applicative.apply(functions, arguments);
    CHECK(result.data == (std::vector<int>{11, 20}));
}

TEST_CASE("ZipListApplicativeTest - InvokeZipsMultipleArguments")
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

    CHECK(result.data == (std::vector<int>{111, 222}));
}
