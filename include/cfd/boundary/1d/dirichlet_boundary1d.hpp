#pragma once

#include "cfd/boundary/types.hpp"

#include <string>
#include <utility>

namespace cfd::boundary
{

class DirichletBoundary1D
{
public:
    DirichletBoundary1D(std::string name, BoundarySide1D side, double value)
        : name_(std::move(name)), side_(side), value_(value)
    {
    }

    [[nodiscard]] const std::string& name() const noexcept { return name_; }
    [[nodiscard]] BoundarySide1D side() const noexcept { return side_; }
    [[nodiscard]] double value() const noexcept { return value_; }

private:
    std::string name_;
    BoundarySide1D side_;
    double value_;
};

}  // namespace cfd::boundary
