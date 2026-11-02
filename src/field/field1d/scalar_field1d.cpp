#include <cfd/field/field1d/scalar_field1d.hpp>

#include <algorithm>
#include <stdexcept>
#include <utility>

#include <cfd/mesh/mesh1d/fv_mesh1d.hpp>

namespace cfd::field
{

template<class LocationTag>
ScalarField1D<LocationTag>::ScalarField1D(std::string name, const cfd::mesh::FvMesh1D& mesh,
                                   double initial_val)
    : name_(std::move(name)), mesh_(mesh)
{
    if constexpr (std::is_same_v<LocationTag, CellLocation>)
        data_.assign(mesh.N_Cells(), initial_val);
    else
        data_.assign(mesh.N_Faces(), initial_val);
}

template<class LocationTag>
ScalarField1D<LocationTag>& ScalarField1D<LocationTag>::operator=(const ScalarField1D<LocationTag>& rhs)
{
    if (&mesh_ != &rhs.mesh_)
    {
        throw std::invalid_argument("ScalarField1D<LocationTag>::operator=: mesh mismatch");
    }
    name_ = rhs.name_;
    data_ = rhs.data_;
    return *this;
}

template<class LocationTag>
ScalarField1D<LocationTag>& ScalarField1D<LocationTag>::operator=(ScalarField1D<LocationTag>&& rhs)
{
    if (this == &rhs) return *this;
    if (&mesh_ != &rhs.mesh_)
    {
        throw std::invalid_argument("ScalarField1D<LocationTag>::operator=: mesh mismatch");
    }
    name_ = std::move(rhs.name_);
    data_ = std::move(rhs.data_);
    return *this;
}

template<class LocationTag>
double& ScalarField1D<LocationTag>::at(std::size_t index)
{
    if (index >= data_.size()) throw std::out_of_range("ScalarField1D<LocationTag>::at: index out of range");
    return data_[index];
}

template<class LocationTag>
const double& ScalarField1D<LocationTag>::at(std::size_t index) const
{
    if (index >= data_.size()) throw std::out_of_range("ScalarField1D<LocationTag>::at: index out of range");
    return data_[index];
}

template<class LocationTag>
void ScalarField1D<LocationTag>::Fill(double value) { std::fill(data_.begin(), data_.end(), value); }

template<class LocationTag>
ScalarField1D<LocationTag>& ScalarField1D<LocationTag>::operator+=(const ScalarField1D<LocationTag>& rhs)
{
    if (&mesh_ != &rhs.mesh_) throw std::invalid_argument("ScalarField1D<LocationTag>::operator+=: mesh mismatch");
    if (data_.size() != rhs.data_.size()) throw std::invalid_argument("ScalarField1D<LocationTag>::operator+=: size mismatch");
    for (std::size_t i = 0; i < data_.size(); ++i) data_[i] += rhs.data_[i];
    return *this;
}

template<class LocationTag>
ScalarField1D<LocationTag>& ScalarField1D<LocationTag>::operator-=(const ScalarField1D<LocationTag>& rhs)
{
    if (&mesh_ != &rhs.mesh_) throw std::invalid_argument("ScalarField1D<LocationTag>::operator-=: mesh mismatch");
    if (data_.size() != rhs.data_.size()) throw std::invalid_argument("ScalarField1D<LocationTag>::operator-=: size mismatch");
    for (std::size_t i = 0; i < data_.size(); ++i) data_[i] -= rhs.data_[i];
    return *this;
}

template<class LocationTag>
ScalarField1D<LocationTag>& ScalarField1D<LocationTag>::operator*=(const ScalarField1D<LocationTag>& rhs)
{
    if (&mesh_ != &rhs.mesh_) throw std::invalid_argument("ScalarField1D<LocationTag>::operator*=: mesh mismatch");
    if (data_.size() != rhs.data_.size()) throw std::invalid_argument("ScalarField1D<LocationTag>::operator*=: size mismatch");
    for (std::size_t i = 0; i < data_.size(); ++i) data_[i] *= rhs.data_[i];
    return *this;
}

template<class LocationTag>
ScalarField1D<LocationTag>& ScalarField1D<LocationTag>::operator/=(const ScalarField1D<LocationTag>& rhs)
{
    if (&mesh_ != &rhs.mesh_) throw std::invalid_argument("ScalarField1D<LocationTag>::operator/=: mesh mismatch");
    if (data_.size() != rhs.data_.size()) throw std::invalid_argument("ScalarField1D<LocationTag>::operator/=: size mismatch");
    if (std::find(rhs.data_.begin(), rhs.data_.end(), 0.0) != rhs.data_.end())
    {
        throw std::invalid_argument("ScalarField1D<LocationTag>::operator/=: division by zero");
    }
    for (std::size_t i = 0; i < data_.size(); ++i)
    {
        data_[i] /= rhs.data_[i];
    }
    return *this;
}

template<class LocationTag>
ScalarField1D<LocationTag>& ScalarField1D<LocationTag>::operator*=(double scalar)
{
    for (double& value : data_) value *= scalar;
    return *this;
}

template<class LocationTag>
ScalarField1D<LocationTag>& ScalarField1D<LocationTag>::operator/=(double scalar)
{
    if (scalar == 0.0) throw std::invalid_argument("ScalarField1D<LocationTag>::operator/=: division by zero");
    for (double& value : data_) value /= scalar;
    return *this;
}


template class ScalarField1D<CellLocation>;
template class ScalarField1D<FaceLocation>;

}  // namespace cfd::field
