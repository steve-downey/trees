cmake_minimum_required(VERSION 4.0..4.2)

project(ConceptMap VERSION 0.0.0 LANGUAGES CXX)

# Enable experimental feature!!
set(CMAKE_EXPERIMENTAL_INSTRUMENTATION ec7aa2dc-b87f-45a3-8022-fe01c5f59984)

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

set(TARGETS_EXPORT_NAME ${CMAKE_PROJECT_NAME}Targets)

option(
    TREE_ENABLE_TESTING
    "Enable building tests and test infrastructure"
    ${PROJECT_IS_TOP_LEVEL}
)

set(CMAKE_VERIFY_INTERFACE_HEADER_SETS ON)

# Add Catch2 from the vendor directory
add_subdirectory(vendor/catch2 EXCLUDE_FROM_ALL)

if(TREE_ENABLE_TESTING)
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/vendor/catch2/extras")
    include(Catch)
endif()

set(OPTIONAL_ENABLE_TESTING OFF)
add_subdirectory(vendor/optional EXCLUDE_FROM_ALL)

# find_package(
#     beman.optional
#     REQUIRED
#     PATHS ${OPTIONAL_INSTALL_DIR}
#     NO_DEFAULT_PATH
# )

# Create the library target and named header set for beman_optional
add_library(concept_map INTERFACE)
target_sources(
    concept_map
    PUBLIC FILE_SET concept_map_headers TYPE HEADERS BASE_DIRS src
)
target_compile_features(concept_map INTERFACE cxx_std_23)

# add_subdirectory(extern)
add_subdirectory(src)

# Keep presentation snippets in the regular build so transcluded slide code
# stays synchronized with compilable sources.
if(TARGET smd_typeclass_slide_examples)
    add_custom_target(slide_snippets_check ALL)
    add_dependencies(slide_snippets_check smd_typeclass_slide_examples)

    if(TARGET smd_typeclass_slide_examples_tests)
        add_dependencies(slide_snippets_check smd_typeclass_slide_examples_tests)
    endif()
endif()

include(GNUInstallDirs)

# Coverage
configure_file("cmake/gcovr.cfg.in" gcovr.cfg @ONLY)

add_custom_target(
    process_coverage
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running gcovr to process coverage results"
    COMMAND mkdir -p coverage
    COMMAND gcovr --config gcovr.cfg .
)
