include(GNUInstallDirs)

# Create typeclass interface library target
add_library(smd_typeclass INTERFACE)

# Add public headers as a file set
target_sources(
    smd_typeclass
    PUBLIC FILE_SET smd_typeclass_headers TYPE HEADERS FILES
        typeclass_base.hpp
        monoid.hpp
        functor.hpp
        foldable.hpp
        applicative.hpp
        traversable.hpp
        zip_list.hpp
        zip_list_applicative.hpp
        law_test_support.hpp
)

# Link with optional dependency
target_link_libraries(smd_typeclass INTERFACE beman.optional)

# Installation configuration
install(
    TARGETS smd_typeclass
    FILE_SET smd_typeclass_headers
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/smd/typeclass
        COMPONENT smd_typeclass_development
)

# Add subdirectories for examples and tests
add_subdirectory(examples)
