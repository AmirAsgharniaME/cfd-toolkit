
#include "cfd/schemes/interpolation/harmonic_mean.hpp"

#include <cmath>
#include <limits>
#include <stdexcept>

namespace cfd::schemes::interpolation
{

double harmonic_mean(
    double gamma_p,
    double gamma_n,
    double distance_p,
    double distance_n)
{
    if (!std::isfinite(gamma_p) || gamma_p <= 0.0 ||
        !std::isfinite(gamma_n) || gamma_n <= 0.0)
    {
        throw std::invalid_argument("harmonic_mean diffusivities must be finite and positive");
    }
    if (!std::isfinite(distance_p) || distance_p <= 0.0 ||
        !std::isfinite(distance_n) || distance_n <= 0.0)
    {
        throw std::invalid_argument("harmonic_mean distances must be finite and positive");
    }

    // Normalize distances first so their sum cannot overflow. Long-double
    // intermediates also retain precision for highly disparate inputs.
    const long double dp = static_cast<long double>(distance_p);
    const long double dn = static_cast<long double>(distance_n);
    const long double distance_scale = dp > dn ? dp : dn;
    const long double weight_p = (dp / distance_scale) /
                                 ((dp / distance_scale) + (dn / distance_scale));
    const long double weight_n = 1.0L - weight_p;
    const long double resistance =
        weight_p / static_cast<long double>(gamma_p) +
        weight_n / static_cast<long double>(gamma_n);
    const long double face_gamma = 1.0L / resistance;

    if (!std::isfinite(face_gamma) ||
        face_gamma > static_cast<long double>(std::numeric_limits<double>::max()))
    {
        throw std::overflow_error("harmonic_mean result is not representable as a positive double");
    }

    const double result = static_cast<double>(face_gamma);

    if (!std::isfinite(result) || result <= 0.0)
    {
        throw std::overflow_error("harmonic_mean result is not representable as a positive double");
    }
    return result;
}

} // namespace cfd::schemes::interpolation
