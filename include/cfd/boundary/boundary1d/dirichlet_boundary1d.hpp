#pragma once

#include "cfd/boundary/types.hpp"
#include "cfd/field/field1d/field1d.hpp"

#include <string>

namespace cfd::fv_matrix
{
class FvMatrix1D;
}

namespace cfd::boundary
{

class DirichletBoundary1D
{
public:
    DirichletBoundary1D(std::string name, BoundarySide1D side, double value);

    [[nodiscard]] const std::string& name() const noexcept;
    [[nodiscard]] BoundarySide1D side() const noexcept;
    [[nodiscard]] double value() const noexcept;

    /// Accumulate the fixed-value diffusion contribution into the boundary row.
    /// For outward flux -gamma grad(phi) . n, adds D to the diagonal and
    /// D * value to the right-hand side (unit face area).
    /// The coefficient field and matrix must refer to the same mesh object.
    void apply(const field::VolScalarField1D& gamma,
               fv_matrix::FvMatrix1D& matrix) const;

private:
    std::string name_;
    BoundarySide1D side_;
    double value_;
};

}  // namespace cfd::boundary
