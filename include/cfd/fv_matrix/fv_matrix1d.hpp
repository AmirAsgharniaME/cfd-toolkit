#pragma once

#include "cfd/linear_algebra/matrix/tridiagonal_matrix.hpp"
#include "cfd/mesh/mesh1d/fv_mesh1d.hpp"

#include <cstddef>

namespace cfd::fv_matrix {

/// Couples a 1D finite-volume mesh to its cell-centered tridiagonal system.
///
/// Coefficients are indexed by cell row p. The west coefficient is the lower
/// diagonal, the central coefficient is the main diagonal, and the east
/// coefficient is the upper diagonal. The source is the right-hand side.
/// The mesh must outlive this object and must not be moved while this object
/// refers to it.
class FvMatrix1D {
public:
    explicit FvMatrix1D(const mesh::FvMesh1D& mesh);

    FvMatrix1D(const FvMatrix1D&) = default;
    FvMatrix1D(FvMatrix1D&&) = default;
    FvMatrix1D& operator=(const FvMatrix1D&) = delete;
    FvMatrix1D& operator=(FvMatrix1D&&) = delete;
    ~FvMatrix1D() = default;

    [[nodiscard]] const mesh::FvMesh1D& Mesh() const noexcept;
    [[nodiscard]] std::size_t Size() const noexcept;

    /// Access the algebraic system for assembly and linear-solver integration.
    [[nodiscard]] linear_algebra::TridiagonalMatrix& Matrix() noexcept;
    [[nodiscard]] const linear_algebra::TridiagonalMatrix& Matrix() const noexcept;

    [[nodiscard]] double a_W(std::size_t p) const;
    [[nodiscard]] double a_P(std::size_t p) const;
    [[nodiscard]] double a_E(std::size_t p) const;
    [[nodiscard]] double b(std::size_t p) const;

    void set_a_W(std::size_t p, double value);
    void set_a_P(std::size_t p, double value);
    void set_a_E(std::size_t p, double value);
    void set_b(std::size_t p, double value);

    void add_a_W(std::size_t p, double value);
    void add_a_P(std::size_t p, double value);
    void add_a_E(std::size_t p, double value);
    void add_b(std::size_t p, double value);

    /// Zero all matrix coefficients and source entries without changing size.
    void Reset() noexcept;

private:
    const mesh::FvMesh1D& mesh_;
    linear_algebra::TridiagonalMatrix matrix_;
};

} // namespace cfd::fv_matrix
