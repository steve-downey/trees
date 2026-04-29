Use the following as the short operational ruleset:

- Generate new C++ code in a merged src/ layout. Put headers, implementations, and tests together under src/<namespace-path>/..., not under a split include/ and tests/ layout, unless you are explicitly working inside an older optional or Beman-style subtree.
- Model each logical component as a local trio: <name>.hpp or <name>.h, <name>.cpp, and <name>.test.cpp or <name>.t.cpp, all in the same directory. Prefer .hpp and .test.cpp for new standalone work.
- Put a canonical repository-relative path comment plus an Emacs mode line on line one of every source-like file. Put SPDX immediately after that, on the first possible comment-capable line.
- Make the canonical include path reflect the namespace and treat that canonical include spelling as authoritative. Never include project headers with . or .., and never depend on non-canonical relative include paths.
- Write target-based CMake. Define a target first, then attach sources and headers with target_sources, using file sets for headers and, when relevant, C++ modules. Turn on interface-header verification.
- Keep each CMakeLists.txt local. List only local files there, and only delegate to immediate child directories. Do not reach upward with .. or sideways into other subtrees.
- Use INTERFACE libraries only for genuinely header-only code. Otherwise use a real library target. Export and install targets, not loose files.
- Prefer find_package for dependencies. Any fallback download mechanism should be opt-in and outside the normal target definitions.
- Declare functions before defining them. Keep normal member and free-function definitions out of class bodies and out of anonymous namespace blocks used as definition scaffolding. Define them out of line and qualify them with their namespace and class names.
- Allow exactly one ordinary exception to the out-of-line rule: hidden friends for customization points. Keep them short, idiomatic, and clearly marked.
- Make headers self-contained, include the component’s own header first in the .cpp, do not rely on transitive includes, and do not use using namespace in headers.
- Prefer C++23 for new code and adopt C++26 facilities when the project toolchain allows it. If an API can meaningfully be constexpr, make it constexpr and add compile-time tests that prove it.
- Treat formatter and lint configuration as binding. Assume clang-format, CMake formatting, spell check, and pre-commit automation are part of the contract, and generate code that is easy for those tools to normalize rather than hand-formatting to older style manuals.
