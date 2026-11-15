#pragma once

#include "cfd/boundary/types.hpp"
#include "cfd/field/field1d/field1d.hpp"

#include <cstddef>
#include <string>
#include <string_view>

namespace cfd::mesh
{
class FvMesh1D;
}  // namespace cfd::mesh

namespace cfd::fv_matrix
{
class FvMatrix1D;
}  // namespace cfd::fv_matrix

namespace cfd::boundary
{

struct BoundaryAssemblyContext1D
{
    const mesh::FvMesh1D& mesh;
    std::size_t face_id;
    std::size_t owner;
    double gamma_owner;
    double face_area;
    double owner_face_distance;
};

class BoundaryCondition1D
{
public:
    virtual ~BoundaryCondition1D() = default;

    [[nodiscard]] const std::string& Name() const noexcept;
    [[nodiscard]] BoundarySide1D Side() const noexcept;

    virtual void Apply(const field::VolScalarField1D& gamma,
                       fv_matrix::FvMatrix1D& matrix) const = 0;

protected:
    BoundaryCondition1D(std::string name, BoundarySide1D side);

    [[nodiscard]] BoundaryAssemblyContext1D Prepare_Assembly(
        const field::VolScalarField1D& gamma,
        const fv_matrix::FvMatrix1D& matrix,
        std::string_view condition_name) const;

private:
    std::string name_;
    BoundarySide1D side_;
};

}  // namespace cfd::boundary
