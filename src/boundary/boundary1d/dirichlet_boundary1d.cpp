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

BoundarySide1D DirichletBoundary1D::side() const noexcept
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

    std::size_t p = 0;
    double distance = 0.0;
    switch (side_)
    {
        case BoundarySide1D::Start:
            p = 0;
            distance = mesh.wFaceDx(p);
            break;
        case BoundarySide1D::End:
            p = n_cells - 1;
            distance = mesh.eFaceDx(p);
            break;
        default:
            throw std::invalid_argument("Invalid Dirichlet boundary side");
    }

    const double gamma_p = gamma[p];
    if (!std::isfinite(gamma_p) || gamma_p < 0.0)
    {
        throw std::invalid_argument(
            "Dirichlet diffusion coefficient must be finite and nonnegative");
    }
    if (!std::isfinite(distance) || distance <= 0.0)
    {
        throw std::invalid_argument("Dirichlet boundary distance must be finite and positive");
    }

    const double conductance = gamma_p / distance;
    const double source = conductance * value_;
    if (!std::isfinite(conductance) || !std::isfinite(source))
    {
        throw std::overflow_error("Dirichlet boundary contribution is not finite");
    }

    const double updated_diagonal = matrix.a_P(p) + conductance;
    const double updated_source = matrix.b(p) + source;
    if (!std::isfinite(updated_diagonal) || !std::isfinite(updated_source))
    {
        throw std::overflow_error("Accumulated Dirichlet boundary row is not finite");
    }

    matrix.Add_a_P(p, conductance);
    matrix.Add_b(p, source);
}

}  // namespace cfd::boundary
