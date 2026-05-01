# etc/clang-toolchain.cmake                                      -*-cmake-*-
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

include_guard(GLOBAL)

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)
set(GCOV_EXECUTABLE "llvm-cov gcov" CACHE STRING "GCOV executable" FORCE)

include("${CMAKE_CURRENT_LIST_DIR}/clang-flags.cmake")
