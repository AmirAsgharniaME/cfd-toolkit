#include <iostream>

#include <cfd/geometry/domain/domain2d.hpp>
#include <cfd/mesh/mesh2d/uniform_mesh2d.hpp>

int main()
{
    const cfd::geometry::Domain2D domain(0.0, 1.0, 0.0, 1.0);
    const cfd::mesh::UniformMesh2D mesh(domain, 11, 11);

    mesh.print_details(std::cout);
    std::cin.get();
    return 0;
}
