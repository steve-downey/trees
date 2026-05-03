# installtest/CMakeLists.txt                                        -*-CMake-*-
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

cmake_minimum_required(VERSION 3.27)
project(TestInstalledOptional)

set(CMAKE_CXX_STANDARD 20) #current minimum C++ version

# Enable testing in this separate project
enable_testing()

# Find the installed package
set(OPTIONAL_INSTALL_DIR "../.install/lib/cmake/beman.optional")
find_package(
    beman.optional
    REQUIRED
    PATHS ${OPTIONAL_INSTALL_DIR}
    NO_DEFAULT_PATH
)

# Add the test executable
add_executable(TestInstalledOptional test.cpp)

# Link against the imported target
target_link_libraries(TestInstalledOptional beman::optional)

# Register the test with CTest
add_test(NAME RunInstalledTest COMMAND TestInstalledOptional)

# Ensure 'make test' first builds the 'all' target
set(CMAKE_SKIP_TEST_ALL_DEPENDENCY FALSE)
