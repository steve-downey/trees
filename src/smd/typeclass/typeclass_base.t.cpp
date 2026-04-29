#include <smd/typeclass/typeclass_base.hpp>

#include <gtest/gtest.h>

#include <type_traits>

TEST(TypeclassBaseTest, RemoveCvrefAlias)
{
    static_assert(std::is_same_v<smd::remove_cvref_t<const int&>, int>);
}
