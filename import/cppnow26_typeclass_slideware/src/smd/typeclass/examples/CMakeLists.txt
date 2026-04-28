add_library(smd_typeclass_slide_examples
    applicative_bad.cpp
    applicative_examples.cpp
    foldable_examples.cpp
    traversable_examples.cpp
)

target_include_directories(smd_typeclass_slide_examples
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../..
)

add_executable(smd_typeclass_slide_examples_tests examples.t.cpp)
target_link_libraries(smd_typeclass_slide_examples_tests
    PRIVATE
        smd_typeclass_slide_examples
        Catch2::Catch2WithMain
)
