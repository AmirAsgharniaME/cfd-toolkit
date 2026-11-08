#include "cfd/fv_matrix/fv_matrix1d.hpp"

#include <stdexcept>

namespace cfd::fv_matrix {

FvMatrix1D::FvMatrix1D(const cfd::mesh::FvMesh1D& mesh)
    : mesh_(mesh),
      matrix_(mesh.N_Cells())
{
    if (mesh.N_Cells() == 0) {
        throw std::invalid_argument("FvMatrix1D requires a mesh with at least one cell");
    }
}

const cfd::mesh::FvMesh1D& FvMatrix1D::Mesh() const noexcept
{
    return mesh_;
}

std::size_t FvMatrix1D::size() const noexcept
{
    return matrix_.size();
}

linear_algebra::TridiagonalMatrix& FvMatrix1D::Matrix() noexcept
{
    return matrix_;
}

const linear_algebra::TridiagonalMatrix& FvMatrix1D::Matrix() const noexcept
{
    return matrix_;
}

double FvMatrix1D::a_W(std::size_t p) const
{
    return matrix_.Lower(p);
}

double FvMatrix1D::a_P(std::size_t p) const
{
    return matrix_.Diag(p);
}

double FvMatrix1D::a_E(std::size_t p) const
{
    return matrix_.Upper(p);
}

double FvMatrix1D::b(std::size_t p) const
{
    return matrix_.Source(p);
}

void FvMatrix1D::Set_a_W(std::size_t p, double value)
{
    matrix_.Set_Lower(p, value);
}

void FvMatrix1D::Set_a_P(std::size_t p, double value)
{
    matrix_.Set_Diag(p, value);
}

void FvMatrix1D::Set_a_E(std::size_t p, double value)
{
    matrix_.Set_Upper(p, value);
}

void FvMatrix1D::Set_b(std::size_t p, double value)
{
    matrix_.Set_Source(p, value);
}

void FvMatrix1D::Add_a_W(std::size_t p, double value)
{
    matrix_.Add_Lower(p, value);
}

void FvMatrix1D::Add_a_P(std::size_t p, double value)
{
    matrix_.Add_Diag(p, value);
}

void FvMatrix1D::Add_a_E(std::size_t p, double value)
{
    matrix_.Add_Upper(p, value);
}

void FvMatrix1D::Add_b(std::size_t p, double value)
{
    matrix_.Add_Source(p, value);
}

void FvMatrix1D::Reset() noexcept
{
    matrix_.Reset();
}

} // namespace cfd::fv_matrix
