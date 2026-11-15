#include "cfd/finite_volume/fvm/fvm1d/source1d.hpp"

#include "cfd/fv_matrix/fv_matrix1d.hpp"
#include "cfd/mesh/mesh1d/fv_mesh1d.hpp"

#include <cmath>
#include <cstddef>
#include <stdexcept>

namespace cfd::fvm
{

Source1D::Source1D(double value) noexcept
    : value_(value)
{
}

double Source1D::Value() const noexcept
{
    return value_;
}

void Source1D::Apply(fv_matrix::FvMatrix1D& matrix) const
{
    const mesh::FvMesh1D& mesh = matrix.Mesh();
    const std::size_t n_cells = mesh.N_Cells();

    if (!std::isfinite(value_))
    {
        throw std::invalid_argument("Volumetric source must be finite");
    }

    if (matrix.size() != n_cells)
    {
        throw std::invalid_argument(
            "Volumetric source matrix size must match the mesh");
    }

    // Validate the whole contribution before touching the matrix so that a
    // rejected call cannot leave a partially assembled source vector.
    for (std::size_t p = 0; p < n_cells; ++p)
    {
        const double cell_volume = mesh.Cell_Volume(p);
        if (!std::isfinite(cell_volume) || !(cell_volume > 0.0))
        {
            throw std::invalid_argument(
                "Volumetric source cell volumes must be finite and positive");
        }

        const double contribution = value_ * cell_volume;
        if (!std::isfinite(contribution))
        {
            throw std::overflow_error(
                "Volumetric source contribution is not finite");
        }

        if (!std::isfinite(matrix.b(p) + contribution))
        {
            throw std::overflow_error(
                "Accumulated volumetric source is not finite");
        }
    }

    // The source is independent of the unknown, so only the right-hand side
    // changes: b_P += S * V_P.
    for (std::size_t p = 0; p < n_cells; ++p)
    {
        matrix.Add_b(p, value_ * mesh.Cell_Volume(p));
    }
}

} // namespace cfd::fvm
