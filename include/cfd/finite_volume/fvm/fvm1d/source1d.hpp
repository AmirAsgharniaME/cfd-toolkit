#pragma once

namespace cfd::fv_matrix
{
class FvMatrix1D;
}

namespace cfd::fvm
{

/// \brief Assemble a uniform volumetric source term.
///
/// The integrated source over control volume P is
///
///     integral over V_P of S dV
///
/// which, for a constant source density S that does not depend on the
/// unknown, reduces to
///
///     S * V_P
///
/// and is added to the right-hand side as
///
///     b_P += S * V_P
///
/// for every cell P. The source is purely explicit: diagonal and
/// off-diagonal coefficients are left unchanged.
class Source1D
{
public:
    /// \brief Constructs a constant volumetric source.
    /// \param value Source density per unit volume. Must be finite.
    explicit Source1D(double value) noexcept;

    /// \brief Returns the stored volumetric source density.
    [[nodiscard]] double Value() const noexcept;

    /// \brief Accumulates S * V_P into the source vector of every cell.
    ///
    /// The mesh is taken from the matrix, matching the diffusion operator.
    /// All cell volumes and contributions are validated before the matrix is
    /// modified, so a rejected call leaves the system unchanged.
    ///
    /// \param matrix System whose source vector receives the contribution.
    /// \throws std::invalid_argument If the source is not finite, the matrix
    /// size does not match the mesh, or a cell volume is not finite and
    /// positive.
    /// \throws std::overflow_error If a contribution or an accumulated source
    /// value is not representable as a finite double.
    void Apply(fv_matrix::FvMatrix1D& matrix) const;

private:
    double value_;
};

} // namespace cfd::fvm
