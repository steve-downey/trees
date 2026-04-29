include(GNUInstallDirs)

# Create tree interface library target for FixTree
add_library(smd_tree INTERFACE)

# Add public headers as a file set
target_sources(
    smd_tree
    PUBLIC FILE_SET smd_tree_headers TYPE HEADERS FILES
    binary_tree.hpp
    binary_tree_foldable.hpp
    binary_tree_applicative.hpp
    binary_tree_traversable.hpp
    fringe_tree.hpp
    fringe_tree_foldable.hpp
    fringe_tree_applicative.hpp
    fringe_tree_traversable.hpp
    finger_tree.hpp
    finger_tree_foldable.hpp
    finger_tree_traversable.hpp
    finger_tree_random_access.hpp
    finger_tree_priority_queue.hpp
    finger_tree_interval_index.hpp
    finger_tree_rope.hpp
    finger_tree_wrappers.hpp
        fix_tree.hpp
        fix_tree_foldable.hpp
        fix_tree_applicative.hpp
        fix_tree_traversable.hpp
)

# Link with typeclass dependency
target_link_libraries(smd_tree INTERFACE smd_typeclass)

# Installation configuration
install(
    TARGETS smd_tree
    FILE_SET smd_tree_headers
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/smd/tree
        COMPONENT smd_tree_development
)

if(TREE_ENABLE_TESTING AND GTest_FOUND)
    add_executable(smd_tree_tests
        fringe_tree.t.cpp
        finger_tree.t.cpp
        finger_tree_random_access.t.cpp
        finger_tree_priority_queue.t.cpp
        finger_tree_interval_index.t.cpp
        finger_tree_rope.t.cpp
        binary_tree_applicative.t.cpp
        binary_tree_foldable.t.cpp
        binary_tree_traversable.t.cpp
        fix_tree_applicative.t.cpp
        fix_tree_foldable.t.cpp
        fix_tree_traversable.t.cpp
        fringe_tree_applicative.t.cpp
        fringe_tree_traversable.t.cpp
    )

    target_include_directories(smd_tree_tests
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/../..
    )

    target_link_libraries(smd_tree_tests
        PRIVATE
            smd_tree
            GTest::gtest
            GTest::gtest_main
    )

    target_compile_features(smd_tree_tests PRIVATE cxx_std_23)

    include(GoogleTest)
    gtest_discover_tests(smd_tree_tests)
endif()
