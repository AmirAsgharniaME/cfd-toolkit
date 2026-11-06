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
    //const std::size_t n_cells = mesh.N_Cells();
    const std::size_t n_face = mesh.N_Faces();

    // Face between cells p and n. The mesh supplies the center-to-face
    // distances used by harmonic interpolation and the central-difference
    // gradient.
    for (std::size_t f = 1; f < n_face-1 ; ++f)
    {
        const std::size_t p_W = f - 1;
        const std::size_t p_E = f;
        const double distance_W = mesh.Dx_W(f);
        const double distance_E = mesh.Dx_E(f);

        // The public harmonic scheme accepts strictly positive diffusivities;
        // preserve the Laplacian's nonnegative-coefficient contract by
        // treating a zero on either side as zero face diffusivity.
        const double gamma_face = (gamma[p_W] == 0.0 || gamma[p_E] == 0.0)
            ? 0.0
            : schemes::interpolation::harmonic_mean(
                  gamma[p_W], gamma[p_E], distance_W, distance_E);

        // A unit jump across the neighboring cell centers gives the gradient
        // per unit field difference. In 1D the face area is one, so this is
        // Gamma_f * Area_f / the center-to-center distance.
        const double gradient_per_unit_jump =
            schemes::gradient::central_difference(
                0.0, 1.0, distance_W, distance_E);
        const double conductance = gamma_face * gradient_per_unit_jump;

        if (!std::isfinite(conductance)) {
            throw std::overflow_error(
                "Laplacian internal-face conductance is not finite");
        }

        // The integrated diffusion flux contributes +coefficients to the
        // diagonal and -coefficients to the neighboring off-diagonal.
        matrix.add_a_P(p_W, conductance);
        matrix.add_a_E(p_W, -conductance);
        matrix.add_a_P(p_E, conductance);
        matrix.add_a_W(p_E, -conductance);
    }
}

} // namespace cfd::fvm
