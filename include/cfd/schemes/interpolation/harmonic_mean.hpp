#pragma once

namespace cfd::schemes::interpolation
{

/// Distance-weighted harmonic interpolation of positive cell diffusivities.
///
/// Gamma_f = (d_P + d_N) / (d_P / Gamma_P + d_N / Gamma_N)
///
/// Distances are measured from the cell centers to their shared face.
/// All arguments must be finite and strictly positive. Throws if the
/// result cannot be represented as a positive double.
[[nodiscard]] double harmonic_mean(
    double gamma_p,
    double gamma_n,
    double distance_p,
    double distance_n);

} // namespace cfd::schemes::interpolation
