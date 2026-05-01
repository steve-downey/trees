include(GNUInstallDirs)

add_library(smd_ranges INTERFACE)

target_sources(
    smd_ranges
    PUBLIC FILE_SET smd_ranges_headers TYPE HEADERS FILES
        range_list.hpp
        range_functor.hpp
        range_applicative.hpp
        range_foldable.hpp
        range_traversable.hpp
)

target_link_libraries(smd_ranges INTERFACE smd_typeclass)

install(
    TARGETS smd_ranges
    FILE_SET smd_ranges_headers
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/smd/ranges
        COMPONENT smd_ranges_development
)

if(TREE_ENABLE_TESTING)
    add_executable(smd_ranges_tests
        range_functor.t.cpp
        range_applicative.t.cpp
        range_foldable.t.cpp
        range_traversable.t.cpp
    )

    target_include_directories(smd_ranges_tests
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/../..
    )

    target_link_libraries(smd_ranges_tests
        PRIVATE
            smd_ranges
            Catch2::Catch2WithMain
    )

    target_compile_features(smd_ranges_tests PRIVATE cxx_std_23)

    catch_discover_tests(smd_ranges_tests)
endif()
