#include <smd/typeclass/typeclass_base.hpp>

#include <catch2/catch_test_macros.hpp>

#include <type_traits>

TEST_CASE("TypeclassBaseTest - RemoveCvrefAlias")
{
    static_assert(std::is_same_v<smd::remove_cvref_t<const int&>, int>);
}
