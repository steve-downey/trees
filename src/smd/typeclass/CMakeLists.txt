# trees/src/smd/typeclass/CMakeLists.txt                        -*-CMake-*-
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

target_sources(
    smd.typeclass
    PUBLIC
        FILE_SET smd_typeclass_headers
            FILES
                typeclass_base.hpp
                monoid.hpp
                functor.hpp
                foldable.hpp
                applicative.hpp
                traversable.hpp
)

install(TARGETS smd.typeclass FILE_SET smd_typeclass_headers)

add_subdirectory(examples)

if(TREE_ENABLE_TESTING)
    add_executable(
        smd_typeclass_tests
        typeclass_base.t.cpp
        functor.t.cpp
        monoid.t.cpp
        applicative.t.cpp
        foldable.t.cpp
        traversable.t.cpp
    )

    target_link_libraries(
        smd_typeclass_tests
        PRIVATE smd::typeclass Catch2::Catch2WithMain
    )

    target_compile_features(smd_typeclass_tests PRIVATE cxx_std_23)

    catch_discover_tests(smd_typeclass_tests)
endif()
