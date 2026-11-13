
#include "cfd/schemes/interpolation/arithmetic_mean.hpp"

#include <cmath>
#include <limits>
#include <stdexcept>

namespace cfd::schemes::interpolation
{

double arithmetic_mean(
    double value_owner,
    double value_neighbour,
    double d_owner,
    double d_neighbour)
{
    if (!std::isfinite(value_owner) || !std::isfinite(value_neighbour))
    {
        throw std::invalid_argument("arithmetic_mean field values must be finite");
    }
    if (!std::isfinite(d_owner) || d_owner <= 0.0 ||
        !std::isfinite(d_neighbour) || d_neighbour <= 0.0)
    {
        throw std::invalid_argument("arithmetic_mean distances must be finite and positive");
    }

    // Normalize distances first so their sum remains bounded. The weights
    // then represent the linear fractions from the opposite center distances.
    const long double owner_distance = static_cast<long double>(d_owner);
    const long double neighbour_distance = static_cast<long double>(d_neighbour);
    const long double distance_scale =
        owner_distance > neighbour_distance ? owner_distance : neighbour_distance;
    const long double scaled_owner = owner_distance / distance_scale;
    const long double scaled_neighbour = neighbour_distance / distance_scale;
    const long double weight_owner = scaled_neighbour / (scaled_owner + scaled_neighbour);
    const long double weight_neighbour = 1.0L - weight_owner;

    const long double face_value =
        weight_owner * static_cast<long double>(value_owner) +
        weight_neighbour * static_cast<long double>(value_neighbour);
    const long double max_double =
        static_cast<long double>(std::numeric_limits<double>::max());

    if (!std::isfinite(face_value) || face_value > max_double || face_value < -max_double)
    {
        throw std::overflow_error("arithmetic_mean result is not representable as a finite double");
    }

    const double result = static_cast<double>(face_value);
    if (!std::isfinite(result))
    {
        throw std::overflow_error("arithmetic_mean result is not representable as a finite double");
    }
    return result;
}

} // namespace cfd::schemes::interpolation
