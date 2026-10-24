#include "cfd/mesh/mesh1d/fv_mesh1d.hpp"

#include <cmath>
#include <iomanip>
#include <ostream>
#include <stdexcept>
#include <string>
#include <utility>

namespace cfd::mesh
{
namespace
{

std::vector<double> validate_and_compute_widths(const std::vector<double>& faces)
{
    if (faces.size() < 2)
    {
        throw std::invalid_argument("FvMesh1D: requires at least two faces (>= 1 cell).");
    }

    std::vector<double> widths(faces.size() - 1);
    for (std::size_t P = 0; P < widths.size(); ++P)
    {
        if (!std::isfinite(faces[P]) || !std::isfinite(faces[P + 1]))
        {
            throw std::invalid_argument("FvMesh1D: non-finite face coordinate at index " + std::to_string(P));
        }

        widths[P] = faces[P + 1] - faces[P];
        if (!std::isfinite(widths[P]) || !(widths[P] > 0.0))
        {
            throw std::invalid_argument("FvMesh1D: cell width must be finite and positive at index " + std::to_string(P));
        }
    }
    return widths;
}

std::vector<double> compute_cell_centers(const std::vector<double>& faces,
                                         const std::vector<double>& widths)
{
    std::vector<double> centers(widths.size());
    for (std::size_t P = 0; P < centers.size(); ++P)
    {
        // Avoid overflow from adding the two face coordinates directly.
        centers[P] = faces[P] + widths[P] * 0.5;
        if (!std::isfinite(centers[P]) || !(faces[P] < centers[P]) ||
            !(centers[P] < faces[P + 1]) || (P > 0 && !(centers[P] > centers[P - 1])))
        {
            throw std::invalid_argument(
                "FvMesh1D: coordinate precision cannot represent a distinct interior cell center at index " +
                std::to_string(P));
        }
    }
    return centers;
}

}  // namespace

FvMesh1D::FvMesh1D(std::vector<double> faces)
    : x_start_(faces.empty() ? 0.0 : faces.front()),
      x_end_(faces.empty() ? 0.0 : faces.back()),
      faces_(std::move(faces)),
      cell_widths_(validate_and_compute_widths(faces_)),
      cell_centers_(compute_cell_centers(faces_, cell_widths_))
{
}

double FvMesh1D::face(std::size_t f) const
{
    return faces_.at(f);
}

double FvMesh1D::cell_center(std::size_t P) const
{
    return cell_centers_.at(P);
}

double FvMesh1D::cell_width(std::size_t P) const
{
    return cell_widths_.at(P);
}

double FvMesh1D::distance_to_east_neighbor(std::size_t P) const
{
    if (P >= num_cells() || P + 1 >= num_cells())
    {
        throw std::out_of_range("FvMesh1D: cell " + std::to_string(P) + " has no eastern neighbor.");
    }
    return cell_centers_[P + 1] - cell_centers_[P];
}

double FvMesh1D::distance_to_west_neighbor(std::size_t P) const
{
    if (P >= num_cells() || P == 0)
    {
        throw std::out_of_range("FvMesh1D: cell " + std::to_string(P) + " has no western neighbor.");
    }
    return cell_centers_[P] - cell_centers_[P - 1];
}

void FvMesh1D::print_details(std::ostream& output) const
{
    const std::ios::fmtflags original_flags = output.flags();
    const std::streamsize original_precision = output.precision();

    output << std::fixed << std::setprecision(6)
           << "FvMesh1D Geometry Details\n"
           << "=========================\n"
           << "Domain bounds: [" << x_start_ << ", " << x_end_ << "]\n"
           << "Cells (N):     " << num_cells() << '\n'
           << "Faces (N+1):   " << num_faces() << "\n\n"
           << "Topology:\n"
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
