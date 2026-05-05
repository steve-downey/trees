# trees/src/smd/thunk/CMakeLists.txt                            -*-CMake-*-
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

target_sources(
    smd.thunk
    PUBLIC FILE_SET smd_thunk_headers FILES delay.hpp memoize.hpp
)

install(TARGETS smd.thunk FILE_SET smd_thunk_headers)

if(TREE_ENABLE_TESTING)
    add_executable(smd_thunk_tests delay.t.cpp memoize.t.cpp)

    target_link_libraries(
        smd_thunk_tests
        PRIVATE smd::thunk Catch2::Catch2WithMain
    )

    target_compile_features(smd_thunk_tests PRIVATE cxx_std_23)

    catch_discover_tests(smd_thunk_tests)
endif()
