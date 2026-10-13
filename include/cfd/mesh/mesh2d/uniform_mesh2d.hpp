#pragma once

#include "cfd/geometry/domain/domain2d.hpp"

#include <cstddef>
#include <vector>

namespace cfd::mesh
{

/// Cartesian coordinate of a 2D mesh node.
struct Point2D
{
    double x;
    double y;
};

/// A uniform, node-centered Cartesian grid over a closed rectangular domain.
///
/// The grid contains node_count_x() by node_count_y() nodes, including all
/// four boundaries, and cell_count_x() by cell_count_y() rectangular cells.
class UniformMesh2D
{
public:
    using Node = Point2D;

    /// Construct a grid over the Domain2D bounds.
    UniformMesh2D(const cfd::geometry::Domain2D& domain,
                  std::size_t node_count_x, std::size_t node_count_y);

    /// Construct a grid over [x_start, x_end] x [y_start, y_end].
    UniformMesh2D(double x_start, double x_end,
                  double y_start, double y_end,
                  std::size_t node_count_x, std::size_t node_count_y);

    [[nodiscard]] std::size_t node_count_x() const noexcept { return x_nodes_.size(); }
    [[nodiscard]] std::size_t node_count_y() const noexcept { return y_nodes_.size(); }
    [[nodiscard]] std::size_t cell_count_x() const noexcept { return x_nodes_.size() - 1; }
    [[nodiscard]] std::size_t cell_count_y() const noexcept { return y_nodes_.size() - 1; }

    [[nodiscard]] double x_start() const noexcept { return x_start_; }
    [[nodiscard]] double x_end() const noexcept { return x_end_; }
    [[nodiscard]] double y_start() const noexcept { return y_start_; }
    [[nodiscard]] double y_end() const noexcept { return y_end_; }
    [[nodiscard]] double spacing_x() const noexcept { return spacing_x_; }
    [[nodiscard]] double spacing_y() const noexcept { return spacing_y_; }

    /// Return an individual axis coordinate, throwing std::out_of_range if invalid.
    [[nodiscard]] double x_node(std::size_t i) const;
    [[nodiscard]] double y_node(std::size_t j) const;

    /// Return node (j, i): j is the matrix row/Y index; i is column/X.
    /// Throws std::out_of_range if either index is invalid.
    [[nodiscard]] Node node(std::size_t j, std::size_t i) const;

    /// Unchecked matrix-style access: first index is row/Y, second is column/X.
    [[nodiscard]] Node operator()(std::size_t j, std::size_t i) const noexcept
    {
        return {x_nodes_[i], y_nodes_[j]};
    }

    /// Read-only access to the independent coordinates along each axis.
    [[nodiscard]] const std::vector<double>& x_nodes() const noexcept { return x_nodes_; }
    [[nodiscard]] const std::vector<double>& y_nodes() const noexcept { return y_nodes_; }

    /// Change both node counts while preserving the rectangular domain.
    void set_node_counts(std::size_t node_count_x, std::size_t node_count_y);

private:
    double x_start_;
    double x_end_;
    double y_start_;
    double y_end_;
    double spacing_x_;
    double spacing_y_;
    std::vector<double> x_nodes_;
    std::vector<double> y_nodes_;
};

using Mesh2D = UniformMesh2D;

}  // namespace cfd::mesh
