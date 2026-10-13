#include "cfd/geometry/domain/domain1d.hpp"

#include <cmath>
#include <stdexcept>

namespace cfd::geometry
{
namespace
{

void validate_bounds(double start, double end)
{
    if (!std::isfinite(start) || !std::isfinite(end) || end <= start ||
        !std::isfinite(end - start))
    {
        throw std::invalid_argument("Domain1D bounds must be finite and strictly increasing.");
    }
}

}  // namespace

Domain1D::Domain1D(double length_x) : Domain1D(0.0, length_x) {}

Domain1D::Domain1D(double x_start, double x_end)
    : x_start_(x_start), x_end_(x_end)
{
    validate_bounds(x_start_, x_end_);
}

void Domain1D::set_bounds(double x_start, double x_end)
{
    validate_bounds(x_start, x_end);
    x_start_ = x_start;
    x_end_ = x_end;
}

void Domain1D::set_x_start(double x_start) { set_bounds(x_start, x_end_); }
void Domain1D::set_x_end(double x_end) { set_bounds(x_start_, x_end); }

void Domain1D::set_length_x(double length_x)
{
    if (!std::isfinite(length_x) || length_x <= 0.0)
    {
        throw std::invalid_argument("Domain1D length must be finite and greater than zero.");
    }
    set_bounds(x_start_, x_start_ + length_x);
}

}  // namespace cfd::geometry
