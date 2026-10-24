#pragma once

#include "cfd/boundary/types.hpp"

#include <string>

namespace cfd::boundary
{

class DirichletBoundary1D
{
public:
    DirichletBoundary1D(std::string name, BoundarySide1D side, double value);

    [[nodiscard]] const std::string& name() const noexcept;
    [[nodiscard]] BoundarySide1D side() const noexcept;
    [[nodiscard]] double value() const noexcept;

private:
    std::string name_;
    BoundarySide1D side_;
    double value_;
};

}  // namespace cfd::boundary
