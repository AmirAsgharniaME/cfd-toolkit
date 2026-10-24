#pragma once

#include <compare>    // if we want defaulted operator<=>
#include <cstddef>

namespace cfd::geometry {

/// Axis-aligned one-dimensional domain on a finite, strictly increasing X interval.
class Domain1D {
public:
    /// Construct the interval [0, length_x].
    explicit Domain1D(double length_x);

    /// Construct an interval from its start and end coordinates.
    Domain1D(double x_start, double x_end);

    /// Construct an interval from its start coordinate and length.
    [[nodiscard]] static Domain1D from_start_and_length(double x_start, double length_x);

    [[nodiscard]] constexpr double x_start() const noexcept { return x_start_; }
    [[nodiscard]] constexpr double x_end()   const noexcept { return x_end_; }
    [[nodiscard]] constexpr double length_x() const noexcept { return x_end_ - x_start_; }

    /// Returns true if x is within [x_start, x_end].
    [[nodiscard]] bool contains(double x) const noexcept { return x >= x_start_ && x <= x_end_; }

    void set_bounds(double x_start, double x_end);
    void set_x_start(double x_start);
    void set_x_end(double x_end);
    void set_length_x(double length_x);

    // Equality for tests and configuration comparisons (exact float compare).
    friend bool operator==(const Domain1D& a, const Domain1D& b) noexcept {
        return a.x_start_ == b.x_start_ && a.x_end_ == b.x_end_;
    }
    friend bool operator!=(const Domain1D& a, const Domain1D& b) noexcept {
        return !(a == b);
    }
    // If desired: Three-way compare (C++20). Comment out if not needed.
    // friend auto operator<=>(const Domain1D& a, const Domain1D& b) noexcept = default;

private:
    double x_start_;
    double x_end_;
};

} // namespace cfd::geometry
