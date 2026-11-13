
#pragma once

/**
 * @brief Linearly interpolates owner and neighbour cell values to a face.
 *Linear Differencing Scheme (CDS)
 * For owner and neighbour cell-center values, the face value is
 * @f[\phi_f = \frac{d_{neighbour}}{d_{owner} + d_{neighbour}}\phi_{owner} +
 *       \frac{d_{owner}}{d_{owner} + d_{neighbour}}\phi_{neighbour}.@f]
 * Here @f$d_{owner}@f$ and @f$d_{neighbour}@f$ are the positive distances from
 * the respective centers to the face. The closer center receives the larger
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
 * @param value_owner Field value at the owner cell center; must be finite.
 * @param value_neighbour Field value at the neighbour cell center; must be finite.
 * @param d_owner Positive distance from the owner center to the face.
 * @param d_neighbour Positive distance from the neighbour center to the face.
 * @return The linearly interpolated face value.
 * @throws std::invalid_argument if an input is non-finite or a distance is not positive.
 * @throws std::overflow_error if the result cannot be represented as a finite double.
 */
[[nodiscard]] double arithmetic_mean(
    double value_owner,
    double value_neighbour,
    double d_owner,
    double d_neighbour);

} // namespace cfd::schemes::interpolation
