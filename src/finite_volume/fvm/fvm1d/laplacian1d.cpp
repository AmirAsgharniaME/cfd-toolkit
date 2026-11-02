#include "cfd/finite_volume/fvm/fvm1d/laplacian1d.hpp"

#include "cfd/fv_matrix/fv_matrix1d.hpp"
#include "cfd/mesh/mesh1d/fv_mesh1d.hpp"

#include <cmath>
#include <cstddef>
#include <stdexcept>

namespace cfd::fvm {
namespace {

double face_diffusivity(double gamma_p, double gamma_n,
                        double distance_p, double distance_n)
{
    if (gamma_p == 0.0 || gamma_n == 0.0) {
        return 0.0;
    }

    // Resistance-weighted harmonic interpolation, with the cell-center to
    // face distances serving as the interpolation weights.
    const double distance = distance_p + distance_n;
    const double resistance = distance_p / gamma_p + distance_n / gamma_n;
    const double value = distance / resistance;

    if (!std::isfinite(value)) {
        throw std::overflow_error("Laplacian face diffusivity is not finite");
    }

    return value;
}

void validate_inputs(const field::VolScalarField1D& gamma,
                     const fv_matrix::FvMatrix1D& matrix)
{
    const mesh::FvMesh1D& mesh = matrix.Mesh();

    if (&gamma.Mesh() != &mesh) {
        throw std::invalid_argument(
            "Laplacian coefficient field and matrix must use the same mesh object");
    }

    if (gamma.Size() != mesh.N_Cells() || matrix.Size() != mesh.N_Cells()) {
        throw std::invalid_argument(
            "Laplacian field and matrix sizes must match the mesh");
    }

    for (std::size_t p = 0; p < gamma.Size(); ++p) {
        const double value = gamma[p];

        if (!std::isfinite(value) || value < 0.0) {
            throw std::invalid_argument(
                "Laplacian diffusion coefficients must be finite and nonnegative");
        }
    }
}

} // namespace

void laplacian(const field::VolScalarField1D& gamma,
               fv_matrix::FvMatrix1D& matrix)
{
    validate_inputs(gamma, matrix);

    const cfd::mesh::FvMesh1D& mesh = matrix.Mesh();
    const std::size_t n_cells = mesh.N_Cells();

    // Face between cells p and p + 1. This includes the internal faces
    // adjacent to the first and last cells, but not the domain boundaries.
    for (std::size_t p = 0; p + 1 < n_cells; ++p)
    {
        const std::size_t n = p + 1;

        const double gamma_face = face_diffusivity(
            gamma[p], gamma[n], mesh.eFaceDx(p), mesh.wFaceDx(n));

        const double conductance = gamma_face / mesh.eCellDx(p);

        if (!std::isfinite(conductance)) {
            throw std::overflow_error(
                "Laplacian internal-face conductance is not finite");
        }

        // Preserve the existing sign convention for div(gamma grad(phi)):
        // positive off-diagonals and negative diagonal contributions.
        matrix.add_a_E(p, conductance);
        matrix.add_a_P(p, -conductance);
        matrix.add_a_W(n, conductance);
        matrix.add_a_P(n, -conductance);
    }
}

} // namespace cfd::fvm
