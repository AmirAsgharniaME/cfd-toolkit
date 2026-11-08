#pragma once

#include <cstddef>
#include <span>
#include <string>
#include <type_traits>
#include <vector>

#include <cfd/vector/vector1d.hpp>

namespace cfd::mesh
{
class FvMesh1D;
}  // namespace cfd::mesh

namespace cfd::field
{

struct CellLocation {};
struct FaceLocation {};

template<typename ValueType, typename LocationTag>
class Field1D
{
    static_assert(std::is_same_v<ValueType, double> ||
                  std::is_same_v<ValueType, cfd::vector::Vector1D>, "Unsupported field type");
    static_assert(std::is_same_v<LocationTag, CellLocation> ||
                  std::is_same_v<LocationTag, FaceLocation>, "Unsupported field location");

public:
    explicit Field1D
    (
        std::string name,
        const cfd::mesh::FvMesh1D& mesh,
        const ValueType& initial_val = ValueType{}
    );
    // Delete rvalue mesh constructor to prevent dangling reference to temporary mesh objects
    Field1D(std::string, cfd::mesh::FvMesh1D&&, const ValueType& = ValueType{}) = delete;

    Field1D(const Field1D&) = default;
    Field1D(Field1D&&) noexcept = default;
    Field1D& operator=(const Field1D& rhs);
    Field1D& operator=(Field1D&& rhs);
    ~Field1D() = default;


    [[nodiscard]] const std::string& Name() const noexcept;
    [[nodiscard]] const cfd::mesh::FvMesh1D& Mesh() const noexcept;
    [[nodiscard]] std::size_t size() const noexcept;
    [[nodiscard]] std::size_t N_Faces() const noexcept requires std::is_same_v<LocationTag, FaceLocation>;
    [[nodiscard]] ValueType& operator[](std::size_t index) noexcept;
    [[nodiscard]] const ValueType& operator[](std::size_t index) const noexcept;
    [[nodiscard]] ValueType& at(std::size_t index);
    [[nodiscard]] const ValueType& at(std::size_t index) const;
    [[nodiscard]] std::span<ValueType> values() noexcept;
    [[nodiscard]] std::span<const ValueType> values() const noexcept;
    [[nodiscard]] std::span<ValueType> span() noexcept;
    [[nodiscard]] std::span<const ValueType> span() const noexcept;
    [[nodiscard]] std::span<ValueType> Values() noexcept;
    [[nodiscard]] std::span<const ValueType> Values() const noexcept;

    void Fill(const ValueType& value);
    Field1D& operator+=(const Field1D& rhs);
    Field1D& operator-=(const Field1D& rhs);
    Field1D& operator*=(const Field1D& rhs) requires std::is_same_v<ValueType, double>;
    Field1D& operator/=(const Field1D& rhs) requires std::is_same_v<ValueType, double>;
    Field1D& operator*=(double scalar);
    Field1D& operator/=(double scalar);

private:
    std::string name_;
    const cfd::mesh::FvMesh1D& mesh_;
    std::vector<ValueType> values_;
};

extern template class Field1D<double, CellLocation>;
extern template class Field1D<double, FaceLocation>;
extern template class Field1D<cfd::vector::Vector1D, CellLocation>;
extern template class Field1D<cfd::vector::Vector1D, FaceLocation>;

using VolScalarField1D = Field1D<double, CellLocation>;
using SurfaceScalarField1D = Field1D<double, FaceLocation>;
using VolVectorField1D = Field1D<cfd::vector::Vector1D, CellLocation>;
using SurfaceVectorField1D = Field1D<cfd::vector::Vector1D, FaceLocation>;

}  // namespace cfd::field
