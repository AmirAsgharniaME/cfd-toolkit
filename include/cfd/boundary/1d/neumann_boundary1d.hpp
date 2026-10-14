#pragma once

#include "cfd/boundary/types.hpp"

#include <string>
#include <utility>

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
    NeumannBoundary1D(std::string name, BoundarySide1D side, double normal_gradient)
        : name_(std::move(name)), side_(side), normal_gradient_(normal_gradient)
    {
    }

    [[nodiscard]] const std::string& name() const noexcept { return name_; }
    [[nodiscard]] BoundarySide1D side() const noexcept { return side_; }
    [[nodiscard]] constexpr BoundaryType type() const noexcept { return BoundaryType::Neumann; }
    [[nodiscard]] double normal_gradient() const noexcept { return normal_gradient_; }

    [[nodiscard]] constexpr double outward_normal() const noexcept
    {
        switch (side_)
        {
            case BoundarySide1D::Start:
                return -1.0;
            case BoundarySide1D::End:
                return 1.0;
        }

        return 0.0;
    }

    [[nodiscard]] double coordinate_gradient() const noexcept
    {
        return normal_gradient_ * outward_normal();
    }

    void set_normal_gradient(double normal_gradient) noexcept
    {
        normal_gradient_ = normal_gradient;
    }

private:
    std::string name_;
    BoundarySide1D side_;
    double normal_gradient_;
};

}  // namespace cfd::boundary
