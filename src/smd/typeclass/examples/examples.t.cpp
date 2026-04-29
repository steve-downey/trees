#include <smd/typeclass/examples/examples.hpp>

#include <gtest/gtest.h>

TEST(TypeclassExamples, SlideExamplesRemainExecutable)
{
    using namespace smd::typeclass::examples;

    EXPECT_EQ(generic_length_example(), 3U);
    EXPECT_EQ(generic_length_binary_tree_example(), 4U);
    auto applicative_result = applicative_invoke_example();
    ASSERT_TRUE(applicative_result);
    EXPECT_EQ(*applicative_result, 6);
    auto traversable_result = traversable_relabel_example();
    ASSERT_TRUE(traversable_result);
    EXPECT_EQ(*traversable_result, 2U);
    EXPECT_EQ(bad_applicative_example(), 1U);

    auto explicit_lookup = explicit_object_lookup_example();
    ASSERT_TRUE(explicit_lookup);
    EXPECT_EQ(*explicit_lookup, 42);

    auto nttp_lookup = nttp_object_lookup_example();
    ASSERT_TRUE(nttp_lookup);
    EXPECT_EQ(*nttp_lookup, 10);
}
