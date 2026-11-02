
#pragma once


/**
 * @brief Computes the face diffusivity using a distance-weighted harmonic mean 
 *        with numerical safety scaling.
 * 
 * Mathematical Formula:
 *                     d_P + d_N
 *     Gamma_f = -----------------------
 *               (d_P / Gamma_P) + (d_N / Gamma_N)
 * 
 * Physical/Mathematical Rationale:
 * In FVM, transport between two cell centers (P and N) acts like two thermal/diffusive
 * resistances in series (R = distance / Gamma). The harmonic mean strictly models 
 * this series behavior, where the lowest diffusivity (the bottleneck) dominates 
 * the overall flux. 
 * 
 * Numerical Scaling & Robustness:
 * To prevent catastrophic numerical overflow or underflow (e.g., when diffusivities 
 * span multiple orders of magnitude between extreme materials like copper and aerogel), 
 * intermediate calculations utilize `long double` precision and safety scaling 
 * (such as distance normalization). This guarantees robust convergence even when 
 * face-to-center distances or material property ratios become extreme.
 */



namespace cfd::schemes::interpolation
{

// Resistance-weighted interpolation of positive cell diffusivities to a face.
// Distances are measured from the neighboring cell centers to that face.
[[nodiscard]] double harmonic_mean(
    double gamma_p,
    double gamma_n,
    double distance_p,
    double distance_n);

} // namespace cfd::schemes::interpolation
