#pragma once

#include "cfd/geometry/domain/domain1d.hpp"

#include <cstddef>
#include <iosfwd>
#include <span>
#include <vector>

namespace cfd::mesh
{

/// Uniform one-dimensional, cell-centered finite-volume mesh.
///
/// A mesh with N cells has N+1 faces and N cell centers.
/// Follows OpenFOAM FVM conventions for volumes and face areas.
class UniformFvMesh1D
{
public:
    /// Construct a mesh with `num_cells` cells over the domain bounds.
    explicit UniformFvMesh1D(const cfd::geometry::Domain1D& domain,
                             std::size_t num_cells);

    /// Construct a mesh with `num_cells` cells over [x_start, x_end].
    explicit UniformFvMesh1D(double x_start, double x_end, std::size_t num_cells);

    UniformFvMesh1D(const UniformFvMesh1D&) = default;
    UniformFvMesh1D(UniformFvMesh1D&&) noexcept = default;
    UniformFvMesh1D& operator=(const UniformFvMesh1D&) = delete;
    UniformFvMesh1D& operator=(UniformFvMesh1D&&) = delete;
    ~UniformFvMesh1D() = default;

    /// Number of control volumes, N.
    [[nodiscard]] std::size_t num_cells() const noexcept { return cell_centers_.size(); }
    [[nodiscard]] std::size_t cell_count() const noexcept { return num_cells(); }

    /// Number of control-volume faces, N+1.
    [[nodiscard]] std::size_t num_faces() const noexcept { return faces_.size(); }
    [[nodiscard]] std::size_t face_count() const noexcept { return num_faces(); }

    [[nodiscard]] double x_start() const noexcept { return x_start_; }
    [[nodiscard]] double x_end() const noexcept { return x_end_; }

    /// Nominal uniform spacing, dx = (x_end - x_start) / num_cells.
    [[nodiscard]] double spacing() const noexcept { return nominal_spacing_; }
    [[nodiscard]] double nominal_spacing() const noexcept { return nominal_spacing_; }

    /// Coordinate of face f, throwing std::out_of_range if f >= num_faces().
    [[nodiscard]] double face(std::size_t f) const;

    /// Coordinate of cell center P, throwing std::out_of_range if P >= num_cells().
    [[nodiscard]] double cell_center(std::size_t P) const;

    /// Width of control volume P (dx_P = x_f[P+1] - x_f[P]).
    [[nodiscard]] double cell_width(std::size_t P) const;

    /// Volume of control volume P (In 1D, volume = width * 1 * 1).
    [[nodiscard]] double cell_volume(std::size_t P) const { return cell_width(P); }

    /// Distance between cell centers: P and P+1 (East).
    [[nodiscard]] double distance_to_east_neighbor(std::size_t P) const;

    /// Distance between cell centers: P and P-1 (West).
    [[nodiscard]] double distance_to_west_neighbor(std::size_t P) const;

    /// Read-only spans for zero-overhead vector passes
    [[nodiscard]] std::span<const double> faces() const noexcept { return faces_; }
    [[nodiscard]] std::span<const double> cell_centers() const noexcept { return cell_centers_; }
    [[nodiscard]] std::span<const double> cell_widths() const noexcept { return cell_widths_; }
    [[nodiscard]] std::span<const double> cell_volumes() const noexcept { return cell_widths_; }

    /// Unchecked access
    [[nodiscard]] double operator[](std::size_t P) const noexcept { return cell_centers_[P]; }

    /// Checked element access
    [[nodiscard]] double cell_center_at(std::size_t P) const { return cell_centers_.at(P); }
    [[nodiscard]] double face_at(std::size_t f) const { return faces_.at(f); }

    void print_details(std::ostream& output) const;

private:
    const double x_start_;
    const double x_end_;
    const double nominal_spacing_;
    const std::vector<double> faces_;
    const std::vector<double> cell_widths_;
    const std::vector<double> cell_centers_;
};

/// Type aliases matching OpenFOAM nomenclature
using UniformMesh1D = UniformFvMesh1D;
using Mesh1D = UniformFvMesh1D;

}  // namespace cfd::mesh
