#include "cfd/schemes/gradient/central_difference.hpp"

#include <cmath>
#include <limits>
#include <stdexcept>

namespace cfd::schemes::gradient
{

double central_difference(
    double phi_p,
    double phi_n,
    double distance_pn)
{
    if (!std::isfinite(phi_p) || !std::isfinite(phi_n))
    {
        throw std::invalid_argument("central_difference field values must be finite");
    }
    if (!std::isfinite(distance_pn) || distance_pn <= 0.0)
    {
        throw std::invalid_argument(
            "central_difference distance must be finite and positive");
    }

    const long double difference =
        static_cast<long double>(phi_n) - static_cast<long double>(phi_p);
    const long double gradient =
        difference / static_cast<long double>(distance_pn);
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
