#include "cfd/mesh/mesh1d/uniform_fv_mesh1d.hpp"

#include <cmath>
#include <limits>
#include <stdexcept>

namespace cfd::mesh
{
namespace
{
inline bool is_finite(double x) noexcept { return std::isfinite(x); }
}

double UniformFvMesh1D::compute_spacing(double x_start, double x_end, std::size_t num_cells)
{
    if (num_cells == 0)
    {
        throw std::invalid_argument("UniformFvMesh1D: cell count must be >= 1.");
    }
    if (num_cells == std::numeric_limits<std::size_t>::max())
    {
        throw std::length_error("UniformFvMesh1D: requested cell count is too large.");
    }
    if (!is_finite(x_start) || !is_finite(x_end) || !(x_end > x_start))
    {
        throw std::invalid_argument("UniformFvMesh1D: bounds must be finite and strictly increasing.");
    }

    const double length = x_end - x_start;
    const double spacing = length / static_cast<double>(num_cells);
    if (!is_finite(spacing) || !(spacing > 0.0))
    {
        throw std::invalid_argument("UniformFvMesh1D: spacing calculation yielded invalid resolution.");
    }
    return spacing;
}

std::vector<double> UniformFvMesh1D::make_faces(double x_start, double x_end,
                                                std::size_t num_cells, double spacing)
{
    std::vector<double> faces(num_cells + 1);
    faces.front() = x_start;

    for (std::size_t f = 1; f < num_cells; ++f)
    {
        const double coordinate = x_start + static_cast<double>(f) * spacing;
        if (!is_finite(coordinate) || !(coordinate > faces[f - 1]) || !(coordinate < x_end))
        {
            throw std::invalid_argument("UniformFvMesh1D: coordinate precision degraded over interval.");
        }
        faces[f] = coordinate;
    }

    faces.back() = x_end;  // pin last face to eliminate FP drift
    if (!(faces.back() > faces[faces.size() - 2]))
    {
        throw std::invalid_argument("UniformFvMesh1D: boundary face collapsed due to extreme cell count.");
    }
    return faces;
}

UniformFvMesh1D::UniformFvMesh1D(const cfd::geometry::Domain1D& domain,
                                 std::size_t num_cells)
    : UniformFvMesh1D(domain.x_start(), domain.x_end(), num_cells)
{
}

UniformFvMesh1D::UniformFvMesh1D(double x_start, double x_end, std::size_t num_cells)
    : FvMesh1D(make_faces(x_start, x_end, num_cells,
                          compute_spacing(x_start, x_end, num_cells))),
      nominal_spacing_(compute_spacing(x_start, x_end, num_cells))
{
}

}  // namespace cfd::mesh
