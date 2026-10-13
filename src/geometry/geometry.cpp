#include "cfd/geometry/geometry.hpp"

#include <stdexcept>

namespace
{
    void ValidateLength(double Length_)
    {
        if (Length_ <= 0.0)
        {
            throw std::invalid_argument("Length must be greater than zero.");
        }
    }
}

Geometry::Geometry(double LengthX_)
    : LengthX(LengthX_),
      LengthY(0.0),
      LengthZ(0.0),
      GeometryDimension(Dimension::OneD)
{
    ValidateLength(LengthX_);
}

Geometry::Geometry(double LengthX_, double LengthY_)
    : LengthX(LengthX_),
      LengthY(LengthY_),
      LengthZ(0.0),
      GeometryDimension(Dimension::TwoD)
{
    ValidateLength(LengthX_);
    ValidateLength(LengthY_);
}

Geometry::Geometry(double LengthX_, double LengthY_, double LengthZ_)
    : LengthX(LengthX_),
      LengthY(LengthY_),
      LengthZ(LengthZ_),
      GeometryDimension(Dimension::ThreeD)
{
    ValidateLength(LengthX_);
    ValidateLength(LengthY_);
    ValidateLength(LengthZ_);
}

double Geometry::GetLengthY() const
{
    if (GeometryDimension == Dimension::OneD)
    {
        throw std::logic_error("A 1D geometry has no Y length.");
    }
    return LengthY;
}

double Geometry::GetLengthZ() const
{
    if (GeometryDimension != Dimension::ThreeD)
    {
        throw std::logic_error("A geometry below 3D has no Z length.");
    }
    return LengthZ;
}

void Geometry::SetLengthX(double LengthX_)
{
    ValidateLength(LengthX_);
    LengthX = LengthX_;
}

void Geometry::SetLengthY(double LengthY_)
{
    if (GeometryDimension == Dimension::OneD)
    {
        throw std::logic_error("Cannot set a Y length on a 1D geometry.");
    }
    ValidateLength(LengthY_);
    LengthY = LengthY_;
}

void Geometry::SetLengthZ(double LengthZ_)
{
    if (GeometryDimension != Dimension::ThreeD)
    {
        throw std::logic_error("Cannot set a Z length on a geometry below 3D.");
    }
    ValidateLength(LengthZ_);
    LengthZ = LengthZ_;
}

void Geometry::SetLength(double LengthX_)
{
    SetLengthX(LengthX_);
}
