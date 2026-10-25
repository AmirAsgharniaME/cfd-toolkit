#include "cfd/mesh/mesh1d/uniform_fv_mesh1d.hpp"

#include <cmath>
#include <limits>
#include <stdexcept>

namespace cfd::mesh
{
double UniformFvMesh1D::compute_spacing(const cfd::geometry::Domain1D& domain,
                                      std::size_t num_cells)
{
    if (num_cells == 0)
    {
        throw std::invalid_argument("UniformFvMesh1D: cell count must be >= 1.");
    }
    if (num_cells == std::numeric_limits<std::size_t>::max() ||
        num_cells >= std::vector<double>().max_size())
    {
        throw std::length_error("UniformFvMesh1D: requested cell count is too large.");
    }
    const double spacing = domain.length() / static_cast<double>(num_cells);
    if (!std::isfinite(spacing) || !(spacing > 0.0))
    {
        throw std::invalid_argument("UniformFvMesh1D: spacing calculation yielded invalid resolution.");
    }
    return spacing;
}

std::vector<double> UniformFvMesh1D::make_faces(const cfd::geometry::Domain1D& domain,
                                             std::size_t num_cells, double spacing)
{
    std::vector<double> faces(num_cells + 1);
    faces.front() = domain.start();

    for (std::size_t f = 1; f < num_cells; ++f)
    {
        const double coordinate = domain.start() + static_cast<double>(f) * spacing;
        if (!std::isfinite(coordinate) || !(coordinate > faces[f - 1]) || !(coordinate < domain.end()))
        {
            throw std::invalid_argument("UniformFvMesh1D: coordinate precision degraded over interval.");
        }
        faces[f] = coordinate;
    }

    faces.back() = domain.end();  // preserve the authoritative endpoint exactly
    if (!(faces.back() > faces[faces.size() - 2]))
    {
        throw std::invalid_argument("UniformFvMesh1D: boundary face collapsed due to extreme cell count.");
    }
    return faces;
}

UniformFvMesh1D::UniformFvMesh1D(const cfd::geometry::Domain1D& domain,
                               std::size_t num_cells)
    : UniformFvMesh1D(domain, num_cells, compute_spacing(domain, num_cells))
{
}

UniformFvMesh1D::UniformFvMesh1D(double start, double end, std::size_t num_cells)
    : UniformFvMesh1D(cfd::geometry::Domain1D(start, end), num_cells)
{
}

UniformFvMesh1D::UniformFvMesh1D(const cfd::geometry::Domain1D& domain,
                               std::size_t num_cells, double spacing)
    : FvMesh1D(make_faces(domain, num_cells, spacing)),
      nominal_spacing_(spacing)
{
}

}  // namespace cfd::mesh
