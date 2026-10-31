#pragma once

#include "cfd/mesh/grading_type.hpp"

#include <cstddef>
#include <iosfwd>
#include <span>
#include <vector>

namespace cfd::geometry { class Domain1D; }

namespace cfd::mesh
{

/// \brief Concrete 1D finite-volume mesh.
///
/// In 1D finite-volume formulations, cell volumes represent cell lengths (dx)
/// assuming a unit cross-sectional area.
class FvMesh1D
{
public:
    /// \brief Constructs a uniform 1D finite-volume mesh.
    /// \param domain The spatial domain interval [x_start, x_end].
    /// \param n_cells Number of cells (control volumes) in the mesh.
    FvMesh1D(const geometry::Domain1D& domain, std::size_t n_cells);

    /// \brief Constructs a graded (non-uniform) 1D finite-volume mesh.
    /// \param domain The spatial domain interval [x_start, x_end].
    /// \param n_cells Number of cells (control volumes) in the mesh.
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
    [[nodiscard]] std::size_t N_Cells() const noexcept { return cell_center_coords_.size(); }
    [[nodiscard]] std::size_t N_Faces() const noexcept { return face_coords_.size(); }
    /// \throws std::logic_error If the mesh is moved-from (empty).
    [[nodiscard]] double Start_Coord() const;
    /// \throws std::logic_error If the mesh is moved-from (empty).
    [[nodiscard]] double End_Coord() const;
    /// \throws std::logic_error If the mesh is moved-from (empty).
    [[nodiscard]] double Length() const;

    // --- Modern span views ---
    [[nodiscard]] std::span<const double> Face_Coords() const noexcept { return face_coords_; }
    [[nodiscard]] std::span<const double> Cell_Center_Coords() const noexcept { return cell_center_coords_; }
    [[nodiscard]] std::span<const double> Cell_Volumes() const noexcept { return cell_volumes_; }

    // --- Indexed geometry queries (f: face index, p: cell index) ---
    [[nodiscard]] double Face_Coord(std::size_t f) const { return face_coords_.at(f); }
    [[nodiscard]] double Cell_Center_Coord(std::size_t p) const { return cell_center_coords_.at(p); }
    [[nodiscard]] double Cell_Volume(std::size_t p) const { return cell_volumes_.at(p); }

    // Bounding face coordinates for cell P
    [[nodiscard]] double wFace_Coord(std::size_t p) const;
    [[nodiscard]] double eFace_Coord(std::size_t p) const;

    // --- FVM stencil distance metrics (Moukalled / Versteeg) ---
    [[nodiscard]] double wFaceDx(std::size_t p) const;
    [[nodiscard]] double eFaceDx(std::size_t p) const;
    [[nodiscard]] double wCellDx(std::size_t p) const;
    [[nodiscard]] double eCellDx(std::size_t p) const;

    /// \brief Prints human-readable table of face and cell-center coordinates.
    void Print_Details(std::ostream& output) const;

private:
    std::vector<double> face_coords_;
    std::vector<double> cell_center_coords_;
    std::vector<double> cell_volumes_;
};

}  // namespace cfd::mesh
