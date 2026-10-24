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
/// Volumes assume Cartesian cells with unit cross-sectional area (A = 1 m^2
/// when coordinates are in meters).
class UniformFvMesh1D final : public FvMesh1D
{
public:
    explicit UniformFvMesh1D(const cfd::geometry::Domain1D& domain,
                             std::size_t num_cells);

    explicit UniformFvMesh1D(double x_start, double x_end, std::size_t num_cells);

    UniformFvMesh1D(const UniformFvMesh1D&) = default;
    UniformFvMesh1D(UniformFvMesh1D&&) noexcept = default;
    UniformFvMesh1D& operator=(const UniformFvMesh1D&) = delete;
    UniformFvMesh1D& operator=(UniformFvMesh1D&&) noexcept = default;
    ~UniformFvMesh1D() override = default;

    /// Nominal/target spacing: (x_end - x_start) / num_cells.
    /// Rounding can affect every face difference; solvers should use
    /// cell_width(P) for the actual finite-volume metric.
    [[nodiscard]] double dx() const noexcept { return nominal_spacing_; }
    /// Alias for dx(); this is nominal spacing, not a stored cell width.
    [[nodiscard]] double spacing() const noexcept { return nominal_spacing_; }
    /// Alias for dx(); use cell_width(P) for actual control-volume widths.
    [[nodiscard]] double nominal_spacing() const noexcept { return nominal_spacing_; }

private:
    /// Receives a validated spacing computed once by the public constructor.
    UniformFvMesh1D(double x_start, double x_end, std::size_t num_cells,
                    double spacing);

    static double compute_spacing(double x_start, double x_end, std::size_t num_cells);
    static std::vector<double> make_faces(double x_start, double x_end,
                                          std::size_t num_cells, double spacing);

    double nominal_spacing_;
};

/// Type aliases matching OpenFOAM nomenclature
using UniformMesh1D = UniformFvMesh1D;
using Mesh1D = UniformFvMesh1D;

}  // namespace cfd::mesh
