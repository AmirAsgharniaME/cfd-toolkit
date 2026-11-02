#include "cfd/finite_volume/fvm/fvm1d/laplacian1d.hpp"

#include "cfd/fv_matrix/fv_matrix1d.hpp"
#include "cfd/mesh/mesh1d/fv_mesh1d.hpp"
#include "cfd/schemes/gradient/central_difference.hpp"
#include "cfd/schemes/interpolation/harmonic_mean.hpp"

#include <cmath>
#include <cstddef>
#include <stdexcept>

namespace cfd::fvm {
namespace {

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

    // Face between cells p and n. The mesh supplies the center-to-face
    // distances used by harmonic interpolation and the center-to-center
    // distance used by the central-difference gradient.
    for (std::size_t p = 0; p + 1 < n_cells; ++p)
    {
        const std::size_t n = p + 1;
        const double distance_p = mesh.eFaceDx(p);
        const double distance_n = mesh.wFaceDx(n);
        const double distance_pn = mesh.eCellDx(p);

        // The public harmonic scheme accepts strictly positive diffusivities;
        // preserve the Laplacian's nonnegative-coefficient contract by
        // treating a zero on either side as zero face diffusivity.
        const double gamma_face = (gamma[p] == 0.0 || gamma[n] == 0.0)
            ? 0.0
            : schemes::interpolation::harmonic_mean(
                  gamma[p], gamma[n], distance_p, distance_n);

        // A unit jump across the neighboring cell centers gives the gradient
        // per unit field difference. In 1D the face area is one, so this is
        // Gamma_f * Area_f / distance_pn.
        const double gradient_per_unit_jump =
            schemes::gradient::central_difference(0.0, 1.0, distance_pn);
        const double conductance = gamma_face * gradient_per_unit_jump;

        if (!std::isfinite(conductance)) {
            throw std::overflow_error(
                "Laplacian internal-face conductance is not finite");
        }

        // The integrated diffusion flux contributes +coefficients to the
        // diagonal and -coefficients to the neighboring off-diagonal.
        matrix.add_a_P(p, conductance);
        matrix.add_a_E(p, -conductance);
        matrix.add_a_P(n, conductance);
        matrix.add_a_W(n, -conductance);
    }
}

} // namespace cfd::fvm
