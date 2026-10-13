#include "cfd/geometry/domain/domain2d.hpp"

#include <cmath>
#include <stdexcept>

namespace cfd::geometry
{
namespace
{

void validate_bounds(double start, double end, const char* axis)
{
    if (!std::isfinite(start) || !std::isfinite(end) || end <= start ||
        !std::isfinite(end - start))
    {
        throw std::invalid_argument(axis);
    }
}

}  // namespace

Domain2D::Domain2D(double length_x, double length_y)
    : Domain2D(0.0, length_x, 0.0, length_y)
{
}

Domain2D::Domain2D(double x_start, double x_end, double y_start, double y_end)
    : x_start_(x_start), x_end_(x_end), y_start_(y_start), y_end_(y_end)
{
    validate_bounds(x_start_, x_end_, "Domain2D X bounds must be finite and increasing.");
    validate_bounds(y_start_, y_end_, "Domain2D Y bounds must be finite and increasing.");
}

void Domain2D::set_x_bounds(double start, double end)
{
    validate_bounds(start, end, "Domain2D X bounds must be finite and increasing.");
    x_start_ = start;
    x_end_ = end;
}

void Domain2D::set_y_bounds(double start, double end)
{
    validate_bounds(start, end, "Domain2D Y bounds must be finite and increasing.");
    y_start_ = start;
    y_end_ = end;
}

void Domain2D::set_x_start(double start) { set_x_bounds(start, x_end_); }
void Domain2D::set_x_end(double end) { set_x_bounds(x_start_, end); }
void Domain2D::set_y_start(double start) { set_y_bounds(start, y_end_); }
void Domain2D::set_y_end(double end) { set_y_bounds(y_start_, end); }

void Domain2D::set_length_x(double length)
{
    if (!std::isfinite(length) || length <= 0.0)
    {
        throw std::invalid_argument("Domain2D X length must be finite and greater than zero.");
    }
    set_x_bounds(x_start_, x_start_ + length);
}

void Domain2D::set_length_y(double length)
{
    if (!std::isfinite(length) || length <= 0.0)
    {
        throw std::invalid_argument("Domain2D Y length must be finite and greater than zero.");
    }
    set_y_bounds(y_start_, y_start_ + length);
}

}  // namespace cfd::geometry
