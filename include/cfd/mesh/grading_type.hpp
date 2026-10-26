#pragma once
#include <cstdint>
namespace cfd::mesh
{

/// Describes how cell widths are distributed across a mesh domain.
///
/// This dimension-agnostic strategy type names the spatial pattern of cell
/// sizes. A mesh generator may apply the pattern along one or more coordinate
/// directions; the enum itself does not prescribe a grading ratio or a
/// particular coordinate transformation.
enum class GradingType : std::uint8_t
{
    /// Use equal-width cells throughout the domain.
    ///
    /// The cell width is constant from one boundary to the other, giving
    /// uniform spatial resolution with no preferential clustering.
    Uniform,

    /// Cluster cells at the domain start and expand toward the end.
    ///
    /// Cell widths increase monotonically from the start boundary toward the
    /// end boundary. This provides finer resolution near the start, for
    /// example near an inlet or a wall where steep solution gradients are
    /// expected, while using progressively coarser cells downstream.
    Forward,

    /// Cluster cells at the domain end and expand toward the start.
    ///
    /// Cell widths decrease monotonically from the start boundary toward the
    /// end boundary. This provides progressively finer resolution approaching
    /// the end, for example near an outlet or a wall where steep solution
    /// gradients are expected.
    Backward,

    /// Cluster cells symmetrically at both domain boundaries.
    ///
    /// Cell widths are smallest near the start and end boundaries and grow
    /// toward a maximum near the domain center. The mirrored distribution is
    /// commonly used for wall-bounded flows, such as channel flow, when strong
    /// gradients near both walls require fine near-wall resolution.
    BothEnds,

    /// Cluster cells symmetrically around the domain center.
    ///
    /// Cell widths are smallest near the center and grow toward both domain
    /// boundaries, where they are largest. This mirrored distribution is
    /// useful for shear layers and mixing layers when the strongest gradients
    /// are expected in the interior and the outer regions can use coarser
    /// resolution.
    Center
};

}  // namespace cfd::mesh
