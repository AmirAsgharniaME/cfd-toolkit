#pragma once

#include "cfd/field/field1d/field1d.hpp"

namespace cfd::fv_matrix
{
class FvMatrix1D;
}

namespace cfd::fvm
{

/// Assemble only the internal-face contributions of the integrated
/// operator -div(gamma grad(phi)).
///
/// The owner outward flux is D_f * (phi_owner - phi_neighbour),
/// where D_f includes the face area supplied by the mesh.
///
/// No boundary-face contributions or boundary conditions are applied.
/// The diffusion coefficient must be finite and nonnegative in every cell.
/// The field and matrix must refer to the same mesh object.
/// Contributions are accumulated into existing matrix entries.
void Laplacian(
    const field::VolScalarField1D& gamma,
    cfd::fv_matrix::FvMatrix1D& matrix
);

} // namespace cfd::fvm
