#include "cfd/boundary/boundary1d/boundary_condition1d.hpp"

#include "cfd/fv_matrix/fv_matrix1d.hpp"
#include "cfd/mesh/mesh1d/fv_mesh1d.hpp"

#include <cmath>
#include <stdexcept>
#include <utility>

namespace cfd::boundary
{

BoundaryCondition1D::BoundaryCondition1D(std::string name, BoundarySide1D side)
    : name_(std::move(name)), side_(side)
{
}

const std::string& BoundaryCondition1D::Name() const noexcept
{
    return name_;
}

BoundarySide1D BoundaryCondition1D::Side() const noexcept
{
    return side_;
}

BoundaryAssemblyContext1D BoundaryCondition1D::Prepare_Assembly(
    const field::VolScalarField1D& gamma,
    const fv_matrix::FvMatrix1D& matrix,
    std::string_view condition_name) const
{
    const mesh::FvMesh1D& mesh = matrix.Mesh();
    const std::string condition(condition_name);

    if (&gamma.Mesh() != &mesh)
    {
        throw std::invalid_argument(condition +
                                    " coefficient field and matrix must use the same mesh object");
    }

    const std::size_t n_cells = mesh.N_Cells();
    if (n_cells == 0 || gamma.size() != n_cells || matrix.size() != n_cells)
    {
        throw std::invalid_argument(condition +
                                    " coefficient field and matrix sizes must match a nonempty mesh");
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
            throw std::invalid_argument("Invalid " + condition + " boundary side");
    }

    const std::size_t owner = mesh.Owner(face_id);
    if (owner >= n_cells)
    {
        throw std::invalid_argument(condition + " boundary owner is outside the mesh");
    }

    const double gamma_owner = gamma[owner];
    if (!std::isfinite(gamma_owner) || gamma_owner < 0.0)
    {
        throw std::invalid_argument(condition +
                                    " diffusion coefficient must be finite and nonnegative");
    }

    const double owner_face_distance = mesh.Distance_Owner_face(face_id);
    if (!std::isfinite(owner_face_distance) || !(owner_face_distance > 0.0))
    {
        throw std::invalid_argument(condition +
                                    " boundary distance must be finite and positive");
    }

    const double face_area = mesh.Face_Area(face_id);
    if (!std::isfinite(face_area) || !(face_area > 0.0))
    {
        throw std::invalid_argument(condition +
                                    " boundary face area must be finite and positive");
    }

    return BoundaryAssemblyContext1D{
        mesh, face_id, owner, gamma_owner, face_area, owner_face_distance};
}

}  // namespace cfd::boundary
