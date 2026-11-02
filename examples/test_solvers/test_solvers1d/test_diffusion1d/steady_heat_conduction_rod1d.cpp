#include "cfd/boundary/types.hpp"
#include <cfd/field/field1d/field1d.hpp>
#include <cfd/geometry/domain/domain1d.hpp>
#include <cfd/mesh/mesh1d/fv_mesh1d.hpp>
#include <cfd/fv_matrix/fv_matrix1d.hpp>
#include<cfd/boundary/boundary1d/dirichlet_boundary1d.hpp>
#include <cstddef>
#include <iomanip>
#include <iostream>

int main()
{
    const cfd::geometry::Domain1D domain(0.0, 1.0);
    const cfd::mesh::FvMesh1D mesh(domain, 5);
    cfd::field::VolScalarField1D T("T", mesh, 300.0);

    std::cout << std::fixed << std::setprecision(3)
              << "Steady heat conduction in a 1D rod\n"
              << "Domain: [" << domain.Start_Coord() << ", " << domain.End_Coord()
              << "] m\n"
              << "Length: " << domain.Length() << " m\n"
              << "Cells: " << mesh.N_Cells() << '\n'
              << "Cell size: " << mesh.Cell_Volume(0) << " m\n\n"
              << "Cell    Center (m)    Temperature (K)\n";

    // 1. Define diffusion coefficient (Gamma = 1.0 everywhere)
    const cfd::field::VolScalarField1D gamma("gamma", mesh, 1.0);

    // 2. Create FVM matrix container linked to the mesh
    cfd::fv_matrix::FvMatrix1D matrix(mesh);

        // 3. Define Boundary Conditions (Dirichlet: Left = 400 K, Right = 200 K)
    const cfd::boundary::DirichletBoundary1D left_bc(
        "left_wall",
        cfd::boundary::BoundarySide1D::Start,
        400.0);
    const cfd::boundary::DirichletBoundary1D right_bc(
        "right_wall",
        cfd::boundary::BoundarySide1D::End,
         200.0);

    for (std::size_t p = 0; p < mesh.N_Cells(); ++p)
    {
        std::cout << std::setw(4) << p << std::setw(14) << mesh.Cell_Center_Coord(p)
                  << std::setw(19) << T[p] << '\n';
    }

    // TODO: Integrate matrix assembly.
    // TODO: Integrate boundary conditions.
    std::cin.get();
    return 0;
}
