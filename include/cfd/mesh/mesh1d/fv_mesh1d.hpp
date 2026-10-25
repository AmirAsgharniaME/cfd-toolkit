#pragma once

#include <cstddef>
#include <iosfwd>
#include <span>
#include <vector>

namespace cfd::mesh
{

/// Base class for all 1D Finite-Volume meshes (Uniform and Non-uniform).
/// Holds common topological arrays and provides unified FVM geometric queries.
/// Volumes assume Cartesian cells with unit cross-sectional area (A = 1 m^2
/// when coordinates are in meters), so their numerical values equal cell widths.
/// Geometry is read-only through this API. Move construction transfers storage;
/// move assignment replaces geometry. Do not move from or assign a mesh while
/// fields or views depend on it. A moved-from mesh may only be destroyed or
/// assigned a new value before geometric queries are made.
class FvMesh1D
{
public:
    virtual ~FvMesh1D() = default;

    FvMesh1D(const FvMesh1D&) = default;
    FvMesh1D(FvMesh1D&&) noexcept = default;
    FvMesh1D& operator=(const FvMesh1D&) = delete;
    FvMesh1D& operator=(FvMesh1D&&) noexcept = default;

    [[nodiscard]] std::size_t num_cells() const noexcept { return cell_centers_.size(); }
    [[nodiscard]] std::size_t cell_count() const noexcept { return num_cells(); }

    [[nodiscard]] std::size_t num_faces() const noexcept { return faces_.size(); }
    [[nodiscard]] std::size_t face_count() const noexcept { return num_faces(); }

    /// Independent snapshot of the authoritative boundary face coordinates.
    [[nodiscard]] double start() const noexcept { return start_; }
    [[nodiscard]] double end() const noexcept { return end_; }
    /// Finite represented endpoint difference, as in Domain1D::length().
    [[nodiscard]] double length() const noexcept { return end_ - start_; }

    /// Compatibility aliases for start() and end().
    [[nodiscard]] double x_start() const noexcept { return start(); }
    [[nodiscard]] double x_end() const noexcept { return end(); }

    [[nodiscard]] double face(std::size_t f) const;
    [[nodiscard]] double cell_center(std::size_t P) const;
    [[nodiscard]] double cell_width(std::size_t P) const;
    /// Cartesian cell volume for unit cross-sectional area: V = A * dx, A = 1.
    [[nodiscard]] double cell_volume(std::size_t P) const { return cell_width(P); }

    /// Center-to-center distance; throws std::out_of_range if P is invalid
    /// or the requested neighboring cell does not exist.
    [[nodiscard]] double distance_to_east_neighbor(std::size_t P) const;
    /// Center-to-center distance with the same bounds rules as the east query.
    [[nodiscard]] double distance_to_west_neighbor(std::size_t P) const;

    [[nodiscard]] std::span<const double> faces() const noexcept { return faces_; }
    [[nodiscard]] std::span<const double> cell_centers() const noexcept { return cell_centers_; }
    [[nodiscard]] std::span<const double> cell_widths() const noexcept { return cell_widths_; }
    /// Read-only volumes for Cartesian cells with unit cross-sectional area.
    [[nodiscard]] std::span<const double> cell_volumes() const noexcept { return cell_widths_; }

    [[nodiscard]] double operator[](std::size_t P) const noexcept { return cell_centers_[P]; }
    [[nodiscard]] double cell_center_at(std::size_t P) const { return cell_centers_.at(P); }
    [[nodiscard]] double face_at(std::size_t f) const { return faces_.at(f); }

    void print_details(std::ostream& output) const;

protected:
    /// Protected: only derived meshes construct with an explicit faces array.
    /// Bounds must satisfy Domain1D's finite, positive represented extent.
    /// Faces must increase strictly and each cell needs a distinct interior
    /// center. Invalid geometry throws std::invalid_argument.
    explicit FvMesh1D(std::vector<double> faces);

private:
    double start_;
    double end_;
    std::vector<double> faces_;
    std::vector<double> cell_widths_;
    std::vector<double> cell_centers_;
};

}  // namespace cfd::mesh
