#include <smd/typeclass/examples/examples.hpp>
#include <smd/typeclass/examples/examples.hpp>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("slide examples remain executable")
{
    using namespace smd::typeclass::examples;

    CHECK(generic_length_example() == 3U);
    REQUIRE(applicative_invoke_example());
    CHECK(*applicative_invoke_example() == 3);
    REQUIRE(traversable_relabel_example());
    CHECK(*traversable_relabel_example() == 2U);
    CHECK(bad_applicative_example() == 1U);
}
