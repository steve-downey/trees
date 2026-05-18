# trees/src/smd/tree/CMakeLists.txt                             -*-CMake-*-
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

target_sources(
    smd.tree
    PUBLIC
        FILE_SET smd_tree_headers
            FILES
                binary_tree.hpp
                binary_tree_foldable.hpp
                binary_tree_applicative.hpp
                binary_tree_traversable.hpp
                fringe_tree.hpp
                fringe_tree_foldable.hpp
                fringe_tree_applicative.hpp
                fringe_tree_traversable.hpp
                finger_tree2.hpp
                finger_tree3.hpp
                finger_tree4.hpp
                finger_tree5.hpp
                finger_tree5_foldable.hpp
                finger_tree5_iterator.hpp
                finger_tree5_pmr.hpp
                finger_tree5_traversable.hpp
                finger_tree_foldable.hpp
                finger_tree_traversable.hpp
                finger_tree_random_access.hpp
                finger_tree_priority_queue.hpp
                finger_tree_interval_index.hpp
                finger_tree_rope.hpp
                finger_tree_wrappers.hpp
                fixpoint_tree.hpp
                fixpoint_tree_foldable.hpp
                fixpoint_tree_traversable.hpp
)

install(TARGETS smd.tree FILE_SET smd_tree_headers)

if(TREE_ENABLE_TESTING)
    add_executable(
        smd_tree_tests
        binary_tree.t.cpp
        binary_tree_applicative.t.cpp
        binary_tree_foldable.t.cpp
        binary_tree_traversable.t.cpp
        finger_tree2.t.cpp
        finger_tree3.t.cpp
        finger_tree4.t.cpp
        finger_tree5.t.cpp
        finger_tree5_foldable.t.cpp
        finger_tree5_iterator.t.cpp
        finger_tree5_pmr.t.cpp
        finger_tree5_traversable.t.cpp
        finger_tree_foldable.t.cpp
        finger_tree_interval_index.t.cpp
        finger_tree_interval_index_ft5.t.cpp
        finger_tree_priority_queue.t.cpp
        finger_tree_priority_queue_ft5.t.cpp
        finger_tree_random_access.t.cpp
        finger_tree_random_access_ft5.t.cpp
        finger_tree_rope.t.cpp
        finger_tree_rope_ft5.t.cpp
        finger_tree_traversable.t.cpp
        finger_tree_wrappers.t.cpp
        fixpoint_tree.t.cpp
        fixpoint_tree_algorithm.t.cpp
        fixpoint_tree_foldable.t.cpp
        fixpoint_tree_traversable.t.cpp
        fringe_tree.t.cpp
        fringe_tree_applicative.t.cpp
        fringe_tree_foldable.t.cpp
        fringe_tree_traversable.t.cpp
    )

    target_link_libraries(
        smd_tree_tests
        PRIVATE smd::tree Catch2::Catch2WithMain
    )

    target_compile_features(smd_tree_tests PRIVATE cxx_std_26)

    catch_discover_tests(smd_tree_tests)

    # -- PMR allocation probe -------------------------------------------------
    #
    # Standalone executable: no test framework, replaces operator new/delete
    # globally to count allocations.  Verifies that FingerTree5 operations
    # with a PMR allocator produce zero global heap allocations.
    #
    # Uses null_memory_resource as the upstream for the monotonic buffer;
    # any allocation that leaks through the PMR system throws bad_alloc.
    #
    # Must NOT be compiled with AddressSanitizer: Asan wraps operator
    # new/delete and may produce false non-zero counts.  The target is
    # compiled for Debug and RelWithDebInfo only; CTest runs it via the
    # RelWithDebInfo binary which has optimisations but no sanitizer.
    add_executable(smd_tree_pmr_probe finger_tree5_pmr_probe.cpp)
    target_link_libraries(smd_tree_pmr_probe PRIVATE smd::tree)
    target_compile_features(smd_tree_pmr_probe PRIVATE cxx_std_26)
    # No sanitizer flags on this target.
    target_compile_options(smd_tree_pmr_probe PRIVATE
        $<$<CXX_COMPILER_ID:GNU,Clang>:-fno-sanitize=all>)
    target_link_options(smd_tree_pmr_probe PRIVATE
        $<$<CXX_COMPILER_ID:GNU,Clang>:-fno-sanitize=all>)
    add_test(
        NAME   "FingerTree5 PMR allocation probe"
        COMMAND "$<TARGET_FILE:smd_tree_pmr_probe>"
        CONFIGURATIONS RelWithDebInfo Debug
    )

    # -- Compile-time regression probes ---------------------------------------
    #
    # One translation unit per tree variant, each instantiating all major
    # public operations with both default (unit-count) and custom measures.
    # These are OBJECT library targets: they compile on every test build,
    # catching template-expansion regressions before they accumulate.
    #
    # All four probes are identical in structure; only the included header and
    # probe function names differ.  The FT2/FT3 probes are expected to produce
    # substantially more DWARF type entries than FT4/FT5 due to the DEPTH NTTP
    # forcing kMaxDepth+1 = 6 distinct class instantiations per <T,Tag,Meas>.
    #
    # Measuring compile time (touch the file first to force a rebuild):
    #
    #   cd .build/build-gcc-16
    #   touch ../../src/smd/tree/finger_tree2_compile_probe.cpp
    #   time ninja -j1 src/smd/tree/CMakeFiles/smd_tree_compile_probes.dir/Debug/finger_tree2_compile_probe.cpp.o
    #
    # Counting DWARF class-type entries (proxy for template instantiation count):
    #
    #   readelf --debug-dump=info src/smd/tree/CMakeFiles/smd_tree_compile_probes.dir/Debug/finger_tree2_compile_probe.cpp.o | grep -c DW_TAG_class_type
    #
    # GCC per-phase time breakdown — add temporarily to inspect template cost:
    #   target_compile_options(smd_tree_compile_probes PRIVATE -ftime-report)
    # Remove after use; -ftime-report prints verbose output on every build.
    #
    # Baseline: build in the Debug config so optimisation does not collapse
    # instantiations and the DWARF reflects the true instantiation count.

    add_library(
        smd_tree_compile_probes OBJECT
        finger_tree2_compile_probe.cpp
        finger_tree3_compile_probe.cpp
        finger_tree4_compile_probe.cpp
        finger_tree5_compile_probe.cpp
    )

    target_link_libraries(smd_tree_compile_probes PRIVATE smd::tree)
    target_compile_features(smd_tree_compile_probes PRIVATE cxx_std_26)

    # Force debug info in every config so DWARF inspection always works.
    target_compile_options(smd_tree_compile_probes PRIVATE -g)
endif()

if(TREE_ENABLE_BENCHMARKS)
    add_executable(
        smd_tree_benchmarks
        finger_tree_compare.bench.cpp
        finger_tree_std_compare.bench.cpp
    )

    target_link_libraries(
        smd_tree_benchmarks
        PRIVATE smd::tree Catch2::Catch2WithMain
    )

    target_compile_features(smd_tree_benchmarks PRIVATE cxx_std_26)
    # Not registered with catch_discover_tests — run directly in RelWithDebInfo:
    #   .build/build-clang/RelWithDebInfo/smd_tree_benchmarks \
    #       --benchmark-samples 100 --benchmark-warmup-time 1
endif()
