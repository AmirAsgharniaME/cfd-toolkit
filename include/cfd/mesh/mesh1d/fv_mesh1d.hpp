#pragma once

#include "cfd/mesh/grading_type.hpp"

#include <cstddef>
#include <iosfwd>
#include <optional>
#include <span>
#include <stdexcept>
#include <vector>

namespace cfd::geometry
{
class Domain1D;
}

namespace cfd::mesh
{

using CellId = std::size_t;
using FaceId = std::size_t;

/// \brief Concrete 1D finite-volume mesh.
///
/// In 1D finite-volume formulations, cell volumes represent cell lengths (dx),
/// assuming a unit cross-sectional area.
class FvMesh1D
{
public:
    /// \brief Constructs a uniform 1D finite-volume mesh.
    /// \param domain Spatial domain interval [x_start, x_end].
    /// \param n_cells Number of cells (control volumes).
    FvMesh1D(const geometry::Domain1D& domain, std::size_t n_cells);

    /// \brief Constructs a graded (non-uniform) 1D finite-volume mesh.
    /// \param domain Spatial domain interval [x_start, x_end].
    /// \param n_cells Number of cells (control volumes).
    /// \param grading_type Expansion/contraction grading profile.
    /// \param grading_ratio Ratio between end-cell and start-cell widths.
    ///
    /// Face generation is delegated to `grading::Make_Faces`. `n_cells == 1`
    /// is accepted for every grading type and yields the same single cell as
    /// the uniform constructor.
    FvMesh1D(
        const geometry::Domain1D& domain,
        std::size_t n_cells,
        GradingType grading_type,
        double grading_ratio
    );

    // Copy/move construction only: fields bind to a mesh by reference, so a
    // constructed mesh is never reassigned. A moved-from mesh is empty and
    // its extent queries throw std::logic_error.
    FvMesh1D(const FvMesh1D&) = default;
    FvMesh1D(FvMesh1D&&) noexcept = default;
    FvMesh1D& operator=(const FvMesh1D&) = delete;
    FvMesh1D& operator=(FvMesh1D&&) = delete;
    ~FvMesh1D() = default;

    // --- Mesh extents and sizes ---
    [[nodiscard]] std::size_t N_Cells() const noexcept
    {
        return cell_center_coords_.size();
    }

    [[nodiscard]] std::size_t N_Faces() const noexcept
    {
        return face_coords_.size();
    }

    /// \throws std::logic_error If the mesh is moved-from (empty).
    [[nodiscard]] double Start_Coord() const;

    /// \throws std::logic_error If the mesh is moved-from (empty).
    [[nodiscard]] double End_Coord() const;

    /// \throws std::logic_error If the mesh is moved-from (empty).
    [[nodiscard]] double Length() const;

    // Canonical face orientation points out of the owner:
    // left at the start boundary and right at all other faces.
    // Invalid indices or non-incident cell-face pairs throw.
    [[nodiscard]] CellId Owner(FaceId f) const
    {
        if (f >= N_Faces())
        {
            throw std::out_of_range("Invalid face index");
        }

        return (f == 0) ? 0 : f - 1;
    }

    [[nodiscard]] std::optional<CellId> Neighbour(FaceId f) const
    {
        if (f >= N_Faces())
        {
            throw std::out_of_range("Invalid face index");
        }

        if (f == 0 || f == N_Cells())
        {
            return std::nullopt;
        }

        return CellId{f};
    }

    [[nodiscard]] double Face_Normal(FaceId f) const
    {
        if (f >= N_Faces())
        {
            throw std::out_of_range("Invalid face index");
        }

        return (f == 0) ? -1.0 : 1.0;
    }

    [[nodiscard]] double Outward_Normal(CellId p, FaceId f) const
    {
        if (p >= N_Cells() ||
            f >= N_Faces() ||
            (f != p && f != p + 1))
        {
            throw std::out_of_range(
                "Cell and face are invalid or not incident"
            );
        }

        return (f == p) ? -1.0 : 1.0;
    }

    // --- Modern span views ---
    [[nodiscard]] std::span<const double> Face_Coords() const noexcept
    {
        return face_coords_;
    }

    [[nodiscard]] std::span<const double> Cell_Center_Coords() const noexcept
    {
        return cell_center_coords_;
    }

    [[nodiscard]] std::span<const double> Cell_Volumes() const noexcept
    {
        return cell_volumes_;
    }

    // --- Indexed geometry queries ---
    // f: face index; p: cell index.
    /// \brief Returns the cross-sectional area of face f.
    /// \param f Face index identifier.
    /// \return Cross-sectional area (1.0 for a unit cross-section).
    /// \throws std::out_of_range If f >= N_Faces().
    [[nodiscard]] double Face_Area(FaceId f) const;

    [[nodiscard]] double Face_Coord(FaceId f) const
    {
        return face_coords_.at(f);
    }

    [[nodiscard]] double Cell_Center_Coord(CellId p) const
    {
        return cell_center_coords_.at(p);
    }

    [[nodiscard]] double Cell_Volume(CellId p) const
    {
        return cell_volumes_.at(p);
    }

    // Bounding face coordinates for cell P.
    [[nodiscard]] double wFace_Coord(CellId p) const;
    [[nodiscard]] double eFace_Coord(CellId p) const;

    // --- FVM stencil distance metrics ---
    [[nodiscard]] double wFaceDx(CellId p) const;
    [[nodiscard]] double eFaceDx(CellId p) const;

    // Distance from the west/owner cell center to face f.
    [[nodiscard]] double Dx_W(FaceId f) const;

    // Distance from face f to the east/neighbour cell center.
    [[nodiscard]] double Dx_E(FaceId f) const;

    // Total distance between cell centers sharing face f (d_PN).
    [[nodiscard]] double Dx_f(FaceId f) const;

    [[nodiscard]] double wCellDx(CellId p) const;
    [[nodiscard]] double eCellDx(CellId p) const;

    /// \brief Prints a human-readable table of face and cell-center coordinates.
    void Print_Details(std::ostream& output) const;

private:
    std::vector<double> face_coords_;
    std::vector<double> cell_center_coords_;
    std::vector<double> cell_volumes_;
};

}  // namespace cfd::mesh
