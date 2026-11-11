#include <cmath>
#include <cstddef>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <cfd/boundary/boundary1d/dirichlet_boundary1d.hpp>
#include <cfd/boundary/boundary1d/neumann_boundary1d.hpp>
#include <cfd/finite_volume/fvm/fvm1d/laplacian1d.hpp>
#include <cfd/fv_matrix/fv_matrix1d.hpp>
#include <cfd/linear_algebra/solvers/tdma_solver.hpp>
#include <cfd/geometry/domain/domain1d.hpp>
#include <cfd/mesh/mesh1d/fv_mesh1d.hpp>

namespace
{

void require(bool condition, const char* message)
{
    if (!condition)
    {
        throw std::runtime_error(message);
    }
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
    throw std::runtime_error("Expected mesh operation exception");
}

void check_contracts()
{
    using cfd::geometry::Domain1D;
    using cfd::mesh::FvMesh1D;

    static_assert(!std::is_copy_assignable_v<FvMesh1D>);
    static_assert(!std::is_move_assignable_v<FvMesh1D>);
    static_assert(std::is_copy_constructible_v<FvMesh1D>);
    static_assert(std::is_move_constructible_v<FvMesh1D>);

    Domain1D domain(-2.0, 2.0);
    const FvMesh1D mesh(domain, 4);
    require(mesh.Start_Coord() == domain.Start_Coord() &&
            mesh.End_Coord() == domain.End_Coord() &&
            mesh.Length() == domain.Length(), "Mesh bounds do not match Domain1D interface");
    require(mesh.N_Cells() == 4 && mesh.N_Faces() == 5, "Incorrect mesh counts");
    require(mesh.Face_Coords().size() == mesh.N_Faces() &&
            mesh.Cell_Center_Coords().size() == mesh.N_Cells() &&
            mesh.Cell_Volumes().size() == mesh.N_Cells(),
            "Geometry storage sizes do not match mesh counts");
    for (std::size_t f = 0; f < mesh.N_Faces(); ++f)
    {
        require(mesh.Face_Coord(f) == -2.0 + static_cast<double>(f),
                "Incorrect uniform face geometry");
        require(mesh.Face_Area(f) == 1.0, "Incorrect face area");
    }
    for (std::size_t p = 0; p < mesh.N_Cells(); ++p)
    {
        require(mesh.Cell_Center_Coord(p) == -1.5 + static_cast<double>(p) &&
                mesh.Cell_Volume(p) == 1.0 &&
                mesh.wFace_Coord(p) == mesh.Face_Coord(p) &&
                mesh.eFace_Coord(p) == mesh.Face_Coord(p + 1) &&
                mesh.wFaceDx(p) == 0.5 && mesh.eFaceDx(p) == 0.5,
                "Incorrect cell geometry");
    }
    for (std::size_t f = 0; f < mesh.N_Faces(); ++f)
    {
        const auto owner = mesh.Owner(f);
        const auto neighbour = mesh.Neighbour(f);
        require(owner == (f == 0 ? 0 : f - 1), "Incorrect face owner");
        require(mesh.s_f(owner, f) == (f == 0 ? -1.0 : 1.0) &&
                mesh.e_PN(owner, f) == mesh.s_f(owner, f),
                "Incorrect owner orientation");
        if (f > 0 && f < mesh.N_Cells())
        {
            require(neighbour && *neighbour == f &&
                    mesh.s_f(*neighbour, f) == -mesh.s_f(owner, f) &&
                    mesh.e_PN(*neighbour, f) == mesh.s_f(*neighbour, f),
                    "Incorrect neighbour orientation");
        }
        else require(!neighbour, "Boundary face has a neighbour");
    }
    require_exception<std::out_of_range>([&] { (void)mesh.Owner(5); });
    require_exception<std::out_of_range>([&] { (void)mesh.Face_Area(mesh.N_Faces()); });
    require_exception<std::out_of_range>([&] { (void)mesh.Neighbour(5); });
    require_exception<std::out_of_range>([&] { (void)mesh.s_f(0, 5); });
    require_exception<std::out_of_range>([&] { (void)mesh.s_f(0, 2); });
    require_exception<std::out_of_range>([&] { (void)mesh.s_f(4, 4); });

    const FvMesh1D one(Domain1D(0.0, 1.0), 1);
    require(one.Owner(0) == 0 && one.Owner(1) == 0 &&
            !one.Neighbour(0) && !one.Neighbour(1) &&
            one.s_f(0, 0) == -1.0 && one.s_f(0, 1) == 1.0,
            "Single cell topology is incorrect");
    const cfd::boundary::NeumannBoundary1D left_gradient(
        "left", cfd::boundary::BoundarySide1D::Start, 2.0);
    const cfd::boundary::NeumannBoundary1D right_gradient(
        "right", cfd::boundary::BoundarySide1D::End, 2.0);
    require(left_gradient.coordinate_gradient() == -2.0 &&
            right_gradient.coordinate_gradient() == 2.0, "Neumann orientation is incorrect");

    cfd::field::VolScalarField1D gamma("gamma", mesh, 1.0);
    cfd::fv_matrix::FvMatrix1D system(mesh);
    cfd::fvm::Laplacian(gamma, system);
    require(system.a_P(0) == 1.0 && system.a_E(0) == -1.0 &&
            system.a_W(1) == -1.0 && system.a_P(1) == 2.0,
            "Internal diffusion stencil is incorrect");
    cfd::boundary::DirichletBoundary1D("left", cfd::boundary::BoundarySide1D::Start, 0.0).Apply(gamma, system);
    cfd::boundary::DirichletBoundary1D("right", cfd::boundary::BoundarySide1D::End, 4.0).Apply(gamma, system);
    require(system.a_P(0) == 3.0 && system.a_P(3) == 3.0 && system.b(3) == 8.0,
            "Dirichlet boundary stencil is incorrect");
    const auto solution = cfd::linear_algebra::TDMASolver::solve(system.Matrix());
    for (std::size_t p = 0; p < mesh.N_Cells(); ++p)
        require(std::abs(solution[p] - (static_cast<double>(p) + 0.5)) < 1e-12,
                "Diffusion solution is not linear");
    cfd::field::VolScalarField1D one_gamma("gamma", one, 1.0);
    cfd::fv_matrix::FvMatrix1D one_system(one);
    cfd::fvm::Laplacian(one_gamma, one_system);
    cfd::boundary::DirichletBoundary1D("left", cfd::boundary::BoundarySide1D::Start, 1.0).Apply(one_gamma, one_system);
    cfd::boundary::DirichletBoundary1D("right", cfd::boundary::BoundarySide1D::End, 3.0).Apply(one_gamma, one_system);
    require(one_system.a_P(0) == 4.0 && one_system.b(0) == 8.0,
            "Single cell boundary assembly is incorrect");
    require(mesh.eCellDx(0) == 1.0 && mesh.wCellDx(3) == 1.0, "Incorrect cell distance");
    require_exception<std::out_of_range>([&] { (void)mesh.wCellDx(0); });
    require_exception<std::out_of_range>([&] { (void)mesh.eCellDx(3); });
    require_exception<std::out_of_range>([&] { (void)mesh.Face_Coord(5); });
    require_exception<std::out_of_range>([&] { (void)mesh.Cell_Volume(4); });

    const auto check_domain_update = [&] {
        require(mesh.Start_Coord() == -2.0 && mesh.End_Coord() == 2.0 && mesh.Length() == 4.0,
                "Domain mutation changed the mesh snapshot");
        const FvMesh1D updated(domain, 4);
        require(updated.Start_Coord() == domain.Start_Coord() &&
                updated.End_Coord() == domain.End_Coord() &&
                updated.Length() == domain.Length() &&
                updated.Cell_Volume(0) == domain.Length() / 4.0 &&
                updated.Face_Coord(0) == domain.Start_Coord() &&
                updated.Face_Coord(4) == domain.End_Coord(),
                "New mesh did not use updated domain interval");
    };
    domain.Set_Start_Coord(-3.0);
    check_domain_update();
    domain.set_End_Coord(5.0);
    check_domain_update();
    domain.Set_Length(4.0);
    check_domain_update();
    domain.Set_Bounds(10.0, 14.0);
    check_domain_update();

    auto rounded = Domain1D::from_start_and_length(1e16, 3.0);
    const FvMesh1D rounded_mesh(rounded, 1);
    require(rounded_mesh.Length() == rounded.Length() && rounded_mesh.Length() != 3.0 &&
            rounded_mesh.Face_Coord(1) == rounded.End_Coord(),
            "Mesh lost represented domain length");

    const FvMesh1D fractional(Domain1D(1.0), 11);
    require(fractional.Face_Coord(0) == 0.0 && fractional.Face_Coord(11) == 1.0,
            "Boundary coordinates were not preserved exactly");
    for (std::size_t p = 0; p < fractional.N_Cells(); ++p)
    {
        require(fractional.Cell_Volume(p) ==
                    fractional.Face_Coord(p + 1) - fractional.Face_Coord(p) &&
                fractional.Face_Coord(p) < fractional.Cell_Center_Coord(p) &&
                fractional.Cell_Center_Coord(p) < fractional.Face_Coord(p + 1),
                "Rounded geometry does not match represented faces");
    }

    const double inf = std::numeric_limits<double>::infinity();
    const double max = std::numeric_limits<double>::max();
    const double tiny = std::numeric_limits<double>::denorm_min();
    const Domain1D unit(0.0, 1.0);
    require_exception<std::invalid_argument>([&] { (void)FvMesh1D(unit, 0); });
    require_exception<std::length_error>([&] {
        (void)FvMesh1D(unit, std::numeric_limits<std::size_t>::max());
    });
    require_exception<std::length_error>([&] {
        (void)FvMesh1D(unit, std::vector<double>().max_size());
    });
    require_exception<std::invalid_argument>([&] { (void)FvMesh1D(Domain1D(tiny), 2); });
    require_exception<std::invalid_argument>([&] {
        (void)FvMesh1D(Domain1D(1.0, std::nextafter(1.0, inf)), 1);
    });
    require_exception<std::invalid_argument>([] {
        (void)FvMesh1D(Domain1D(1e16, 1e16 + 4.0), 4);
    });

    const FvMesh1D extreme(Domain1D(0.0, max), 1);
    require(extreme.Cell_Volume(0) == max && extreme.Cell_Center_Coord(0) == max * 0.5,
            "Finite extreme domain geometry failed");

    FvMesh1D source(unit, 3);
    const FvMesh1D copy(source);
    require(copy.N_Cells() == 3 && source.N_Cells() == 3, "Copy changed the source mesh");
    const FvMesh1D moved(std::move(source));
    require(moved.N_Cells() == 3, "Move construction lost geometry");
    // NOLINTNEXTLINE(bugprone-use-after-move): the moved-from contract is under test.
    require(source.N_Cells() == 0 && source.N_Faces() == 0, "Moved-from mesh is not empty");
    require_exception<std::out_of_range>([&] { (void)source.Owner(0); });
    require_exception<std::out_of_range>([&] { (void)source.s_f(0, 0); });
    require_exception<std::logic_error>([&] { (void)source.Start_Coord(); });
    require_exception<std::logic_error>([&] { (void)source.End_Coord(); });
    require_exception<std::logic_error>([&] { (void)source.Length(); });

    std::ostringstream details;
    FvMesh1D tiny_cells(Domain1D(0.0, 1.0), 4, cfd::mesh::GradingType::Forward, 1.0);
    tiny_cells.Print_Details(details);
    require(details.str().find("FvMesh1D Geometry Details") != std::string::npos,
            "Print_Details produced no header");
    std::cout << "FvMesh1D contract checks passed\n";
}

} // namespace

int main()
{
    try
    {
        check_contracts();
    }
    catch (const std::exception& error)
    {
        std::cerr << "Uniform mesh contract check failed: " << error.what() << '\n';
        return 1;
    }
    const cfd::geometry::Domain1D domain(0.0, 1.0);
    const cfd::mesh::FvMesh1D mesh(domain, 11);

    mesh.Print_Details(std::cout);
    std::cin.get();
    return 0;
}
