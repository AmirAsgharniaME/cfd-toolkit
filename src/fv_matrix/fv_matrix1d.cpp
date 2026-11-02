#include "cfd/fv_matrix/fv_matrix1d.hpp"

#include <stdexcept>

namespace cfd::fv_matrix {

FvMatrix1D::FvMatrix1D(const mesh::FvMesh1D& mesh)
    : mesh_(mesh),
      matrix_(mesh.N_Cells()) {
    if (mesh.N_Cells() == 0) {
        throw std::invalid_argument("FvMatrix1D requires a mesh with at least one cell");
    }
}

const mesh::FvMesh1D& FvMatrix1D::Mesh() const noexcept {
    return mesh_;
}

std::size_t FvMatrix1D::Size() const noexcept {
    return matrix_.size();
}

linear_algebra::TridiagonalMatrix& FvMatrix1D::Matrix() noexcept {
    return matrix_;
}

const linear_algebra::TridiagonalMatrix& FvMatrix1D::Matrix() const noexcept {
    return matrix_;
}

double FvMatrix1D::a_W(std::size_t p) const {
    return matrix_.lower(p);
}

double FvMatrix1D::a_P(std::size_t p) const {
    return matrix_.diag(p);
}

double FvMatrix1D::a_E(std::size_t p) const {
    return matrix_.upper(p);
}

double FvMatrix1D::b(std::size_t p) const {
    return matrix_.source(p);
}

void FvMatrix1D::set_a_W(std::size_t p, double value) {
    matrix_.set_lower(p, value);
}

void FvMatrix1D::set_a_P(std::size_t p, double value) {
    matrix_.set_diag(p, value);
}

void FvMatrix1D::set_a_E(std::size_t p, double value) {
    matrix_.set_upper(p, value);
}

void FvMatrix1D::set_b(std::size_t p, double value) {
    matrix_.set_source(p, value);
}

void FvMatrix1D::add_a_W(std::size_t p, double value) {
    matrix_.add_lower(p, value);
}

void FvMatrix1D::add_a_P(std::size_t p, double value) {
    matrix_.add_diag(p, value);
}

void FvMatrix1D::add_a_E(std::size_t p, double value) {
    matrix_.add_upper(p, value);
}

void FvMatrix1D::add_b(std::size_t p, double value) {
    matrix_.add_source(p, value);
}

void FvMatrix1D::Reset() noexcept {
    matrix_.reset();
}

} // namespace cfd::fv_matrix
