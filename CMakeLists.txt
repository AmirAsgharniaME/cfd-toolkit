cmake_minimum_required(VERSION 3.20)

project(cfd-toolkit
    VERSION 0.1.0
    DESCRIPTION "Foundational C++20 infrastructure for computational fluid dynamics"
    LANGUAGES CXX
)

option(BUILD_EXAMPLES "Build cfd-toolkit example/sanity executables" ON)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Keep this source list explicit so new modules are added intentionally and
# source discovery remains deterministic across generators and platforms.
add_library(cfd_toolkit STATIC
    # Boundary conditions
    src/boundary/boundary1d/dirichlet_boundary1d.cpp
    src/boundary/boundary1d/neumann_boundary1d.cpp
    src/boundary/boundary1d/patch_boundary1d.cpp

    # Volume and surface fields
    src/field/field1d/surface_scalar_field1d.cpp
    src/field/field1d/surface_vector_field1d.cpp
    src/field/field1d/vol_scalar_field1d.cpp
    src/field/field1d/vol_vector_field1d.cpp

    # Explicit and implicit finite volume operators
    src/finite_volume/fvc/fvc1d/div1d.cpp
    src/finite_volume/fvc/fvc1d/grad1d.cpp
    src/finite_volume/fvm/fvm1d/ddt1d.cpp
    src/finite_volume/fvm/fvm1d/div1d.cpp
    src/finite_volume/fvm/fvm1d/laplacian1d.cpp
    src/finite_volume/fvm/fvm1d/source1d.cpp

    # Discretized equation containers
    src/fv_matrix/fv_matrix1d.cpp
    src/fv_matrix/fv_matrix2d.cpp
    src/fv_matrix/fv_matrix3d.cpp

    # Geometry
    src/geometry/domain/domain1d.cpp
    src/geometry/domain/domain2d.cpp
    src/geometry/domain/domain3d.cpp

    # Output writers
    src/io/gnuplot_writer.cpp
    src/io/time_directory_writer.cpp
    src/io/vtk_writer.cpp

    # Algebraic matrices, vectors, and solvers
    src/linear_algebra/matrix/sparse_matrix.cpp
    src/linear_algebra/matrix/tridiagonal_matrix.cpp
    src/linear_algebra/solvers/conjugate_gradient.cpp
    src/linear_algebra/solvers/gauss_seidel_solver.cpp
    src/linear_algebra/solvers/tdma_solver.cpp
    src/linear_algebra/vector.cpp

    # Finite volume meshes
    src/mesh/mesh1d/fv_mesh1d.cpp
    src/mesh/mesh1d/non_uniform_fv_mesh1d.cpp
    src/mesh/mesh1d/uniform_fv_mesh1d.cpp
    src/mesh/mesh2d/uniform_fv_mesh2d.cpp
    src/mesh/mesh3d/uniform_fv_mesh3d.cpp

    # Interpolation schemes
    src/schemes/central_difference_scheme.cpp
    src/schemes/interpolation_scheme.cpp
    src/schemes/upwind_scheme.cpp

    # Equation solver workflows
    src/solvers/solvers1d/advection_diffusion_solver1d.cpp
    src/solvers/solvers1d/laplace_solver1d.cpp
    src/solvers/solvers1d/pimple_solver1d.cpp

    # Simulation time and transport properties
    src/time/time.cpp
    src/transport_model/transport_properties.cpp
)

# Consumers use the same namespaced target in add_subdirectory, FetchContent,
# and after find_package(cfd-toolkit CONFIG).
add_library(cfd::toolkit ALIAS cfd_toolkit)
set_target_properties(cfd_toolkit PROPERTIES
    EXPORT_NAME toolkit
)

# Publish the language requirement to consumers, while leaving the global
# CMAKE_CXX_STANDARD setting untouched for sibling projects.
target_compile_features(cfd_toolkit PUBLIC cxx_std_20)

# Keep source-tree paths out of installed interfaces and downstream targets.
target_include_directories(cfd_toolkit
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Apply diagnostics only when compiling this library with GNU or Clang.
target_compile_options(cfd_toolkit
    PRIVATE
        $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall>
        $<$<CXX_COMPILER_ID:GNU,Clang>:-Wextra>
        $<$<CXX_COMPILER_ID:GNU,Clang>:-Wpedantic>
        $<$<CXX_COMPILER_ID:GNU,Clang>:-Wconversion>
        $<$<CXX_COMPILER_ID:GNU,Clang>:-Wshadow>
)

set(cfd_toolkit_cmake_dir "${CMAKE_INSTALL_LIBDIR}/cmake/cfd-toolkit")

# Install the library and its public headers, and export an imported target
# named cfd::toolkit for package consumers.
install(TARGETS cfd_toolkit
    EXPORT cfd-toolkitTargets
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

install(EXPORT cfd-toolkitTargets
    FILE cfd-toolkitTargets.cmake
    NAMESPACE cfd::
    DESTINATION "${cfd_toolkit_cmake_dir}"
)

# Generate the config and version files required by find_package(CONFIG).
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cfd-toolkitConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cfd-toolkitConfig.cmake"
    INSTALL_DESTINATION "${cfd_toolkit_cmake_dir}"
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/cfd-toolkitConfigVersion.cmake"
    VERSION "${PROJECT_VERSION}"
    COMPATIBILITY SameMajorVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/cfd-toolkitConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/cfd-toolkitConfigVersion.cmake"
    DESTINATION "${cfd_toolkit_cmake_dir}"
)

if(BUILD_EXAMPLES)
    add_executable(test_domain1d
        examples/test_geometry/test_domain/test_domain1d/test_domain1d.cpp
    )
    target_link_libraries(test_domain1d PRIVATE cfd_toolkit)
    set_target_properties(test_domain1d PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )

    add_executable(test_uniform_mesh1d
        examples/test_mesh/test_mesh1d/test_uniform_mesh1d/test_uniform_mesh1d.cpp
    )
    target_link_libraries(test_uniform_mesh1d PRIVATE cfd_toolkit)
    set_target_properties(test_uniform_mesh1d PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )

    add_executable(test_non_uniform_mesh1d
        examples/test_mesh/test_mesh1d/test_non_uniform_mesh1d/test_non_uniform_mesh1d.cpp
    )
    target_link_libraries(test_non_uniform_mesh1d PRIVATE cfd_toolkit)
    set_target_properties(test_non_uniform_mesh1d PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )

    add_executable(test_scalar_field1d
        examples/test_field/test_field1d/test_scalar_field1d/test_scalar_field1d.cpp
    )
    target_link_libraries(test_scalar_field1d PRIVATE cfd_toolkit)
    set_target_properties(test_scalar_field1d PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )

endif()
