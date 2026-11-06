#pragma once

/**
 * @file central_difference.hpp
 * @brief Central-difference gradient scheme for neighboring cell values.
 *
 * For values at neighboring cell centers P and N, this scheme estimates the
 * gradient at the midpoint between them as
 * @f[ \left.\frac{\partial \phi}{\partial x}\right|_f
 *     \approx \frac{\phi_N - \phi_P}{d_P + d_N}, @f]
 * where @f$d_P@f$ and @f$d_N@f$ are the distances from the neighboring cell
 * centers to their shared face.
 * For a smooth field on a uniform one-dimensional grid, the estimate has a
 * second-order truncation error, @f$O(\Delta x^2)@f$. It uses both cell-center
 * values symmetrically, making it physically appropriate for diffusion
 * problems where the flux is driven by the difference in potential across
 * the center-to-center distance.
 *
 * The implementation rejects non-finite field values and non-finite or
 * non-positive distances. It evaluates the difference and quotient using
 * @c long @c double intermediates, then checks that the result is finite and
 * representable as a @c double before returning it.
 */
namespace cfd::schemes::gradient
{

/**
 * @brief Computes the central-difference gradient between neighboring cells.
 * @param phi_p Finite field value at cell center P.
 * @param phi_n Finite field value at cell center N.
 * @param distance_p Finite, strictly positive distance from center P to the
 *        shared face.
 * @param distance_n Finite, strictly positive distance from center N to the
 *        shared face.
 * @return The estimated gradient, @f$(\phi_N - \phi_P) / (d_P + d_N)@f$.
 * @throws std::invalid_argument if either field value is non-finite, or if
 *         either distance is non-finite or not strictly positive.
 * @throws std::overflow_error if the computed gradient is not representable
 *         as a finite @c double.
 */
[[nodiscard]] double central_difference(
    double phi_p,
    double phi_n,
    double distance_p,
    double distance_n);

} // namespace cfd::schemes::gradient
