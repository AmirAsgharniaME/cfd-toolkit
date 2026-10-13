#pragma once

namespace cfd::geometry
{

/// Axis-aligned two-dimensional domain on finite, increasing X and Y intervals.
class Domain2D
{
public:
    Domain2D(double length_x, double length_y);
    Domain2D(double x_start, double x_end, double y_start, double y_end);

    [[nodiscard]] double x_start() const noexcept { return x_start_; }
    [[nodiscard]] double x_end() const noexcept { return x_end_; }
    [[nodiscard]] double y_start() const noexcept { return y_start_; }
    [[nodiscard]] double y_end() const noexcept { return y_end_; }
    [[nodiscard]] double length_x() const noexcept { return x_end_ - x_start_; }
    [[nodiscard]] double length_y() const noexcept { return y_end_ - y_start_; }

    void set_x_bounds(double start, double end);
    void set_y_bounds(double start, double end);
    void set_x_start(double start);
    void set_x_end(double end);
    void set_y_start(double start);
    void set_y_end(double end);
    void set_length_x(double length);
    void set_length_y(double length);

private:
    double x_start_;
    double x_end_;
    double y_start_;
    double y_end_;
};

}  // namespace cfd::geometry
