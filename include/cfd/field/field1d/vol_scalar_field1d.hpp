#pragma once

#include <cstddef>
#include <span>
#include <string>
#include <vector>

namespace cfd::mesh
{
class FvMesh1D;
}  // namespace cfd::mesh

namespace cfd::field
{

/// A scalar field with one value at each cell center of a one-dimensional mesh.
class VolScalarField1D
{
public:
    explicit VolScalarField1D
    (
        std::string name, 
        const cfd::mesh::FvMesh1D& mesh,
        double initial_val = 0.0
    );
    // Delete rvalue mesh constructor to prevent dangling reference to temporary mesh objects
    VolScalarField1D(std::string, cfd::mesh::FvMesh1D&&, double = 0.0) = delete;

    VolScalarField1D(const VolScalarField1D&) = default;
    VolScalarField1D(VolScalarField1D&&) noexcept = default;
    VolScalarField1D& operator=(const VolScalarField1D& rhs);
    VolScalarField1D& operator=(VolScalarField1D&& rhs);
    ~VolScalarField1D() = default;

    [[nodiscard]] const std::string& Name() const noexcept 
    { return name_; }
    [[nodiscard]] const cfd::mesh::FvMesh1D& Mesh() const noexcept 
    { return mesh_; }
    [[nodiscard]] std::size_t Size() const noexcept 
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
    VolScalarField1D& operator+=(const VolScalarField1D& rhs);
    VolScalarField1D& operator-=(const VolScalarField1D& rhs);
    VolScalarField1D& operator*=(const VolScalarField1D& rhs);
    VolScalarField1D& operator/=(const VolScalarField1D& rhs);
    VolScalarField1D& operator*=(double scalar);
    VolScalarField1D& operator/=(double scalar);

private:
    std::string name_;
    const cfd::mesh::FvMesh1D& mesh_;
    std::vector<double> data_;
};

[[nodiscard]] VolScalarField1D operator+(VolScalarField1D lhs, const VolScalarField1D& rhs);
[[nodiscard]] VolScalarField1D operator-(VolScalarField1D lhs, const VolScalarField1D& rhs);
[[nodiscard]] VolScalarField1D operator*(VolScalarField1D lhs, const VolScalarField1D& rhs);
[[nodiscard]] VolScalarField1D operator/(VolScalarField1D lhs, const VolScalarField1D& rhs);
[[nodiscard]] VolScalarField1D operator*(VolScalarField1D lhs, double scalar);
[[nodiscard]] VolScalarField1D operator*(double scalar, VolScalarField1D rhs);
[[nodiscard]] VolScalarField1D operator/(VolScalarField1D lhs, double scalar);
using ScalarField1D = VolScalarField1D;

}  // namespace cfd::field
