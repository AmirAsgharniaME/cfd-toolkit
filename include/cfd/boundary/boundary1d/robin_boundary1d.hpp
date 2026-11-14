#pragma once

#include "cfd/boundary/types.hpp"
#include "cfd/field/field1d/field1d.hpp"

#include <string>

namespace cfd::fv_matrix
{
class FvMatrix1D;
}

namespace cfd::boundary
{

/**
 * @brief First-order Robin boundary condition for a 1D domain.
 *
 * The condition is -Gamma * grad(phi) . n_out = h * (phi_b - phi_inf),
 * where n_out is the outward unit normal. The current first-order closure
 * sets phi_b = phi_P, giving the assembled contributions
 * a_P += h * A_b and b_P += h * A_b * phi_inf.
 * A higher-order face-value closure may be added later.
 */
class RobinBoundary1D
{
public:
    /// Construct a boundary; an invalid side throws std::invalid_argument.
    RobinBoundary1D(std::string name, BoundarySide1D side, double transfer_coefficient,
                    double ambient_value);

    [[nodiscard]] const std::string& name() const noexcept;
    [[nodiscard]] BoundarySide1D side() const noexcept;
    [[nodiscard]] constexpr BoundaryType type() const noexcept { return BoundaryType::Robin; }
    [[nodiscard]] double transfer_coefficient() const noexcept;
    [[nodiscard]] double ambient_value() const noexcept;
    void set_transfer_coefficient(double transfer_coefficient) noexcept;
    void set_ambient_value(double ambient_value) noexcept;

    /// Accumulate the Robin contribution in the boundary owner's row.
    /// The coefficient field and matrix must refer to the same mesh object.
    void Apply(const field::VolScalarField1D& gamma,
               fv_matrix::FvMatrix1D& matrix) const;

private:
    std::string name_;
    BoundarySide1D side_;
    double transfer_coefficient_;
    double ambient_value_;
};

}  // namespace cfd::boundary
