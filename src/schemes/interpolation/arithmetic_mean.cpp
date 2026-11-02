
#include "cfd/schemes/interpolation/arithmetic_mean.hpp"

#include <cmath>
#include <limits>
#include <stdexcept>

namespace cfd::schemes::interpolation
{

double arithmetic_mean(
    double phi_p,
    double phi_n,
    double distance_p,
    double distance_n)
{
    if (!std::isfinite(phi_p) || !std::isfinite(phi_n))
    {
        throw std::invalid_argument("arithmetic_mean field values must be finite");
    }
    if (!std::isfinite(distance_p) || distance_p <= 0.0 ||
        !std::isfinite(distance_n) || distance_n <= 0.0)
    {
        throw std::invalid_argument("arithmetic_mean distances must be finite and positive");
    }

    // Normalize distances first so their sum remains bounded. The weights
    // then represent the linear fractions from the opposite center distances.
    const long double dp = static_cast<long double>(distance_p);
    const long double dn = static_cast<long double>(distance_n);
    const long double distance_scale = dp > dn ? dp : dn;
    const long double scaled_dp = dp / distance_scale;
    const long double scaled_dn = dn / distance_scale;
    const long double weight_p = scaled_dn / (scaled_dp + scaled_dn);
    const long double weight_n = 1.0L - weight_p;

    const long double face_value =
        weight_p * static_cast<long double>(phi_p) +
        weight_n * static_cast<long double>(phi_n);
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
