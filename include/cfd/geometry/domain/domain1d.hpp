#pragma once

namespace cfd::geometry {

/// Coordinate-agnostic one-dimensional domain on a finite, strictly increasing interval.
/// Both endpoints and their represented difference must be finite.
/// The stored endpoints are authoritative; length() returns end() - start().
///
/// Finite-volume meshes constructed from this domain take an independent
/// snapshot of its bounds. Later domain mutations neither synchronize nor
/// invalidate those meshes; construct a new mesh to use the updated bounds.
class Domain1D {
public:
    /// Construct the interval [0, length].
    explicit Domain1D(double domain_length);

    /// Construct an interval from its start and end coordinates.
    Domain1D(double start_coord, double end_coord);

    /// Construct an interval from its start coordinate and length.
    /// The end is computed as start + length in floating-point arithmetic.
    /// Finite precision (machine epsilon) can make the represented length differ
    /// from the request, especially at large coordinate magnitudes. Throws
    /// std::invalid_argument for a nonfinite/nonpositive length, nonfinite bounds
    /// or extent, or an end that rounds back to the start.
    [[nodiscard]] static Domain1D from_start_and_length(double start_coord, double domain_length);

    [[nodiscard]] constexpr double Start_Coord() const noexcept { return start_coord_; }
    [[nodiscard]] constexpr double End_Coord() const noexcept { return end_coord_; }
    /// Represented endpoint difference; may differ from a requested input length.
    [[nodiscard]] constexpr double Length() const noexcept { return end_coord_ - start_coord_; }

    /// Exact closed-interval membership; NaN and infinities return false.
    /// No tolerance is applied; this does not assign points to mesh cells.
    [[nodiscard]] bool IsContain(double coordinate) const noexcept {
        return coordinate >= start_coord_ && coordinate <= end_coord_;
    }

    /// Replace both bounds after validation; throws std::invalid_argument for
    /// nonfinite bounds, nonincreasing bounds, or a nonfinite difference.
    /// Every rejected update leaves the domain unchanged.
    void Set_Bounds(double start_coord, double end_coord);
    void Set_Start_Coord(double start_coord);
    void set_End_Coord(double end_coord);
    /// Preserve the start and compute the end as start() + length.
    /// Endpoints remain authoritative: finite-precision rounding can make
    /// length() differ from the request. Uses the same validation and rounding
    /// contract as from_start_and_length(); rejected updates leave bounds unchanged.
    void Set_Length(double domain_length);

    // Equality for tests and configuration comparisons (exact float compare).
    friend bool operator==(const Domain1D& a, const Domain1D& b) noexcept {
        return a.start_coord_ == b.start_coord_ && a.end_coord_ == b.end_coord_;
    }
    friend bool operator!=(const Domain1D& a, const Domain1D& b) noexcept {
        return !(a == b);
    }
private:
    double start_coord_;
    double end_coord_;
};

} // namespace cfd::geometry
