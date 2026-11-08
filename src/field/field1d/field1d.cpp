#include <cfd/field/field1d/field1d.hpp>

#include <algorithm>
#include <stdexcept>
#include <utility>

#include <cfd/mesh/mesh1d/fv_mesh1d.hpp>

namespace cfd::field
{

template<typename ValueType, typename LocationTag>
Field1D<ValueType, LocationTag>::Field1D(std::string name, const cfd::mesh::FvMesh1D& mesh,
                                   const ValueType& initial_val)
    : name_(std::move(name)), mesh_(mesh)
{
    if constexpr (std::is_same_v<LocationTag, CellLocation>)
        values_.assign(mesh.N_Cells(), initial_val);
    else
        values_.assign(mesh.N_Faces(), initial_val);
}



template<typename ValueType, typename LocationTag>
const std::string& Field1D<ValueType, LocationTag>::Name() const noexcept
{ return name_; }

template<typename ValueType, typename LocationTag>
const cfd::mesh::FvMesh1D& Field1D<ValueType, LocationTag>::Mesh() const noexcept
{ return mesh_; }

template<typename ValueType, typename LocationTag>
std::size_t Field1D<ValueType, LocationTag>::size() const noexcept
{ return values_.size(); }

template<typename ValueType, typename LocationTag>
std::size_t Field1D<ValueType, LocationTag>::N_Faces() const noexcept requires std::is_same_v<LocationTag, FaceLocation>
{ return values_.size(); }

template<typename ValueType, typename LocationTag>
ValueType& Field1D<ValueType, LocationTag>::operator[](std::size_t index) noexcept
{ return values_[index]; }

template<typename ValueType, typename LocationTag>
const ValueType& Field1D<ValueType, LocationTag>::operator[](std::size_t index) const noexcept
{ return values_[index]; }

template<typename ValueType, typename LocationTag>
std::span<ValueType> Field1D<ValueType, LocationTag>::values() noexcept
{ return values_; }

template<typename ValueType, typename LocationTag>
std::span<const ValueType> Field1D<ValueType, LocationTag>::values() const noexcept
{ return values_; }

template<typename ValueType, typename LocationTag>
std::span<ValueType> Field1D<ValueType, LocationTag>::span() noexcept
{ return values_; }

template<typename ValueType, typename LocationTag>
std::span<const ValueType> Field1D<ValueType, LocationTag>::span() const noexcept
{ return values_; }

template<typename ValueType, typename LocationTag>
std::span<ValueType> Field1D<ValueType, LocationTag>::Values() noexcept
{ return values_; }

template<typename ValueType, typename LocationTag>
std::span<const ValueType> Field1D<ValueType, LocationTag>::Values() const noexcept
{ return values_; }

template<typename ValueType, typename LocationTag>
Field1D<ValueType, LocationTag>& Field1D<ValueType, LocationTag>::operator=(const Field1D<ValueType, LocationTag>& rhs)
{
    if (&mesh_ != &rhs.mesh_)
    {
        throw std::invalid_argument("Field1D<Type, LocationTag>::operator=: mesh mismatch");
    }
    name_ = rhs.name_;
    values_ = rhs.values_;
    return *this;
}

template<typename ValueType, typename LocationTag>
Field1D<ValueType, LocationTag>& Field1D<ValueType, LocationTag>::operator=(Field1D<ValueType, LocationTag>&& rhs)
{
    if (this == &rhs) return *this;
    if (&mesh_ != &rhs.mesh_)
    {
        throw std::invalid_argument("Field1D<Type, LocationTag>::operator=: mesh mismatch");
    }
    name_ = std::move(rhs.name_);
    values_ = std::move(rhs.values_);
    return *this;
}

template<typename ValueType, typename LocationTag>
ValueType& Field1D<ValueType, LocationTag>::at(std::size_t index)
{
    if (index >= values_.size()) throw std::out_of_range("Field1D<Type, LocationTag>::at: index out of range");
    return values_[index];
}

template<typename ValueType, typename LocationTag>
const ValueType& Field1D<ValueType, LocationTag>::at(std::size_t index) const
{
    if (index >= values_.size()) throw std::out_of_range("Field1D<Type, LocationTag>::at: index out of range");
    return values_[index];
}

template<typename ValueType, typename LocationTag>
void Field1D<ValueType, LocationTag>::Fill(const ValueType& value) { std::fill(values_.begin(), values_.end(), value); }

template<typename ValueType, typename LocationTag>
Field1D<ValueType, LocationTag>& Field1D<ValueType, LocationTag>::operator+=(const Field1D<ValueType, LocationTag>& rhs)
{
    if (&mesh_ != &rhs.mesh_) throw std::invalid_argument("Field1D<Type, LocationTag>::operator+=: mesh mismatch");
    if (values_.size() != rhs.values_.size()) throw std::invalid_argument("Field1D<Type, LocationTag>::operator+=: size mismatch");
    for (std::size_t i = 0; i < values_.size(); ++i) values_[i] += rhs.values_[i];
    return *this;
}

template<typename ValueType, typename LocationTag>
Field1D<ValueType, LocationTag>& Field1D<ValueType, LocationTag>::operator-=(const Field1D<ValueType, LocationTag>& rhs)
{
    if (&mesh_ != &rhs.mesh_) throw std::invalid_argument("Field1D<Type, LocationTag>::operator-=: mesh mismatch");
    if (values_.size() != rhs.values_.size()) throw std::invalid_argument("Field1D<Type, LocationTag>::operator-=: size mismatch");
    for (std::size_t i = 0; i < values_.size(); ++i) values_[i] -= rhs.values_[i];
    return *this;
}

template<typename ValueType, typename LocationTag>
Field1D<ValueType, LocationTag>& Field1D<ValueType, LocationTag>::operator*=(const Field1D<ValueType, LocationTag>& rhs) requires std::is_same_v<ValueType, double>
{
    if (&mesh_ != &rhs.mesh_) throw std::invalid_argument("Field1D<Type, LocationTag>::operator*=: mesh mismatch");
    if (values_.size() != rhs.values_.size()) throw std::invalid_argument("Field1D<Type, LocationTag>::operator*=: size mismatch");
    for (std::size_t i = 0; i < values_.size(); ++i) values_[i] *= rhs.values_[i];
    return *this;
}

template<typename ValueType, typename LocationTag>
Field1D<ValueType, LocationTag>& Field1D<ValueType, LocationTag>::operator/=(const Field1D<ValueType, LocationTag>& rhs) requires std::is_same_v<ValueType, double>
{
    if (&mesh_ != &rhs.mesh_) throw std::invalid_argument("Field1D<Type, LocationTag>::operator/=: mesh mismatch");
    if (values_.size() != rhs.values_.size()) throw std::invalid_argument("Field1D<Type, LocationTag>::operator/=: size mismatch");
    if (std::find(rhs.values_.begin(), rhs.values_.end(), 0.0) != rhs.values_.end())
    {
        throw std::invalid_argument("Field1D<Type, LocationTag>::operator/=: division by zero");
    }
    for (std::size_t i = 0; i < values_.size(); ++i)
    {
        values_[i] /= rhs.values_[i];
    }
    return *this;
}

template<typename ValueType, typename LocationTag>
Field1D<ValueType, LocationTag>& Field1D<ValueType, LocationTag>::operator*=(double scalar)
{
    for (ValueType& value : values_) value *= scalar;
    return *this;
}

template<typename ValueType, typename LocationTag>
Field1D<ValueType, LocationTag>& Field1D<ValueType, LocationTag>::operator/=(double scalar)
{
    if (scalar == 0.0) throw std::invalid_argument("Field1D<Type, LocationTag>::operator/=: division by zero");
    for (ValueType& value : values_) value /= scalar;
    return *this;
}


template class Field1D<double, CellLocation>;
template class Field1D<double, FaceLocation>;
template class Field1D<cfd::vector::Vector1D, CellLocation>;
template class Field1D<cfd::vector::Vector1D, FaceLocation>;

}  // namespace cfd::field
