#include "cfd/schemes/interpolation/harmonic_mean.hpp"

#include <cmath>
#include <limits>
#include <stdexcept>

namespace cfd::schemes::interpolation
{

double harmonic_mean(
    double gamma_owner,
    double gamma_neighbour,
    double d_owner,
    double d_neighbour)
{
    if (!std::isfinite(gamma_owner) || gamma_owner <= 0.0 ||
        !std::isfinite(gamma_neighbour) || gamma_neighbour <= 0.0)
    {
        throw std::invalid_argument(
            "harmonic_mean diffusivities must be finite and positive");
    }

    if (!std::isfinite(d_owner) || d_owner <= 0.0 ||
        !std::isfinite(d_neighbour) || d_neighbour <= 0.0)
    {
        throw std::invalid_argument(
            "harmonic_mean distances must be finite and positive");
    }

    const long double owner_distance = static_cast<long double>(d_owner);
    const long double neighbour_distance = static_cast<long double>(d_neighbour);

    // Scale distances before adding them to avoid overflow in dp + dn.
    const long double distance_scale =
        owner_distance > neighbour_distance ? owner_distance : neighbour_distance;
    const long double scaled_owner = owner_distance / distance_scale;
    const long double scaled_neighbour = neighbour_distance / distance_scale;
    const long double scaled_sum = scaled_owner + scaled_neighbour;

    // Compute both weights directly. Deriving one as 1 - the other can
    // erase a small but physically important resistance contribution.
    const long double weight_owner = scaled_owner / scaled_sum;
    const long double weight_neighbour = scaled_neighbour / scaled_sum;

    const long double resistance =
        weight_owner / static_cast<long double>(gamma_owner) +
        weight_neighbour / static_cast<long double>(gamma_neighbour);

    const long double face_gamma = 1.0L / resistance;

    if (!std::isfinite(face_gamma) ||
        face_gamma > static_cast<long double>(
            std::numeric_limits<double>::max()))
    {
        throw std::overflow_error(
            "harmonic_mean result is not representable as a positive double");
    }

    const double result = static_cast<double>(face_gamma);

    if (!std::isfinite(result) || result <= 0.0)
    {
        throw std::overflow_error(
            "harmonic_mean result is not representable as a positive double");
    }

    return result;
}

} // namespace cfd::schemes::interpolation
