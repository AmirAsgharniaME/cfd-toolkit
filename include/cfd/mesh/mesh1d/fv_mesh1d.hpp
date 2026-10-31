#pragma once

#include <cstddef>
#include <iosfwd>
#include <initializer_list>
#include <span>
#include <vector>

namespace cfd::geometry { class Domain1D; }

namespace cfd::mesh
{

/// Concrete, value-oriented 1D finite-volume mesh.
/// In 1D, cell volumes equal cell lengths (dx) assuming unit cross-sectional area.
class FvMesh1D
{
public:
    // --- Constructors & Factories ---
    explicit FvMesh1D(std::vector<double> face_coords);
    FvMesh1D(std::initializer_list<double> face_coords);
    FvMesh1D(const geometry::Domain1D& domain, std::vector<double> face_coords);

    [[nodiscard]] static FvMesh1D create_uniform
    (
        double x_min,
        double x_max,
        std::size_t n_cells
    );
    [[nodiscard]] static FvMesh1D create_uniform
    (
        const geometry::Domain1D& domain,
        std::size_t n_cells
    );
    [[nodiscard]] static FvMesh1D create_graded
    (
        double x_min,
        double x_max,
        std::size_t n_cells,
        double grading_ratio
    );

    // Rule of 5 (Standard value semantics)
    FvMesh1D(const FvMesh1D&) = default;
    FvMesh1D(FvMesh1D&&) noexcept = default;
    FvMesh1D& operator=(const FvMesh1D&) = default;
    FvMesh1D& operator=(FvMesh1D&&) noexcept = default;
    ~FvMesh1D() = default;

    // --- Topology & Global Extents ---
    [[nodiscard]] std::size_t N_Cells() const noexcept { return cell_center_coords_.size(); }
    [[nodiscard]] std::size_t N_Faces() const noexcept { return face_coords_.size(); }
    [[nodiscard]] double Start_Coord() const noexcept { return face_coords_.front(); }
    [[nodiscard]] double End_Coord() const noexcept { return face_coords_.back(); }
    [[nodiscard]] double Length() const noexcept { return End_Coord() - Start_Coord(); }

    // --- Bulk Span Accessors (Modern C++ Views) ---
    [[nodiscard]] std::span<const double> Face_Coords() const noexcept { return face_coords_; }
    [[nodiscard]] std::span<const double> Cell_Center_Coords() const noexcept { return cell_center_coords_; }
    [[nodiscard]] std::span<const double> Cell_Volumes() const noexcept { return cell_volumes_; }

    // --- Indexed Queries ---
    [[nodiscard]] double Face_Coord(std::size_t f) const { return face_coords_.at(f); }
    [[nodiscard]] double Cell_Center_Coord(std::size_t p) const { return cell_center_coords_.at(p); }
    [[nodiscard]] double Cell_Volume(std::size_t p) const { return cell_volumes_.at(p); }
    [[nodiscard]] double eFace_Coord(std::size_t p) const { return face_coords_.at(p+1); }
    [[nodiscard]] double wFace_Coord(std::size_t p) const { return face_coords_.at(p); }

    // --- FVM Stencil Geometric Distances ---
    [[nodiscard]] double wFaceDx(std::size_t p) const;
    [[nodiscard]] double eFaceDx(std::size_t p) const;
    [[nodiscard]] double wCellDx(std::size_t p) const;
    [[nodiscard]] double eCellDx(std::size_t p) const;

    void Print_Details(std::ostream& output) const;

    // Backward compatibility aliases (if existing tests use them, can be pruned later)
    [[nodiscard]] double Start() const noexcept { return Start_Coord(); }
    [[nodiscard]] double End() const noexcept { return End_Coord(); }
    [[nodiscard]] double Cell_Width(std::size_t p) const { return Cell_Volume(p); }

private:
    std::vector<double> face_coords_;
    std::vector<double> cell_center_coords_;
    std::vector<double> cell_volumes_;
};

}  // namespace cfd::mesh
