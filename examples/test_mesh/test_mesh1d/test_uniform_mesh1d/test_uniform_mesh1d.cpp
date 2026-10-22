#include <iostream>

#include <cfd/geometry/domain/domain1d.hpp>
#include <cfd/mesh/mesh1d/uniform_mesh1d.hpp>


int main()
{
    const cfd::geometry::Domain1D domain(0.0, 1.0);
    //const cfd::geometry::Domain1D domain1(1.0);
    const cfd::mesh::UniformMesh1D mesh(domain, 11);

    mesh.print_details(std::cout);
    std::cin.get();
    return 0;
}
