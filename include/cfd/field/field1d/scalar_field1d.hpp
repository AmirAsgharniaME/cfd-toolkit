#ifndef CFD_FIELD_FIELD1D_SCALAR_FIELD1D_HPP
#define CFD_FIELD_FIELD1D_SCALAR_FIELD1D_HPP

#include <cstddef>
#include <string>
#include <vector>

#include <cfd/field/types.hpp>
#include <cfd/mesh/mesh1d/uniform_mesh1d.hpp>

namespace cfd::field::field1d
{

/// A scalar field sampled on a one-dimensional uniform mesh.
///
/// Values live at either mesh nodes or mesh cells, and the storage is sized
/// once from the mesh and kept for the lifetime of the field.
class ScalarField1D
{
public:
    /// Construct a field and allocate one value per node or cell.
    ///
    /// @param name        Human readable field name.
    /// @param mesh        Mesh the field is sampled on; stored by reference.
    /// @param location    Sampling location, nodes by default.
    /// @param initial_val Value assigned to every allocated entry.
    ScalarField1D(std::string name,
                  const mesh::UniformMesh1D& mesh,
                  FieldLocation location = FieldLocation::Node,
                  double initial_val = 0.0);

    /// Field name.
    /// @return The stored name.
    [[nodiscard]] const std::string& name() const noexcept { return name_; }

    /// Mesh the field is sampled on.
    /// @return The referenced mesh.
    [[nodiscard]] const mesh::UniformMesh1D& mesh() const noexcept { return mesh_; }

    /// Sampling location of the stored values.
    /// @return Node or Cell.
    [[nodiscard]] FieldLocation location() const noexcept { return location_; }

    /// Number of stored values.
    /// @return Node count or cell count, depending on location().
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
    double& at(std::size_t index);

    /// Bounds-checked element access.
    /// @param index Zero-based entry index.
    /// @return Const reference to the stored value.
    /// @throws std::out_of_range If index >= size().
    [[nodiscard]] const double& at(std::size_t index) const;

    /// Mutable access to the underlying buffer.
    /// @return The storage vector.
    [[nodiscard]] std::vector<double>& raw_data() noexcept { return data_; }

    /// Read-only access to the underlying buffer.
    /// @return The storage vector.
    [[nodiscard]] const std::vector<double>& raw_data() const noexcept { return data_; }

    /// Assign the same value to every entry.
    /// @param value Value written to each entry.
    void fill(double value);

    /// Add another field entry by entry.
    /// @param rhs Field to add.
    /// @return *this.
    /// @throws std::invalid_argument If size() != rhs.size().
    ScalarField1D& operator+=(const ScalarField1D& rhs);

    /// Subtract another field entry by entry.
    /// @param rhs Field to subtract.
    /// @return *this.
    /// @throws std::invalid_argument If size() != rhs.size().
    ScalarField1D& operator-=(const ScalarField1D& rhs);

    /// Multiply every entry by a scalar.
    /// @param scalar Factor applied to each entry.
    /// @return *this.
    ScalarField1D& operator*=(double scalar);

    /// Divide every entry by a scalar.
    /// @param scalar Divisor applied to each entry.
    /// @return *this.
    /// @throws std::invalid_argument If scalar == 0.0.
    ScalarField1D& operator/=(double scalar);

private:
    std::string name_;
    const mesh::UniformMesh1D& mesh_;
    FieldLocation location_;
    std::vector<double> data_;
};

/// Add two fields entry by entry.
/// @param lhs Left operand, taken by value.
/// @param rhs Right operand.
/// @return lhs after adding rhs.
/// @throws std::invalid_argument If the sizes differ.
[[nodiscard]] ScalarField1D operator+(ScalarField1D lhs, const ScalarField1D& rhs);

/// Subtract rhs from lhs entry by entry.
/// @param lhs Left operand, taken by value.
/// @param rhs Right operand.
/// @return lhs after subtracting rhs.
/// @throws std::invalid_argument If the sizes differ.
[[nodiscard]] ScalarField1D operator-(ScalarField1D lhs, const ScalarField1D& rhs);

/// Scale a field by a scalar.
/// @param lhs Field, taken by value.
/// @param scalar Factor applied to each entry.
/// @return The scaled field.
[[nodiscard]] ScalarField1D operator*(ScalarField1D lhs, double scalar);

/// Scale a field by a scalar.
/// @param scalar Factor applied to each entry.
/// @param rhs Field, taken by value.
/// @return The scaled field.
[[nodiscard]] ScalarField1D operator*(double scalar, ScalarField1D rhs);

/// Divide a field by a scalar.
/// @param lhs Field, taken by value.
/// @param scalar Divisor applied to each entry.
/// @return The scaled field.
/// @throws std::invalid_argument If scalar == 0.0.
[[nodiscard]] ScalarField1D operator/(ScalarField1D lhs, double scalar);

}  // namespace cfd::field::field1d

#endif  // CFD_FIELD_FIELD1D_SCALAR_FIELD1D_HPP
