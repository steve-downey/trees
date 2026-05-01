# Src Cleanup Pass TODO

This checklist captures trees/src files that are present on main but are not explicitly listed in CMake target_sources() for live build targets.

Scope used for this list:
- Included candidates: trees/src/**/*.hpp, trees/src/**/*.h, trees/src/**/*.cpp
- Excluded candidates: trees/src/deadcode/**, trees/src/smd/conceptmap/**, trees/src/**/CMakeLists.txt
- Built set: explicit file entries in current trees/src/**/CMakeLists.txt target_sources() blocks

## Removal Candidates (verify before deleting)
- [x] smd/tree/finger_tree_interval_index_foldable.hpp — NOT removed; actively #include'd by finger_tree_interval_index.hpp and finger_tree_wrappers.hpp. Added to FILE_SET HEADERS.
- [x] smd/tree/finger_tree_interval_index_traversable.hpp — same, added to FILE_SET HEADERS.
- [x] smd/tree/finger_tree_priority_queue_foldable.hpp — same, added to FILE_SET HEADERS.
- [x] smd/tree/finger_tree_priority_queue_traversable.hpp — same, added to FILE_SET HEADERS.
- [x] smd/tree/finger_tree_random_access_foldable.hpp — same, added to FILE_SET HEADERS.
- [x] smd/tree/finger_tree_random_access_traversable.hpp — same, added to FILE_SET HEADERS. Also: removed misleading "lazy traversal" comment; inlined traverse_tree_elements helper.
- [x] smd/tree/finger_tree_rope_foldable.hpp — same, added to FILE_SET HEADERS.
- [x] smd/tree/finger_tree_rope_traversable.hpp — same, added to FILE_SET HEADERS.
- [x] smd/typeclass/examples/examples.hpp — NOT removed; included by 5+ example .cpp files.
- [x] smd/typeclass/test/test_monoids.hpp — DELETED. Empty stub (include guard + test_support.hpp only), never included.
- [x] smd/typeclass/test/test_optional_applicative.hpp — DELETED. Empty stub, never included.
- [x] smd/typeclass/test/test_support.hpp — NOT removed; included by 4 test files.

All items resolved. 172/172 tests pass after changes.
