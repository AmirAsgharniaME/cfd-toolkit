#include "cfd/mesh/mesh1d/grading1d.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <iomanip>
#include <limits>
#include <locale>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace cfd::mesh::grading
{
namespace
{

[[nodiscard]] std::vector<double> make_faces(const geometry::Domain1D& domain,
                                             const std::size_t cell_count,
                                             const std::vector<long double>& exponents,
                                             const long double log_ratio)
{
    const double start = domain.start();
    const double end = domain.end();
    const double length = domain.length();
    if (!std::isfinite(start) || !std::isfinite(end) || !std::isfinite(length) ||
        length <= 0.0) {
        throw std::invalid_argument("1D grading requires a finite positive domain");
    }

    std::vector<double> faces(cell_count + 1U);
    faces.front() = start;
    faces.back() = end;

    long double largest_log_weight = 0.0L;
    for (const long double exponent : exponents) {
        const long double log_weight = exponent * log_ratio;
        if (!std::isfinite(log_weight)) {
            throw std::invalid_argument("1D grading ratio produces non-finite cell weights");
        }
        largest_log_weight = std::max(largest_log_weight, log_weight);
    }

    long double weight_sum = 0.0L;
    for (const long double exponent : exponents) {
        weight_sum += std::exp(exponent * log_ratio - largest_log_weight);
    }
    if (!std::isfinite(weight_sum) || weight_sum <= 0.0L) {
        throw std::invalid_argument("1D grading could not normalize cell widths");
    }

    long double cumulative_weight = 0.0L;
    for (std::size_t i = 1; i < cell_count; ++i) {
        const long double exponent = exponents[i - 1U];
        cumulative_weight += std::exp(exponent * log_ratio - largest_log_weight);
        const long double fraction = cumulative_weight / weight_sum;
        const long double coordinate = static_cast<long double>(start) +
                                       static_cast<long double>(length) * fraction;
        faces[i] = static_cast<double>(coordinate);
    }

    for (std::size_t i = 1; i < faces.size(); ++i) {
        if (!std::isfinite(faces[i]) || !(faces[i] > faces[i - 1U])) {
            throw std::invalid_argument(
                "1D grading faces are not representable as increasing doubles");
        }
    }

    for (std::size_t i = 0; i < cell_count; ++i) {
        const double width = faces[i + 1U] - faces[i];
        const double center = faces[i] + width * 0.5;
        if (!std::isfinite(center) || !(faces[i] < center) ||
            !(center < faces[i + 1U])) {
            std::ostringstream message;
            message.imbue(std::locale::classic());
            message << std::setprecision(std::numeric_limits<double>::max_digits10)
                    << "1D grading cell center is not representable strictly inside cell at index "
                    << i << " [left=" << faces[i] << ", center=" << center
                    << ", right=" << faces[i + 1U] << ']';
            throw std::invalid_argument(message.str());
        }
    }
    return faces;
}

}  // namespace

std::vector<double> make_1d_faces(const geometry::Domain1D& domain,
                                      const int n_cells,
                                      const GradingType type,
                                      const double ratio)
{
    if (n_cells < 2) {
        throw std::invalid_argument("1D grading requires at least two cells");
    }
    if (!std::isfinite(ratio) || ratio <= 0.0) {
        throw std::invalid_argument("1D grading ratio must be positive and finite");
    }
    if (type != GradingType::Uniform && ratio < 1.0) {
        throw std::invalid_argument("graded 1D modes require a ratio greater than or equal to 1");
    }

    const auto cell_count = static_cast<std::size_t>(n_cells);
    std::vector<long double> exponents(cell_count, 0.0L);
    switch (type) {
    case GradingType::Uniform:
        break;
    case GradingType::Forward:
        for (std::size_t i = 0; i < cell_count; ++i) {
            exponents[i] = static_cast<long double>(i);
        }
        break;
    case GradingType::Backward:
        for (std::size_t i = 0; i < cell_count; ++i) {
            exponents[i] = static_cast<long double>(cell_count - 1U - i);
        }
        break;
    case GradingType::BothEnds:
        for (std::size_t i = 0; i < cell_count; ++i) {
            exponents[i] = static_cast<long double>(std::min(i, cell_count - 1U - i));
        }
        break;
    case GradingType::Center: {
        const std::size_t center_left = (cell_count - 1U) / 2U;
        const std::size_t center_right = cell_count / 2U;
        for (std::size_t i = 0; i < cell_count; ++i) {
            const std::size_t distance = i < center_left ? center_left - i
                                      : i > center_right ? i - center_right
                                                         : 0U;
            exponents[i] = static_cast<long double>(distance);
        }
        break;
    }
    default:
        throw std::invalid_argument("1D grading type is not a supported GradingType value");
    }

    const long double log_ratio = std::log(static_cast<long double>(ratio));
    return make_faces(domain, cell_count, exponents, log_ratio);
}

}  // namespace cfd::mesh::grading
