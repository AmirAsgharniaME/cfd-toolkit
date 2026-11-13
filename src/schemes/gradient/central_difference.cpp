#include "cfd/schemes/gradient/central_difference.hpp"

#include <cmath>
#include <limits>
#include <stdexcept>

namespace cfd::schemes::gradient
{

double central_difference(
    double phi_owner,
    double phi_neighbour,
    double distance_owner,
    double distance_neighbour)
{
    if (!std::isfinite(phi_owner) || !std::isfinite(phi_neighbour))
    {
        throw std::invalid_argument("central_difference field values must be finite");
    }
    if (!std::isfinite(distance_owner) || distance_owner <= 0.0
        || !std::isfinite(distance_neighbour) || distance_neighbour <= 0.0)
    {
        throw std::invalid_argument(
            "central_difference distances must be finite and positive");
    }

    const long double difference =
        static_cast<long double>(phi_neighbour)
        - static_cast<long double>(phi_owner);
    const long double distance =
        static_cast<long double>(distance_owner)
        + static_cast<long double>(distance_neighbour);
    if (!std::isfinite(distance))
    {
        throw std::overflow_error(
            "central_difference distance sum is not representable");
    }
    const long double gradient =
        difference / distance;
    const long double max_double =
        static_cast<long double>(std::numeric_limits<double>::max());

    if (!std::isfinite(gradient) || gradient > max_double || gradient < -max_double)
    {
        throw std::overflow_error(
            "central_difference result is not representable as a finite double");
    }

    const double result = static_cast<double>(gradient);
    if (!std::isfinite(result))
    {
        throw std::overflow_error(
            "central_difference result is not representable as a finite double");
    }
    return result;
}

} // namespace cfd::schemes::gradient
