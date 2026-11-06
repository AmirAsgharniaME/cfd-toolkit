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

template<typename Type, typename LocationTag>
class Field1D
{
    static_assert(std::is_same_v<Type, double> ||
                  std::is_same_v<Type, cfd::vector::Vector1D>, "Unsupported field type");
    static_assert(std::is_same_v<LocationTag, CellLocation> ||
                  std::is_same_v<LocationTag, FaceLocation>, "Unsupported field location");

public:
    explicit Field1D
    (
        std::string name,
        const cfd::mesh::FvMesh1D& mesh,
        const Type& initial_val = Type{}
    );
    // Delete rvalue mesh constructor to prevent dangling reference to temporary mesh objects
    Field1D(std::string, cfd::mesh::FvMesh1D&&, const Type& = Type{}) = delete;

    Field1D(const Field1D&) = default;
    Field1D(Field1D&&) noexcept = default;
    Field1D& operator=(const Field1D& rhs);
    Field1D& operator=(Field1D&& rhs);
    ~Field1D() = default;


    [[nodiscard]] const std::string& Name() const noexcept;
    [[nodiscard]] const cfd::mesh::FvMesh1D& Mesh() const noexcept;
    [[nodiscard]] std::size_t Size() const noexcept;
    [[nodiscard]] std::size_t N_Faces() const noexcept requires std::is_same_v<LocationTag, FaceLocation>;
    [[nodiscard]] Type& operator[](std::size_t index) noexcept;
    [[nodiscard]] const Type& operator[](std::size_t index) const noexcept;
    [[nodiscard]] Type& at(std::size_t index);
    [[nodiscard]] const Type& at(std::size_t index) const;
    [[nodiscard]] std::span<Type> values() noexcept;
    [[nodiscard]] std::span<const Type> values() const noexcept;
    [[nodiscard]] std::span<Type> span() noexcept;
    [[nodiscard]] std::span<const Type> span() const noexcept;
    [[nodiscard]] std::span<Type> Values() noexcept;
    [[nodiscard]] std::span<const Type> Values() const noexcept;

    void Fill(const Type& value);
    Field1D& operator+=(const Field1D& rhs);
    Field1D& operator-=(const Field1D& rhs);
    Field1D& operator*=(const Field1D& rhs) requires std::is_same_v<Type, double>;
    Field1D& operator/=(const Field1D& rhs) requires std::is_same_v<Type, double>;
    Field1D& operator*=(double scalar);
    Field1D& operator/=(double scalar);

private:
    std::string name_;
    const cfd::mesh::FvMesh1D& mesh_;
    std::vector<Type> values_;
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
