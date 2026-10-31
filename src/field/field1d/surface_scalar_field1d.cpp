#include <cfd/field/field1d/surface_scalar_field1d.hpp>

#include <algorithm>
#include <stdexcept>
#include <utility>

#include <cfd/mesh/mesh1d/fv_mesh1d.hpp>

namespace cfd::field
{

SurfaceScalarField1D::SurfaceScalarField1D(std::string name, const cfd::mesh::FvMesh1D& mesh,
                                           double initial_val)
    : name_(std::move(name)), mesh_(mesh), data_(mesh.N_Faces(), initial_val)
{
}

SurfaceScalarField1D& SurfaceScalarField1D::operator=(const SurfaceScalarField1D& rhs)
{
    if (&mesh_ != &rhs.mesh_) throw std::invalid_argument("SurfaceScalarField1D::operator=: mesh mismatch");
    name_ = rhs.name_;
    data_ = rhs.data_;
    return *this;
}

SurfaceScalarField1D& SurfaceScalarField1D::operator=(SurfaceScalarField1D&& rhs)
{
    if (this == &rhs) return *this;
    if (&mesh_ != &rhs.mesh_) throw std::invalid_argument("SurfaceScalarField1D::operator=: mesh mismatch");
    name_ = std::move(rhs.name_);
    data_ = std::move(rhs.data_);
    return *this;
}

double& SurfaceScalarField1D::at(std::size_t index)
{
    if (index >= data_.size()) throw std::out_of_range("SurfaceScalarField1D::at: index out of range");
    return data_[index];
}

const double& SurfaceScalarField1D::at(std::size_t index) const
{
    if (index >= data_.size()) throw std::out_of_range("SurfaceScalarField1D::at: index out of range");
    return data_[index];
}

void SurfaceScalarField1D::Fill(double value) { std::fill(data_.begin(), data_.end(), value); }

SurfaceScalarField1D& SurfaceScalarField1D::operator+=(const SurfaceScalarField1D& rhs)
{
    if (&mesh_ != &rhs.mesh_) throw std::invalid_argument("SurfaceScalarField1D::operator+=: mesh mismatch");
    if (data_.size() != rhs.data_.size()) throw std::invalid_argument("SurfaceScalarField1D::operator+=: size mismatch");
    for (std::size_t i = 0; i < data_.size(); ++i) data_[i] += rhs.data_[i];
    return *this;
}

SurfaceScalarField1D& SurfaceScalarField1D::operator-=(const SurfaceScalarField1D& rhs)
{
    if (&mesh_ != &rhs.mesh_) throw std::invalid_argument("SurfaceScalarField1D::operator-=: mesh mismatch");
    if (data_.size() != rhs.data_.size()) throw std::invalid_argument("SurfaceScalarField1D::operator-=: size mismatch");
    for (std::size_t i = 0; i < data_.size(); ++i) data_[i] -= rhs.data_[i];
    return *this;
}

SurfaceScalarField1D& SurfaceScalarField1D::operator*=(const SurfaceScalarField1D& rhs)
{
    if (&mesh_ != &rhs.mesh_) throw std::invalid_argument("SurfaceScalarField1D::operator*=: mesh mismatch");
    if (data_.size() != rhs.data_.size()) throw std::invalid_argument("SurfaceScalarField1D::operator*=: size mismatch");
    for (std::size_t i = 0; i < data_.size(); ++i) data_[i] *= rhs.data_[i];
    return *this;
}

SurfaceScalarField1D& SurfaceScalarField1D::operator/=(const SurfaceScalarField1D& rhs)
{
    if (&mesh_ != &rhs.mesh_) throw std::invalid_argument("SurfaceScalarField1D::operator/=: mesh mismatch");
    if (data_.size() != rhs.data_.size()) throw std::invalid_argument("SurfaceScalarField1D::operator/=: size mismatch");
    if (std::find(rhs.data_.begin(), rhs.data_.end(), 0.0) != rhs.data_.end())
    {
        throw std::invalid_argument("SurfaceScalarField1D::operator/=: division by zero");
    }
    for (std::size_t i = 0; i < data_.size(); ++i)
    {
        data_[i] /= rhs.data_[i];
    }
    return *this;
}

SurfaceScalarField1D& SurfaceScalarField1D::operator*=(double scalar)
{
    for (double& value : data_) value *= scalar;
    return *this;
}

SurfaceScalarField1D& SurfaceScalarField1D::operator/=(double scalar)
{
    if (scalar == 0.0) throw std::invalid_argument("SurfaceScalarField1D::operator/=: division by zero");
    for (double& value : data_) value /= scalar;
    return *this;
}

SurfaceScalarField1D operator+(SurfaceScalarField1D lhs, const SurfaceScalarField1D& rhs) { return lhs += rhs; }
SurfaceScalarField1D operator-(SurfaceScalarField1D lhs, const SurfaceScalarField1D& rhs) { return lhs -= rhs; }
SurfaceScalarField1D operator*(SurfaceScalarField1D lhs, const SurfaceScalarField1D& rhs) { return lhs *= rhs; }
SurfaceScalarField1D operator/(SurfaceScalarField1D lhs, const SurfaceScalarField1D& rhs) { return lhs /= rhs; }
SurfaceScalarField1D operator*(SurfaceScalarField1D lhs, double scalar) { return lhs *= scalar; }
SurfaceScalarField1D operator*(double scalar, SurfaceScalarField1D rhs) { return rhs *= scalar; }
SurfaceScalarField1D operator/(SurfaceScalarField1D lhs, double scalar) { return lhs /= scalar; }

}  // namespace cfd::field
