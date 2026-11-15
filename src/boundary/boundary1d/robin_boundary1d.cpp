#include "cfd/boundary/boundary1d/robin_boundary1d.hpp"

#include "cfd/fv_matrix/fv_matrix1d.hpp"

#include <cmath>
#include <stdexcept>
#include <utility>

namespace cfd::boundary
{

RobinBoundary1D::RobinBoundary1D(std::string name, BoundarySide1D side,
                                 double transfer_coefficient, double ambient_value)
    : BoundaryCondition1D(std::move(name), side), transfer_coefficient_(transfer_coefficient),
      ambient_value_(ambient_value)
{
    switch (Side())
    {
        case BoundarySide1D::Start:
        case BoundarySide1D::End:
            break;
        default:
            throw std::invalid_argument("Invalid Robin boundary side");
    }
}

const std::string& RobinBoundary1D::name() const noexcept { return Name(); }
BoundarySide1D RobinBoundary1D::side() const noexcept { return Side(); }
double RobinBoundary1D::transfer_coefficient() const noexcept { return transfer_coefficient_; }
double RobinBoundary1D::ambient_value() const noexcept { return ambient_value_; }
void RobinBoundary1D::set_transfer_coefficient(double value) noexcept { transfer_coefficient_ = value; }
void RobinBoundary1D::set_ambient_value(double value) noexcept { ambient_value_ = value; }

void RobinBoundary1D::Apply(const field::VolScalarField1D& gamma,
                            fv_matrix::FvMatrix1D& matrix) const
{
    if (!std::isfinite(transfer_coefficient_) || transfer_coefficient_ < 0.0)
        throw std::invalid_argument("Robin transfer coefficient must be finite and nonnegative");
    if (!std::isfinite(ambient_value_))
        throw std::invalid_argument("Robin ambient value must be finite");

    const BoundaryAssemblyContext1D context = Prepare_Assembly(
        gamma,
        matrix,
        to_string(BoundaryType::Robin));
    const std::size_t owner = context.owner;

    const double boundary_diagonal = transfer_coefficient_ * context.face_area;
    const double boundary_source = boundary_diagonal * ambient_value_;
    if (!std::isfinite(boundary_diagonal) || !std::isfinite(boundary_source))
        throw std::overflow_error("Robin boundary contribution is not finite");
    const double updated_diagonal = matrix.a_P(owner) + boundary_diagonal;
    const double updated_source = matrix.b(owner) + boundary_source;
    if (!std::isfinite(updated_diagonal) || !std::isfinite(updated_source))
        throw std::overflow_error("Accumulated Robin boundary row is not finite");

    matrix.Add_a_P(owner, boundary_diagonal);
    matrix.Add_b(owner, boundary_source);
}

}  // namespace cfd::boundary
