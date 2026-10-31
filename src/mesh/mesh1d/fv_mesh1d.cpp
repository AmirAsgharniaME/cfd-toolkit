#include "cfd/mesh/mesh1d/fv_mesh1d.hpp"

#include "cfd/geometry/domain/domain1d.hpp"
#include "cfd/mesh/grading_type.hpp"
#include "cfd/mesh/mesh1d/grading1d.hpp"

#include <cmath>
#include <iomanip>
#include <numeric>
#include <ostream>
#include <stdexcept>
#include <utility>

namespace cfd::mesh
{

namespace
{

std::vector<double> make_uniform_faces(
    const geometry::Domain1D& domain,
    std::size_t n_cells)
{
    if (n_cells == 0UL)
        throw std::invalid_argument(
            "FvMesh1D: cell count must be at least one.");

    std::vector<double> faces;
    if (n_cells >= faces.max_size())
        throw std::length_error(
            "FvMesh1D: requested face count is too large.");

    faces.resize(n_cells + 1UL);

    const long double first = domain.Start_Coord();
    const long double extent = domain.Length();
    const long double count = static_cast<long double>(n_cells);

    faces.front() = domain.Start_Coord();
    faces.back() = domain.End_Coord();

    for (std::size_t f = 1; f < n_cells; ++f)
    {
        faces[f] = static_cast<double>(
            first + extent * static_cast<long double>(f) / count);
    }

    return faces;
}

void build_cell_geometry(
    const std::vector<double>& face_coords,
    std::vector<double>& cell_center_coords,
    std::vector<double>& cell_volumes)
{
    if (face_coords.size() < 2UL)
        throw std::invalid_argument(
            "FvMesh1D: requires at least two faces (>= 1 cell).");

    const std::size_t n_cells = face_coords.size() - 1UL;

    cell_center_coords.resize(n_cells);
    cell_volumes.resize(n_cells);

    for (std::size_t p = 0; p < n_cells; ++p)
    {
        const double west_face_coord = face_coords[p];
        const double east_face_coord = face_coords[p + 1];

        if (!std::isfinite(west_face_coord) || !std::isfinite(east_face_coord))
            throw std::invalid_argument(
                "FvMesh1D: face coordinates must be finite.");

        const double cell_width = east_face_coord - west_face_coord;

        if (cell_width <= 0.0 || !std::isfinite(cell_width))
            throw std::invalid_argument(
                "FvMesh1D: faces must increase strictly with finite cell widths.");

        const double cell_center_coord =
            std::midpoint(west_face_coord, east_face_coord);

        if (!std::isfinite(cell_center_coord) ||
            cell_center_coord <= west_face_coord ||
            cell_center_coord >= east_face_coord)
            throw std::invalid_argument(
                "FvMesh1D: cell has no distinct representable interior center.");

        if (p > 0 && cell_center_coord <= cell_center_coords[p - 1])
            throw std::invalid_argument(
                "FvMesh1D: cell centers must be strictly increasing.");

        cell_center_coords[p] = cell_center_coord;
        cell_volumes[p] = cell_width;
    }
}

}  // namespace

FvMesh1D::FvMesh1D(const geometry::Domain1D& domain, std::size_t n_cells)
    : face_coords_(make_uniform_faces(domain, n_cells))
{
    build_cell_geometry(face_coords_, cell_center_coords_, cell_volumes_);
}

FvMesh1D::FvMesh1D(
    const geometry::Domain1D& domain,
    std::size_t n_cells,
    GradingType grading_type,
    double grading_ratio)
    : face_coords_(cfd::mesh::grading::Make_Faces(domain, n_cells, grading_type, grading_ratio))
{
    build_cell_geometry(face_coords_, cell_center_coords_, cell_volumes_);
}

double FvMesh1D::Start_Coord() const
{
    if (face_coords_.empty())
        throw std::logic_error("FvMesh1D: mesh is empty (moved-from).");

    return face_coords_.front();
}

double FvMesh1D::End_Coord() const
{
    if (face_coords_.empty())
        throw std::logic_error("FvMesh1D: mesh is empty (moved-from).");

    return face_coords_.back();
}

double FvMesh1D::Length() const
{
    return End_Coord() - Start_Coord();
}

double FvMesh1D::wFace_Coord(std::size_t p) const
{
    if (p >= N_Cells())
        throw std::out_of_range("FvMesh1D: cell index out of range.");

    return face_coords_[p];
}

double FvMesh1D::eFace_Coord(std::size_t p) const
{
    if (p >= N_Cells())
        throw std::out_of_range("FvMesh1D: cell index out of range.");

    return face_coords_[p + 1];
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

    if (face_coords_.empty())
        throw std::logic_error("FvMesh1D: mesh is empty (moved-from).");

    output << std::fixed << std::setprecision(6)
           << "FvMesh1D Geometry Details\n"
           << "=========================\n"
           << "Domain bounds: [" << Start_Coord() << ", " << End_Coord() << "]\n"
           << "Cells (N):     " << N_Cells() << '\n'
           << "Faces (N+1):   " << N_Faces() << "\n\n"
           << "Cell    Face_W           Center_P         Face_E           Volume (dx)\n";

    for (std::size_t i = 0; i < N_Cells(); ++i)
    {
        output << std::fixed << std::setprecision(6)
               << std::setw(4) << i << "    "
               << std::setw(14) << face_coords_[i] << "    "
               << std::setw(14) << cell_center_coords_[i] << "    "
               << std::setw(14) << face_coords_[i + 1] << "    "
               << std::scientific << std::setprecision(6)
               << std::setw(14) << cell_volumes_[i] << '\n';
    }

    output.flags(flags);
    output.precision(precision);
}

}  // namespace cfd::mesh
