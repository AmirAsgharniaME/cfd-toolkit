#pragma once

#include "cfd/linear_algebra/matrix/tridiagonal_matrix.hpp"

#include <vector>

namespace cfd::linear_algebra {

class TDMASolver {
public:
    static void solve(TridiagonalMatrix& matrix, std::vector<double>& solution);

    [[nodiscard]] static std::vector<double> solve(TridiagonalMatrix matrix);
};

} // namespace cfd::linear_algebra
