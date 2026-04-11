# example/CMakeLists.txt                                            -*-cmake-*-
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

cmake_minimum_required(VERSION 4.0..4.2)

project(name VERSION 0.0.0 LANGUAGES CXX)

# Instrumentation query
# find_package(Python COMPONENTS Interpreter)
cmake_instrumentation(
  API_VERSION 1
  DATA_VERSION 1
  HOOKS postGenerate preBuild postBuild preCMakeBuild postCMakeBuild postCMakeInstall postCTest
  OPTIONS trace
  CALLBACK ${CMAKE_SOURCE_DIR}/scripts/instrument.sh
)

# Includes
include(CTest)
include(FetchContent)

option(
    NAME_ENABLE_TESTING
    "Enable building tests and test infrastructure"
    ${PROJECT_IS_TOP_LEVEL}
)

set(CMAKE_VERIFY_INTERFACE_HEADER_SETS ON)

# Create the library target and named header set for beman.optional
add_library(example.name STATIC "")

target_sources(
    example.name
    PUBLIC FILE_SET example_name_headers TYPE HEADERS BASE_DIRS src
)

if(NAME_ENABLE_TESTING)
    find_package(GTest QUIET)
    if(GTest_FOUND)
        include(GoogleTest)
    else()
        message(
            WARNING
            "
No provider for GTest. Unable to build tests.
Consider using CMAKE_PROJECT_TOP_LEVEL_INCLUDES=./cmake/use-fetch-content.cmake as documented in the README.md
"
        )
    endif()
endif()

add_subdirectory(src)

include(infra/cmake/beman-install-library.cmake)
beman_install_library(example.name TARGETS example.name NAMESPACE smd::)

# Coverage
configure_file("cmake/gcovr.cfg.in" gcovr.cfg @ONLY)

add_custom_target(
    process_coverage
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running gcovr to process coverage results"
    COMMAND mkdir -p coverage
    COMMAND gcovr --config gcovr.cfg .
)
