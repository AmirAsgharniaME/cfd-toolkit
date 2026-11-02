#include "cfd/linear_algebra/matrix/tridiagonal_matrix.hpp"

#include <algorithm>
#include <stdexcept>

namespace cfd::linear_algebra {

namespace
{

void check_row(std::size_t row, std::size_t size) {
    if (row >= size) {
        throw std::out_of_range("TridiagonalMatrix row index out of range");
    }
}

} // namespace

TridiagonalMatrix::TridiagonalMatrix(std::size_t size)
    : diag_(size, 0.0),
      lower_(size, 0.0),
      upper_(size, 0.0),
      source_(size, 0.0) {}

std::size_t TridiagonalMatrix::size() const noexcept
{
    return diag_.size();
}

double TridiagonalMatrix::diag(std::size_t row) const
{
    check_row(row, size());
    return diag_[row];
}

void TridiagonalMatrix::set_diag(std::size_t row, double value)
{
    check_row(row, size());
    diag_[row] = value;
}

void TridiagonalMatrix::add_diag(std::size_t row, double value)
{
    check_row(row, size());
    diag_[row] += value;
}

double TridiagonalMatrix::lower(std::size_t row) const
{
    check_row(row, size());
    return lower_[row];
}

void TridiagonalMatrix::set_lower(std::size_t row, double value)
{
    check_row(row, size());
    if (row == 0) {
        return;
    }
    lower_[row] = value;
}

void TridiagonalMatrix::add_lower(std::size_t row, double value)
{
    check_row(row, size());
    if (row == 0) {
        return;
    }
    lower_[row] += value;
}

double TridiagonalMatrix::upper(std::size_t row) const
{
    check_row(row, size());
    return upper_[row];
}

void TridiagonalMatrix::set_upper(std::size_t row, double value)
{
    check_row(row, size());
    if (row + 1 == size()) {
        return;
    }
    upper_[row] = value;
}

void TridiagonalMatrix::add_upper(std::size_t row, double value)
{
    check_row(row, size());
    if (row + 1 == size()) {
        return;
    }
    upper_[row] += value;
}

double TridiagonalMatrix::source(std::size_t row) const
{
    check_row(row, size());
    return source_[row];
}

void TridiagonalMatrix::set_source(std::size_t row, double value)
{
    check_row(row, size());
    source_[row] = value;
}

void TridiagonalMatrix::add_source(std::size_t row, double value)
{
    check_row(row, size());
    source_[row] += value;
}

std::vector<double>& TridiagonalMatrix::diag() noexcept
{
    return diag_;
}

const std::vector<double>& TridiagonalMatrix::diag() const noexcept
{
    return diag_;
}

std::vector<double>& TridiagonalMatrix::lower() noexcept
{
    return lower_;
}

const std::vector<double>& TridiagonalMatrix::lower() const noexcept
{
    return lower_;
}

std::vector<double>& TridiagonalMatrix::upper() noexcept
{
    return upper_;
}

const std::vector<double>& TridiagonalMatrix::upper() const noexcept
{
    return upper_;
}

std::vector<double>& TridiagonalMatrix::source() noexcept
{
    return source_;
}

const std::vector<double>& TridiagonalMatrix::source() const noexcept
{
    return source_;
}

void TridiagonalMatrix::reset() noexcept
{
    std::fill(diag_.begin(), diag_.end(), 0.0);
    std::fill(lower_.begin(), lower_.end(), 0.0);
    std::fill(upper_.begin(), upper_.end(), 0.0);
    std::fill(source_.begin(), source_.end(), 0.0);
}

} // namespace cfd::linear_algebra
