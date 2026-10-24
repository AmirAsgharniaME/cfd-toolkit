#include "cfd/field/field1d/vol_scalar_field1d.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace cfd::field::field1d
{

ScalarField1D::ScalarField1D(std::string name,
                             const mesh::UniformMesh1D& mesh,
                             FieldLocation location,
                             double initial_val)
    : name_(std::move(name)),
      mesh_(mesh),
      location_(location)
{
    const std::size_t entry_count = (location_ == FieldLocation::Face)
                                        ? mesh_.num_faces()
                                        : mesh_.num_cells();

    data_.assign(entry_count, initial_val);
}

double& ScalarField1D::at(std::size_t index)
{
    if (index >= data_.size())
    {
        throw std::out_of_range("ScalarField1D::at: index out of range");
    }

    return data_[index];
}

const double& ScalarField1D::at(std::size_t index) const
{
    if (index >= data_.size())
    {
        throw std::out_of_range("ScalarField1D::at: index out of range");
    }

    return data_[index];
}

void ScalarField1D::fill(double value)
{
    // Using std::fill for cleaner code and compiler vectorization optimization (SIMD)
    std::fill(data_.begin(), data_.end(), value);
}

ScalarField1D& ScalarField1D::operator+=(const ScalarField1D& rhs)
{
    // Strict physics/mesh safety checks for FVM
    if (&mesh_ != &rhs.mesh_)
    {
        throw std::invalid_argument("ScalarField1D::operator+=: mesh mismatch");
    }
    if (location_ != rhs.location_)
    {
        throw std::invalid_argument("ScalarField1D::operator+=: location mismatch (Cell vs Face)");
    }
    if (data_.size() != rhs.size())
    {
        throw std::invalid_argument("ScalarField1D::operator+=: size mismatch");
    }

    for (std::size_t index = 0; index < data_.size(); ++index)
    {
        data_[index] += rhs[index];
    }

    return *this;
}

ScalarField1D& ScalarField1D::operator-=(const ScalarField1D& rhs)
{
    // Strict physics/mesh safety checks for FVM
    if (&mesh_ != &rhs.mesh_)
    {
        throw std::invalid_argument("ScalarField1D::operator-=: mesh mismatch");
    }
    if (location_ != rhs.location_)
    {
        throw std::invalid_argument("ScalarField1D::operator-=: location mismatch (Cell vs Face)");
    }
    if (data_.size() != rhs.size())
    {
        throw std::invalid_argument("ScalarField1D::operator-=: size mismatch");
    }

    for (std::size_t index = 0; index < data_.size(); ++index)
    {
        this->data_[index] -= rhs[index];
    }

    return *this;
}

ScalarField1D& ScalarField1D::operator*=(double scalar)
{
    for (double& entry : data_)
    {
        entry *= scalar;
    }

    return *this;
}

ScalarField1D& ScalarField1D::operator/=(double scalar)
{
    if (scalar == 0.0)
    {
        throw std::invalid_argument("ScalarField1D::operator/=: division by zero");
    }

    for (double& entry : data_)
    {
        entry /= scalar;
    }

    return *this;
}

ScalarField1D operator+(ScalarField1D lhs, const ScalarField1D& rhs)
{
    lhs += rhs;
    return lhs;
}

ScalarField1D operator-(ScalarField1D lhs, const ScalarField1D& rhs)
{
    lhs -= rhs;
    return lhs;
}

ScalarField1D operator*(ScalarField1D lhs, double scalar)
{
    lhs *= scalar;
    return lhs;
}

ScalarField1D operator*(double scalar, ScalarField1D rhs)
{
    rhs *= scalar;
    return rhs;
}

ScalarField1D operator/(ScalarField1D lhs, double scalar)
{
    lhs /= scalar;
    return lhs;
}

}  // namespace cfd::field::field1d
