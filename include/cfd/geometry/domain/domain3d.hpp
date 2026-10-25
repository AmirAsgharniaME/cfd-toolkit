#pragma once

#include "cfd/geometry/domain/domain1d.hpp"

namespace cfd::geometry {

/// Axis-aligned three-dimensional cuboid domain.
///
/// The domain is composed of independent, finite, strictly increasing
/// intervals along the X, Y, and Z axes.
/// Each interval retains Domain1D's finite-extent and endpoint contracts.
/// Construction and setters copy axis values; getters return independent
/// snapshots. Later mutations do not change previously obtained snapshots.
class Domain3D {
public:
    /// Construct a 3D domain from its X-, Y-, and Z-axis domains.
    Domain3D(Domain1D x_domain, Domain1D y_domain, Domain1D z_domain) noexcept;

    /// Return independent snapshots of the axis domains.
    [[nodiscard]] Domain1D x_domain() const noexcept { return x_domain_; }
    [[nodiscard]] Domain1D y_domain() const noexcept { return y_domain_; }
    [[nodiscard]] Domain1D z_domain() const noexcept { return z_domain_; }

    /// Return true if (x, y, z) lies within the closed cuboid domain.
    [[nodiscard]] bool contains(double x, double y, double z) const noexcept {
        return x_domain_.contains(x) &&
               y_domain_.contains(y) &&
               z_domain_.contains(z);
    }

    /// Replace all axes after argument construction succeeds.
    /// Invalid Domain1D arguments throw before entry and leave this unchanged.
    void set_domains(Domain1D x_domain, Domain1D y_domain, Domain1D z_domain) noexcept;

    void set_x_domain(Domain1D x_domain) noexcept;
    void set_y_domain(Domain1D y_domain) noexcept;
    void set_z_domain(Domain1D z_domain) noexcept;

    // Exact endpoint equality, without a geometric tolerance.
    friend bool operator==(const Domain3D& a, const Domain3D& b) noexcept {
        return a.x_domain_ == b.x_domain_ &&
               a.y_domain_ == b.y_domain_ &&
               a.z_domain_ == b.z_domain_;
    }

    friend bool operator!=(const Domain3D& a, const Domain3D& b) noexcept {
        return !(a == b);
    }

private:
    Domain1D x_domain_;
    Domain1D y_domain_;
    Domain1D z_domain_;
};

} // namespace cfd::geometry
