# trees/src/CMakeLists.txt                                      -*-CMake-*-
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

include(GNUInstallDirs)

# Library targets declared here so BASE_DIRS . resolves to the include root.
# Subdirectory CMakeLists.txt files add local files to these FILE_SETs.

add_library(smd.fixpoint INTERFACE)
add_library(smd::fixpoint ALIAS smd.fixpoint)
target_sources(smd.fixpoint PUBLIC FILE_SET smd_fixpoint_headers TYPE HEADERS BASE_DIRS .)
target_compile_features(smd.fixpoint INTERFACE cxx_std_23)

add_library(smd.thunk INTERFACE)
add_library(smd::thunk ALIAS smd.thunk)
target_sources(smd.thunk PUBLIC FILE_SET smd_thunk_headers TYPE HEADERS BASE_DIRS .)
target_compile_features(smd.thunk INTERFACE cxx_std_23)

add_library(smd.typeclass INTERFACE)
add_library(smd::typeclass ALIAS smd.typeclass)
target_sources(smd.typeclass PUBLIC FILE_SET smd_typeclass_headers TYPE HEADERS BASE_DIRS .)
target_link_libraries(smd.typeclass INTERFACE beman.optional)
target_compile_features(smd.typeclass INTERFACE cxx_std_23)

add_library(smd.tree INTERFACE)
add_library(smd::tree ALIAS smd.tree)
target_sources(smd.tree PUBLIC FILE_SET smd_tree_headers TYPE HEADERS BASE_DIRS .)
target_link_libraries(smd.tree INTERFACE smd::typeclass smd::fixpoint)
target_compile_features(smd.tree INTERFACE cxx_std_23)

add_library(smd.ziplist INTERFACE)
add_library(smd::ziplist ALIAS smd.ziplist)
target_sources(smd.ziplist PUBLIC FILE_SET smd_ziplist_headers TYPE HEADERS BASE_DIRS .)
target_link_libraries(smd.ziplist INTERFACE smd::typeclass)
target_compile_features(smd.ziplist INTERFACE cxx_std_23)

add_library(smd.ranges INTERFACE)
add_library(smd::ranges ALIAS smd.ranges)
target_sources(smd.ranges PUBLIC FILE_SET smd_ranges_headers TYPE HEADERS BASE_DIRS .)
target_link_libraries(smd.ranges INTERFACE smd::typeclass)
target_compile_features(smd.ranges INTERFACE cxx_std_23)

add_library(smd.conceptmap INTERFACE)
add_library(smd::conceptmap ALIAS smd.conceptmap)
target_sources(smd.conceptmap PUBLIC FILE_SET smd_conceptmap_headers TYPE HEADERS BASE_DIRS .)
target_link_libraries(smd.conceptmap INTERFACE beman.optional)
target_compile_features(smd.conceptmap INTERFACE cxx_std_23)

add_subdirectory(smd)
add_subdirectory(examples)
