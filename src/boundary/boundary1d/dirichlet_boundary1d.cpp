#include "cfd/boundary/boundary1d/dirichlet_boundary1d.hpp"

#include <utility>

namespace cfd::boundary
{

DirichletBoundary1D::DirichletBoundary1D(std::string name, BoundarySide1D side, double value)
    : name_(std::move(name)), side_(side), value_(value)
{
}

const std::string& DirichletBoundary1D::name() const noexcept
{
    return name_;
}

BoundarySide1D DirichletBoundary1D::side() const noexcept
{
    return side_;
}

double DirichletBoundary1D::value() const noexcept
{
    return value_;
}

}  // namespace cfd::boundary
