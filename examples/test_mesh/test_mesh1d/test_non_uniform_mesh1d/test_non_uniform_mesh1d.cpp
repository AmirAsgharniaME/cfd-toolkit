#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>

#include <cfd/geometry/domain/domain1d.hpp>
#include <cfd/mesh/mesh1d/non_uniform_fv_mesh1d.hpp>

namespace
{

void require(bool condition, const char* message)
{
    if (!condition)
        throw std::runtime_error(message);
}

template<class Exception, class Operation>
void require_exception(Operation operation)
{
    try
    {
        operation();
    }
    catch (const Exception&)
    {
        return;
    }
    throw std::runtime_error("Expected non-uniform mesh operation exception");
}

void check_contracts()
{
    using cfd::geometry::Domain1D;
    using cfd::mesh::NonUniformFvMesh1D;

    Domain1D domain(-2.0, 4.0);
    std::vector<double> faces{-2.0, -1.5, 0.5, 4.0};
    const NonUniformFvMesh1D mesh(domain, faces);
    require(mesh.start() == -2.0 && mesh.end() == 4.0 && mesh.length() == 6.0,
            "Mesh bounds do not match the domain snapshot");
    require(mesh.num_cells() == 3 && mesh.num_faces() == 4,
            "Incorrect non-uniform mesh counts");

    const double expected_faces[]{-2.0, -1.5, 0.5, 4.0};
    const double expected_centers[]{-1.75, -0.5, 2.25};
    const double expected_widths[]{0.5, 2.0, 3.5};
    for (std::size_t face = 0; face < mesh.num_faces(); ++face)
        require(mesh.face(face) == expected_faces[face], "Face snapshot differs from input");
    for (std::size_t cell = 0; cell < mesh.num_cells(); ++cell)
    {
        require(mesh.cell_center(cell) == expected_centers[cell], "Incorrect cell center");
        require(mesh.cell_width(cell) == expected_widths[cell], "Incorrect cell width");
        require(mesh.cell_volume(cell) == expected_widths[cell], "Incorrect unit-area cell volume");
    }

    faces[1] = -1.0;
    domain.set_bounds(10.0, 20.0);
    require(mesh.start() == -2.0 && mesh.end() == 4.0 &&
            mesh.face(1) == -1.5 && mesh.cell_center(1) == -0.5,
            "Input mutation changed the mesh snapshot");

    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double inf = std::numeric_limits<double>::infinity();
    require_exception<std::invalid_argument>([&] { (void)NonUniformFvMesh1D(domain, {}); });
    require_exception<std::invalid_argument>([&] { (void)NonUniformFvMesh1D(domain, {10.0}); });
    require_exception<std::invalid_argument>([&] { (void)NonUniformFvMesh1D(domain, {10.0, 20.0, 19.0}); });
    require_exception<std::invalid_argument>([&] { (void)NonUniformFvMesh1D(domain, {10.0, 10.0, 20.0}); });
    require_exception<std::invalid_argument>([&] { (void)NonUniformFvMesh1D(domain, {9.0, 15.0, 20.0}); });
    require_exception<std::invalid_argument>([&] { (void)NonUniformFvMesh1D(domain, {10.0, 15.0, 21.0}); });
    require_exception<std::invalid_argument>([&] { (void)NonUniformFvMesh1D(domain, {10.0, nan, 20.0}); });
    require_exception<std::invalid_argument>([&] { (void)NonUniformFvMesh1D(domain, {10.0, inf}); });
    require_exception<std::invalid_argument>([&] { (void)NonUniformFvMesh1D(domain, {-inf, 20.0}); });

    const Domain1D adjacent_bounds(1.0, std::nextafter(1.0, inf));
    require_exception<std::invalid_argument>([&] {
        (void)NonUniformFvMesh1D(adjacent_bounds, {1.0, std::nextafter(adjacent_bounds.end(), inf)});
    });
    std::cout << "NonUniformFvMesh1D contract checks passed\n";
}

}  // namespace

int main()
{
    try
    {
        check_contracts();
    }
    catch (const std::exception& error)
    {
        std::cerr << "Non-uniform mesh contract check failed: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
