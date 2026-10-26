#pragma once

#include "cfd/geometry/domain/domain1d.hpp"
#include "cfd/mesh/grading_type.hpp"

#include <vector>

namespace cfd::mesh::grading
{

/// Generate face coordinates for a one-dimensional mesh using `type`.
///
/// For graded modes, `ratio` is the factor by which cell widths grow when
/// moving away from the refined region. Thus Forward refines the start,
/// Backward refines the end, BothEnds refines both boundaries, and Center
/// refines the domain center. A ratio of 1.0 produces uniform widths for all
/// modes. Uniform ignores the ratio after checking that it is positive and
/// finite.
///
/// @param domain Valid, finite domain whose exact endpoints bound the mesh.
/// @param n_cells Number of cells; must be at least two.
/// @param type Cell-width distribution to generate.
/// @param ratio Positive finite width-growth factor; graded modes require a
///              value greater than or equal to 1.0.
/// @return `n_cells + 1` strictly increasing face coordinates, including the
///         exact domain endpoints. A successful return also guarantees that
///         every cell center is representable strictly inside its faces using
///         the arithmetic used by `FvMesh1D`.
/// @throws std::invalid_argument If an input is invalid or the faces cannot be
///         represented as a strictly increasing sequence of finite doubles
///         with distinct interior cell centers.
[[nodiscard]] std::vector<double> make_1d_faces(
    const geometry::Domain1D& domain,
    int n_cells,
    GradingType type,
    double ratio);

}  // namespace cfd::mesh::grading
