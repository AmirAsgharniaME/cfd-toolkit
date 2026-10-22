#pragma once

namespace cfd::geometry
{

/// Axis-aligned one-dimensional domain on a finite, increasing X interval.
class Domain1D
{
public:
    /// Construct the interval [0, length_x].
    explicit Domain1D(double length_x);

    /// Construct an interval from its start and end coordinates.
    Domain1D(double x_start, double x_end);

    /// Construct an interval from its start coordinate and length.
    [[nodiscard]] static Domain1D from_start_and_length(double x_start, double length_x);

    [[nodiscard]] double x_start() const noexcept { return x_start_; }
    [[nodiscard]] double x_end() const noexcept { return x_end_; }
    [[nodiscard]] double length_x() const noexcept { return x_end_ - x_start_; }

    void set_bounds(double x_start, double x_end);
    void set_x_start(double x_start);
    void set_x_end(double x_end);
    void set_length_x(double length_x);

private:
    double x_start_;
    double x_end_;
};

}  // namespace cfd::geometry
