# trees/src/smd/ranges/CMakeLists.txt                           -*-CMake-*-
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

target_sources(smd.ranges
    PUBLIC FILE_SET smd_ranges_headers FILES
        range_list.hpp
        range_functor.hpp
        range_applicative.hpp
        range_foldable.hpp
        range_traversable.hpp
)

install(TARGETS smd.ranges FILE_SET smd_ranges_headers)

if(TREE_ENABLE_TESTING)
    add_executable(smd_ranges_tests
        range_list.t.cpp
        range_functor.t.cpp
        range_applicative.t.cpp
        range_foldable.t.cpp
        range_traversable.t.cpp
    )

    target_link_libraries(smd_ranges_tests
        PRIVATE
            smd::ranges
            Catch2::Catch2WithMain
    )

    target_compile_features(smd_ranges_tests PRIVATE cxx_std_23)

    catch_discover_tests(smd_ranges_tests)
endif()
