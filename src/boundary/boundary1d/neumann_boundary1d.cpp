#include "cfd/boundary/boundary1d/neumann_boundary1d.hpp"

#include "cfd/fv_matrix/fv_matrix1d.hpp"
#include "cfd/mesh/mesh1d/fv_mesh1d.hpp"

#include <cmath>
#include <limits>
#include <stdexcept>
#include <utility>

namespace cfd::boundary
{

NeumannBoundary1D::NeumannBoundary1D(std::string name, BoundarySide1D side,
                                     double normal_gradient)
    : name_(std::move(name)), side_(side), normal_gradient_(normal_gradient)
{
    switch (side_)
    {
        case BoundarySide1D::Start:
        case BoundarySide1D::End:
            break;
        default:
            throw std::invalid_argument("Invalid Neumann boundary side");
    }
}

const std::string& NeumannBoundary1D::name() const noexcept
{
    return name_;
}

BoundarySide1D NeumannBoundary1D::side() const noexcept
{
    return side_;
}

double NeumannBoundary1D::normal_gradient() const noexcept
{
    return normal_gradient_;
}

double NeumannBoundary1D::outward_normal() const noexcept
{
    switch (side_)
    {
        case BoundarySide1D::Start:
            return -1.0;
        case BoundarySide1D::End:
            return 1.0;
        default:
            return std::numeric_limits<double>::quiet_NaN();
    }
}

double NeumannBoundary1D::coordinate_gradient() const noexcept
{
    return normal_gradient_ * outward_normal();
}

void NeumannBoundary1D::set_normal_gradient(double normal_gradient) noexcept
{
    normal_gradient_ = normal_gradient;
}

void NeumannBoundary1D::Apply(const field::VolScalarField1D& gamma,
                              fv_matrix::FvMatrix1D& matrix) const
{
    const cfd::mesh::FvMesh1D& mesh = matrix.Mesh();
    if (&gamma.Mesh() != &mesh)
    {
        throw std::invalid_argument(
            "Neumann coefficient field and matrix must use the same mesh object");
    }

    const std::size_t n_cells = mesh.N_Cells();
    if (n_cells == 0 || gamma.size() != n_cells || matrix.size() != n_cells)
    {
        throw std::invalid_argument(
            "Neumann coefficient field and matrix sizes must match a nonempty mesh");
    }
    if (!std::isfinite(normal_gradient_))
    {
        throw std::invalid_argument("Neumann normal gradient must be finite");
    }

    std::size_t face_id = 0;
    switch (side_)
    {
        case BoundarySide1D::Start:
            face_id = 0;
            break;
        case BoundarySide1D::End:
            face_id = mesh.N_Faces() - 1;
            break;
        default:
            throw std::invalid_argument("Invalid Neumann boundary side");
    }

    const std::size_t owner = mesh.Owner(face_id);
    if (owner >= n_cells)
    {
        throw std::invalid_argument("Neumann boundary owner is outside the mesh");
    }
    const double distance = mesh.Distance_Owner_face(face_id);
    const double face_area = mesh.Face_Area(face_id);
    const double gamma_owner = gamma[owner];
    if (!std::isfinite(gamma_owner) || gamma_owner < 0.0)
    {
        throw std::invalid_argument(
            "Neumann diffusion coefficient must be finite and nonnegative");
    }
    if (!std::isfinite(distance) || !(distance > 0.0))
    {
        throw std::invalid_argument("Neumann boundary distance must be finite and positive");
    }
    if (!std::isfinite(face_area) || face_area <= 0.0)
    {
        throw std::invalid_argument("Neumann boundary face area must be finite and positive");
    }

    const double boundary_source = gamma_owner * normal_gradient_ * face_area;
    if (!std::isfinite(boundary_source))
    {
        throw std::overflow_error("Neumann boundary contribution is not finite");
    }

    const double updated_source = matrix.b(owner) + boundary_source;
    if (!std::isfinite(updated_source))
    {
        throw std::overflow_error("Accumulated Neumann boundary source is not finite");
    }

    matrix.Add_b(owner, boundary_source);
}

}  // namespace cfd::boundary
