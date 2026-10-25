#pragma once

#include "cfd/geometry/domain/domain1d.hpp"

namespace cfd::geometry {

/// Axis-aligned two-dimensional rectangular domain.
///
/// The domain is composed of independent, finite, strictly increasing
/// intervals along the X and Y axes.
/// Each interval retains Domain1D's finite-extent and endpoint contracts.
/// Construction and setters copy axis values; getters return independent
/// snapshots. Later mutations do not change previously obtained snapshots.
class Domain2D {
public:
    /// Construct a 2D domain from its X- and Y-axis domains.
    Domain2D(Domain1D x_domain, Domain1D y_domain) noexcept;

    /// Return a snapshot of the domain along the X axis.
    [[nodiscard]] Domain1D x_domain() const noexcept {
        return x_domain_;
    }

    /// Return a snapshot of the domain along the Y axis.
    [[nodiscard]] Domain1D y_domain() const noexcept {
        return y_domain_;
    }

    /// Return true if (x, y) lies within the closed rectangular domain.
    [[nodiscard]] bool contains(double x, double y) const noexcept {
        return x_domain_.contains(x) && y_domain_.contains(y);
    }

    /// Replace both axes after argument construction succeeds.
    /// Invalid Domain1D arguments throw before entry and leave this unchanged.
    void set_domains(Domain1D x_domain, Domain1D y_domain) noexcept;

    /// Replace the X-axis domain.
    void set_x_domain(Domain1D x_domain) noexcept;

    /// Replace the Y-axis domain.
    void set_y_domain(Domain1D y_domain) noexcept;

    // Exact endpoint equality, without a geometric tolerance.
    friend bool operator==(const Domain2D& a, const Domain2D& b) noexcept {
        return a.x_domain_ == b.x_domain_ &&
               a.y_domain_ == b.y_domain_;
    }

    friend bool operator!=(const Domain2D& a, const Domain2D& b) noexcept {
        return !(a == b);
    }

private:
    Domain1D x_domain_;
    Domain1D y_domain_;
};

} // namespace cfd::geometry
