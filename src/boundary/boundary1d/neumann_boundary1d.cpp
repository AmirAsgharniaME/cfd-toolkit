#include "cfd/boundary/boundary1d/neumann_boundary1d.hpp"

#include <utility>

namespace cfd::boundary
{

NeumannBoundary1D::NeumannBoundary1D(std::string name, BoundarySide1D side,
                                     double normal_gradient)
    : name_(std::move(name)), side_(side), normal_gradient_(normal_gradient)
{
}

const std::string& NeumannBoundary1D::name() const noexcept
{
    return name_;
}

BoundarySide1D NeumannBoundary1D::side() const noexcept
{
    return side_;
}

double NeumannBoundary1D::normal_gradient() const noexcept
{
    return normal_gradient_;
}

double NeumannBoundary1D::outward_normal() const noexcept
{
    switch (side_)
    {
        case BoundarySide1D::Start:
            return -1.0;
        case BoundarySide1D::End:
            return 1.0;
    }

    return 0.0;
}

double NeumannBoundary1D::coordinate_gradient() const noexcept
{
    return normal_gradient_ * outward_normal();
}

void NeumannBoundary1D::set_normal_gradient(double normal_gradient) noexcept
{
    normal_gradient_ = normal_gradient;
}

}  // namespace cfd::boundary
