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
    const std::size_t n_faces = face_coords_.size();

    // At least two faces are required to define a single cell
    if (n_faces < 2UL) {
        throw std::invalid_argument(
            "FvMesh1D: requires at least two faces (>= 1 cell).");
    }

    const std::size_t n_Cells = n_faces - 1UL;

    // Pre-allocate and resize vectors for direct indexing (more efficient than reserve + push_back)
    cell_center_coords_.resize(n_Cells);
    cell_volumes_.resize(n_Cells);

    for (std::size_t p = 0; p < n_Cells; ++p)
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
        if (p > 0 && cell_center_coord <= cell_center_coords_[p - 1]) {
            throw std::invalid_argument(
                "FvMesh1D: cell centers must be strictly monotonically increasing.");
        }

        // Direct-index assignment: blazing fast with zero dynamic reallocation overhead
        cell_center_coords_[p] = cell_center_coord;
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
    if (Start_Coord() != domain.Start_Coord() || End_Coord() != domain.End_Coord())
        throw std::invalid_argument(
            "FvMesh1D: face endpoints must match domain bounds.");
}

FvMesh1D FvMesh1D::create_uniform(
    double x_min,
    double x_max,
    std::size_t n_cells)
{
    const cfd::geometry::Domain1D domain(x_min, x_max);

    if (n_cells == 0)
        throw std::invalid_argument(
            "FvMesh1D: cell count must be at least one.");

    std::vector<double> faces;
    if (n_cells >= faces.max_size())
        throw std::length_error(
            "FvMesh1D: requested face count is too large.");

    faces.resize(n_cells + 1);

    const long double first = domain.Start_Coord();
    const long double extent = domain.Length();

    faces.front() = domain.Start_Coord();
    faces.back() = domain.End_Coord();

    for (std::size_t f = 1; f < n_cells; ++f)
    {
        faces[f] = static_cast<double>(
            first + extent * static_cast<long double>(f) /
                        static_cast<long double>(n_cells));
    }

    return FvMesh1D(std::move(faces));
}

FvMesh1D FvMesh1D::create_uniform(
    const geometry::Domain1D& domain,
    std::size_t num_cells)
{
    return create_uniform(domain.Start_Coord(), domain.End_Coord(), num_cells);
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

double FvMesh1D::wFaceDx(std::size_t p) const
{
    return Cell_Center_Coord(p) - wFace_Coord(p);
}

double FvMesh1D::eFaceDx(std::size_t p) const
{
    return eFace_Coord(p) - Cell_Center_Coord(p);
}

double FvMesh1D::wCellDx(std::size_t p) const
{
    if (p == 0 || p >= N_Cells())
        throw std::out_of_range(
            "FvMesh1D: cell has no western neighbor.");

    return cell_center_coords_[p] - cell_center_coords_[p - 1];
}

double FvMesh1D::eCellDx(std::size_t p) const
{
    if (p >= N_Cells() || p == N_Cells() - 1)
        throw std::out_of_range(
            "FvMesh1D: cell has no eastern neighbor.");

    return cell_center_coords_[p + 1] - cell_center_coords_[p];
}

void FvMesh1D::Print_Details(std::ostream& output) const
{
    const auto flags = output.flags();
    const auto precision = output.precision();

    output << std::fixed << std::setprecision(6)
           << "FvMesh1D Geometry Details\n"
           << "=========================\n"
           << "Domain bounds: [" << Start_Coord() << ", " << End_Coord() << "]\n"
           << "Cells (N):     " << N_Cells() << '\n'
           << "Faces (N+1):   " << N_Faces() << "\n\n"
           << "Cell    Face_W           Center_P         Face_E           Volume (dx)\n";

    for (std::size_t i = 0; i < N_Cells(); ++i)
    {
        output << std::setw(4) << i << "    "
               << std::setw(14) << face_coords_[i] << "    "
               << std::setw(14) << cell_center_coords_[i] << "    "
               << std::setw(14) << face_coords_[i + 1] << "    "
               << std::setw(14) << cell_volumes_[i] << '\n';
    }

    output.flags(flags);
    output.precision(precision);
}

}  // namespace cfd::mesh
