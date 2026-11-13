#pragma once

namespace cfd::schemes::interpolation
{

/// Distance-weighted harmonic interpolation of positive cell diffusivities.
///
/// Gamma_f = (d_owner + d_neighbour) /
///           (d_owner / Gamma_owner + d_neighbour / Gamma_neighbour)
///
/// Distances are measured from the cell centers to their shared face.
/// All arguments must be finite and strictly positive. Throws if the
/// result cannot be represented as a positive double.
[[nodiscard]] double harmonic_mean(
    double gamma_owner,
    double gamma_neighbour,
    double d_owner,
    double d_neighbour);

} // namespace cfd::schemes::interpolation
