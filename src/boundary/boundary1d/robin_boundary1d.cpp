#include "cfd/boundary/boundary1d/robin_boundary1d.hpp"

#include "cfd/fv_matrix/fv_matrix1d.hpp"
#include "cfd/mesh/mesh1d/fv_mesh1d.hpp"

#include <cmath>
#include <stdexcept>
#include <utility>

namespace cfd::boundary
{

RobinBoundary1D::RobinBoundary1D(std::string name, BoundarySide1D side,
                                 double transfer_coefficient, double ambient_value)
    : name_(std::move(name)), side_(side), transfer_coefficient_(transfer_coefficient),
      ambient_value_(ambient_value)
{
    switch (side_)
    {
        case BoundarySide1D::Start:
        case BoundarySide1D::End:
            break;
        default:
            throw std::invalid_argument("Invalid Robin boundary side");
    }
}

const std::string& RobinBoundary1D::name() const noexcept { return name_; }
BoundarySide1D RobinBoundary1D::side() const noexcept { return side_; }
double RobinBoundary1D::transfer_coefficient() const noexcept { return transfer_coefficient_; }
double RobinBoundary1D::ambient_value() const noexcept { return ambient_value_; }
void RobinBoundary1D::set_transfer_coefficient(double value) noexcept { transfer_coefficient_ = value; }
void RobinBoundary1D::set_ambient_value(double value) noexcept { ambient_value_ = value; }

void RobinBoundary1D::Apply(const field::VolScalarField1D& gamma,
                            fv_matrix::FvMatrix1D& matrix) const
{
    const cfd::mesh::FvMesh1D& mesh = matrix.Mesh();
    if (&gamma.Mesh() != &mesh)
        throw std::invalid_argument("Robin coefficient field and matrix must use the same mesh object");

    const std::size_t n_cells = mesh.N_Cells();
    if (n_cells == 0 || gamma.size() != n_cells || matrix.size() != n_cells)
        throw std::invalid_argument("Robin coefficient field and matrix sizes must match a nonempty mesh");
    if (!std::isfinite(transfer_coefficient_) || transfer_coefficient_ < 0.0)
        throw std::invalid_argument("Robin transfer coefficient must be finite and nonnegative");
    if (!std::isfinite(ambient_value_))
        throw std::invalid_argument("Robin ambient value must be finite");

    std::size_t face_id = 0;
    switch (side_)
    {
        case BoundarySide1D::Start: face_id = 0; break;
        case BoundarySide1D::End: face_id = mesh.N_Faces() - 1; break;
        default: throw std::invalid_argument("Invalid Robin boundary side");
    }
    const std::size_t owner = mesh.Owner(face_id);
    if (owner >= n_cells)
        throw std::invalid_argument("Robin boundary owner is outside the mesh");
    const double distance = mesh.Distance_Owner_face(face_id);
    const double face_area = mesh.Face_Area(face_id);
    const double gamma_owner = gamma[owner];
    if (!std::isfinite(gamma_owner) || gamma_owner < 0.0)
        throw std::invalid_argument("Robin diffusion coefficient must be finite and nonnegative");
    if (!std::isfinite(distance) || !(distance > 0.0))
        throw std::invalid_argument("Robin boundary distance must be finite and positive");
    if (!std::isfinite(face_area) || face_area <= 0.0)
        throw std::invalid_argument("Robin boundary face area must be finite and positive");

    const double boundary_diagonal = transfer_coefficient_ * face_area;
    const double boundary_source = boundary_diagonal * ambient_value_;
    if (!std::isfinite(boundary_diagonal) || !std::isfinite(boundary_source))
        throw std::overflow_error("Robin boundary contribution is not finite");
    const double updated_diagonal = matrix.a_P(owner) + boundary_diagonal;
    const double updated_source = matrix.b(owner) + boundary_source;
    if (!std::isfinite(updated_diagonal) || !std::isfinite(updated_source))
        throw std::overflow_error("Accumulated Robin boundary row is not finite");

    matrix.Add_a_P(owner, boundary_diagonal);
    matrix.Add_b(owner, boundary_source);
}

}  // namespace cfd::boundary
