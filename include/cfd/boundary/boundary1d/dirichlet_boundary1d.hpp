#pragma once

#include "cfd/boundary/boundary1d/boundary_condition1d.hpp"

#include <string>

namespace cfd::fv_matrix
{
class FvMatrix1D;
}

namespace cfd::boundary
{

class DirichletBoundary1D : public BoundaryCondition1D
{
public:
    DirichletBoundary1D(std::string name, BoundarySide1D side, double value);

    [[nodiscard]] double Value() const noexcept;

    /// Accumulate the fixed-value diffusion contribution into the boundary owner's row.
    /// For outward flux -gamma grad(phi) . n, adds D to the diagonal and
    /// D * value to the right-hand side, where D = gamma_owner * face_area / distance.
    /// The coefficient field and matrix must refer to the same mesh object.
    void Apply(const field::VolScalarField1D& gamma,
               fv_matrix::FvMatrix1D& matrix) const override;

private:
    double value_;
};

}  // namespace cfd::boundary
