include(GNUInstallDirs)

add_library(smd_fixpoint INTERFACE)

target_sources(
    smd_fixpoint
    PUBLIC FILE_SET smd_fixpoint_headers TYPE HEADERS FILES
    box.hpp
    overloaded.hpp
    fix.hpp
    cata.hpp
)

install(
    TARGETS smd_fixpoint
    FILE_SET smd_fixpoint_headers
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/smd/fixpoint
        COMPONENT smd_fixpoint_development
)

if(TREE_ENABLE_TESTING)
    add_executable(smd_fixpoint_tests
        box.t.cpp
        fix.t.cpp
        cata.t.cpp
    )

    target_include_directories(smd_fixpoint_tests
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/../..
    )

    target_link_libraries(smd_fixpoint_tests
        PRIVATE
            smd_fixpoint
            Catch2::Catch2WithMain
    )

    target_compile_features(smd_fixpoint_tests PRIVATE cxx_std_23)

    catch_discover_tests(smd_fixpoint_tests)
endif()
