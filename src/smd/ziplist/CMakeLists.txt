# trees/src/smd/ziplist/CMakeLists.txt                          -*-CMake-*-
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

target_sources(
    smd.ziplist
    PUBLIC
        FILE_SET smd_ziplist_headers FILES zip_list.hpp zip_list_applicative.hpp
)

install(TARGETS smd.ziplist FILE_SET smd_ziplist_headers)

if(TREE_ENABLE_TESTING)
    add_executable(smd_ziplist_tests zip_list.t.cpp zip_list_applicative.t.cpp)

    target_link_libraries(
        smd_ziplist_tests
        PRIVATE smd::ziplist Catch2::Catch2WithMain
    )

    target_compile_features(smd_ziplist_tests PRIVATE cxx_std_23)

    catch_discover_tests(smd_ziplist_tests)
endif()
