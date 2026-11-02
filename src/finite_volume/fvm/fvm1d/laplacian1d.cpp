#include "cfd/finite_volume/fvm/fvm1d/laplacian1d.hpp"

#include "cfd/field/field1d/field1d.hpp"
#include "cfd/fv_matrix/fv_matrix1d.hpp"
#include "cfd/mesh/mesh1d/fv_mesh1d.hpp"

#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <type_traits>

namespace cfd::fvm {
namespace {

double face_diffusivity(double gamma_p, double gamma_n,
                        double distance_p, double distance_n)
{
    if (gamma_p == 0.0 || gamma_n == 0.0) {
        return 0.0;
    }

    // Resistance-weighted harmonic interpolation, with the cell-center to
    // face distances serving as the interpolation weights.
    const double distance = distance_p + distance_n;
    const double resistance = distance_p / gamma_p + distance_n / gamma_n;
    const double value = distance / resistance;
    if (!std::isfinite(value)) {
        throw std::overflow_error("Laplacian face diffusivity is not finite");
    }
    return value;
}

void validate_inputs(const field::VolScalarField1D& gamma,
                     const fv_matrix::FvMatrix1D& matrix)
{
    const mesh::FvMesh1D& mesh = matrix.Mesh();
    if (&gamma.Mesh() != &mesh) {
        throw std::invalid_argument(
            "Laplacian coefficient field and matrix must use the same mesh object");
    }
    if (gamma.Size() != mesh.N_Cells() || matrix.Size() != mesh.N_Cells()) {
        throw std::invalid_argument("Laplacian field and matrix sizes must match the mesh");
    }

    for (std::size_t p = 0; p < gamma.Size(); ++p) {
        const double value = gamma[p];
        if (!std::isfinite(value) || value < 0.0) {
            throw std::invalid_argument(
                "Laplacian diffusion coefficients must be finite and nonnegative");
        }
    }
}

void validate_boundary(const BoundaryCondition1D& condition,
                       boundary::BoundarySide1D expected_side)
{
    std::visit([expected_side](const auto& boundary_condition) {
        using Condition = std::decay_t<decltype(boundary_condition)>;
        if (boundary_condition.side() != expected_side) {
            throw std::invalid_argument("Laplacian boundary condition is on the wrong side");
        }

        double value = 0.0;
        if constexpr (std::is_same_v<Condition, boundary::DirichletBoundary1D>) {
            value = boundary_condition.value();
        } else {
            value = boundary_condition.normal_gradient();
        }
        if (!std::isfinite(value)) {
            throw std::invalid_argument("Laplacian boundary value must be finite");
        }
    }, condition);
}

void assemble(const field::VolScalarField1D& gamma,
              fv_matrix::FvMatrix1D& matrix,
              const BoundaryCondition1D& start_boundary,
              const BoundaryCondition1D& end_boundary)
{
    validate_inputs(gamma, matrix);
    validate_boundary(start_boundary, boundary::BoundarySide1D::Start);
    validate_boundary(end_boundary, boundary::BoundarySide1D::End);

    const mesh::FvMesh1D& mesh = matrix.Mesh();
    const std::size_t n_cells = mesh.N_Cells();

    // Each internal face contributes equal and opposite fluxes to its two
    // adjacent control volumes. In the matrix convention used here, the
    // off-diagonals are positive and the diagonal is negative for div(grad).
    for (std::size_t p = 0; p + 1 < n_cells; ++p) {
        const std::size_t n = p + 1;
        const double gamma_face = face_diffusivity(
            gamma[p], gamma[n], mesh.eFaceDx(p), mesh.wFaceDx(n));
        const double conductance = gamma_face / mesh.eCellDx(p);
        if (!std::isfinite(conductance)) {
            throw std::overflow_error("Laplacian internal-face conductance is not finite");
        }

        matrix.add_a_E(p, conductance);
        matrix.add_a_P(p, -conductance);
        matrix.add_a_W(n, conductance);
        matrix.add_a_P(n, -conductance);
    }

    const auto add_boundary = [&gamma, &matrix, &mesh](
        const BoundaryCondition1D& condition, std::size_t p, bool is_start) {
        std::visit([&gamma, &matrix, &mesh, p, is_start](const auto& boundary_condition) {
            using Condition = std::decay_t<decltype(boundary_condition)>;
            if constexpr (std::is_same_v<Condition, boundary::DirichletBoundary1D>) {
                const double distance = is_start ? mesh.wFaceDx(p) : mesh.eFaceDx(p);
                const double conductance = gamma[p] / distance;
                const double source = conductance * boundary_condition.value();
                if (!std::isfinite(conductance) || !std::isfinite(source)) {
                    throw std::overflow_error("Laplacian boundary contribution is not finite");
                }
                matrix.add_a_P(p, -conductance);
                matrix.add_b(p, source);
            } else {
                const double contribution = gamma[p] * boundary_condition.normal_gradient();
                if (!std::isfinite(contribution)) {
                    throw std::overflow_error("Laplacian Neumann contribution is not finite");
                }
                matrix.add_b(p, contribution);
            }
        }, condition);
    };

    add_boundary(start_boundary, 0, true);
    add_boundary(end_boundary, n_cells - 1, false);
}

} // namespace

void laplacian(const field::VolScalarField1D& gamma,
               fv_matrix::FvMatrix1D& matrix)
{
    const BoundaryCondition1D zero_start_gradient{
        boundary::NeumannBoundary1D{"zero_gradient_start", boundary::BoundarySide1D::Start, 0.0}};
    const BoundaryCondition1D zero_end_gradient{
        boundary::NeumannBoundary1D{"zero_gradient_end", boundary::BoundarySide1D::End, 0.0}};
    assemble(gamma, matrix, zero_start_gradient, zero_end_gradient);
}

void laplacian(const field::VolScalarField1D& gamma,
               fv_matrix::FvMatrix1D& matrix,
               const BoundaryCondition1D& start_boundary,
               const BoundaryCondition1D& end_boundary)
{
    assemble(gamma, matrix, start_boundary, end_boundary);
}

} // namespace cfd::fvm
