#include "cfd/mesh/mesh1d/fv_mesh1d.hpp"

#include "cfd/geometry/domain/domain1d.hpp"
#include "cfd/mesh/grading_type.hpp"
#include "cfd/mesh/mesh1d/grading1d.hpp"

#include <cmath>
#include <iomanip>
#include <ostream>
#include <stdexcept>
#include <utility>
#include <numeric> // Required for std::midpoint in C++20

namespace cfd::mesh
{

FvMesh1D::FvMesh1D(std::vector<double> face_coords)
    : face_coords_(std::move(face_coords))
{
    const std::size_t N_Faces = face_coords_.size();

    // At least two faces are required to define a single cell
    if (N_Faces < 2UL) {
        throw std::invalid_argument(
            "FvMesh1D: requires at least two faces (>= 1 cell).");
    }

    const std::size_t N_Cells = N_Faces - 1UL;

    // Pre-allocate and resize vectors for direct indexing (more efficient than reserve + push_back)
    cell_centers_.resize(N_Cells);
    cell_volumes_.resize(N_Cells);

    for (std::size_t p = 0; p < N_Cells; ++p)
    {
        // In a 1D structured mesh: west face is at 'p', east face is at 'p + 1'
        const double west_face_coord = face_coords_[p];
        const double east_face_coord = face_coords_[p + 1];

        // Ensure face coordinates are valid finite numbers (prevent NaN/Inf propagation)
        if (!std::isfinite(west_face_coord) || !std::isfinite(east_face_coord)) {
            throw std::invalid_argument(
                "FvMesh1D: face coordinates must be finite.");
        }

        const double cell_width = east_face_coord - west_face_coord;

        // Verify strictly increasing coordinates, positive cell width, and valid float state
        if (cell_width <= 0.0 || !std::isfinite(cell_width)) {
            throw std::invalid_argument(
                "FvMesh1D: faces must increase strictly with finite cell widths.");
        }

        // Use C++20 std::midpoint for highest numerical precision without overflow risk
        const double cell_center_coord = std::midpoint(west_face_coord, east_face_coord);

        // Ensure the computed center strictly resides inside the cell bounds
        if (!std::isfinite(cell_center_coord) || cell_center_coord <= west_face_coord || cell_center_coord >= east_face_coord) {
            throw std::invalid_argument(
                "FvMesh1D: cell has no distinct representable interior center.");
        }

        // Guarantee strict monotonic increase of cell centers compared to the previous cell
        if (p > 0 && cell_center_coord <= cell_centers_[p - 1]) {
            throw std::invalid_argument(
                "FvMesh1D: cell centers must be strictly monotonically increasing.");
        }

        // Direct-index assignment: blazing fast with zero dynamic reallocation overhead
        cell_centers_[p] = cell_center_coord;
        cell_volumes_[p] = cell_width;
    }
}


FvMesh1D::FvMesh1D(std::initializer_list<double> face_coords)
    : FvMesh1D(std::vector<double>(face_coords))
{
}

FvMesh1D::FvMesh1D(
    const geometry::Domain1D& domain,
    std::vector<double> face_coords)
    : FvMesh1D(std::move(face_coords))
{
    if (start() != domain.start() || end() != domain.end())
        throw std::invalid_argument(
            "FvMesh1D: face endpoints must match domain bounds.");
}

FvMesh1D FvMesh1D::create_uniform(
    double x_min,
    double x_max,
    std::size_t num_cells)
{
    const geometry::Domain1D domain(x_min, x_max);

    if (num_cells == 0)
        throw std::invalid_argument(
            "FvMesh1D: cell count must be at least one.");

    std::vector<double> faces;
    if (num_cells >= faces.max_size())
        throw std::length_error(
            "FvMesh1D: requested face count is too large.");

    faces.resize(num_cells + 1);

    const long double first = domain.start();
    const long double extent = domain.length();

    faces.front() = domain.start();
    faces.back() = domain.end();

    for (std::size_t i = 1; i < num_cells; ++i)
    {
        faces[i] = static_cast<double>(
            first + extent * static_cast<long double>(i) /
                        static_cast<long double>(num_cells));
    }

    return FvMesh1D(std::move(faces));
}

FvMesh1D FvMesh1D::create_uniform(
    const geometry::Domain1D& domain,
    std::size_t num_cells)
{
    return create_uniform(domain.start(), domain.end(), num_cells);
}

FvMesh1D FvMesh1D::create_graded(
    double x_min,
    double x_max,
    std::size_t num_cells,
    double grading_ratio)
{
    const geometry::Domain1D domain(x_min, x_max);

    return FvMesh1D(grading::make_1d_faces(
        domain, num_cells, GradingType::Forward, grading_ratio));
}

double FvMesh1D::distance_to_west_face(std::size_t cell_idx) const
{
    return cell_center(cell_idx) - face_coord(cell_idx);
}

double FvMesh1D::distance_to_east_face(std::size_t cell_idx) const
{
    const double center = cell_center(cell_idx);
    return face_coord(cell_idx + 1) - center;
}

double FvMesh1D::distance_to_west_neighbor(std::size_t cell_idx) const
{
    if (cell_idx == 0 || cell_idx >= num_cells())
        throw std::out_of_range(
            "FvMesh1D: cell has no western neighbor.");

    return cell_centers_[cell_idx] - cell_centers_[cell_idx - 1];
}

double FvMesh1D::distance_to_east_neighbor(std::size_t cell_idx) const
{
    if (cell_idx >= num_cells() || cell_idx == num_cells() - 1)
        throw std::out_of_range(
            "FvMesh1D: cell has no eastern neighbor.");

    return cell_centers_[cell_idx + 1] - cell_centers_[cell_idx];
}

void FvMesh1D::print_details(std::ostream& output) const
{
    const auto flags = output.flags();
    const auto precision = output.precision();

    output << std::fixed << std::setprecision(6)
           << "FvMesh1D Geometry Details\n"
           << "=========================\n"
           << "Domain bounds: [" << start() << ", " << end() << "]\n"
           << "Cells (N):     " << num_cells() << '\n'
           << "Faces (N+1):   " << num_faces() << "\n\n"
           << "Cell    Face_W           Center_P         Face_E           Volume (dx)\n";

    for (std::size_t i = 0; i < num_cells(); ++i)
    {
        output << std::setw(4) << i << "    "
               << std::setw(14) << face_coords_[i] << "    "
               << std::setw(14) << cell_centers_[i] << "    "
               << std::setw(14) << face_coords_[i + 1] << "    "
               << std::setw(14) << cell_volumes_[i] << '\n';
    }

    output.flags(flags);
    output.precision(precision);
}

}  // namespace cfd::mesh
