
#pragma once

/**
 * @brief Linearly interpolates neighboring cell values to a face.
 *Linear Differencing Scheme (CDS)
 * For P and N cell-center values, the face value is
 * @f[\phi_f = \frac{d_N}{d_P + d_N}\phi_P +
 *       \frac{d_P}{d_P + d_N}\phi_N.@f]
 * Here @f$d_P@f$ and @f$d_N@f$ are the positive distances from the
 * respective centers to the face. The closer center receives the larger
 * weight, which is the linear interpolation fraction for this geometry.
 * Field values may be any finite doubles, including negative values. The
 * implementation scales distances before forming weights and uses long-double
 * intermediates to avoid overflow in their sum and to retain precision for
 * highly unequal distances.
 */
namespace cfd::schemes::interpolation
{

/**
 * @brief Computes the distance-weighted arithmetic mean at a face.
 * @param phi_p Field value at the P cell center; must be finite.
 * @param phi_n Field value at the N cell center; must be finite.
 * @param distance_p Positive distance from the P center to the face.
 * @param distance_n Positive distance from the N center to the face.
 * @return The linearly interpolated face value.
 * @throws std::invalid_argument if an input is non-finite or a distance is not positive.
 * @throws std::overflow_error if the result cannot be represented as a finite double.
 */
[[nodiscard]] double arithmetic_mean(
    double phi_p,
    double phi_n,
    double distance_p,
    double distance_n);

} // namespace cfd::schemes::interpolation
