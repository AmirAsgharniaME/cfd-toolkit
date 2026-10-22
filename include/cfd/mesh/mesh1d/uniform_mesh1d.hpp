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
/// A mesh with N cells has N+1 faces and N cell centers. The first and last
/// faces coincide with the domain boundaries. Cell values are associated with
/// cell centers; face coordinates bound the corresponding control volumes.
/// Geometry is fixed after construction so fields holding a reference to this
/// mesh cannot become desynchronized by resizing it.
class UniformMesh1D
{
public:
    /// Construct a mesh with `num_cells` cells over the domain bounds.
    explicit UniformMesh1D(const cfd::geometry::Domain1D& domain,
                           std::size_t num_cells);

    /// Construct a mesh with `num_cells` cells over [x_start, x_end].
    explicit UniformMesh1D(double x_start, double x_end, std::size_t num_cells);

    UniformMesh1D(const UniformMesh1D&) = default;
    UniformMesh1D(UniformMesh1D&&) = default;
    UniformMesh1D& operator=(const UniformMesh1D&) = delete;
    UniformMesh1D& operator=(UniformMesh1D&&) = delete;

    /// Number of control volumes, N.
    [[nodiscard]] std::size_t num_cells() const noexcept 
    {
        return cell_centers_.size(); 
    }

    /// Number of control-volume faces, N+1.
    [[nodiscard]] std::size_t num_faces() const noexcept 
    { 
        return faces_.size(); 
    }

    [[nodiscard]] double x_start() const noexcept
     { 
        return x_start_;
     }
    [[nodiscard]] double x_end() const noexcept 
    { 
        return x_end_; 
    }

    /// Nominal uniform spacing, (x_end - x_start) / num_cells.
    /// For robust geometric sums, use cell_width(i), which is computed from
    /// the actual adjacent face coordinates.
    [[nodiscard]] double nominal_spacing() const noexcept 
    { 
    return nominal_spacing_;
    }

    /// Coordinate of face f, throwing std::out_of_range if f >= num_faces().
    [[nodiscard]] double face(std::size_t f) const;

    /// Coordinate of cell center P, throwing std::out_of_range if P >= num_cells().
    [[nodiscard]] double cell_center(std::size_t P) const;

    /// Width of control volume P: x_f[P+1] - x_f[P].
    [[nodiscard]] double cell_width(std::size_t P) const;

    /// Distance from cell P center to its eastern neighbor's center.
    /// Throws std::out_of_range if P has no eastern neighbor.
    [[nodiscard]] double distance_to_east_neighbor(std::size_t P) const;

    /// Distance from cell P center to its western neighbor's center.
    /// Throws std::out_of_range if P has no western neighbor.
    [[nodiscard]] double distance_to_west_neighbor(std::size_t P) const;

    /// Read-only, contiguous view of all face coordinates.
    [[nodiscard]] std::span<const double> faces() const noexcept { return faces_; }

    /// Read-only, contiguous view of all cell-center coordinates.
    [[nodiscard]] std::span<const double> cell_centers() const noexcept { return cell_centers_; }

    /// Read-only, contiguous view of actual control-volume widths.
    [[nodiscard]] std::span<const double> cell_widths() const noexcept { return cell_widths_; }

    /// Unchecked cell-center indexing, following std::vector::operator[].
    [[nodiscard]] double operator[](std::size_t P) const noexcept { return cell_centers_[P]; }

    /// Checked cell-center indexing.
    [[nodiscard]] double cell_center_at(std::size_t P) const { return cell_centers_.at(P); }

    /// Checked face indexing.
    [[nodiscard]] double face_at(std::size_t f) const { return faces_.at(f); }

    /// Print mesh bounds, cell/face counts, and the geometric coordinates.
    void print_details(std::ostream& output) const;

    // Compatibility aliases for the former node-centered API. In the FVM
    // representation, old "nodes" correspond to faces and node_count is the
    // face count. Prefer num_faces(), faces(), and face().
    [[nodiscard]] std::size_t node_count() const noexcept { return num_faces(); }
    [[nodiscard]] std::size_t cell_count() const noexcept { return num_cells(); }
    [[nodiscard]] double node(std::size_t f) const { return face(f); }
    [[nodiscard]] std::span<const double> nodes() const noexcept { return faces(); }
    [[nodiscard]] double spacing() const noexcept { return nominal_spacing(); }

private:
    const double x_start_;
    const double x_end_;
    const double nominal_spacing_;
    const std::vector<double> faces_;
    const std::vector<double> cell_widths_;
    const std::vector<double> cell_centers_;
};

/// Compatibility alias for code using the shorter generic name.
using Mesh1D = UniformMesh1D;

}  // namespace cfd::mesh
