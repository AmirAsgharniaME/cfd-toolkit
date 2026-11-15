#include "cfd/boundary/boundary1d/dirichlet_boundary1d.hpp"

#include "cfd/boundary/types.hpp"
#include "cfd/fv_matrix/fv_matrix1d.hpp"

#include <cmath>
#include <stdexcept>
#include <utility>

namespace cfd::boundary
{

DirichletBoundary1D::DirichletBoundary1D(std::string name, BoundarySide1D side, double value)
    : BoundaryCondition1D(std::move(name), side), value_(value)
{
}

double DirichletBoundary1D::Value() const noexcept
{
    return value_;
}

void DirichletBoundary1D::Apply(const field::VolScalarField1D& gamma,
                                fv_matrix::FvMatrix1D& matrix) const
{
    if (!std::isfinite(value_))
    {
        throw std::invalid_argument("Dirichlet boundary value must be finite");
    }

    const BoundaryAssemblyContext1D context = Prepare_Assembly(
        gamma,
        matrix,
        to_string(BoundaryType::Dirichlet));
    const std::size_t owner = context.owner;

    const double D_b =
        (context.gamma_owner * context.face_area) / context.owner_face_distance;
    const double boundary_source = D_b * value_;
    if (!std::isfinite(D_b) || !std::isfinite(boundary_source))
    {
        throw std::overflow_error("Dirichlet boundary contribution is not finite");
    }

    const double updated_diagonal = matrix.a_P(owner) + D_b;
    const double updated_source = matrix.b(owner) + boundary_source;
    if (!std::isfinite(updated_diagonal) || !std::isfinite(updated_source))
    {
        throw std::overflow_error("Accumulated Dirichlet boundary row is not finite");
    }

    matrix.Add_a_P(owner, D_b);
    matrix.Add_b(owner, boundary_source);
}

}  // namespace cfd::boundary
