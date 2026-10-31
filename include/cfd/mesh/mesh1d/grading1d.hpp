#pragma once

#include "cfd/geometry/domain/domain1d.hpp"
#include "cfd/mesh/grading_type.hpp"

#include <cstddef>
#include <vector>

namespace cfd::mesh::grading
{

/// Convert a requested largest-to-smallest cell-width ratio to the
/// per-exponent-step ratio used by `Make_Faces`.
///
/// `total_ratio` means the largest generated cell width divided by the
/// smallest. The returned ratio is the cell-to-cell growth factor for one
/// exponent increment, calculated as
/// `total_ratio^(1 / exponent_span)`. This definition is independent of
/// physical left-to-right direction: Forward and Backward have the same
/// exponent span and return the same value for the same inputs.
///
/// @param n_cells Number of cells; must be at least one. Patterns with no
///                positive exponent span (e.g. BothEnds/Center with fewer than
///                three cells) return 1.0 for R == 1.0 and throw otherwise.
/// @param type Cell-width distribution whose exponent span determines the
///             conversion.
/// @param R Requested largest-to-smallest cell-width ratio; must be
///                    finite and greater than or equal to 1.0.
/// @return The finite per-exponent-step ratio to pass to `Make_Faces`.
/// @throws std::invalid_argument If an input is invalid, Uniform is requested
///         with a non-unity total ratio, or the grading pattern has no
///         positive exponent span and R is not 1.0.
[[nodiscard]] double Convert_R_To_r(
    std::size_t n_cells,
    GradingType type,
    const double R);

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
/// @param n_cells Number of cells; must be at least one. A single cell spans
///                the whole domain for every grading type.
/// @param type Cell-width distribution to generate.
/// @param r Positive finite width-growth factor; graded modes require a
///              value greater than or equal to 1.0.
/// @return `n_cells + 1` strictly increasing face coordinates, including the
///         exact domain endpoints. A successful return also guarantees that
///         every cell center is representable strictly inside its faces using
///         the arithmetic used by `FvMesh1D`.
/// @throws std::invalid_argument If an input is invalid or the faces cannot be
///         represented as a strictly increasing sequence of finite doubles
///         with distinct interior cell centers.
/// @throws std::length_error If the requested cell count exceeds the maximum
///         size supported by the face or weight vectors.
[[nodiscard]] std::vector<double> Make_Faces(
    const geometry::Domain1D& domain,
    std::size_t n_cells,
    GradingType type,
    double r);

}  // namespace cfd::mesh::grading
