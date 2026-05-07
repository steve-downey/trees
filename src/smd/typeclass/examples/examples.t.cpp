#include <smd/typeclass/examples/examples.hpp>
#include <smd/typeclass/examples/examples.hpp> // Re-inclusion check

#include <catch2/catch_test_macros.hpp>

TEST_CASE("TypeclassExamples - SlideExamplesRemainExecutable") {
    using namespace smd::typeclass::examples;

    CHECK(generic_length_example() == 3U);
    CHECK(generic_length_binary_tree_example() == 4U);
    CHECK(generic_length_fringe_tree_example() == 3U);
    auto applicative_result = applicative_invoke_example();
    REQUIRE(applicative_result);
    CHECK(*applicative_result == 6);
    auto traversable_result = traversable_relabel_example();
    REQUIRE(traversable_result);
    CHECK(*traversable_result == 2U);
    CHECK(traversable_preserves_shape_example());
    CHECK(foldable_flattens_shape_example());
    CHECK(bad_applicative_example() == 1U);

    auto explicit_lookup = explicit_object_lookup_example();
    REQUIRE(explicit_lookup);
    CHECK(*explicit_lookup == 42);

    auto nttp_lookup = nttp_object_lookup_example();
    REQUIRE(nttp_lookup);
    CHECK(*nttp_lookup == 10);

    CHECK(blog_foldable_showcase());
    CHECK(blog_traverse_validate());
    CHECK(blog_traverse_relabel());

    CHECK(blog_numeric_limits_parallel());
    CHECK(blog_three_lookup_modes());
    CHECK(blog_derived_operations());
}
