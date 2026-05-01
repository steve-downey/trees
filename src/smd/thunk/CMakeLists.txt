include(GNUInstallDirs)

add_library(smd_thunk INTERFACE)

target_sources(
    smd_thunk
    PUBLIC FILE_SET smd_thunk_headers TYPE HEADERS FILES
        delay.hpp
        memoize.hpp
)

install(
    TARGETS smd_thunk
    FILE_SET smd_thunk_headers
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/smd/thunk
        COMPONENT smd_thunk_development
)

if(TREE_ENABLE_TESTING)
    add_executable(smd_thunk_tests
        delay.t.cpp
        memoize.t.cpp
    )

    target_include_directories(smd_thunk_tests
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/../..
    )

    target_link_libraries(smd_thunk_tests
        PRIVATE
            smd_thunk
            Catch2::Catch2WithMain
    )

    target_compile_features(smd_thunk_tests PRIVATE cxx_std_23)

    catch_discover_tests(smd_thunk_tests)
endif()
