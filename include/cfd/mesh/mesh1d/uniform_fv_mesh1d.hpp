#pragma once

#include "cfd/geometry/domain/domain1d.hpp"
#include "cfd/mesh/mesh1d/fv_mesh1d.hpp"

#include <cstddef>
#include <vector>

namespace cfd::mesh
{

/// Uniform one-dimensional, cell-centered finite-volume mesh.
/// Builds faces with nominally constant spacing over the domain; FvMesh1D
/// derives the actual widths and centers from the represented coordinates.
/// Takes an independent snapshot of the domain's authoritative endpoints;
/// later domain mutations do not change this mesh. Inherited start(), end(),
/// and length() describe that snapshot using the Domain1D naming convention.
/// Volumes assume Cartesian cells with unit cross-sectional area (A = 1 m^2
/// when coordinates are in meters).
class UniformFvMesh1D final : public FvMesh1D
{
public:
    /// Use domain.length() / num_cells as the nominal spacing. A valid domain
    /// can still be too small to represent distinct faces or interior centers.
    /// Throws std::invalid_argument for zero cells or unrepresentable geometry,
    /// and std::length_error if the requested face count is too large.
    explicit UniformFvMesh1D(const cfd::geometry::Domain1D& domain,
                            std::size_t num_cells);

    /// Equivalent to constructing from Domain1D(start, end); uses its finite,
    /// strictly increasing bounds and finite represented length validation.
    explicit UniformFvMesh1D(double start, double end, std::size_t num_cells);

    UniformFvMesh1D(const UniformFvMesh1D&) = default;
    UniformFvMesh1D(UniformFvMesh1D&&) noexcept = default;
    UniformFvMesh1D& operator=(const UniformFvMesh1D&) = delete;
    UniformFvMesh1D& operator=(UniformFvMesh1D&&) noexcept = default;
    ~UniformFvMesh1D() override = default;

    /// Nominal/target spacing: domain.length() / num_cells.
    /// Uses the represented endpoint difference, not a requested domain length.
    /// Rounding can affect every face difference; solvers should use
    /// cell_width(P) for the actual finite-volume metric.
    [[nodiscard]] double dx() const noexcept { return nominal_spacing_; }
    /// Alias for dx(); this is nominal spacing, not a stored cell width.
    [[nodiscard]] double spacing() const noexcept { return nominal_spacing_; }
    /// Alias for dx(); use cell_width(P) for actual control-volume widths.
    [[nodiscard]] double nominal_spacing() const noexcept { return nominal_spacing_; }

private:
    /// Receives a validated spacing computed once by the public constructor.
    UniformFvMesh1D(const cfd::geometry::Domain1D& domain, std::size_t num_cells,
                   double spacing);

    static double compute_spacing(const cfd::geometry::Domain1D& domain,
                                  std::size_t num_cells);
    static std::vector<double> make_faces(const cfd::geometry::Domain1D& domain,
                                         std::size_t num_cells, double spacing);

    double nominal_spacing_;
};

/// Type aliases matching OpenFOAM nomenclature
using UniformMesh1D = UniformFvMesh1D;
using Mesh1D = UniformFvMesh1D;

}  // namespace cfd::mesh
