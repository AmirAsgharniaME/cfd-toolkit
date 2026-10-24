#include "cfd/mesh/mesh1d/uniform_fv_mesh1d.hpp"

#include <cmath>
#include <iomanip>
#include <limits>
#include <ostream>
#include <stdexcept>

namespace cfd::mesh
{
namespace
{

double compute_spacing(double x_start, double x_end, std::size_t num_cells)
{
    if (num_cells == 0)
    {
        throw std::invalid_argument("UniformFvMesh1D: cell count must be >= 1.");
    }
    if (num_cells == std::numeric_limits<std::size_t>::max())
    {
        throw std::length_error("UniformFvMesh1D: requested cell count is too large.");
    }
    if (!std::isfinite(x_start) || !std::isfinite(x_end) || x_end <= x_start)
    {
        throw std::invalid_argument("UniformFvMesh1D: bounds must be finite and strictly increasing.");
    }

    const double length = x_end - x_start;
    const double spacing = length / static_cast<double>(num_cells);
    if (!std::isfinite(spacing) || spacing <= 0.0)
    {
        throw std::invalid_argument("UniformFvMesh1D: spacing calculation yielded invalid resolution.");
    }
    return spacing;
}

std::vector<double> make_faces(double x_start, double x_end,
                               std::size_t num_cells, double spacing)
{
    std::vector<double> faces(num_cells + 1);
    faces.front() = x_start;

    for (std::size_t f = 1; f < num_cells; ++f)
    {
        const double coordinate = x_start + static_cast<double>(f) * spacing;
        if (!std::isfinite(coordinate) || coordinate <= faces[f - 1] || coordinate >= x_end)
        {
            throw std::invalid_argument(
                "UniformFvMesh1D: coordinate precision degraded over interval.");
        }
        faces[f] = coordinate;
    }

    // Pin the last face to eliminate floating-point drift at boundary
    faces.back() = x_end;
    if (faces.back() <= faces[faces.size() - 2])
    {
        throw std::invalid_argument(
            "UniformFvMesh1D: boundary face collapsed due to extreme cell count.");
    }
    return faces;
}

std::vector<double> make_cell_widths(const std::vector<double>& faces)
{
    std::vector<double> widths(faces.size() - 1);
    for (std::size_t P = 0; P < widths.size(); ++P)
    {
        widths[P] = faces[P + 1] - faces[P];
        if (!std::isfinite(widths[P]) || widths[P] <= 0.0)
        {
            throw std::invalid_argument("UniformFvMesh1D: non-positive cell volume detected.");
        }
    }
    return widths;
}

std::vector<double> make_cell_centers(const std::vector<double>& faces,
                                      const std::vector<double>& widths)
{
    std::vector<double> centers(widths.size());
    for (std::size_t P = 0; P < centers.size(); ++P)
    {
        centers[P] = faces[P] + widths[P] * 0.5;
        if (!std::isfinite(centers[P]) || centers[P] <= faces[P] || centers[P] >= faces[P + 1] ||
            (P != 0 && centers[P] <= centers[P - 1]))
        {
            throw std::invalid_argument(
                "UniformFvMesh1D: non-monotonic cell centers generated.");
        }
    }
    return centers;
}

}  // namespace

UniformFvMesh1D::UniformFvMesh1D(const cfd::geometry::Domain1D& domain,
                                 std::size_t num_cells)
    : UniformFvMesh1D(domain.x_start(), domain.x_end(), num_cells)
{
}

UniformFvMesh1D::UniformFvMesh1D(double x_start, double x_end, std::size_t num_cells)
    : x_start_(x_start),
      x_end_(x_end),
      nominal_spacing_(compute_spacing(x_start, x_end, num_cells)),
      faces_(make_faces(x_start_, x_end_, num_cells, nominal_spacing_)),
      cell_widths_(make_cell_widths(faces_)),
      cell_centers_(make_cell_centers(faces_, cell_widths_))
{
}

double UniformFvMesh1D::face(std::size_t f) const
{
    return faces_.at(f);
}

double UniformFvMesh1D::cell_center(std::size_t P) const
{
    return cell_centers_.at(P);
}

double UniformFvMesh1D::cell_width(std::size_t P) const
{
    return cell_widths_.at(P);
}

double UniformFvMesh1D::distance_to_east_neighbor(std::size_t P) const
{
    if (P + 1 >= num_cells())
    {
        throw std::out_of_range("UniformFvMesh1D: cell has no eastern neighbor (boundary cell).");
    }
    return cell_centers_[P + 1] - cell_centers_[P];
}

double UniformFvMesh1D::distance_to_west_neighbor(std::size_t P) const
{
    if (P == 0 || P >= num_cells())
    {
        throw std::out_of_range("UniformFvMesh1D: cell has no western neighbor (boundary cell).");
    }
    return cell_centers_[P] - cell_centers_[P - 1];
}

void UniformFvMesh1D::print_details(std::ostream& output) const
{
    const std::ios::fmtflags original_flags = output.flags();
    const std::streamsize original_precision = output.precision();

    output << std::fixed << std::setprecision(6)
           << "UniformFvMesh1D (OpenFOAM aligned)\n"
           << "=================================\n"
           << "Start:       " << x_start_ << '\n'
           << "End:         " << x_end_ << '\n'
           << "Cells (N):   " << num_cells() << '\n'
           << "Faces (N+1): " << num_faces() << '\n'
           << "Nominal dx:  " << nominal_spacing_ << "\n\n"
           << "Cell topology\n"
           << "-------------\n"
           << "Cell    Face_W           Center_P         Face_E           Volume (dx)\n";

    for (std::size_t P = 0; P < num_cells(); ++P)
    {
        output << std::setw(4) << P << "    "
               << std::setw(14) << faces_[P] << "    "
               << std::setw(14) << cell_centers_[P] << "    "
               << std::setw(14) << faces_[P + 1] << "    "
               << std::setw(14) << cell_widths_[P] << '\n';
    }

    output.flags(original_flags);
    output.precision(original_precision);
}

}  // namespace cfd::mesh
