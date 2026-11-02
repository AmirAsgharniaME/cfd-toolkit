#include "cfd/linear_algebra/solvers/tdma_solver.hpp"

#include <cmath>
#include <stdexcept>

namespace cfd::linear_algebra {

namespace {

constexpr double pivot_tolerance = 1.0e-14;

void check_pivot(const std::vector<double>& diag, std::size_t row) {
    if (std::fabs(diag[row]) < pivot_tolerance) {
        throw std::runtime_error("TDMASolver encountered a near-zero pivot");
    }
}

void forward_elimination(
    std::vector<double>& diag,
    std::vector<double>& lower,
    const std::vector<double>& upper,
    std::vector<double>& source) {
    for (std::size_t row = 1; row < diag.size(); ++row) {
        check_pivot(diag, row - 1);
        const double factor = lower[row] / diag[row - 1];
        diag[row] -= factor * upper[row - 1];
        source[row] -= factor * source[row - 1];
        lower[row] = 0.0;
    }
}

void back_substitution(
    const std::vector<double>& diag,
    const std::vector<double>& upper,
    const std::vector<double>& source,
    std::vector<double>& solution) {
    const std::size_t last = diag.size() - 1;
    check_pivot(diag, last);
    solution[last] = source[last] / diag[last];

    for (std::size_t j = last; j-- > 0;) {
        check_pivot(diag, j);
        solution[j] = (source[j] - upper[j] * solution[j + 1]) / diag[j];
    }
}

} // namespace

void TDMASolver::solve(TridiagonalMatrix& matrix, std::vector<double>& solution) {
    const std::size_t n = matrix.size();
    if (n == 0) {
        throw std::invalid_argument("TDMASolver requires a non-empty matrix");
    }

    auto& diag = matrix.diag();
    auto& lower = matrix.lower();
    auto& upper = matrix.upper();
    auto& source = matrix.source();
    if (diag.size() != n || lower.size() != n || upper.size() != n || source.size() != n) {
        throw std::invalid_argument("TridiagonalMatrix coefficient vectors must have equal sizes");
    }

    solution.resize(n);
    forward_elimination(diag, lower, upper, source);
    back_substitution(diag, upper, source, solution);
}

std::vector<double> TDMASolver::solve(TridiagonalMatrix matrix) {
    std::vector<double> solution;
    solve(matrix, solution);
    return solution;
}

} // namespace cfd::linear_algebra
