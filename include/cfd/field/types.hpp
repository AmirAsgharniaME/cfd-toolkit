#ifndef CFD_FIELD_TYPES_HPP
#define CFD_FIELD_TYPES_HPP

namespace cfd::field
{

/// Where a field's values are sampled on the mesh.
enum class FieldLocation
{
    Node,
    Cell
};

}  // namespace cfd::field

#endif  // CFD_FIELD_TYPES_HPP
