# trees/src/smd/conceptmap/CMakeLists.txt                       -*-CMake-*-
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

target_sources(smd.conceptmap
    PUBLIC FILE_SET smd_conceptmap_headers FILES
    functor.h
)

install(TARGETS smd.conceptmap FILE_SET smd_conceptmap_headers)

if(TREE_ENABLE_TESTING)
    add_executable(smd_conceptmap_tests monoid.t.cpp)

    target_link_libraries(smd_conceptmap_tests
        PRIVATE
            smd::conceptmap
            Catch2::Catch2WithMain
    )

    target_compile_features(smd_conceptmap_tests PRIVATE cxx_std_23)

    catch_discover_tests(smd_conceptmap_tests)
endif()
