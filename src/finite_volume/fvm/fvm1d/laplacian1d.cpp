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

    const mesh::FvMesh1D& mesh = matrix.Mesh();
    const std::size_t n_face = mesh.N_Faces();

    // Assemble internal faces only. Boundary contributions are handled
    // separately by the boundary-condition code.
    for (mesh::FaceId f = 1; f + 1 < n_face; ++f)
    {
        const mesh::CellId p_W = mesh.Owner(f);
        const mesh::CellId p_E = *mesh.Neighbour(f);

        const double distance_W = mesh.Dx_W(f);
        const double distance_E = mesh.Dx_E(f);
        const double face_area = mesh.Face_Area(f);

        // The harmonic scheme accepts strictly positive diffusivities.
        // A zero coefficient on either side gives zero face diffusivity.
        const double gamma_face = (gamma[p_W] == 0.0 || gamma[p_E] == 0.0)
            ? 0.0
            : schemes::interpolation::harmonic_mean(
                  gamma[p_W], gamma[p_E], distance_W, distance_E);

        // For a unit difference between cell values, this is the
        // center-to-center gradient. Face conductance includes face area:
        // D_f = gamma_f * A_f / (distance_W + distance_E).
        const double gradient_per_unit_jump =
            schemes::gradient::central_difference(
                0.0, 1.0, distance_W, distance_E);

        const double conductance =
            gamma_face * face_area * gradient_per_unit_jump;

        if (!std::isfinite(conductance)) {
            throw std::overflow_error(
                "Laplacian internal-face conductance is not finite");
        }

        // Owner outward flux: D_f * (phi_owner - phi_neighbour).
        // The neighbour receives the opposite flux.
        matrix.add_a_P(p_W, conductance);
        matrix.add_a_E(p_W, -conductance);
        matrix.add_a_P(p_E, conductance);
        matrix.add_a_W(p_E, -conductance);
    }
}

} // namespace cfd::fvm
