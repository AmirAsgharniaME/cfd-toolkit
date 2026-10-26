#pragma once

#include "cfd/geometry/domain/domain1d.hpp"
#include "cfd/mesh/mesh1d/fv_mesh1d.hpp"

#include <vector>

namespace cfd::mesh
{

/// One-dimensional finite-volume mesh with caller-specified face positions.
/// The face array must contain at least two finite, strictly increasing
/// coordinates, and its first and last coordinates must exactly equal the
/// domain's start and end. Construction snapshots the domain bounds and moves
/// or copies the face coordinates into the base mesh; later changes to the
/// domain or the caller's vector do not alter this mesh. Cell widths and
/// centers are derived from the represented faces by FvMesh1D. Volumes assume
/// unit cross-sectional area.
class NonUniformFvMesh1D final : public FvMesh1D
{
public:
    /// Construct from explicit faces spanning the complete domain.
    /// Throws std::invalid_argument for fewer than two faces, non-finite or
    /// non-increasing coordinates, mismatched endpoints, or geometry whose
    /// cell centers cannot be represented distinctly.
    NonUniformFvMesh1D(const geometry::Domain1D& domain,
                       std::vector<double> faces);

    NonUniformFvMesh1D(const NonUniformFvMesh1D&) = default;
    NonUniformFvMesh1D(NonUniformFvMesh1D&&) noexcept = default;
    NonUniformFvMesh1D& operator=(const NonUniformFvMesh1D&) = delete;
    NonUniformFvMesh1D& operator=(NonUniformFvMesh1D&&) noexcept = default;
    ~NonUniformFvMesh1D() override = default;
};

}  // namespace cfd::mesh
