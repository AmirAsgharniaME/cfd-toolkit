#pragma once

#include <cstddef>
#include <span>
#include <string>
#include <type_traits>
#include <vector>

namespace cfd::mesh
{
class FvMesh1D;
}  // namespace cfd::mesh

namespace cfd::field
{

struct CellLocation {};
struct FaceLocation {};

template<class LocationTag>
class ScalarField1D
{
    static_assert(std::is_same_v<LocationTag, CellLocation> ||
                  std::is_same_v<LocationTag, FaceLocation>, "Unsupported field location");

public:
    explicit ScalarField1D
    (
        std::string name, 
        const cfd::mesh::FvMesh1D& mesh,
        double initial_val = 0.0
    );
    // Delete rvalue mesh constructor to prevent dangling reference to temporary mesh objects
    ScalarField1D(std::string, cfd::mesh::FvMesh1D&&, double = 0.0) = delete;

    ScalarField1D(const ScalarField1D&) = default;
    ScalarField1D(ScalarField1D&&) noexcept = default;
    ScalarField1D& operator=(const ScalarField1D& rhs);
    ScalarField1D& operator=(ScalarField1D&& rhs);
    ~ScalarField1D() = default;

    [[nodiscard]] const std::string& Name() const noexcept 
    { return name_; }
    [[nodiscard]] const cfd::mesh::FvMesh1D& Mesh() const noexcept 
    { return mesh_; }
    [[nodiscard]] std::size_t Size() const noexcept 
    { return data_.size(); }

    [[nodiscard]] std::size_t N_Faces() const noexcept requires std::is_same_v<LocationTag, FaceLocation>
    { return data_.size(); }

    [[nodiscard]] double& operator[](std::size_t index) noexcept 
    { return data_[index]; }
    [[nodiscard]] const double& operator[](std::size_t index) const noexcept 
    { return data_[index]; }

    [[nodiscard]] double& at(std::size_t index);
    [[nodiscard]] const double& at(std::size_t index) const;
    [[nodiscard]] std::span<double> values() noexcept 
    { return data_; }
    [[nodiscard]] std::span<const double> values() const noexcept 
    { return data_; }
    [[nodiscard]] std::span<double> span() noexcept 
    { return data_; }
    [[nodiscard]] std::span<const double> span() const noexcept 
    { return data_; }
    [[nodiscard]] std::span<double> Values() noexcept 
    { return data_; }
    [[nodiscard]] std::span<const double> Values() const noexcept 
    { return data_; }
    
    void Fill(double value);
    ScalarField1D& operator+=(const ScalarField1D& rhs);
    ScalarField1D& operator-=(const ScalarField1D& rhs);
    ScalarField1D& operator*=(const ScalarField1D& rhs);
    ScalarField1D& operator/=(const ScalarField1D& rhs);
    ScalarField1D& operator*=(double scalar);
    ScalarField1D& operator/=(double scalar);

private:
    std::string name_;
    const cfd::mesh::FvMesh1D& mesh_;
    std::vector<double> data_;
};

template<class LocationTag>
[[nodiscard]] ScalarField1D<LocationTag> operator+(ScalarField1D<LocationTag> lhs, const ScalarField1D<LocationTag>& rhs) { return lhs += rhs; }
template<class LocationTag>
[[nodiscard]] ScalarField1D<LocationTag> operator-(ScalarField1D<LocationTag> lhs, const ScalarField1D<LocationTag>& rhs) { return lhs -= rhs; }
template<class LocationTag>
[[nodiscard]] ScalarField1D<LocationTag> operator*(ScalarField1D<LocationTag> lhs, const ScalarField1D<LocationTag>& rhs) { return lhs *= rhs; }
template<class LocationTag>
[[nodiscard]] ScalarField1D<LocationTag> operator/(ScalarField1D<LocationTag> lhs, const ScalarField1D<LocationTag>& rhs) { return lhs /= rhs; }
template<class LocationTag>
[[nodiscard]] ScalarField1D<LocationTag> operator*(ScalarField1D<LocationTag> lhs, double scalar) { return lhs *= scalar; }
template<class LocationTag>
[[nodiscard]] ScalarField1D<LocationTag> operator*(double scalar, ScalarField1D<LocationTag> rhs) { return rhs *= scalar; }
template<class LocationTag>
[[nodiscard]] ScalarField1D<LocationTag> operator/(ScalarField1D<LocationTag> lhs, double scalar) { return lhs /= scalar; }

extern template class ScalarField1D<CellLocation>;
extern template class ScalarField1D<FaceLocation>;

using VolScalarField1D = ScalarField1D<CellLocation>;
using SurfaceScalarField1D = ScalarField1D<FaceLocation>;

}  // namespace cfd::field
