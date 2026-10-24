#pragma once

#include "cfd/geometry/domain/domain1d.hpp"
#include "cfd/mesh/mesh1d/fv_mesh1d.hpp"

#include <cstddef>

namespace cfd::mesh
{

/// Uniform one-dimensional, cell-centered finite-volume mesh.
/// Builds faces with constant spacing over the domain, then leverages FvMesh1D.
class UniformFvMesh1D final : public FvMesh1D
{
public:
    explicit UniformFvMesh1D(const cfd::geometry::Domain1D& domain,
                             std::size_t num_cells);

    explicit UniformFvMesh1D(double x_start, double x_end, std::size_t num_cells);

    UniformFvMesh1D(const UniformFvMesh1D&) = default;
    UniformFvMesh1D(UniformFvMesh1D&&) noexcept = default;
    UniformFvMesh1D& operator=(const UniformFvMesh1D&) = delete;
    UniformFvMesh1D& operator=(UniformFvMesh1D&&) = delete;
    ~UniformFvMesh1D() override = default;

    /// dx = (x_end - x_start) / num_cells (nominal, exact everywhere except pinned end).
    [[nodiscard]] double dx() const noexcept { return nominal_spacing_; }
    [[nodiscard]] double spacing() const noexcept { return nominal_spacing_; }
    [[nodiscard]] double nominal_spacing() const noexcept { return nominal_spacing_; }

private:
    static double compute_spacing(double x_start, double x_end, std::size_t num_cells);
    static std::vector<double> make_faces(double x_start, double x_end,
                                          std::size_t num_cells, double spacing);

private:
    const double nominal_spacing_;
};

/// Type aliases matching OpenFOAM nomenclature
using UniformMesh1D = UniformFvMesh1D;
using Mesh1D = UniformFvMesh1D;

}  // namespace cfd::mesh
