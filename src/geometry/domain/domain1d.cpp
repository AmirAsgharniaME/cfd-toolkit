#include "cfd/geometry/domain/domain1d.hpp"

#include <cmath>
#include <iomanip>
#include <limits>
#include <locale>
#include <sstream>
#include <stdexcept>
#include <string>

namespace cfd::geometry {

namespace {

// Preserve distinguishable double values in validation diagnostics.
std::string format_coordinate(double value) {
    std::ostringstream output;
    output.imbue(std::locale::classic());
    output << std::setprecision(std::numeric_limits<double>::max_digits10) << value;
    return output.str();
}

// Match the finite, increasing bounds and finite-extent invariants in 2D/3D.
inline void validate_bounds(double start, double end) {
    if (!std::isfinite(start) || !std::isfinite(end)) {
        throw std::invalid_argument(
            "Domain1D bounds must be finite: start=" + format_coordinate(start) +
            ", end=" + format_coordinate(end));
    }
    if (end <= start) {
        throw std::invalid_argument(
            "Domain1D bounds must be strictly increasing: start=" + format_coordinate(start) +
            ", end=" + format_coordinate(end));
    }
    if (!std::isfinite(end - start)) {
        throw std::invalid_argument(
            "Domain1D interval length must be finite: start=" + format_coordinate(start) +
            ", end=" + format_coordinate(end));
    }
}

inline void validate_length(double length) {
    if (!std::isfinite(length) || length <= 0.0) {
        throw std::invalid_argument(
            "Domain1D length must be finite and > 0: length=" + format_coordinate(length));
    }
}

} // namespace

Domain1D::Domain1D(double length)
    : Domain1D(0.0, length) {}

Domain1D::Domain1D(double start, double end)
    : start_(start), end_(end) {
    validate_bounds(start_, end_);
}

Domain1D Domain1D::from_start_and_length(double start, double length) {
    validate_length(length);
    // Store the rounded endpoint; validate overflow/collapse and derive length
    // from those authoritative bounds rather than caching the requested length.
    return Domain1D(start, start + length);
}

void Domain1D::set_bounds(double start, double end) {
    validate_bounds(start, end);
    start_ = start;
    end_ = end;
}

void Domain1D::set_start(double start) { set_bounds(start, end_); }
void Domain1D::set_end(double end) { set_bounds(start_, end); }

void Domain1D::set_length(double length) {
    validate_length(length);
    // set_bounds validates the rounded endpoint before committing either bound.
    set_bounds(start_, start_ + length);
}

} // namespace cfd::geometry
