include(GNUInstallDirs)

# Create tree interface library target for FixTree
add_library(smd_tree INTERFACE)

# Add public headers as a file set
target_sources(
    smd_tree
    PUBLIC FILE_SET smd_tree_headers TYPE HEADERS FILES
    binary_tree.hpp
    binary_tree_foldable.hpp
    binary_tree_applicative.hpp
    binary_tree_traversable.hpp
        fix_tree.hpp
        fix_tree_foldable.hpp
        fix_tree_applicative.hpp
        fix_tree_traversable.hpp
)

# Link with typeclass dependency
target_link_libraries(smd_tree INTERFACE smd_typeclass)

# Installation configuration
install(
    TARGETS smd_tree
    FILE_SET smd_tree_headers
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/smd/tree
        COMPONENT smd_tree_development
)
