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
    //const std::size_t n_face = mesh.N_Faces();
    const std::size_t n_cell = mesh.N_Cells();

    // Assemble internal faces only. Boundary-condition APIs add applicable
    // diagonal and source contributions to the boundary cell row separately.
// Only internal cells are iterated. Their faces are all internal.
for (mesh::CellId p = 1; p < n_cell - 1; ++p)
{
    for (std::size_t local_face = 0; local_face < 2; ++local_face)
    {
        const mesh::FaceId f = static_cast<mesh::FaceId>(p + local_face);
        const mesh::CellId owner     = mesh.Owner(f);
        const mesh::CellId neighbour = *mesh.Neighbour(f);

        // Distances and area are face properties, not cell-directional.
        const double d_owner     = mesh.Dx_Owner(f);
        const double d_neighbour = mesh.Dx_Neighbour(f);
        const double face_area   = mesh.Face_Area(f);

        const double gamma_face =
            (gamma[owner] == 0.0 || gamma[neighbour] == 0.0)
                ? 0.0
                : schemes::interpolation::harmonic_mean(
                      gamma[owner], gamma[neighbour], d_owner, d_neighbour);


        const double D_f = (mesh.s_f(p,f)*mesh.e_PN(p, f))*gamma_face * face_area/mesh.d_PN(f);

        if (!std::isfinite(D_f))
        {
            throw std::overflow_error(
                "Laplacian internal-face conductance is not finite");
        }

        // For a diffusion flux, regardless of which side p is on,
        // the diagonal coefficient is +D_f and the off-diagonal is -D_f.
        matrix.Add_a_P(p, D_f);
        matrix.Add_OffDiagonal(p, mesh.N(p, f), (-D_f));
    }
}

}

} // namespace cfd::fvm
