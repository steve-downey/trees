include(GNUInstallDirs)

target_sources(
    example.name
    PUBLIC FILE_SET example_name_headers TYPE HEADERS FILES name.hpp
    PRIVATE name.cpp
)

## Tests
if(NAME_ENABLE_TESTING AND NOT GTest_FOUND)
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

if(NAME_ENABLE_TESTING)
    add_executable(name_test)

    target_sources(name_test PRIVATE name.test.cpp)

    target_link_libraries(name_test example.name)
    target_link_libraries(name_test GTest::gtest)
    target_link_libraries(name_test GTest::gtest_main)

    include(GoogleTest)
    gtest_discover_tests(name_test)
endif()
