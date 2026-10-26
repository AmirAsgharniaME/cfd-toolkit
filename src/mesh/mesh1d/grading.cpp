#include "cfd/mesh/mesh1d/grading.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <stdexcept>

namespace cfd::mesh::grading
{

std::vector<double> geometric(const geometry::Domain1D& domain,
                              const int n_cells,
                              const double ratio)
{
    if (n_cells < 2) {
        throw std::invalid_argument("geometric grading requires at least two cells");
    }
    if (!std::isfinite(ratio) || ratio <= 0.0) {
        throw std::invalid_argument("geometric grading ratio must be positive and finite");
    }

    const double start = domain.start();
    const double end = domain.end();
    const double length = end - start;
    if (!std::isfinite(start) || !std::isfinite(end) || !std::isfinite(length) ||
        length <= 0.0) {
        throw std::invalid_argument("geometric grading requires a finite positive domain");
    }

    const auto cell_count = static_cast<std::size_t>(n_cells);
    std::vector<double> faces(cell_count + 1U);
    faces.front() = start;
    faces.back() = end;

    if (ratio == 1.0) {
        for (std::size_t i = 1; i < cell_count; ++i) {
            const long double fraction = static_cast<long double>(i) /
                                         static_cast<long double>(cell_count);
            faces[i] = static_cast<double>(static_cast<long double>(start) +
                                           static_cast<long double>(length) * fraction);
        }
    } else {
        // Normalize the geometric terms in log space to avoid overflow in
        // ratio^n and cancellation when ratio is close to one.
        const long double log_ratio = std::log(static_cast<long double>(ratio));
        const long double largest_exponent =
            std::max(0.0L, static_cast<long double>(cell_count - 1U) * log_ratio);
        long double series_sum = 0.0L;
        for (std::size_t i = 0; i < cell_count; ++i) {
            const long double exponent = static_cast<long double>(i) * log_ratio;
            series_sum += std::exp(exponent - largest_exponent);
        }

        long double cumulative = 0.0L;
        for (std::size_t i = 1; i < cell_count; ++i) {
            const long double exponent = static_cast<long double>(i - 1U) * log_ratio;
            cumulative += std::exp(exponent - largest_exponent);
            const long double fraction = cumulative / series_sum;
            faces[i] = static_cast<double>(static_cast<long double>(start) +
                                           static_cast<long double>(length) * fraction);
        }
    }

    for (std::size_t i = 1; i < faces.size(); ++i) {
        if (!std::isfinite(faces[i]) || !(faces[i] > faces[i - 1U])) {
            throw std::invalid_argument(
                "geometric grading faces are not representable as increasing doubles");
        }
    }

    return faces;
}

}  // namespace cfd::mesh::grading
