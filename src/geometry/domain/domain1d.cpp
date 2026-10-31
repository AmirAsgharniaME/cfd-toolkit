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
std::string Format_Coordinate(double coord_value) {
    std::ostringstream output;
    output.imbue(std::locale::classic());
    output << std::setprecision(std::numeric_limits<double>::max_digits10) << coord_value;
    return output.str();
}

// Match the finite, increasing bounds and finite-extent invariants in 2D/3D.
inline void Validate_Bounds(double start_coord, double end_coord) {
    if (!std::isfinite(start_coord) || !std::isfinite(end_coord)) {
        throw std::invalid_argument(
            "Domain1D bounds must be finite: start=" + Format_Coordinate(start_coord) +
            ", end=" + Format_Coordinate(end_coord));
    }
    if (end_coord <= start_coord) {
        throw std::invalid_argument(
            "Domain1D bounds must be strictly increasing: start=" + Format_Coordinate(start_coord) +
            ", end=" + Format_Coordinate(end_coord));
    }
    if (!std::isfinite(end_coord - start_coord)) {
        throw std::invalid_argument(
            "Domain1D interval length must be finite: start=" + Format_Coordinate(start_coord) +
            ", end=" + Format_Coordinate(end_coord));
    }
}

inline void Validate_Length(double domain_length) {
    if (!std::isfinite(domain_length) || domain_length <= 0.0) {
        throw std::invalid_argument(
            "Domain1D length must be finite and > 0: length=" + Format_Coordinate(domain_length));
    }
}

} // namespace

Domain1D::Domain1D(double domain_length)
    : Domain1D(0.0, domain_length) {}

Domain1D::Domain1D(double start_coord, double end_coord)
    : start_coord_(start_coord), end_coord_(end_coord) {
    Validate_Bounds(start_coord_, end_coord_);
}

Domain1D Domain1D::from_start_and_length(double start_coord, double domain_length) {
    Validate_Length(domain_length);
    // Store the rounded endpoint; validate overflow/collapse and derive length
    // from those authoritative bounds rather than caching the requested length.
    return Domain1D(start_coord, start_coord + domain_length);
}

void Domain1D::Set_Bounds(double start_coord, double end_coord) {
    Validate_Bounds(start_coord, end_coord);
    start_coord_ = start_coord;
    end_coord_ = end_coord;
}

void Domain1D::Set_Start_Coord(double start_coord) { Set_Bounds(start_coord, end_coord_); }
void Domain1D::set_End_Coord(double end_coord) { Set_Bounds(start_coord_, end_coord); }

void Domain1D::Set_Length(double domain_length) {
    Validate_Length(domain_length);
    // Set_Bounds validates the rounded endpoint before committing either bound.
    Set_Bounds(start_coord_, start_coord_ + domain_length);
}

} // namespace cfd::geometry
