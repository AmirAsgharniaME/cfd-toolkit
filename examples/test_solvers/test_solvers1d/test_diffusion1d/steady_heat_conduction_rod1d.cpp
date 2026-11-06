#include "cfd/boundary/types.hpp"
#include <cfd/field/field1d/field1d.hpp>
#include <cfd/geometry/domain/domain1d.hpp>
#include <cfd/mesh/mesh1d/fv_mesh1d.hpp>
#include <cfd/fv_matrix/fv_matrix1d.hpp>
#include <cfd/finite_volume/fvm/fvm1d/laplacian1d.hpp>
#include <cfd/boundary/boundary1d/dirichlet_boundary1d.hpp>
#include <cfd/linear_algebra/solvers/tdma_solver.hpp>

#include <algorithm>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <vector>

int main()
{
    // Define the computational domain and 1D mesh with 5 cells
    const cfd::geometry::Domain1D domain(0.0, 1.0);
    const cfd::mesh::FvMesh1D mesh(domain, 5);

    // Define the volume scalar field for temperature (T) initialized to 300.0 K
    cfd::field::VolScalarField1D T("T", mesh, 300.0);

    std::cout << std::fixed << std::setprecision(3)
              << "========================================\n"
              << "  Steady Heat Conduction in a 1D Rod    \n"
              << "========================================\n"
              << "Domain: [" << domain.Start_Coord() << ", " << domain.End_Coord() << "] m\n"
              << "Length: " << domain.Length() << " m\n"
              << "Cells: " << mesh.N_Cells() << '\n'
              << "Cell size: " << mesh.Cell_Volume(0) << " m\n\n";

    std::cout << "Initial temperature field (K):\n";
    for (std::size_t p = 0; p < T.Size(); ++p)
    {
        std::cout << "  cell " << p << ": " << T[p] << '\n';
    }
    std::cout << '\n';

    // 1. Define the diffusion coefficient field (Gamma = 1.0 everywhere)
    const cfd::field::VolScalarField1D gamma("gamma", mesh, 1.0);

    // 2. Initialize the FVM coefficient matrix container
    cfd::fv_matrix::FvMatrix1D matrix(mesh);

    // 3. Define boundary conditions (Dirichlet: Left = 400 K, Right = 200 K)
    const cfd::boundary::DirichletBoundary1D left_bc(
        "left_wall",
        cfd::boundary::BoundarySide1D::Start,
        400.0);

    const cfd::boundary::DirichletBoundary1D right_bc(
        "right_wall",
        cfd::boundary::BoundarySide1D::End,
        200.0);

    // 4. Assemble internal faces contributions using the Laplacian operator
    cfd::fvm::laplacian(gamma, matrix);

    // 5. Apply boundary conditions to the matrix
    left_bc.apply(gamma, matrix);
    right_bc.apply(gamma, matrix);

    // 7. Solve the linear system
    const std::vector<double> solution =
    cfd::linear_algebra::TDMASolver::solve(matrix.Matrix());
    std::copy(solution.begin(), solution.end(), T.values().begin());

    std::cout << "Steady-state temperature solution (K):\n";
    for (std::size_t p = 0; p < T.Size(); ++p)
    {
        std::cout << "  cell " << p
                  << " (x = " << mesh.Cell_Center_Coord(p) << " m): "
                  << T[p] << '\n';
    }
    std::cout << '\n';

    // 6. Print the assembled matrix coefficients (Tridiagonal check)
    std::cout << "--- Assembled Tridiagonal Matrix ---\n";
    std::cout << "Row |   Lower (A_w)  | Diagonal (A_p) |   Upper (A_e)  |    RHS (b)    \n";
    std::cout << "----+----------------+----------------+----------------+---------------\n";
    for (std::size_t p = 0; p < mesh.N_Cells(); ++p)
    {
        std::cout << std::setw(3) << p << " | "
                  << std::setw(14) << matrix.Matrix().lower()[p] << " | "
                  << std::setw(14) << matrix.Matrix().diag()[p]  << " | "
                  << std::setw(14) << matrix.Matrix().upper()[p] << " | "
                  << std::setw(14) << matrix.Matrix().source()[p] << '\n';
    }
    std::cout << "--------------------------------------------------------------------\n\n";

    std::cout << "Press Enter to exit...";
    std::cin.get();
    return 0;
}
