#include "cfd/geometry/domain/domain2d.hpp"

#include <utility>

namespace cfd::geometry {

Domain2D::Domain2D(Domain1D x_domain, Domain1D y_domain) noexcept
    : x_domain_(std::move(x_domain)),
      y_domain_(std::move(y_domain)) {}

void Domain2D::Set_Domains(Domain1D x_domain, Domain1D y_domain) noexcept {
    // Domain1D is a small value type. Assign both only after both arguments
    // have already been constructed successfully.
    x_domain_ = std::move(x_domain);
    y_domain_ = std::move(y_domain);
}

void Domain2D::Set_X_Domain(Domain1D x_domain) noexcept {
    x_domain_ = std::move(x_domain);
}

void Domain2D::Set_Y_Domain(Domain1D y_domain) noexcept {
    y_domain_ = std::move(y_domain);
}

} // namespace cfd::geometry
