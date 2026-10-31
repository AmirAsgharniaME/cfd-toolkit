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

/// A scalar field with one value at each face of a one-dimensional mesh.
class SurfaceScalarField1D
{
public:
    explicit SurfaceScalarField1D
    (
    std::string name,
    const cfd::mesh::FvMesh1D& mesh,
    double initial_val = 0.0
    );

    SurfaceScalarField1D(std::string, cfd::mesh::FvMesh1D&&, double = 0.0) = delete;

    SurfaceScalarField1D(const SurfaceScalarField1D&) = default;
    SurfaceScalarField1D(SurfaceScalarField1D&&) noexcept = default;
    SurfaceScalarField1D& operator=(const SurfaceScalarField1D& rhs);
    SurfaceScalarField1D& operator=(SurfaceScalarField1D&& rhs);
    ~SurfaceScalarField1D() = default;

    [[nodiscard]] const std::string& Name() const noexcept 
    { return name_; }
    [[nodiscard]] const cfd::mesh::FvMesh1D& Mesh() const noexcept 
    { return mesh_; }
    [[nodiscard]] std::size_t N_Faces() const noexcept 
    { return data_.size(); }
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
    [[nodiscard]] std::span<const double> Values() const noexcept {
     return data_; }
    void Fill(double value);
    SurfaceScalarField1D& operator+=(const SurfaceScalarField1D& rhs);
    SurfaceScalarField1D& operator-=(const SurfaceScalarField1D& rhs);
    SurfaceScalarField1D& operator*=(const SurfaceScalarField1D& rhs);
    SurfaceScalarField1D& operator/=(const SurfaceScalarField1D& rhs);
    SurfaceScalarField1D& operator*=(double scalar);
    SurfaceScalarField1D& operator/=(double scalar);

private:
    std::string name_;
    const cfd::mesh::FvMesh1D& mesh_;
    std::vector<double> data_;
};

[[nodiscard]] SurfaceScalarField1D operator+(SurfaceScalarField1D lhs, const SurfaceScalarField1D& rhs);
[[nodiscard]] SurfaceScalarField1D operator-(SurfaceScalarField1D lhs, const SurfaceScalarField1D& rhs);
[[nodiscard]] SurfaceScalarField1D operator*(SurfaceScalarField1D lhs, const SurfaceScalarField1D& rhs);
[[nodiscard]] SurfaceScalarField1D operator/(SurfaceScalarField1D lhs, const SurfaceScalarField1D& rhs);
[[nodiscard]] SurfaceScalarField1D operator*(SurfaceScalarField1D lhs, double scalar);
[[nodiscard]] SurfaceScalarField1D operator*(double scalar, SurfaceScalarField1D rhs);
[[nodiscard]] SurfaceScalarField1D operator/(SurfaceScalarField1D lhs, double scalar);

}  // namespace cfd::field
