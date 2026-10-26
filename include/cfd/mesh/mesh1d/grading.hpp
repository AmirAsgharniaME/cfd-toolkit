#pragma once

#include "cfd/geometry/domain/domain1d.hpp"

#include <vector>

namespace cfd::mesh::grading
{

/// Generate face positions for a one-dimensional geometrically graded mesh.
/// Successive cell widths are multiplied by `ratio`.
///
/// @param domain Domain whose start and end coordinates bound the mesh.
/// @param n_cells Number of cells; must be at least two.
/// @param ratio Positive finite ratio between successive cell widths. A ratio
///              of 1.0 produces uniform spacing.
/// @return The `n_cells + 1` face coordinates, including the exact endpoints.
/// @throws std::invalid_argument If inputs are invalid or the faces cannot be
///         represented as a strictly increasing sequence of finite doubles.
[[nodiscard]] std::vector<double> geometric(const geometry::Domain1D& domain,
                                            int n_cells,
                                            double ratio);

}  // namespace cfd::mesh::grading
