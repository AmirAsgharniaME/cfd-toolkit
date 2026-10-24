#pragma once

#include "cfd/boundary/types.hpp"

#include <string>

namespace cfd::boundary
{

class NeumannBoundary1D
{
public:
    /**
     * @brief Construct a new NeumannBoundary1D object.
     *
     * @param name Descriptive identifier (e.g., "adiabatic_wall", "zero_gradient_outlet").
     * @param side Spatial location (BoundarySide1D::Start or BoundarySide1D::End).
     * @param normal_gradient Prescribed outward normal derivative value: d(phi)/dn.
     */
    NeumannBoundary1D(std::string name, BoundarySide1D side, double normal_gradient);

    [[nodiscard]] const std::string& name() const noexcept;
    [[nodiscard]] BoundarySide1D side() const noexcept;
    [[nodiscard]] constexpr BoundaryType type() const noexcept { return BoundaryType::Neumann; }
    [[nodiscard]] double normal_gradient() const noexcept;
    [[nodiscard]] double outward_normal() const noexcept;
    [[nodiscard]] double coordinate_gradient() const noexcept;

    void set_normal_gradient(double normal_gradient) noexcept;

private:
    std::string name_;
    BoundarySide1D side_;
    double normal_gradient_;
};

}  // namespace cfd::boundary
