#include "cfd/finite_volume/fvm/fvm1d/laplacian1d.hpp"

#include "cfd/fv_matrix/fv_matrix1d.hpp"
#include "cfd/mesh/mesh1d/fv_mesh1d.hpp"
#include "cfd/schemes/gradient/central_difference.hpp"
#include "cfd/schemes/interpolation/harmonic_mean.hpp"

#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <cassert>
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

    if (gamma.size() != mesh.N_Cells() || matrix.size() != mesh.N_Cells()) 
    {
        throw std::invalid_argument(
            "Laplacian field and matrix sizes must match the mesh");
    }

    for (std::size_t p = 0; p < gamma.size(); ++p) 
    {
        const double value = gamma[p];

        if (!std::isfinite(value) || value < 0.0) {
            throw std::invalid_argument(
                "Laplacian diffusion coefficients must be finite and nonnegative");
        }
    }
}

} // namespace

void Laplacian(const field::VolScalarField1D& gamma,
               fv_matrix::FvMatrix1D& matrix)
{
    validate_inputs(gamma, matrix);

    const mesh::FvMesh1D& mesh = matrix.Mesh();
    const std::size_t n_face = mesh.N_Faces();

    // Assemble internal faces only. Boundary-condition APIs add applicable
    // diagonal and source contributions to the boundary cell row separately.
// Only internal cells are iterated. Their faces are all internal.
// Loop only over internal faces: from face 1 to n_face - 1
for (mesh::FaceId f = 1; f < n_face - 1; ++f)
{
    const mesh::CellId owner     = mesh.Owner(f);
    const mesh::CellId neighbour = *mesh.Neighbour(f);

    // Face geometry: independent of the direction from which we look at it.
    const double d_owner     = mesh.Dx_Owner(f);
    const double d_neighbour = mesh.Dx_Neighbour(f);
    const double face_area   = mesh.Face_Area(f);

    // Face diffusivity by harmonic (distance-weighted) interpolation
    const double gamma_face =
        (gamma[owner] == 0.0 || gamma[neighbour] == 0.0)
            ? 0.0
            : schemes::interpolation::harmonic_mean(
                  gamma[owner], gamma[neighbour], d_owner, d_neighbour);

    // Geometric orientation factor: s_f . e_PN (always +1 on a 1D Cartesian mesh,
    // kept explicit for dimension-independent, unstructured-mesh readiness).
    const double geom_factor = mesh.s_f(owner, f) * mesh.e_PN(owner, f);

    // Face conductance: D_f = (Gamma_f * A_f) / d_PN
    const double D_f = geom_factor * gamma_face * face_area / mesh.d_PN(f);

    if (!std::isfinite(D_f))
    {
        throw std::overflow_error(
            "Laplacian internal-face conductance is not finite");
    }

    // Contribution of this face to the owner row (cell P):
    matrix.Add_a_P(owner, D_f);
    matrix.Add_OffDiagonal(owner, neighbour, -D_f);

    // Contribution of the same face to the neighbour row (cell N):
    matrix.Add_a_P(neighbour, D_f);
    matrix.Add_OffDiagonal(neighbour, owner, -D_f);
}


}

} // namespace cfd::fvm
