#pragma once
#include <cstdint>

namespace cfd::field
{

/// Where a field's values are sampled on the mesh.
enum class FieldLocation : std::uint8_t
{
    Face,
    Cell
};

}  // namespace cfd::field