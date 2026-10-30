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
                                             const long double log_r)
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
        const long double log_weight = exponent * log_r;
        if (!std::isfinite(log_weight)) {
            throw std::invalid_argument("1D grading ratio produces non-finite cell weights");
        }
        largest_log_weight = std::max(largest_log_weight, log_weight);
    }

    long double weight_sum = 0.0L;
    for (const long double exponent : exponents) {
        weight_sum += std::exp(exponent * log_r - largest_log_weight);
    }
    if (!std::isfinite(weight_sum) || weight_sum <= 0.0L) {
        throw std::invalid_argument("1D grading could not normalize cell widths");
    }

    long double cumulative_weight = 0.0L;
for (std::size_t f = 1U; f < cell_count; ++f) {
    const std::size_t p = f - 1U;
    const long double exponent = exponents[p];
    cumulative_weight += std::exp(exponent * log_r - largest_log_weight);
    const long double fraction = cumulative_weight / weight_sum;
    const long double coordinate = static_cast<long double>(start) +
                                   static_cast<long double>(length) * fraction;
    faces[f] = static_cast<double>(coordinate);
}


    for (std::size_t f = 1; f < faces.size(); ++f) {
        const std::size_t p = f - 1U;
        if (!std::isfinite(faces[f]) || !(faces[f] > faces[p])) {
            throw std::invalid_argument(
                "1D grading faces are not representable as increasing doubles");
        }
    }

    for (std::size_t f = 0; f < cell_count; ++f) {
        const double width = faces[f + 1U] - faces[f];
        const double center = faces[f] + width * 0.5;
        if (!std::isfinite(center) || !(faces[f] < center) ||
            !(center < faces[f + 1U])) {
            std::ostringstream message;
            message.imbue(std::locale::classic());
            message << std::setprecision(std::numeric_limits<double>::max_digits10)
                    << "1D grading cell center is not representable strictly inside cell at index "
                    << f << " [left=" << faces[f] << ", center=" << center
                    << ", right=" << faces[f + 1U] << ']';
            throw std::invalid_argument(message.str());
        }
    }
    return faces;
}

}  // namespace

double ratio_from_total_ratio(const std::size_t n_cells,
                              const GradingType type,
                              const double R)
{
    if (n_cells < 2U) {
        throw std::invalid_argument("1D grading requires at least two cells");
    }
    if (!std::isfinite(R) || R < 1.0) {
        throw std::invalid_argument(
            "1D total cell-width ratio must be finite and greater than or equal to 1");
    }

    if (type == GradingType::Uniform) {
        if (R != 1.0) {
            throw std::invalid_argument(
                "Uniform grading cannot produce a non-unity total cell-width ratio");
        }
        return 1.0;
    }

    std::size_t max_exponent = 0U;
    switch (type) {
    case GradingType::Forward:
    case GradingType::Backward:
        max_exponent = n_cells - 1U;
        break;
    case GradingType::BothEnds:
    case GradingType::Center:
        // Both patterns peak at floor((N - 1) / 2). For Center this differs
        // from floor(N / 2) when N is even, as confirmed by its two central
        // zero exponents.
        max_exponent = (n_cells - 1U) / 2U;
        break;
    case GradingType::Uniform:
        // Handled above; retain an exhaustive enum switch.
        break;
    default:
        throw std::invalid_argument("1D grading type is not a supported GradingType value");
    }
    if (max_exponent == 0U) {
        throw std::invalid_argument("1D grading pattern requires a positive exponent span");
    }

    const long double log_R =
        std::log(static_cast<long double>(R));
    const long double log_r =
        log_R / static_cast<long double>(max_exponent);
    const long double r = std::exp(log_r);
    if (!std::isfinite(r) || r <= 0.0L ||
        r > static_cast<long double>(std::numeric_limits<double>::max())) {
        throw std::invalid_argument(
            "computed 1D cell-width growth factor is not representable as a finite double");
    }

    const double result = static_cast<double>(r);
    if (!std::isfinite(result) || result <= 0.0 || result < 1.0) {
        throw std::invalid_argument(
            "computed 1D cell-width growth factor is not representable as a finite double");
    }
    return result;
}

std::vector<double> make_1d_faces(const geometry::Domain1D& domain,
                                  const std::size_t n_cells,
                                  const GradingType type,
                                  const double r)
{
    // =========================================================================
    // ENGINEERING NOTE: C++ LITERAL SUFFIXES & PRECISION CHOICE
    // =========================================================================
    // 1. Unsigned Literal ('U'):
    //    Used on integer constants (e.g., 2U) to match 'std::size_t' type.
    //    This prevents strict compiler warnings/errors under -Wsign-compare
    //    and -Wconversion flags.
    //
    // 2. Long Double Literal ('L'):
    //    Used on floating-point constants (e.g., 0.0L) to initialize containers
    //    of type 'long double' directly, avoiding implicit type-casting from 'double'.
    //
    // 3. Why 'long double' here instead of standard 'double'?
    //    Although 99% of CFD uses 'double', geometric grading formulas (like 
    //    geometric progression sums) accumulate round-off errors over many cells.
    //    Using 80-bit 'long double' locally for face/exponent generation prevents 
    //    boundary mismatch errors (e.g., grid not landing precisely on domain ends).
    // =========================================================================
    if (n_cells < 2U) {
        throw std::invalid_argument("1D grading requires at least two cells");
    }
    if (n_cells >= std::vector<double>{}.max_size() ||
        n_cells > std::vector<long double>{}.max_size()) {
        throw std::length_error("1D grading cell count exceeds maximum vector size");
    }
    if (!std::isfinite(r) || r <= 0.0) {
        throw std::invalid_argument("1D grading ratio must be positive and finite");
    }
    if (type != GradingType::Uniform && r < 1.0) {
        throw std::invalid_argument("graded 1D modes require a ratio greater than or equal to 1");
    }

    const std::size_t cell_count = n_cells;
    std::vector<long double> exponents(cell_count, 0.0L);
    switch (type) {
    case GradingType::Uniform:
        break;
    case GradingType::Forward:
        for (std::size_t p = 0; p < cell_count; ++p) {
            exponents[p] = static_cast<long double>(p);
        }
        break;
    case GradingType::Backward:
        for (std::size_t p = 0; p < cell_count; ++p) {
            exponents[p] = static_cast<long double>(cell_count - 1U - p);
        }
        break;
    case GradingType::BothEnds:
        for (std::size_t p = 0; p < cell_count; ++p) {
            exponents[p] = static_cast<long double>(std::min(p, cell_count - 1U - p));
        }
        break;
    case GradingType::Center: {
        const std::size_t center_left = (cell_count - 1U) / 2U;
        const std::size_t center_right = cell_count / 2U;
        for (std::size_t p = 0; p < cell_count; ++p) {
            const std::size_t distance = p < center_left ? center_left - p
                                      : p > center_right ? p - center_right
                                                         : 0U;
            exponents[p] = static_cast<long double>(distance);
        }
        break;
    }
    default:
        throw std::invalid_argument("1D grading type is not a supported GradingType value");
    }

    const long double log_r = std::log(static_cast<long double>(r));
    return make_faces(domain, cell_count, exponents, log_r);
}

}  // namespace cfd::mesh::grading


/* =========================================================================
 * C++ LITERAL SUFFIXES QUICK REFERENCE GUIDE
 * =========================================================================
 * Code Notation    | Data Type          | Suffix Name          | Main Use Case
 * -------------------------------------------------------------------------
 * 2                | int                | (Default)            | Simple counters & standard integers
 * 2U / 2u          | unsigned int       | Unsigned             | Comparing with vector size/indices w/o warnings
 * 2L / 2l          | long int           | Long                 | Very large integers (64-bit)
 * 2ULL             | unsigned long long | Unsigned Long Long   | Maximum possible unsigned integer
 * 2.0              | double             | (Default)            | 99% of floating-point math & CFD
 * 2.0f / 2.0F      | float              | Float                | Graphics & CUDA programming (32-bit precision)
 * 2.0L             | long double        | Long Double          | Extended precision (80/128-bit) for sensitive math
 * ========================================================================= */

/*
 * ====================================================================================
 *  APEX-FVM INTEGERS REFERENCE GUIDE (Integer Types, Modifiers & Size Aliases)
 * ====================================================================================
 *  Format: [Type / Modifier] -> Bytes | Approximate Range | CFD Use Case
 * ------------------------------------------------------------------------------------
 *  1. short               -> 2 Bytes | -32,768 to 32,767                 | Tiny local counters
 *  2. unsigned short      -> 2 Bytes | 0 to 65,535                       | Small positive IDs
 *  3. int                 -> 4 Bytes | -2B to +2B (~ -2.1e9 to +2.1e9)     | Standard loops, general vars
 *  4. unsigned int        -> 4 Bytes | 0 to 4.2B (~ 4.29e9)              | Standard cell/face IDs (> 0)
 *  5. long                -> 8 Bytes | -9E18 to +9E18 (on Linux 64-bit)  | Large scale counters
 *  6. unsigned long       -> 8 Bytes | 0 to 1.8E19 (on Linux 64-bit)     | Massive mesh indices
 *  7. long long           -> 8 Bytes | -9E18 to +9E18 (Guaranteed)       | Astronomical / Global IDs
 *  8. unsigned long long  -> 8 Bytes | 0 to 1.8E19 (Guaranteed)          | Ultimate non-negative scale
 * ------------------------------------------------------------------------------------
 *  SPECIAL SYSTEM ALIAS:
 *  9. std::size_t         -> 8 Bytes (on Linux 64-bit) | 0 to 1.8E19     | Container sizes & Loop indices (.size())
 * ====================================================================================
 */
