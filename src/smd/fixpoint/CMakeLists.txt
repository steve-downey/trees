# trees/src/smd/fixpoint/CMakeLists.txt                         -*-CMake-*-
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

target_sources(
    smd.fixpoint
    PUBLIC
        FILE_SET smd_fixpoint_headers
            FILES box.hpp overloaded.hpp fix.hpp cata.hpp
)

install(TARGETS smd.fixpoint FILE_SET smd_fixpoint_headers)

if(TREE_ENABLE_TESTING)
    add_executable(smd_fixpoint_tests box.t.cpp fix.t.cpp cata.t.cpp overloaded.t.cpp)

    target_link_libraries(
        smd_fixpoint_tests
        PRIVATE smd::fixpoint Catch2::Catch2WithMain
    )

    target_compile_features(smd_fixpoint_tests PRIVATE cxx_std_23)

    catch_discover_tests(smd_fixpoint_tests)
endif()
