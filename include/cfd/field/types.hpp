#pragma once
#include <cstdint>

namespace cfd::field
{

/// Where a field's values are sampled on the mesh.
enum class FieldLocation : std::uint8_t
{
    Cell_Face,
    Cell_Center
};

}  // namespace cfd::field