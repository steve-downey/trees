#include <smd/typeclass/typeclass_base.hpp>

#include <gtest/gtest.h>

#include <type_traits>

namespace {

struct sample_tag {};

template <class T>
struct sample_value {
    using type = T;
};

}  // namespace

namespace smd {

template <>
struct map<sample_tag, sample_value<int> > {
    static auto value() -> int { return 42; }
};

}  // namespace smd

TEST(TypeclassBaseTest, RemoveCvrefAlias)
{
    static_assert(std::is_same_v<smd::remove_cvref_t<const int&>, int>);
}

TEST(TypeclassBaseTest, MapForwardDeclarationSpecializationWorks)
{
    EXPECT_EQ((smd::map<sample_tag, sample_value<int> >::value()), 42);
}
