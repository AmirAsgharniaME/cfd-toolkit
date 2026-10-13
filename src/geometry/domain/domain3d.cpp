#include "cfd/geometry/domain/domain3d.hpp"

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

Domain3D::Domain3D(double length_x, double length_y, double length_z)
    : Domain3D(0.0, length_x, 0.0, length_y, 0.0, length_z)
{
}

Domain3D::Domain3D(double x_start, double x_end,
                   double y_start, double y_end,
                   double z_start, double z_end)
    : x_start_(x_start), x_end_(x_end),
      y_start_(y_start), y_end_(y_end),
      z_start_(z_start), z_end_(z_end)
{
    validate_bounds(x_start_, x_end_, "Domain3D X bounds must be finite and increasing.");
    validate_bounds(y_start_, y_end_, "Domain3D Y bounds must be finite and increasing.");
    validate_bounds(z_start_, z_end_, "Domain3D Z bounds must be finite and increasing.");
}

void Domain3D::set_x_bounds(double start, double end)
{
    validate_bounds(start, end, "Domain3D X bounds must be finite and increasing.");
    x_start_ = start;
    x_end_ = end;
}

void Domain3D::set_y_bounds(double start, double end)
{
    validate_bounds(start, end, "Domain3D Y bounds must be finite and increasing.");
    y_start_ = start;
    y_end_ = end;
}

void Domain3D::set_z_bounds(double start, double end)
{
    validate_bounds(start, end, "Domain3D Z bounds must be finite and increasing.");
    z_start_ = start;
    z_end_ = end;
}

void Domain3D::set_x_start(double start) { set_x_bounds(start, x_end_); }
void Domain3D::set_x_end(double end) { set_x_bounds(x_start_, end); }
void Domain3D::set_y_start(double start) { set_y_bounds(start, y_end_); }
void Domain3D::set_y_end(double end) { set_y_bounds(y_start_, end); }
void Domain3D::set_z_start(double start) { set_z_bounds(start, z_end_); }
void Domain3D::set_z_end(double end) { set_z_bounds(z_start_, end); }

void Domain3D::set_length_x(double length)
{
    if (!std::isfinite(length) || length <= 0.0)
    {
        throw std::invalid_argument("Domain3D X length must be finite and greater than zero.");
    }
    set_x_bounds(x_start_, x_start_ + length);
}

void Domain3D::set_length_y(double length)
{
    if (!std::isfinite(length) || length <= 0.0)
    {
        throw std::invalid_argument("Domain3D Y length must be finite and greater than zero.");
    }
    set_y_bounds(y_start_, y_start_ + length);
}

void Domain3D::set_length_z(double length)
{
    if (!std::isfinite(length) || length <= 0.0)
    {
        throw std::invalid_argument("Domain3D Z length must be finite and greater than zero.");
    }
    set_z_bounds(z_start_, z_start_ + length);
}

}  // namespace cfd::geometry
