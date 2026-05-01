include(GNUInstallDirs)

add_library(smd_ziplist INTERFACE)

target_sources(
    smd_ziplist
    PUBLIC FILE_SET smd_ziplist_headers TYPE HEADERS FILES
        zip_list.hpp
        zip_list_applicative.hpp
)

target_link_libraries(smd_ziplist INTERFACE smd_typeclass)

install(
    TARGETS smd_ziplist
    FILE_SET smd_ziplist_headers
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/smd/ziplist
        COMPONENT smd_ziplist_development
)

if(TREE_ENABLE_TESTING)
    add_executable(smd_ziplist_tests zip_list_applicative.t.cpp)

    target_include_directories(smd_ziplist_tests
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/../..
    )

    target_link_libraries(smd_ziplist_tests
        PRIVATE
            smd_ziplist
            Catch2::Catch2WithMain
    )

    target_compile_features(smd_ziplist_tests PRIVATE cxx_std_23)

    catch_discover_tests(smd_ziplist_tests)
endif()
