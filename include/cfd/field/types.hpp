#pragma once

namespace cfd::field
{

/// Where a field's values are sampled on the mesh.
enum class FieldLocation
{
    Face,
    Cell
};

}  // namespace cfd::field