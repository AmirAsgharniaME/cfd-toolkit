#pragma once

namespace cfd::field
{

/// A lightweight one-dimensional vector value.
struct Vector1D
{
    double x{0.0};

    constexpr Vector1D() noexcept = default;
    explicit constexpr Vector1D(double val) noexcept : x(val) {}

    constexpr Vector1D& operator+=(const Vector1D& rhs) noexcept
    {
        x += rhs.x;
        return *this;
    }

    constexpr Vector1D& operator-=(const Vector1D& rhs) noexcept
    {
        x -= rhs.x;
        return *this;
    }

    constexpr Vector1D& operator*=(double scalar) noexcept
    {
        x *= scalar;
        return *this;
    }

    constexpr Vector1D& operator/=(double scalar) noexcept
    {
        x /= scalar;
        return *this;
    }

    [[nodiscard]] friend constexpr Vector1D operator+(Vector1D lhs, const Vector1D& rhs) noexcept
    {
        lhs += rhs;
        return lhs;
    }

    [[nodiscard]] friend constexpr Vector1D operator-(Vector1D lhs, const Vector1D& rhs) noexcept
    {
        lhs -= rhs;
        return lhs;
    }

    [[nodiscard]] friend constexpr Vector1D operator*(Vector1D value, double scalar) noexcept
    {
        value *= scalar;
        return value;
    }

    [[nodiscard]] friend constexpr Vector1D operator*(double scalar, Vector1D value) noexcept
    {
        value *= scalar;
        return value;
    }

    [[nodiscard]] friend constexpr Vector1D operator/(Vector1D value, double scalar) noexcept
    {
        value /= scalar;
        return value;
    }

    [[nodiscard]] constexpr Vector1D operator-() const noexcept
    {
        return Vector1D{-x};
    }

    constexpr bool operator==(const Vector1D&) const noexcept = default;
};

}  // namespace cfd::field
