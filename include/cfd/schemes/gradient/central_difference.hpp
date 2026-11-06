#pragma once

/**
 * @file central_difference.hpp
 * @brief Two-point gradient estimate from neighboring cell-center values.
 *
 * For cell centers P and N separated by a shared face, computes
 *
 * @f[
 *     \frac{\phi_N - \phi_P}{d_P + d_N},
 * @f]
 *
 * where d_P and d_N are the positive center-to-face distances.
 *
 * For a smooth field, this secant approximates the derivative at the
 * midpoint between the two cell centers with second-order accuracy.
 * On a nonuniform mesh, the shared face need not lie at that midpoint;
 * second-order accuracy at the face is therefore not guaranteed.
 *
 * The implementation rejects non-finite field values and non-finite or
 * non-positive distances. It uses long double intermediates and throws
 * if the computed result is not finite or cannot be returned as a
 * finite double.
 */
namespace cfd::schemes::gradient
{

/**
 * @brief Computes the two-point gradient between neighboring cells.
 * @param phi_p Finite field value at cell center P.
 * @param phi_n Finite field value at cell center N.
 * @param distance_p Finite, strictly positive distance from P to the face.
 * @param distance_n Finite, strictly positive distance from N to the face.
 * @return (phi_n - phi_p) / (distance_p + distance_n).
 * @throws std::invalid_argument for invalid field values or distances.
 * @throws std::overflow_error if an intermediate calculation or the
 *         resulting gradient is not finite or representable as a double.
 */
[[nodiscard]] double central_difference(
    double phi_p,
    double phi_n,
    double distance_p,
    double distance_n);

} // namespace cfd::schemes::gradient
