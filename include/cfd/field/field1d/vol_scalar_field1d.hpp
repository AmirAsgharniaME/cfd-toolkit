#pragma once

#include <cstddef>
#include <span>
#include <string>
#include <vector>

#include <cfd/field/types.hpp>
#include <cfd/mesh/mesh1d/fv_mesh1d.hpp>

namespace cfd::field::field1d
{

/// A scalar field sampled at the cells or faces of a one-dimensional
/// finite-volume mesh.
///
/// Values live at either mesh faces or mesh cells, and the storage is sized
/// once from the mesh. The field is bound to the mesh object by reference;
/// that mesh must outlive the field and all fields produced by copying or
/// arithmetic. Copy and move construction keep the same binding; assignment
/// is deleted. Mesh identity (not geometric equality) decides compatibility.
class VolScalarField1D
{
public:
    /// Construct a field and allocate one value per face or cell.
    ///
    /// @param name        Human readable field name.
    /// @param mesh        Mesh the field is sampled on; stored by reference.
    /// @param location    Sampling location, cells by default.
    /// @param initial_val Value assigned to every allocated entry.
    /// @throws std::invalid_argument If location is not a valid FieldLocation.
    VolScalarField1D(std::string name,
                     const cfd::mesh::FvMesh1D& mesh,
                     FieldLocation location = FieldLocation::Cell,
                     double initial_val = 0.0);

    VolScalarField1D(const VolScalarField1D&) = default;
    VolScalarField1D(VolScalarField1D&&) noexcept = default;
    VolScalarField1D& operator=(const VolScalarField1D&) = delete;
    VolScalarField1D& operator=(VolScalarField1D&&) = delete;
    ~VolScalarField1D() = default;

    /// Field name.
    /// @return The stored name.
    [[nodiscard]] const std::string& name() const noexcept { return name_; }

    /// Mesh the field is sampled on.
    /// @return The referenced mesh.
    [[nodiscard]] const cfd::mesh::FvMesh1D& mesh() const noexcept { return mesh_; }

    /// Sampling location of the stored values.
    /// @return Face or Cell.
    [[nodiscard]] FieldLocation location() const noexcept { return location_; }

    /// Number of stored values.
    /// @return Face count or cell count, depending on location().
    [[nodiscard]] std::size_t size() const noexcept { return data_.size(); }

    /// Unchecked element access, following the convention of std::vector.
    /// @param index Zero-based entry index.
    /// @return Reference to the stored value.
    double& operator[](std::size_t index) noexcept { return data_[index]; }

    /// Unchecked element access, following the convention of std::vector.
    /// @param index Zero-based entry index.
    /// @return Const reference to the stored value.
    [[nodiscard]] const double& operator[](std::size_t index) const noexcept
    {
        return data_[index];
    }

    /// Bounds-checked element access.
    /// @param index Zero-based entry index.
    /// @return Reference to the stored value.
    /// @throws std::out_of_range If index >= size().
    [[nodiscard]] double& at(std::size_t index);

    /// Bounds-checked element access.
    /// @param index Zero-based entry index.
    /// @return Const reference to the stored value.
    /// @throws std::out_of_range If index >= size().
    [[nodiscard]] const double& at(std::size_t index) const;

    /// Mutable contiguous view of the values; its size cannot be changed.
    /// @return Span over the storage.
    [[nodiscard]] std::span<double> values() noexcept { return data_; }

    /// Read-only contiguous view of the values.
    /// @return Span over the storage.
    [[nodiscard]] std::span<const double> values() const noexcept { return data_; }

    /// Assign the same value to every entry.
    /// @param value Value written to each entry.
    void fill(double value);

    /// Add another field entry by entry.
    /// @param rhs Field to add.
    /// @return *this.
    /// @throws std::invalid_argument If mesh identity, location, or size differs.
    VolScalarField1D& operator+=(const VolScalarField1D& rhs);

    /// Subtract another field entry by entry.
    /// @param rhs Field to subtract.
    /// @return *this.
    /// @throws std::invalid_argument If mesh identity, location, or size differs.
    VolScalarField1D& operator-=(const VolScalarField1D& rhs);

    /// Multiply every entry by a scalar.
    /// @param scalar Factor applied to each entry.
    /// @return *this.
    VolScalarField1D& operator*=(double scalar);

    /// Divide every entry by a scalar.
    /// @param scalar Divisor applied to each entry.
    /// @return *this.
    /// @throws std::invalid_argument If scalar == 0.0.
    VolScalarField1D& operator/=(double scalar);

private:
    std::string name_;
    const cfd::mesh::FvMesh1D& mesh_;
    FieldLocation location_;
    std::vector<double> data_;
};

/// Add two fields entry by entry.
/// @param lhs Left operand, taken by value.
/// @param rhs Right operand.
/// @return lhs after adding rhs.
/// @throws std::invalid_argument If mesh identity, location, or size differs.
[[nodiscard]] VolScalarField1D operator+(VolScalarField1D lhs, const VolScalarField1D& rhs);

/// Subtract rhs from lhs entry by entry.
/// @param lhs Left operand, taken by value.
/// @param rhs Right operand.
/// @return lhs after subtracting rhs.
/// @throws std::invalid_argument If mesh identity, location, or size differs.
[[nodiscard]] VolScalarField1D operator-(VolScalarField1D lhs, const VolScalarField1D& rhs);

/// Scale a field by a scalar.
/// @param lhs Field, taken by value.
/// @param scalar Factor applied to each entry.
/// @return The scaled field.
[[nodiscard]] VolScalarField1D operator*(VolScalarField1D lhs, double scalar);

/// Scale a field by a scalar.
/// @param scalar Factor applied to each entry.
/// @param rhs Field, taken by value.
/// @return The scaled field.
[[nodiscard]] VolScalarField1D operator*(double scalar, VolScalarField1D rhs);

/// Divide a field by a scalar.
/// @param lhs Field, taken by value.
/// @param scalar Divisor applied to each entry.
/// @return The scaled field.
/// @throws std::invalid_argument If scalar == 0.0.
[[nodiscard]] VolScalarField1D operator/(VolScalarField1D lhs, double scalar);

/// Compatibility alias for existing scalar field code.
using ScalarField1D = VolScalarField1D;

}  // namespace cfd::field::field1d
