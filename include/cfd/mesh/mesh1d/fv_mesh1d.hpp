#pragma once

#include <cstddef>
#include <iosfwd>
#include <span>
#include <vector>

namespace cfd::mesh
{

/// Base class for all 1D Finite-Volume meshes (Uniform and Non-uniform).
/// Holds common topological arrays and provides unified FVM geometric queries.
class FvMesh1D
{
public:
    virtual ~FvMesh1D() = default;

    FvMesh1D(const FvMesh1D&) = default;
    FvMesh1D(FvMesh1D&&) noexcept = default;
    FvMesh1D& operator=(const FvMesh1D&) = delete;
    FvMesh1D& operator=(FvMesh1D&&) = delete;

    [[nodiscard]] std::size_t num_cells() const noexcept { return cell_centers_.size(); }
    [[nodiscard]] std::size_t cell_count() const noexcept { return num_cells(); }

    [[nodiscard]] std::size_t num_faces() const noexcept { return faces_.size(); }
    [[nodiscard]] std::size_t face_count() const noexcept { return num_faces(); }

    [[nodiscard]] double x_start() const noexcept { return x_start_; }
    [[nodiscard]] double x_end() const noexcept { return x_end_; }

    [[nodiscard]] double face(std::size_t f) const;
    [[nodiscard]] double cell_center(std::size_t P) const;
    [[nodiscard]] double cell_width(std::size_t P) const;
    [[nodiscard]] double cell_volume(std::size_t P) const { return cell_width(P); }

    [[nodiscard]] double distance_to_east_neighbor(std::size_t P) const;
    [[nodiscard]] double distance_to_west_neighbor(std::size_t P) const;

    [[nodiscard]] std::span<const double> faces() const noexcept { return faces_; }
    [[nodiscard]] std::span<const double> cell_centers() const noexcept { return cell_centers_; }
    [[nodiscard]] std::span<const double> cell_widths() const noexcept { return cell_widths_; }
    [[nodiscard]] std::span<const double> cell_volumes() const noexcept { return cell_widths_; }

    [[nodiscard]] double operator[](std::size_t P) const noexcept { return cell_centers_[P]; }
    [[nodiscard]] double cell_center_at(std::size_t P) const { return cell_centers_.at(P); }
    [[nodiscard]] double face_at(std::size_t f) const { return faces_.at(f); }

    void print_details(std::ostream& output) const;

protected:
    /// Protected: only derived meshes construct with an explicit faces array.
    explicit FvMesh1D(std::vector<double> faces);

private:
    const double x_start_;
    const double x_end_;
    const std::vector<double> faces_;
    const std::vector<double> cell_widths_;
    const std::vector<double> cell_centers_;
};

}  // namespace cfd::mesh
