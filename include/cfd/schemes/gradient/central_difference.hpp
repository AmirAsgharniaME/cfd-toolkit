#pragma once

/**
 * @file central_difference.hpp
 * @brief Two-point gradient estimate across an internal face.
 *
 * For the owner and neighbour cell centers separated by an internal face,
 * computes the directional derivative along the owner-to-neighbour vector:
 *
 * @f[
 *     \frac{\phi_{neighbour} - \phi_{owner}}
 *          {d_{owner} + d_{neighbour}},
 * @f]
 *
 * where each distance is the positive center-to-face distance for its cell.
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
 * @brief Computes the directional gradient along an internal face's
 *        owner-to-neighbour vector.
 * @param phi_owner Finite field value at the owner cell center.
 * @param phi_neighbour Finite field value at the neighbour cell center.
 * @param distance_owner Finite, strictly positive owner-center-to-face distance.
 * @param distance_neighbour Finite, strictly positive neighbour-center-to-face
 *        distance.
 * @return (phi_neighbour - phi_owner) / (distance_owner + distance_neighbour).
 * @throws std::invalid_argument for invalid field values or distances.
 * @throws std::overflow_error if an intermediate calculation or the
 *         resulting gradient is not finite or representable as a double.
 */
[[nodiscard]] double central_difference(
    double phi_owner,
    double phi_neighbour,
    double distance_owner,
    double distance_neighbour);

} // namespace cfd::schemes::gradient
