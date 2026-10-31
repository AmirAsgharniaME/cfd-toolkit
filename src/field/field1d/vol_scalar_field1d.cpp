#include <cfd/field/field1d/vol_scalar_field1d.hpp>

#include <algorithm>
#include <stdexcept>
#include <utility>

#include <cfd/mesh/mesh1d/fv_mesh1d.hpp>

namespace cfd::field
{

VolScalarField1D::VolScalarField1D(std::string name,
                                   const cfd::mesh::FvMesh1D& mesh,
                                   FieldLocation location,
                                   double initial_val)
    : name_(std::move(name)),
      mesh_(mesh),
      location_(location)
{
    switch (location_)
    {
    case FieldLocation::Cell_Center:
        data_.assign(mesh_.N_Cells(), initial_val);
        break;
    case FieldLocation::Cell_Face:
        data_.assign(mesh_.N_Faces(), initial_val);
        break;
    default:
        throw std::invalid_argument("VolScalarField1D: invalid field location");
    }
}

double& VolScalarField1D::at(std::size_t index)
{
    if (index >= data_.size())
    {
        throw std::out_of_range("VolScalarField1D::at: index out of range");
    }

    return data_[index];
}

const double& VolScalarField1D::at(std::size_t index) const
{
    if (index >= data_.size())
    {
        throw std::out_of_range("VolScalarField1D::at: index out of range");
    }

    return data_[index];
}

void VolScalarField1D::Fill(double value)
{
    std::fill(data_.begin(), data_.end(), value);
}

VolScalarField1D& VolScalarField1D::operator+=(const VolScalarField1D& rhs)
{
    // Strict physics/mesh safety checks for FVM
    if (&mesh_ != &rhs.mesh_)
    {
        throw std::invalid_argument("VolScalarField1D::operator+=: mesh mismatch");
    }
    if (location_ != rhs.location_)
    {
        throw std::invalid_argument("VolScalarField1D::operator+=: location mismatch (Cell_Center vs Cell_Face)");
    }
    if (data_.size() != rhs.Size())
    {
        throw std::invalid_argument("VolScalarField1D::operator+=: size mismatch");
    }

    for (std::size_t index = 0; index < data_.size(); ++index)
    {
        data_[index] += rhs[index];
    }

    return *this;
}

VolScalarField1D& VolScalarField1D::operator-=(const VolScalarField1D& rhs)
{
    // Strict physics/mesh safety checks for FVM
    if (&mesh_ != &rhs.mesh_)
    {
        throw std::invalid_argument("VolScalarField1D::operator-=: mesh mismatch");
    }
    if (location_ != rhs.location_)
    {
        throw std::invalid_argument("VolScalarField1D::operator-=: location mismatch (Cell_Center vs Cell_Face)");
    }
    if (data_.size() != rhs.Size())
    {
        throw std::invalid_argument("VolScalarField1D::operator-=: size mismatch");
    }

    for (std::size_t index = 0; index < data_.size(); ++index)
    {
        data_[index] -= rhs[index];
    }

    return *this;
}

VolScalarField1D& VolScalarField1D::operator*=(double scalar)
{
    for (double& entry : data_)
    {
        entry *= scalar;
    }

    return *this;
}

VolScalarField1D& VolScalarField1D::operator/=(double scalar)
{
    if (scalar == 0.0)
    {
        throw std::invalid_argument("VolScalarField1D::operator/=: division by zero");
    }

    for (double& entry : data_)
    {
        entry /= scalar;
    }

    return *this;
}

VolScalarField1D operator+(VolScalarField1D lhs, const VolScalarField1D& rhs)
{
    lhs += rhs;
    return lhs;
}

VolScalarField1D operator-(VolScalarField1D lhs, const VolScalarField1D& rhs)
{
    lhs -= rhs;
    return lhs;
}

VolScalarField1D operator*(VolScalarField1D lhs, double scalar)
{
    lhs *= scalar;
    return lhs;
}

VolScalarField1D operator*(double scalar, VolScalarField1D rhs)
{
    rhs *= scalar;
    return rhs;
}

VolScalarField1D operator/(VolScalarField1D lhs, double scalar)
{
    lhs /= scalar;
    return lhs;
}

}  // namespace cfd::field
