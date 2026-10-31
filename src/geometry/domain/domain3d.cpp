#include "cfd/geometry/domain/domain3d.hpp"

namespace cfd::geometry {

Domain3D::Domain3D(Domain1D x_domain, Domain1D y_domain, Domain1D z_domain) noexcept
    : x_domain_(x_domain),
      y_domain_(y_domain),
      z_domain_(z_domain) {}

void Domain3D::Set_Domains(Domain1D x_domain, Domain1D y_domain, Domain1D z_domain) noexcept {
    x_domain_ = x_domain;
    y_domain_ = y_domain;
    z_domain_ = z_domain;
}

void Domain3D::Set_X_Domain(Domain1D x_domain) noexcept { x_domain_ = x_domain; }
void Domain3D::Set_Y_Domain(Domain1D y_domain) noexcept { y_domain_ = y_domain; }
void Domain3D::Set_Z_Domain(Domain1D z_domain) noexcept { z_domain_ = z_domain; }

} // namespace cfd::geometry
