#include "cfd/boundary/boundary1d/neumann_boundary1d.hpp"

#include "cfd/fv_matrix/fv_matrix1d.hpp"

#include <cmath>
#include <limits>
#include <stdexcept>
#include <utility>

namespace cfd::boundary
{

NeumannBoundary1D::NeumannBoundary1D(std::string name, BoundarySide1D side,
                                     double normal_gradient)
    : BoundaryCondition1D(std::move(name), side), normal_gradient_(normal_gradient)
{
    switch (Side())
    {
        case BoundarySide1D::Start:
        case BoundarySide1D::End:
            break;
        default:
            throw std::invalid_argument("Invalid Neumann boundary side");
    }
}

NeumannBoundary1D::NeumannBoundary1D(std::string name, BoundarySide1D side, double gradient,
                                     GradientType type)
    : NeumannBoundary1D(std::move(name), side, gradient)
{
    switch (type)
    {
        case GradientType::Normal:
            break;
        case GradientType::Coordinate:
            normal_gradient_ = outward_normal() * gradient;
            break;
        default:
            throw std::invalid_argument("Invalid Neumann gradient type");
    }
}

const std::string& NeumannBoundary1D::name() const noexcept
{
    return Name();
}

BoundarySide1D NeumannBoundary1D::side() const noexcept
{
    return Side();
}

double NeumannBoundary1D::normal_gradient() const noexcept
{
    return normal_gradient_;
}

double NeumannBoundary1D::outward_normal() const noexcept
{
    switch (Side())
    {
        case BoundarySide1D::Start:
            return -1.0;
        case BoundarySide1D::End:
            return 1.0;
        default:
            return std::numeric_limits<double>::quiet_NaN();
    }
}

double NeumannBoundary1D::coordinate_gradient() const noexcept
{
    return normal_gradient_ * outward_normal();
}

void NeumannBoundary1D::set_normal_gradient(double normal_gradient) noexcept
{
    normal_gradient_ = normal_gradient;
}

void NeumannBoundary1D::Apply(const field::VolScalarField1D& gamma,
                              fv_matrix::FvMatrix1D& matrix) const
{
    if (!std::isfinite(normal_gradient_))
    {
        throw std::invalid_argument("Neumann normal gradient must be finite");
    }

    const BoundaryAssemblyContext1D context = Prepare_Assembly(
        gamma,
        matrix,
        to_string(BoundaryType::Neumann));
    const std::size_t owner = context.owner;

    const double boundary_source =
        context.gamma_owner * normal_gradient_ * context.face_area;
    if (!std::isfinite(boundary_source))
    {
        throw std::overflow_error("Neumann boundary contribution is not finite");
    }

    const double updated_source = matrix.b(owner) + boundary_source;
    if (!std::isfinite(updated_source))
    {
        throw std::overflow_error("Accumulated Neumann boundary source is not finite");
    }

    matrix.Add_b(owner, boundary_source);
}

}  // namespace cfd::boundary
