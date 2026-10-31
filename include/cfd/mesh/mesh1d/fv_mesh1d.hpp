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
        std::size_t num_cells
    );
    [[nodiscard]] static FvMesh1D create_uniform
    (
        const geometry::Domain1D& domain,
        std::size_t num_cells
    );
    [[nodiscard]] static FvMesh1D create_graded
    (
        double x_min,
        double x_max,
        std::size_t num_cells,
        double grading_ratio
    );

    // Rule of 5 (Standard value semantics)
    FvMesh1D(const FvMesh1D&) = default;
    FvMesh1D(FvMesh1D&&) noexcept = default;
    FvMesh1D& operator=(const FvMesh1D&) = default;
    FvMesh1D& operator=(FvMesh1D&&) noexcept = default;
    ~FvMesh1D() = default;

    // --- Topology & Global Extents ---
    [[nodiscard]] std::size_t num_cells() const noexcept { return cell_centers_.size(); }
    [[nodiscard]] std::size_t num_faces() const noexcept { return face_coords_.size(); }
    [[nodiscard]] double x_min() const noexcept { return face_coords_.front(); }
    [[nodiscard]] double x_max() const noexcept { return face_coords_.back(); }
    [[nodiscard]] double length() const noexcept { return x_max() - x_min(); }

    // --- Bulk Span Accessors (Modern C++ Views) ---
    [[nodiscard]] std::span<const double> face_coords() const noexcept { return face_coords_; }
    [[nodiscard]] std::span<const double> cell_centers() const noexcept { return cell_centers_; }
    [[nodiscard]] std::span<const double> cell_volumes() const noexcept { return cell_volumes_; }

    // --- Indexed Queries ---
    [[nodiscard]] double face_coord(std::size_t face_idx) const { return face_coords_.at(face_idx); }
    [[nodiscard]] double cell_center(std::size_t cell_idx) const { return cell_centers_.at(cell_idx); }
    [[nodiscard]] double cell_volume(std::size_t cell_idx) const { return cell_volumes_.at(cell_idx); }

    // --- FVM Stencil Geometric Distances ---
    [[nodiscard]] double distance_to_west_face(std::size_t cell_idx) const;
    [[nodiscard]] double distance_to_east_face(std::size_t cell_idx) const;
    [[nodiscard]] double distance_to_west_neighbor(std::size_t cell_idx) const;
    [[nodiscard]] double distance_to_east_neighbor(std::size_t cell_idx) const;

    void print_details(std::ostream& output) const;

    // Backward compatibility aliases (if existing tests use them, can be pruned later)
    [[nodiscard]] double start() const noexcept { return x_min(); }
    [[nodiscard]] double end() const noexcept { return x_max(); }
    [[nodiscard]] double cell_width(std::size_t cell_idx) const { return cell_volume(cell_idx); }

private:
    std::vector<double> face_coords_;
    std::vector<double> cell_centers_;
    std::vector<double> cell_volumes_;
};

}  // namespace cfd::mesh
