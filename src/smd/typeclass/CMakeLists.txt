include(GNUInstallDirs)

# Design invariants for this subtree:
# - Headers define concept APIs; map specializations provide per-type behavior.
# - Each concept should have a matching .t.cpp test file when feasible.
# - Tests are optional and only created when TREE_ENABLE_TESTING and GTest_FOUND.
# - Slide snippet examples stay separate under examples/ and compile independently.

# Create typeclass interface library target
add_library(smd_typeclass INTERFACE)

# Add public headers as a file set
target_sources(
    smd_typeclass
    PUBLIC FILE_SET smd_typeclass_headers TYPE HEADERS FILES
        typeclass_base.hpp
        monoid.hpp
        functor.hpp
        foldable.hpp
        applicative.hpp
        traversable.hpp
        zip_list.hpp
        zip_list_applicative.hpp
        law_test_support.hpp
)

# Link with optional dependency
target_link_libraries(smd_typeclass INTERFACE beman.optional)

# Installation configuration
install(
    TARGETS smd_typeclass
    FILE_SET smd_typeclass_headers
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/smd/typeclass
        COMPONENT smd_typeclass_development
)

# Add subdirectories for examples and tests
add_subdirectory(examples)

if(TREE_ENABLE_TESTING AND GTest_FOUND)
    add_executable(smd_typeclass_tests
        typeclass_base.t.cpp
        functor.t.cpp
        monoid.t.cpp
        applicative.t.cpp
        foldable.t.cpp
        fringe_tree.t.cpp
        finger_tree.t.cpp
        traversable.t.cpp
        zip_list_applicative.t.cpp
    )

    target_include_directories(smd_typeclass_tests
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/../..
    )

    target_link_libraries(smd_typeclass_tests
        PRIVATE
            smd_typeclass
            smd_tree
            GTest::gtest
            GTest::gtest_main
    )

    target_compile_features(smd_typeclass_tests PRIVATE cxx_std_23)

    include(GoogleTest)
    gtest_discover_tests(smd_typeclass_tests)
endif()
