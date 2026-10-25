#include <cfd/geometry/domain/domain1d.hpp>
#include <cfd/geometry/domain/domain2d.hpp>
#include <cfd/geometry/domain/domain3d.hpp>
#include <cfd/mesh/mesh1d/uniform_fv_mesh1d.hpp>

#include <cmath>
#include <type_traits>
#include <utility>

#include <iomanip>
#include <initializer_list>
#include <iostream>
#include <limits>
#include <stdexcept>

namespace
{

void require(bool condition, const char* message)
{
    if (!condition) {
        throw std::runtime_error(message);
    }
}

template<class Operation>
void require_invalid(Operation operation)
{
    try {
        operation();
    } catch (const std::invalid_argument&) {
        return;
    }
    throw std::runtime_error("Expected std::invalid_argument");
}

void check_contracts()
{
    using cfd::geometry::Domain1D;
    using cfd::geometry::Domain2D;
    using cfd::geometry::Domain3D;
    static_assert(std::is_nothrow_copy_constructible_v<Domain1D>);
    static_assert(std::is_nothrow_copy_assignable_v<Domain1D>);
    static_assert(std::is_nothrow_move_constructible_v<Domain1D>);
    static_assert(std::is_nothrow_move_assignable_v<Domain1D>);
    static_assert(std::is_nothrow_constructible_v<Domain2D, Domain1D, Domain1D>);
    static_assert(std::is_nothrow_constructible_v<Domain3D, Domain1D, Domain1D, Domain1D>);
    static_assert(noexcept(std::declval<Domain2D&>().set_domains(
        std::declval<Domain1D>(), std::declval<Domain1D>())));
    static_assert(noexcept(std::declval<Domain3D&>().set_domains(
        std::declval<Domain1D>(), std::declval<Domain1D>(), std::declval<Domain1D>())));
    static_assert(std::is_same_v<decltype(std::declval<const Domain2D&>().x_domain()), Domain1D>);
    static_assert(std::is_same_v<decltype(std::declval<const Domain3D&>().z_domain()), Domain1D>);

    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double inf = std::numeric_limits<double>::infinity();
    const double max = std::numeric_limits<double>::max();
    const double tiny = std::numeric_limits<double>::denorm_min();
    for (double invalid : {nan, inf, -inf, 0.0, -1.0}) {
        require_invalid([&] { (void)Domain1D(invalid); });
        require_invalid([&] { (void)Domain1D::from_start_and_length(1.0, invalid); });
    }
    for (double invalid : {nan, inf, -inf}) {
        require_invalid([&] { (void)Domain1D(invalid, 1.0); });
        require_invalid([&] { (void)Domain1D(0.0, invalid); });
    }
    require_invalid([&] { (void)Domain1D(-max, max); });
    require_invalid([] { (void)Domain1D(1.0, 1.0); });
    require_invalid([] { (void)Domain1D(2.0, 1.0); });
    require_invalid([&] { (void)Domain1D::from_start_and_length(max, max); });
    require_invalid([] { (void)Domain1D::from_start_and_length(1e16, 1.0); });
    require(Domain1D(0.0, tiny).length_x() == tiny, "Subnormal extent rejected");
    const double next = std::nextafter(1.0, inf);
    require(Domain1D(1.0, next).length_x() == next - 1.0, "Adjacent bounds lost");
    auto rounded = Domain1D::from_start_and_length(1e16, 3.0);
    require(rounded.length_x() == rounded.x_end() - rounded.x_start() &&
            rounded.length_x() != 3.0, "Requested length replaced represented extent");
    rounded.set_length_x(5.0);
    require(rounded.x_end() == 1e16 + 5.0 && rounded.length_x() != 5.0,
            "Length setter ignored endpoint rounding");
    const auto before = rounded;
    require_invalid([&] { rounded.set_length_x(1.0); });
    require(rounded == before, "Collapsed length setter changed bounds");

    Domain1D interval(-1.0, 1.0);
    const auto original = interval;
    for (double invalid : {nan, inf, -inf}) {
        require_invalid([&] { interval.set_x_start(invalid); });
        require_invalid([&] { interval.set_x_end(invalid); });
        require_invalid([&] { interval.set_length_x(invalid); });
        require(interval == original, "Rejected setter changed bounds");
    }
    require_invalid([&] { interval.set_bounds(-max, max); });
    require_invalid([&] { interval.set_x_start(1.0); });
    require_invalid([&] { interval.set_x_end(-1.0); });
    require_invalid([&] { interval.set_length_x(0.0); });
    require(interval == original, "Rejected update changed bounds");
    require(interval.contains(-1.0) && interval.contains(1.0) && interval.contains(0.0),
            "Closed containment failed");
    require(!interval.contains(nan) && !interval.contains(inf) && !interval.contains(-inf) &&
            !interval.contains(std::nextafter(1.0, inf)), "Invalid containment accepted");
    require(Domain1D(-0.0, 1.0) == Domain1D(0.0, 1.0), "Signed zero equality failed");
    require(Domain1D(0.0, 1.0) != Domain1D(0.0, next), "Equality applied a tolerance");

    Domain2D rectangle(interval, Domain1D(2.0));
    Domain3D cuboid(interval, Domain1D(2.0), Domain1D(3.0));
    const auto rectangle_before = rectangle;
    const auto cuboid_before = cuboid;
    const auto& snapshot = rectangle.x_domain();
    const auto& temporary_snapshot = Domain3D(interval, interval, interval).z_domain();
    require(temporary_snapshot == interval, "Temporary axis snapshot lost bounds");
    interval.set_bounds(10.0, 11.0);
    require(rectangle.x_domain() == original && cuboid.x_domain() == original,
            "Composition retained an external reference");
    require_invalid([&] { rectangle.set_domains(interval, Domain1D(0.0)); });
    require_invalid([&] { cuboid.set_domains(interval, interval, Domain1D(0.0)); });
    require(rectangle == rectangle_before && cuboid == cuboid_before,
            "Failed argument construction changed axes");
    require(rectangle.contains(-1.0, 2.0) && cuboid.contains(1.0, 2.0, 3.0),
            "Corner containment failed");
    require(!rectangle.contains(nan, 1.0) && !rectangle.contains(0.0, inf) &&
            !cuboid.contains(0.0, nan, 1.0) && !cuboid.contains(0.0, 1.0, inf) &&
            !cuboid.contains(2.0, 1.0, 1.0), "Invalid composed containment accepted");
    cfd::mesh::UniformFvMesh1D mesh(rectangle.x_domain(), 2);
    rectangle.set_domains(interval, original);
    cuboid.set_domains(interval, original, original);
    require(snapshot == original && mesh.x_start() == -1.0 && mesh.x_end() == 1.0,
            "Parent mutation changed an axis or mesh snapshot");
    require(rectangle != rectangle_before && cuboid != cuboid_before,
            "Composed equality missed axis changes");
    rectangle.set_x_domain(original);
    rectangle.set_y_domain(Domain1D(2.0));
    cuboid.set_x_domain(original);
    cuboid.set_y_domain(Domain1D(2.0));
    cuboid.set_z_domain(Domain1D(3.0));
    require(rectangle == rectangle_before && cuboid == cuboid_before,
            "Axis replacement failed");
    std::cout << "Domain1D/2D/3D contract checks passed\n";
}

void print_domain(const char* label, const cfd::geometry::Domain1D& domain)
{
    std::cout << label << ": [" << domain.x_start() << ", " << domain.x_end()
              << "], length = " << domain.length_x() << '\n';
}

}  // namespace

int main()
{
    try {
        check_contracts();
    } catch (const std::exception& error) {
        std::cerr << "Domain contract check failed: " << error.what() << '\n';
        return 1;
    }
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "Domain1D example\n"
              << "=================\n\n"
              << "Domain1D stores the finite interval bounds. Its length is\n"
              << "calculated from those bounds; it does not store mesh data.\n\n";

    // One argument means a length starting at x = 0.
    cfd::geometry::Domain1D from_origin(4.0);
    print_domain("Constructed from length", from_origin);

    // Two constructor arguments are the start and end coordinates.
    cfd::geometry::Domain1D from_bounds(-2.0, 3.0);
    print_domain("Constructed from bounds", from_bounds);

    // Use the named factory when the inputs are a start coordinate and length.
    cfd::geometry::Domain1D shifted =
        cfd::geometry::Domain1D::from_start_and_length(5.0, 2.5);
    print_domain("Constructed from start and length", shifted);

    std::cout << "\nUpdating bounds and length:\n";
    from_bounds.set_x_start(-3.0);
    print_domain("After set_x_start(-3.0)", from_bounds);

    from_bounds.set_x_end(4.0);
    print_domain("After set_x_end(4.0)", from_bounds);

    from_bounds.set_length_x(2.0);
    print_domain("After set_length_x(2.0)", from_bounds);

    from_bounds.set_bounds(1.0, 6.0);
    print_domain("After set_bounds(1.0, 6.0)", from_bounds);

    std::cout << "\nInvalid updates are rejected and preserve the current interval:\n";
    try
    {
        from_bounds.set_x_end(0.0);  // The end must be greater than the start.
    }
    catch (const std::invalid_argument& error)
    {
        std::cout << "Rejected update: " << error.what() << '\n';
        print_domain("Interval after rejection", from_bounds);
    }

    try
    {
        from_bounds.set_length_x(std::numeric_limits<double>::infinity());
    }
    catch (const std::invalid_argument& error)
    {
        std::cout << "Rejected non-finite length: " << error.what() << '\n';
        print_domain("Interval after rejection", from_bounds);
    }

    std::cout << "\nDomain1D limitations:\n"
              << "  - It stores only one finite, increasing X interval.\n"
              << "  - It does not create cells, faces, or cell centers.\n"
              << "  - It does not store boundary conditions, field values, or\n"
              << "    cross-sectional area. Those belong to mesh/solver objects.\n";
    std::cin.get();
    return 0;
}
