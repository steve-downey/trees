add_library(smd_typeclass_slide_examples OBJECT)

target_sources(smd_typeclass_slide_examples
    PRIVATE
        applicative_bad.cpp
        applicative_examples.cpp
        foldable_examples.cpp
        lookup_modes_examples.cpp
        traversable_examples.cpp
)

target_include_directories(smd_typeclass_slide_examples
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../../..
)

target_link_libraries(smd_typeclass_slide_examples
    PUBLIC
        smd_typeclass
)

target_compile_features(smd_typeclass_slide_examples PUBLIC cxx_std_23)

if(TREE_ENABLE_TESTING)
    add_executable(smd_typeclass_slide_examples_tests examples.t.cpp)

    target_include_directories(smd_typeclass_slide_examples_tests
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/../../..
    )

    target_link_libraries(smd_typeclass_slide_examples_tests
        PRIVATE
            smd_typeclass_slide_examples
            smd_typeclass
            Catch2::Catch2WithMain
    )

    target_compile_features(smd_typeclass_slide_examples_tests PRIVATE cxx_std_23)

    catch_discover_tests(smd_typeclass_slide_examples_tests)
endif()
