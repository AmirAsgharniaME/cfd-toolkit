#include "cfd/geometry/domain/domain3d.hpp"

namespace cfd::geometry {

Domain3D::Domain3D(Domain1D x_domain, Domain1D y_domain, Domain1D z_domain) noexcept
    : x_domain_(x_domain),
      y_domain_(y_domain),
      z_domain_(z_domain) {}

void Domain3D::set_domains(Domain1D x_domain, Domain1D y_domain, Domain1D z_domain) noexcept {
    x_domain_ = x_domain;
    y_domain_ = y_domain;
    z_domain_ = z_domain;
}

void Domain3D::set_x_domain(Domain1D x_domain) noexcept { x_domain_ = x_domain; }
void Domain3D::set_y_domain(Domain1D y_domain) noexcept { y_domain_ = y_domain; }
void Domain3D::set_z_domain(Domain1D z_domain) noexcept { z_domain_ = z_domain; }

} // namespace cfd::geometry
