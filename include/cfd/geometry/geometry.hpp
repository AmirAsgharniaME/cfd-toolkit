#pragma once

class Geometry
{
public:
    enum class Dimension
    {
        OneD,
        TwoD,
        ThreeD
    };

private:
    double LengthX;
    double LengthY;
    double LengthZ;
    Dimension GeometryDimension;

public:
    explicit Geometry(double LengthX_);
    Geometry(double LengthX_, double LengthY_);
    Geometry(double LengthX_, double LengthY_, double LengthZ_);

    [[nodiscard]] Dimension GetDimension() const noexcept
    {
        return GeometryDimension;
    }

    [[nodiscard]] double GetLengthX() const noexcept
    {
        return LengthX;
    }

    [[nodiscard]] double GetLengthY() const;
    [[nodiscard]] double GetLengthZ() const;

    // Kept for compatibility with existing 1D code; returns the X length.
    [[nodiscard]] double GetLength() const noexcept
    {
        return LengthX;
    }

    void SetLengthX(double LengthX_);
    void SetLengthY(double LengthY_);
    void SetLengthZ(double LengthZ_);
    void SetLength(double LengthX_); // Compatibility alias for SetLengthX.
};
