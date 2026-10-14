#include "cfd/mesh/mesh1d/uniform_mesh1d.hpp"

#include <cmath>
#include <iomanip>
#include <iostream>
#include <stdexcept>

namespace cfd::mesh
{
namespace
{

double compute_spacing(double x_start, double x_end, std::size_t node_count)
{
    if (node_count < 2)
    {
        throw std::invalid_argument("A 1D mesh must contain at least two nodes.");
    }
    if (!std::isfinite(x_start) || !std::isfinite(x_end) || x_end <= x_start)
    {
        throw std::invalid_argument("Mesh interval bounds must be finite and increasing.");
    }

    const double interval_length = x_end - x_start;
    if (!std::isfinite(interval_length))
    {
        throw std::invalid_argument("Mesh interval length must be finite.");
    }

    const double spacing = interval_length / static_cast<double>(node_count - 1);
    if (!std::isfinite(spacing) || spacing <= 0.0)
    {
        throw std::invalid_argument("Mesh spacing must be finite and greater than zero.");
    }
    return spacing;
}

std::vector<double> make_nodes(double x_start, double x_end,
                               std::size_t node_count, double spacing)
{
    std::vector<double> nodes(node_count);
    nodes.front() = x_start;

    for (std::size_t index = 1; index + 1 < node_count; ++index)
    {
        const double coordinate = x_start + static_cast<double>(index) * spacing;
        if (!std::isfinite(coordinate) || coordinate <= nodes[index - 1] || coordinate >= x_end)
        {
            throw std::invalid_argument(
                "The requested node count exceeds the coordinate precision for this interval.");
        }
        nodes[index] = coordinate;
    }

    // Avoid accumulated rounding error at the right boundary.
    nodes.back() = x_end;
    return nodes;
}

}  // namespace

UniformMesh1D::UniformMesh1D(const cfd::geometry::Domain1D& domain, std::size_t node_count)
    : UniformMesh1D(domain.x_start(), domain.x_end(), node_count)
{
}

UniformMesh1D::UniformMesh1D(double x_start, double x_end, std::size_t node_count)
    : x_start_(x_start),
      x_end_(x_end),
      spacing_(compute_spacing(x_start, x_end, node_count)),
      nodes_(make_nodes(x_start_, x_end_, node_count, spacing_))
{
}

double UniformMesh1D::node(std::size_t index) const
{
    return nodes_.at(index);
}

void UniformMesh1D::set_node_count(std::size_t node_count)
{
    const double new_spacing = compute_spacing(x_start_, x_end_, node_count);
    std::vector<double> new_nodes = make_nodes(x_start_, x_end_, node_count, new_spacing);

    // Commit the new state only after validation and allocation have succeeded.
    nodes_.swap(new_nodes);
    spacing_ = new_spacing;
}

void UniformMesh1D::print_details(std::ostream& output) const
{
    const std::ios::fmtflags original_flags = output.flags();
    const std::streamsize original_precision = output.precision();

    output << std::fixed << std::setprecision(4)
           << "UniformMesh1D details\n"
           << "---------------------\n"
           << "Start:  " << x_start_ << '\n'
           << "End:    " << x_end_ << '\n'
           << "Length: " << x_end_ - x_start_ << '\n'
           << "Nodes:  " << node_count() << '\n'
           << "Cells:  " << cell_count() << '\n'
           << "dx:     " << spacing_ << "\n\n"
           << "Node coordinates\n"
           << "----------------\n"
           << "Index    x\n";

    for (std::size_t index = 0; index < node_count(); ++index)
    {
        output << std::setw(5) << index << "    " << nodes_[index] << '\n';
    }

    output.flags(original_flags);
    output.precision(original_precision);
}

}  // namespace cfd::mesh
