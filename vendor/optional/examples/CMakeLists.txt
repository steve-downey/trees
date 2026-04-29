# examples/CMakeLists.txt                                           -*-cmake-*-
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

include(GNUInstallDirs)

# List of all buildable examples.
set(EXAMPLES
    base_derived_cast
    concept_checks
    optional_ref
    pythagorean_triples
    range_loop
    sample
    std_vs_beman
)

foreach(example ${EXAMPLES})
    # Add example executable.
    add_executable(${example} "")

    # Add example source file.
    target_sources(${example} PRIVATE ${example}.cpp)

    # Link example with the library.
    target_link_libraries(${example} PRIVATE beman.optional)

    # Install .
    install(
        TARGETS ${example}
        COMPONENT beman.optional_examples
        DESTINATION ${CMAKE_INSTALL_BINDIR}
        EXCLUDE_FROM_ALL
    )
endforeach()
