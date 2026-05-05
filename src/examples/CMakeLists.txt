# trees/src/examples/CMakeLists.txt                             -*-CMake-*-
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

include(GNUInstallDirs)

add_executable(map_example map_example.cpp)
target_link_libraries(map_example PRIVATE smd::conceptmap)

add_executable(cpo_example cpo_example.cpp)
target_link_libraries(cpo_example PRIVATE smd::conceptmap)

add_executable(fixpoint_tree_example fixpoint_tree_example.cpp)
target_compile_features(fixpoint_tree_example PRIVATE cxx_std_23)

install(
    TARGETS map_example cpo_example fixpoint_tree_example
    DESTINATION ${CMAKE_INSTALL_BINDIR}
)
