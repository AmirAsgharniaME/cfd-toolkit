#pragma once

#include "cfd/boundary/boundary1d/dirichlet_boundary1d.hpp"
#include "cfd/boundary/boundary1d/neumann_boundary1d.hpp"
#include "cfd/field/field1d/field1d.hpp"

#include <variant>

namespace cfd::fv_matrix {
class FvMatrix1D;
}

namespace cfd::fvm {

/// Boundary condition accepted by the 1D implicit Laplacian operator.
using BoundaryCondition1D = std::variant<
    boundary::DirichletBoundary1D,
    boundary::NeumannBoundary1D
>;

/// Assemble the integrated operator div(gamma grad(phi)) with homogeneous
/// Neumann conditions at both domain boundaries. The diffusion coefficient
/// must be finite and nonnegative in every cell. The field and matrix must
/// refer to the same mesh object. Existing matrix entries are accumulated.
void laplacian(
    const field::VolScalarField1D& gamma,
    fv_matrix::FvMatrix1D& matrix
);

/// Assemble div(gamma grad(phi)) with explicit conditions at the west/start
/// and east/end boundaries. Dirichlet contributions are evaluated from the
/// boundary face to the adjacent cell center; Neumann values are outward
/// normal gradients. Existing matrix entries are accumulated.
void laplacian(
    const field::VolScalarField1D& gamma,
    fv_matrix::FvMatrix1D& matrix,
    const BoundaryCondition1D& start_boundary,
    const BoundaryCondition1D& end_boundary
);

} // namespace cfd::fvm
