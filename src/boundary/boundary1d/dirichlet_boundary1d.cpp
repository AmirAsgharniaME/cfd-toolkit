#include "cfd/boundary/boundary1d/dirichlet_boundary1d.hpp"

#include "cfd/fv_matrix/fv_matrix1d.hpp"
#include "cfd/mesh/mesh1d/fv_mesh1d.hpp"

#include <cmath>
#include <stdexcept>
#include <utility>

namespace cfd::boundary
{

DirichletBoundary1D::DirichletBoundary1D(std::string name, BoundarySide1D side, double value)
    : name_(std::move(name)), side_(side), value_(value)
{
}

const std::string& DirichletBoundary1D::Name() const noexcept
{
    return name_;
}

BoundarySide1D DirichletBoundary1D::Side() const noexcept
{
    return side_;
}

double DirichletBoundary1D::Value() const noexcept
{
    return value_;
}

void DirichletBoundary1D::Apply(const field::VolScalarField1D& gamma,
                                fv_matrix::FvMatrix1D& matrix) const
{
    const cfd::mesh::FvMesh1D& mesh = matrix.Mesh();
    if (&gamma.Mesh() != &mesh)
    {
        throw std::invalid_argument(
            "Dirichlet coefficient field and matrix must use the same mesh object");
    }

    const std::size_t n_cells = mesh.N_Cells();
    if (n_cells == 0 || gamma.size() != n_cells || matrix.size() != n_cells)
    {
        throw std::invalid_argument(
            "Dirichlet coefficient field and matrix sizes must match a nonempty mesh");
    }
    if (!std::isfinite(value_))
    {
        throw std::invalid_argument("Dirichlet boundary value must be finite");
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
            throw std::invalid_argument("Invalid Dirichlet boundary side");
    }

    const std::size_t owner = mesh.Owner(face_id);
    const double distance = mesh.Distance_Owner_face(face_id);
    const double face_area = mesh.Face_Area(face_id);
    const double gamma_owner = gamma[owner];
    if (!std::isfinite(gamma_owner) || gamma_owner < 0.0)
    {
        throw std::invalid_argument(
            "Dirichlet diffusion coefficient must be finite and nonnegative");
    }
    if (!std::isfinite(distance) || !(distance > 0.0))
    {
        throw std::invalid_argument("Dirichlet boundary distance must be finite and positive");
    }
    if (!std::isfinite(face_area) || face_area <= 0.0)
    {
        throw std::invalid_argument("Dirichlet boundary face area must be finite and positive");
    }

    const double D_b = (gamma_owner * face_area) / distance;
    const double boundary_source = D_b * value_;
    if (!std::isfinite(D_b) || !std::isfinite(boundary_source))
    {
        throw std::overflow_error("Dirichlet boundary contribution is not finite");
    }

    const double updated_diagonal = matrix.a_P(owner) + D_b;
    const double updated_source = matrix.b(owner) + boundary_source;
    if (!std::isfinite(updated_diagonal) || !std::isfinite(updated_source))
    {
        throw std::overflow_error("Accumulated Dirichlet boundary row is not finite");
    }

    matrix.Add_a_P(owner, D_b);
    matrix.Add_b(owner, boundary_source);
}

}  // namespace cfd::boundary
