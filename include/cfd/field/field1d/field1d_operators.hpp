#pragma once

#include <cfd/field/field1d/field1d.hpp>

#include <type_traits>
#include <utility>

namespace cfd::field
{

/// Add two fields with matching mesh and location.
template<typename ValueType, typename LocationTag>
[[nodiscard]] Field1D<ValueType, LocationTag> operator+
(
    Field1D<ValueType, LocationTag> lhs,
    const Field1D<ValueType, LocationTag>& rhs
)
{
    lhs += rhs;
    return lhs;
}

/// Subtract one field from another with matching mesh and location.
template<typename ValueType, typename LocationTag>
[[nodiscard]] Field1D<ValueType, LocationTag> operator-
(
    Field1D<ValueType, LocationTag> lhs,
    const Field1D<ValueType, LocationTag>& rhs
)
{
    lhs -= rhs;
    return lhs;
}

/// Negate every value in a field.
template<typename ValueType, typename LocationTag>
[[nodiscard]] Field1D<ValueType, LocationTag> operator-(Field1D<ValueType, LocationTag> field)
{
    field *= -1.0;
    return field;
}

/// Multiply a field by a scalar.
template<typename ValueType, typename LocationTag>
[[nodiscard]] Field1D<ValueType, LocationTag> operator*(Field1D<ValueType, LocationTag> lhs, double scalar)
{
    lhs *= scalar;
    return lhs;
}

/// Multiply a field by a scalar.
template<typename ValueType, typename LocationTag>
[[nodiscard]] Field1D<ValueType, LocationTag> operator*(double scalar, Field1D<ValueType, LocationTag> rhs)
{
    rhs *= scalar;
    return std::move(rhs);
}

/// Divide a field by a scalar.
template<typename ValueType, typename LocationTag>
[[nodiscard]] Field1D<ValueType, LocationTag> operator/(Field1D<ValueType, LocationTag> lhs, double scalar)
{
    lhs /= scalar;
    return lhs;
}

/// Multiply two scalar fields with matching mesh and location.
template<typename ValueType, typename LocationTag>
    requires std::is_same_v<ValueType, double>
[[nodiscard]] Field1D<ValueType, LocationTag> operator*
(
    Field1D<ValueType, LocationTag> lhs,
    const Field1D<ValueType, LocationTag>& rhs
)
{
    lhs *= rhs;
    return lhs;
}

/// Divide one scalar field by another with matching mesh and location.
template<typename ValueType, typename LocationTag>
    requires std::is_same_v<ValueType, double>
[[nodiscard]] Field1D<ValueType, LocationTag> operator/
(
    Field1D<ValueType, LocationTag> lhs,
    const Field1D<ValueType, LocationTag>& rhs
)
{
    lhs /= rhs;
    return lhs;
}

}  // namespace cfd::field
