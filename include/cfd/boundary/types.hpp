#pragma once

#include <string_view>

namespace cfd::boundary
{

enum class BoundaryType
{
    Dirichlet,
    Neumann,
    Robin
};

enum class BoundarySide1D
{
    Start,
    End
};

[[nodiscard]] constexpr std::string_view to_string(BoundarySide1D side) noexcept
{
    switch (side)
    {
        case BoundarySide1D::Start:
            return "Start";
        case BoundarySide1D::End:
            return "End";
    }

    return "Unknown";
}

}  // namespace cfd::boundary
