#include "cfd/mesh/mesh1d/non_uniform_fv_mesh1d.hpp"

#include <cmath>
#include <stdexcept>
#include <string>
#include <utility>

namespace cfd::mesh
{
namespace
{

std::vector<double> validate_faces(const geometry::Domain1D& domain,
                                   std::vector<double> faces)
{
    if (faces.size() < 2)
    {
        throw std::invalid_argument(
            "NonUniformFvMesh1D: requires at least two face positions (>= 1 cell).");
    }

    for (std::size_t face = 0; face < faces.size(); ++face)
    {
        if (!std::isfinite(faces[face]))
            throw std::invalid_argument("NonUniformFvMesh1D: non-finite face coordinate at index " +
                                        std::to_string(face));
        if (face > 0 && !(faces[face] > faces[face - 1]))
            throw std::invalid_argument("NonUniformFvMesh1D: face coordinates must increase strictly at index " +
                                        std::to_string(face));
    }

    if (faces.front() != domain.start() || faces.back() != domain.end())
    {
        throw std::invalid_argument(
            "NonUniformFvMesh1D: first and last face positions must exactly match the domain bounds.");
    }

    return faces;
}

}  // namespace

NonUniformFvMesh1D::NonUniformFvMesh1D(const geometry::Domain1D& domain,
                                       std::vector<double> faces)
    : FvMesh1D(validate_faces(domain, std::move(faces)))
{
}

}  // namespace cfd::mesh
