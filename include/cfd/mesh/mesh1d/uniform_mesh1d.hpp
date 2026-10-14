#pragma once

#include "cfd/geometry/domain/domain1d.hpp"

#include <cstddef>
#include <iosfwd>
#include <vector>

namespace cfd::mesh
{

/// A uniform, node-centered 1D grid over a closed interval [x_start, x_end].
///
/// The grid contains node_count() points, including both endpoints, and
/// cell_count() intervals between those points.
class UniformMesh1D
{
public:

    /// Construct a grid over the domain's [x_start, x_end] bounds.
    UniformMesh1D(const cfd::geometry::Domain1D& domain, std::size_t node_count);

    /// Construct a grid over [x_start, x_end], including both endpoints.
    UniformMesh1D(double x_start, double x_end, std::size_t node_count);

    [[nodiscard]] std::size_t node_count() const noexcept { return nodes_.size(); }
    [[nodiscard]] std::size_t cell_count() const noexcept { return nodes_.size() - 1; }

    [[nodiscard]] double x_start() const noexcept { return x_start_; }
    [[nodiscard]] double x_end() const noexcept { return x_end_; }
    [[nodiscard]] double spacing() const noexcept { return spacing_; }

    /// Return a node position, throwing std::out_of_range for an invalid index.
    [[nodiscard]] double node(std::size_t index) const;

    /// Unchecked node access, following the convention of std::vector::operator[].
    [[nodiscard]] double operator[](std::size_t index) const noexcept
    {
        return nodes_[index];
    }

    /// Read-only access to all node positions.
    [[nodiscard]] const std::vector<double>& nodes() const noexcept { return nodes_; }

    /// Change the number of nodes while preserving the interval.
    void set_node_count(std::size_t node_count);

    /// Print mesh properties and all node coordinates.
    void print_details(std::ostream& output) const;

private:
    double x_start_;
    double x_end_;
    double spacing_;
    std::vector<double> nodes_;
};

// Retain the shorter name for code that refers to a generic 1D mesh.
using Mesh1D = UniformMesh1D;

}  // namespace cfd::mesh
