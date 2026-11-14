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

class NeumannBoundary1D
{
public:
    /**
     * @brief Construct a Neumann boundary condition for a 1D domain.
     *
     * @param name Descriptive identifier (e.g., "adiabatic_wall", "zero_gradient_outlet").
     * @param side Boundary location, either BoundarySide1D::Start or BoundarySide1D::End.
     * @param normal_gradient Prescribed scalar outward-normal derivative
     *        \f$g_n \equiv \partial\phi/\partial n_{\mathrm{out}}
     *        = \nabla\phi\cdot\mathbf{n}_{\mathrm{out}}\f$. This is a
     *        derivative, not a flux; it contains neither diffusivity
     *        \f$\Gamma_b\f$ nor boundary face area \f$A_b\f$. The integrated
     *        outward diffusive flux is
     *        \f$Q_b=-\Gamma_b g_n A_b\f$, so its contribution to the
     *        right-hand-side source \f$b_P\f$ is
     *        \f$+\Gamma_b g_n A_b\f$.
     * @throws std::invalid_argument if side is not Start or End.
     */
    NeumannBoundary1D(std::string name, BoundarySide1D side, double normal_gradient);

    /**
     * @brief Construct a Neumann boundary condition from a gradient of the given type.
     *
     * The outward unit normal is \f$n_x=-1\f$ at BoundarySide1D::Start and
     * \f$n_x=+1\f$ at BoundarySide1D::End. The value is converted once, at
     * construction, to the outward-normal derivative
     * \f$g_n=\nabla\phi\cdot\mathbf{n}_{\mathrm{out}}\f$ that is stored:
     * - GradientType::Normal: \p gradient is already \f$g_n\f$ and is used directly.
     * - GradientType::Coordinate: \p gradient is \f$\partial\phi/\partial x\f$ and
     *   \f$g_n=n_x\,\partial\phi/\partial x\f$.
     *
     * Assembly then adds \f$\Gamma_b A_b g_n\f$ to the boundary owner's source
     * exactly once, with no further sign change and no matrix coefficient change.
     *
     * @param name Descriptive identifier.
     * @param side Boundary location.
     * @param gradient Prescribed gradient, interpreted according to \p type.
     * @param type Meaning of \p gradient.
     * @throws std::invalid_argument if side or type is not a valid enumerator.
     */
    NeumannBoundary1D(std::string name, BoundarySide1D side, double gradient,
                      GradientType type);

    /// @brief Return the boundary name.
    [[nodiscard]] const std::string& name() const noexcept;
    /// @brief Return whether this boundary is at the start or end of the domain.
    [[nodiscard]] BoundarySide1D side() const noexcept;
    [[nodiscard]] constexpr BoundaryType type() const noexcept { return BoundaryType::Neumann; }

    /**
     * @brief Return the prescribed outward-normal derivative
     *        \f$g_n \equiv \partial\phi/\partial n_{\mathrm{out}}
     *        = \nabla\phi\cdot\mathbf{n}_{\mathrm{out}}\f$.
     *
     * This scalar derivative is not a diffusive flux and includes neither
     * diffusivity Gamma nor boundary face area A_b.
     */
    [[nodiscard]] double normal_gradient() const noexcept;

    /**
     * @brief Return \f$n_x\f$, the x-component of the outward unit normal.
     *
     * Returns \f$n_x=-1\f$ at BoundarySide1D::Start and \f$n_x=+1\f$ at
     * BoundarySide1D::End.
     * Returns quiet NaN for an invalid enum value; construction rejects such
     * values, so this is a defensive result.
     */
    [[nodiscard]] double outward_normal() const noexcept;

    /**
     * @brief Return the coordinate derivative \f$\partial\phi/\partial x\f$.
     *
     * In one dimension,
     * \f$\partial\phi/\partial x=n_x g_n\f$, where \f$n_x\f$ is the outward
     * normal component returned by outward_normal() and \f$g_n\f$ is returned
     * by normal_gradient(). An invalid stored side produces quiet NaN.
     */
    [[nodiscard]] double coordinate_gradient() const noexcept;

    /// @brief Replace the prescribed outward-normal derivative \f$g_n\f$.
    void set_normal_gradient(double normal_gradient) noexcept;

    /// Accumulate the prescribed-gradient contribution in the boundary owner's source.
    /// Adds gamma_owner * normal_gradient * face_area without changing coefficients.
    /// The coefficient field and matrix must refer to the same mesh object.
    void Apply(const field::VolScalarField1D& gamma,
               fv_matrix::FvMatrix1D& matrix) const;

private:
    std::string name_;
    BoundarySide1D side_;
    double normal_gradient_;
};

}  // namespace cfd::boundary
