#include <cfd/field/field1d/vol_scalar_field1d.hpp>

#include <algorithm>
#include <stdexcept>
#include <utility>

#include <cfd/mesh/mesh1d/fv_mesh1d.hpp>

namespace cfd::field
{

VolScalarField1D::VolScalarField1D(std::string name, const cfd::mesh::FvMesh1D& mesh,
                                   double initial_val)
    : name_(std::move(name)), mesh_(mesh), data_(mesh.N_Cells(), initial_val)
{
}

VolScalarField1D& VolScalarField1D::operator=(const VolScalarField1D& rhs)
{
    if (&mesh_ != &rhs.mesh_)
    {
        throw std::invalid_argument("VolScalarField1D::operator=: mesh mismatch");
    }
    name_ = rhs.name_;
    data_ = rhs.data_;
    return *this;
}

VolScalarField1D& VolScalarField1D::operator=(VolScalarField1D&& rhs)
{
    if (this == &rhs) return *this;
    if (&mesh_ != &rhs.mesh_)
    {
        throw std::invalid_argument("VolScalarField1D::operator=: mesh mismatch");
    }
    name_ = std::move(rhs.name_);
    data_ = std::move(rhs.data_);
    return *this;
}

double& VolScalarField1D::at(std::size_t index)
{
    if (index >= data_.size()) throw std::out_of_range("VolScalarField1D::at: index out of range");
    return data_[index];
}

const double& VolScalarField1D::at(std::size_t index) const
{
    if (index >= data_.size()) throw std::out_of_range("VolScalarField1D::at: index out of range");
    return data_[index];
}

void VolScalarField1D::Fill(double value) { std::fill(data_.begin(), data_.end(), value); }

VolScalarField1D& VolScalarField1D::operator+=(const VolScalarField1D& rhs)
{
    if (&mesh_ != &rhs.mesh_) throw std::invalid_argument("VolScalarField1D::operator+=: mesh mismatch");
    if (data_.size() != rhs.data_.size()) throw std::invalid_argument("VolScalarField1D::operator+=: size mismatch");
    for (std::size_t i = 0; i < data_.size(); ++i) data_[i] += rhs.data_[i];
    return *this;
}

VolScalarField1D& VolScalarField1D::operator-=(const VolScalarField1D& rhs)
{
    if (&mesh_ != &rhs.mesh_) throw std::invalid_argument("VolScalarField1D::operator-=: mesh mismatch");
    if (data_.size() != rhs.data_.size()) throw std::invalid_argument("VolScalarField1D::operator-=: size mismatch");
    for (std::size_t i = 0; i < data_.size(); ++i) data_[i] -= rhs.data_[i];
    return *this;
}

VolScalarField1D& VolScalarField1D::operator*=(const VolScalarField1D& rhs)
{
    if (&mesh_ != &rhs.mesh_) throw std::invalid_argument("VolScalarField1D::operator*=: mesh mismatch");
    if (data_.size() != rhs.data_.size()) throw std::invalid_argument("VolScalarField1D::operator*=: size mismatch");
    for (std::size_t i = 0; i < data_.size(); ++i) data_[i] *= rhs.data_[i];
    return *this;
}

VolScalarField1D& VolScalarField1D::operator/=(const VolScalarField1D& rhs)
{
    if (&mesh_ != &rhs.mesh_) throw std::invalid_argument("VolScalarField1D::operator/=: mesh mismatch");
    if (data_.size() != rhs.data_.size()) throw std::invalid_argument("VolScalarField1D::operator/=: size mismatch");
    if (std::find(rhs.data_.begin(), rhs.data_.end(), 0.0) != rhs.data_.end())
    {
        throw std::invalid_argument("VolScalarField1D::operator/=: division by zero");
    }
    for (std::size_t i = 0; i < data_.size(); ++i)
    {
        data_[i] /= rhs.data_[i];
    }
    return *this;
}

VolScalarField1D& VolScalarField1D::operator*=(double scalar)
{
    for (double& value : data_) value *= scalar;
    return *this;
}

VolScalarField1D& VolScalarField1D::operator/=(double scalar)
{
    if (scalar == 0.0) throw std::invalid_argument("VolScalarField1D::operator/=: division by zero");
    for (double& value : data_) value /= scalar;
    return *this;
}

VolScalarField1D operator+(VolScalarField1D lhs, const VolScalarField1D& rhs) { return lhs += rhs; }
VolScalarField1D operator-(VolScalarField1D lhs, const VolScalarField1D& rhs) { return lhs -= rhs; }
VolScalarField1D operator*(VolScalarField1D lhs, const VolScalarField1D& rhs) { return lhs *= rhs; }
VolScalarField1D operator/(VolScalarField1D lhs, const VolScalarField1D& rhs) { return lhs /= rhs; }
VolScalarField1D operator*(VolScalarField1D lhs, double scalar) { return lhs *= scalar; }
VolScalarField1D operator*(double scalar, VolScalarField1D rhs) { return rhs *= scalar; }
VolScalarField1D operator/(VolScalarField1D lhs, double scalar) { return lhs /= scalar; }

}  // namespace cfd::field
