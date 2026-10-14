#include "cfd/mesh/mesh2d/uniform_mesh2d.hpp"

#include <cmath>
#include <iomanip>
#include <ostream>
#include <stdexcept>
#include <utility>

namespace cfd::mesh
{
namespace
{

double compute_spacing(double start, double end, std::size_t node_count)
{
    if (node_count < 2)
    {
        throw std::invalid_argument("A 2D mesh must have at least two nodes per axis.");
    }
    if (!std::isfinite(start) || !std::isfinite(end) || end <= start)
    {
        throw std::invalid_argument("Mesh bounds must be finite and strictly increasing.");
    }

    const double length = end - start;
    if (!std::isfinite(length))
    {
        throw std::invalid_argument("Mesh interval length must be finite.");
    }

    const double spacing = length / static_cast<double>(node_count - 1);
    if (!std::isfinite(spacing) || spacing <= 0.0)
    {
        throw std::invalid_argument("Mesh spacing must be finite and greater than zero.");
    }
    return spacing;
}

std::vector<double> make_axis_nodes(double start, double end,
                                   std::size_t node_count, double spacing)
{
    std::vector<double> nodes(node_count);
    nodes.front() = start;

    for (std::size_t index = 1; index + 1 < node_count; ++index)
    {
        const double coordinate = start + static_cast<double>(index) * spacing;
        if (!std::isfinite(coordinate) || coordinate <= nodes[index - 1] || coordinate >= end)
        {
            throw std::invalid_argument(
                "The requested node count exceeds coordinate precision for this interval.");
        }
        nodes[index] = coordinate;
    }

    nodes.back() = end;
    return nodes;
}

}  // namespace

UniformMesh2D::UniformMesh2D(const cfd::geometry::Domain2D& domain,
                             std::size_t node_count_x, std::size_t node_count_y)
    : UniformMesh2D(domain.x_start(), domain.x_end(),
                    domain.y_start(), domain.y_end(), node_count_x, node_count_y)
{
}

UniformMesh2D::UniformMesh2D(double x_start, double x_end,
                             double y_start, double y_end,
                             std::size_t node_count_x, std::size_t node_count_y)
    : x_start_(x_start),
      x_end_(x_end),
      y_start_(y_start),
      y_end_(y_end),
      spacing_x_(compute_spacing(x_start, x_end, node_count_x)),
      spacing_y_(compute_spacing(y_start, y_end, node_count_y)),
      x_nodes_(make_axis_nodes(x_start_, x_end_, node_count_x, spacing_x_)),
      y_nodes_(make_axis_nodes(y_start_, y_end_, node_count_y, spacing_y_))
{
}

double UniformMesh2D::x_node(std::size_t i) const
{
    return x_nodes_.at(i);
}

double UniformMesh2D::y_node(std::size_t j) const
{
    return y_nodes_.at(j);
}

UniformMesh2D::Node UniformMesh2D::node(std::size_t j, std::size_t i) const
{
    return {x_nodes_.at(i), y_nodes_.at(j)};
}

void UniformMesh2D::set_node_counts(std::size_t node_count_x, std::size_t node_count_y)
{
    const double new_spacing_x = compute_spacing(x_start_, x_end_, node_count_x);
    const double new_spacing_y = compute_spacing(y_start_, y_end_, node_count_y);
    std::vector<double> new_x_nodes =
        make_axis_nodes(x_start_, x_end_, node_count_x, new_spacing_x);
    std::vector<double> new_y_nodes =
        make_axis_nodes(y_start_, y_end_, node_count_y, new_spacing_y);

    x_nodes_.swap(new_x_nodes);
    y_nodes_.swap(new_y_nodes);
    spacing_x_ = new_spacing_x;
    spacing_y_ = new_spacing_y;
}

void UniformMesh2D::print_details(std::ostream& output) const
{
    const std::ios::fmtflags original_flags = output.flags();
    const std::streamsize original_precision = output.precision();

    output.setf(std::ios::fixed, std::ios::floatfield);
    output.precision(3);

    output << "Mesh (" << node_count_y() << " rows x " << node_count_x() << " columns)\n"
           << "Spacing: dx = " << spacing_x_
           << ", dy = " << spacing_y_ << "\n\n";

    const int label_width = 20;
    const int column_width = 12;

    output << std::setw(label_width) << "x ->";
    for (std::size_t i = 0; i < node_count_x(); ++i)
    {
        output << std::setw(column_width) << i;
    }
    output << '\n';

    output << std::setw(label_width) << " ";
    for (double x : x_nodes_)
    {
        output << std::setw(column_width) << x;
    }
    output << '\n';

    for (std::size_t row = 0; row < node_count_y(); ++row)
    {
        const std::size_t j = node_count_y() - 1 - row;

        output << std::setw(4) << "j=" << std::setw(3) << j
               << std::setw(4) << "y=" << std::setw(9) << y_nodes_[j];

        for (std::size_t i = 0; i < node_count_x(); ++i)
        {
            output << std::setw(column_width) << "o";
        }
        output << '\n';

        if (row != node_count_y() - 1)
        {
            output << std::setw(label_width) << " ";
            for (std::size_t i = 0; i < node_count_x(); ++i)
            {
                output << std::setw(column_width) << "|";
            }
            output << '\n';
        }
    }

    output.flags(original_flags);
    output.precision(original_precision);
}

}  // namespace cfd::mesh
