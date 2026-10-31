#include <cmath>
#include <initializer_list>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>

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

    Domain1D domain(-2.0, 2.0);
    const FvMesh1D mesh(domain, 4);
    const FvMesh1D direct(domain.start(), domain.end(), 4);
    require(mesh.start() == domain.start() && mesh.end() == domain.end() &&
            mesh.length() == domain.length() && mesh.x_start() == mesh.start() &&
            mesh.x_end() == mesh.end(), "Mesh bounds do not match Domain1D interface");
    require(mesh.num_cells() == 4 && mesh.num_faces() == 5, "Incorrect mesh counts");
    require(mesh.faces().size() == mesh.num_faces() &&
            mesh.cell_centers().size() == mesh.num_cells() &&
            mesh.cell_widths().size() == mesh.num_cells() &&
            mesh.cell_volumes().size() == mesh.num_cells(),
            "Geometry storage sizes do not match mesh counts");
    require(mesh.dx() == 1.0 && mesh.spacing() == mesh.dx() &&
            mesh.nominal_spacing() == mesh.dx(), "Incorrect nominal spacing");
    for (std::size_t f = 0; f < mesh.num_faces(); ++f)
    {
        require(mesh.face(f) == -2.0 + static_cast<double>(f) &&
                mesh.face(f) == direct.face(f), "Constructor face geometry differs");
    }
    for (std::size_t p = 0; p < mesh.num_cells(); ++p)
    {
        require(mesh.cell_center(p) == -1.5 + static_cast<double>(p) &&
                mesh.cell_width(p) == 1.0 && mesh.cell_volume(p) == 1.0,
                "Incorrect cell geometry");
    }

    const auto check_domain_update = [&] {
        require(mesh.start() == -2.0 && mesh.end() == 2.0 && mesh.length() == 4.0 &&
                mesh.dx() == 1.0, "Domain mutation changed the mesh snapshot");
        for (std::size_t f = 0; f < mesh.num_faces(); ++f)
        {
            require(mesh.face(f) == direct.face(f), "Domain mutation changed stored faces");
        }
        for (std::size_t p = 0; p < mesh.num_cells(); ++p)
        {
            require(mesh.cell_center(p) == direct.cell_center(p) &&
                    mesh.cell_width(p) == direct.cell_width(p),
                    "Domain mutation changed stored cell geometry");
        }
        const FvMesh1D updated(domain, 4);
        require(updated.start() == domain.start() && updated.end() == domain.end() &&
                updated.length() == domain.length() && updated.dx() == domain.length() / 4.0 &&
                updated.face(0) == domain.start() && updated.face(4) == domain.end(),
                "New mesh did not use updated domain interval");
    };
    domain.set_start(-3.0);
    check_domain_update();
    domain.set_end(5.0);
    check_domain_update();
    domain.set_length(4.0);
    check_domain_update();
    domain.set_bounds(10.0, 14.0);
    check_domain_update();

    auto rounded = Domain1D::from_start_and_length(1e16, 3.0);
    const FvMesh1D rounded_mesh(rounded, 1);
    require(rounded_mesh.dx() == rounded.length() && rounded_mesh.dx() != 3.0 &&
            rounded_mesh.face(1) == rounded.end(), "Mesh lost represented domain length");
    rounded.set_length(7.0);
    const FvMesh1D resized(rounded, 2);
    require(resized.dx() == rounded.length() / 2.0 && resized.face(2) == rounded.end(),
            "Mesh ignored rounded length update");

    const FvMesh1D fractional(Domain1D(1.0), 11);
    require(fractional.dx() == fractional.length() / 11.0,
            "Fractional spacing does not use the represented interval length");
    require(fractional.face(0) == 0.0 && fractional.face(11) == 1.0,
            "Boundary coordinates were not preserved exactly");
    for (std::size_t p = 0; p < fractional.num_cells(); ++p)
    {
        require(fractional.cell_width(p) == fractional.face(p + 1) - fractional.face(p) &&
                fractional.face(p) < fractional.cell_center(p) &&
                fractional.cell_center(p) < fractional.face(p + 1),
                "Rounded geometry does not match represented faces");
    }

    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double inf = std::numeric_limits<double>::infinity();
    const double max = std::numeric_limits<double>::max();
    const double tiny = std::numeric_limits<double>::denorm_min();
    for (double invalid : {nan, inf, -inf})
    {
        require_exception<std::invalid_argument>([&] { (void)FvMesh1D(invalid, 1.0, 1); });
        require_exception<std::invalid_argument>([&] { (void)FvMesh1D(0.0, invalid, 1); });
    }
    require_exception<std::invalid_argument>([] { (void)FvMesh1D(1.0, 1.0, 1); });
    require_exception<std::invalid_argument>([] { (void)FvMesh1D(2.0, 1.0, 1); });
    require_exception<std::invalid_argument>([&] { (void)FvMesh1D(-max, max, 1); });
    require_exception<std::invalid_argument>([&] { (void)FvMesh1D(domain, 0); });
    require_exception<std::invalid_argument>([] { (void)FvMesh1D(0.0, 1.0, 0); });
    require_exception<std::length_error>([&] {
        (void)FvMesh1D(domain, std::numeric_limits<std::size_t>::max());
    });
    // A valid domain may not have enough representable coordinates for a mesh.
    require_exception<std::invalid_argument>([&] { (void)FvMesh1D(Domain1D(tiny), 2); });
    require_exception<std::invalid_argument>([&] {
        (void)FvMesh1D(Domain1D(1.0, std::nextafter(1.0, inf)), 1);
    });
    require_exception<std::invalid_argument>([] {
        (void)FvMesh1D(Domain1D(1e16, 1e16 + 4.0), 4);
    });
    const FvMesh1D explicit_mesh({-2.0, -1.0, 2.0});
    require(explicit_mesh.start() == -2.0 && explicit_mesh.end() == 2.0 &&
            explicit_mesh.length() == 4.0, "Explicit face bounds are incorrect");
    require(explicit_mesh.cell_width(0) == 1.0 && explicit_mesh.cell_width(1) == 3.0 &&
            explicit_mesh.cell_center(0) == -1.5 && explicit_mesh.cell_center(1) == 0.5,
            "Base mesh did not preserve nonuniform interval geometry");
    require_exception<std::invalid_argument>([] { (void)cfd::mesh::FvMesh1D({}); });
    require_exception<std::invalid_argument>([] { (void)cfd::mesh::FvMesh1D({0.0}); });
    require_exception<std::invalid_argument>([] { (void)cfd::mesh::FvMesh1D({0.0, 0.0}); });
    require_exception<std::invalid_argument>([] { (void)cfd::mesh::FvMesh1D({1.0, 0.0}); });
    require_exception<std::invalid_argument>([] { (void)cfd::mesh::FvMesh1D({0.0, 2.0, 1.0}); });
    require_exception<std::invalid_argument>([&] { (void)cfd::mesh::FvMesh1D({0.0, nan, 1.0}); });
    require_exception<std::invalid_argument>([&] { (void)cfd::mesh::FvMesh1D({0.0, inf}); });
    require_exception<std::invalid_argument>([&] {
        (void)cfd::mesh::FvMesh1D({1.0, std::nextafter(1.0, inf)});
    });
    // Each cell width is finite, but the complete interval is not.
    require_exception<std::invalid_argument>([&] {
        (void)cfd::mesh::FvMesh1D({-max, 0.0, max});
    });
    require_exception<std::length_error>([&] {
        (void)FvMesh1D(domain, std::vector<double>().max_size());
    });
    const FvMesh1D extreme(Domain1D(0.0, max), 1);
    require(extreme.dx() == max && extreme.cell_center(0) == max * 0.5,
            "Finite extreme domain geometry failed");
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

    mesh.print_details(std::cout);
    std::cin.get();
    return 0;
}
