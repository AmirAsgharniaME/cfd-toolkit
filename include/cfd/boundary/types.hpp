#pragma once
#include <cstdint>

#include <string_view>

namespace cfd::boundary
{

enum class BoundaryType : std::uint8_t
{
    Dirichlet,
    Neumann,
    Robin
};

enum class BoundarySide1D : std::uint8_t
{
    Start,
    End
};

enum class GradientType : std::uint8_t
{ 
    Normal, 
    Coordinate
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
