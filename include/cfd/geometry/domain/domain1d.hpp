#pragma once

namespace cfd::geometry {

/// Axis-aligned one-dimensional domain on a finite, strictly increasing X interval.
/// Both endpoints and their represented difference must be finite.
/// The stored endpoints are authoritative; length_x() returns x_end() - x_start().
///
/// Finite-volume meshes constructed from this domain take an independent
/// snapshot of its bounds. Later domain mutations neither synchronize nor
/// invalidate those meshes; construct a new mesh to use the updated bounds.
class Domain1D {
public:
    /// Construct the interval [0, length_x].
    explicit Domain1D(double length_x);

    /// Construct an interval from its start and end coordinates.
    Domain1D(double x_start, double x_end);

    /// Construct an interval from its start coordinate and length.
    /// The end is computed as x_start + length_x in floating-point arithmetic.
    /// Finite precision (machine epsilon) can make the represented length differ
    /// from the request, especially at large coordinate magnitudes. Throws
    /// std::invalid_argument for a nonfinite/nonpositive length, nonfinite bounds
    /// or extent, or an end that rounds back to the start.
    [[nodiscard]] static Domain1D from_start_and_length(double x_start, double length_x);

    [[nodiscard]] constexpr double x_start() const noexcept { return x_start_; }
    [[nodiscard]] constexpr double x_end()   const noexcept { return x_end_; }
    /// Represented endpoint difference; may differ from a requested input length.
    [[nodiscard]] constexpr double length_x() const noexcept { return x_end_ - x_start_; }

    /// Exact closed-interval membership; NaN and infinities return false.
    /// No tolerance is applied; this does not assign points to mesh cells.
    [[nodiscard]] bool contains(double x) const noexcept { return x >= x_start_ && x <= x_end_; }

    /// Replace both bounds after validation; throws std::invalid_argument for
    /// nonfinite bounds, nonincreasing bounds, or a nonfinite difference.
    /// Every rejected update leaves the domain unchanged.
    void set_bounds(double x_start, double x_end);
    void set_x_start(double x_start);
    void set_x_end(double x_end);
    /// Preserve the start and compute the end as x_start() + length_x.
    /// Endpoints remain authoritative: finite-precision rounding can make
    /// length_x() differ from the request. Uses the same validation and rounding
    /// contract as from_start_and_length(); rejected updates leave bounds unchanged.
    void set_length_x(double length_x);

    // Equality for tests and configuration comparisons (exact float compare).
    friend bool operator==(const Domain1D& a, const Domain1D& b) noexcept {
        return a.x_start_ == b.x_start_ && a.x_end_ == b.x_end_;
    }
    friend bool operator!=(const Domain1D& a, const Domain1D& b) noexcept {
        return !(a == b);
    }
private:
    double x_start_;
    double x_end_;
};

} // namespace cfd::geometry
