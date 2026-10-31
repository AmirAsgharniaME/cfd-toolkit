#include <cfd/geometry/domain/domain1d.hpp>

#include <cmath>
#include <type_traits>

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
    static_assert(std::is_nothrow_copy_constructible_v<Domain1D>);
    static_assert(std::is_nothrow_copy_assignable_v<Domain1D>);
    static_assert(std::is_nothrow_move_constructible_v<Domain1D>);
    static_assert(std::is_nothrow_move_assignable_v<Domain1D>);

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
    require(Domain1D(0.0, tiny).Length() == tiny, "Subnormal extent rejected");
    const double next = std::nextafter(1.0, inf);
    require(Domain1D(1.0, next).Length() == next - 1.0, "Adjacent bounds lost");
    auto rounded = Domain1D::from_start_and_length(1e16, 3.0);
    require(rounded.Length() == rounded.End_Coord() - rounded.Start_Coord() &&
            rounded.Length() != 3.0, "Requested length replaced represented extent");
    rounded.Set_Length(5.0);
    require(rounded.End_Coord() == 1e16 + 5.0 && rounded.Length() != 5.0,
            "Length setter ignored endpoint rounding");
    const auto before = rounded;
    require_invalid([&] { rounded.Set_Length(1.0); });
    require(rounded == before, "Collapsed length setter changed bounds");

    Domain1D interval(-1.0, 1.0);
    const auto original = interval;
    for (double invalid : {nan, inf, -inf}) {
        require_invalid([&] { interval.Set_Start_Coord(invalid); });
        require_invalid([&] { interval.set_End_Coord(invalid); });
        require_invalid([&] { interval.Set_Length(invalid); });
        require(interval == original, "Rejected setter changed bounds");
    }
    require_invalid([&] { interval.Set_Bounds(-max, max); });
    require_invalid([&] { interval.Set_Start_Coord(1.0); });
    require_invalid([&] { interval.set_End_Coord(-1.0); });
    require_invalid([&] { interval.Set_Length(0.0); });
    require(interval == original, "Rejected update changed bounds");
    require(interval.IsContain(-1.0) && interval.IsContain(1.0) && interval.IsContain(0.0),
            "Closed containment failed");
    require(!interval.IsContain(nan) && !interval.IsContain(inf) && !interval.IsContain(-inf) &&
            !interval.IsContain(std::nextafter(1.0, inf)), "Invalid containment accepted");
    require(Domain1D(-0.0, 1.0) == Domain1D(0.0, 1.0), "Signed zero equality failed");
    require(Domain1D(0.0, 1.0) != Domain1D(0.0, next), "Equality applied a tolerance");

    std::cout << "Domain1D contract checks passed\n";
}

void print_domain(const char* label, const cfd::geometry::Domain1D& domain)
{
    std::cout << label << ": [" << domain.Start_Coord() << ", " << domain.End_Coord()
              << "], length = " << domain.Length() << '\n';
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

    // One argument means a length starting at zero.
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
    from_bounds.Set_Start_Coord(-3.0);
    print_domain("After Set_Start_Coord(-3.0)", from_bounds);

    from_bounds.set_End_Coord(4.0);
    print_domain("After set_End_Coord(4.0)", from_bounds);

    from_bounds.Set_Length(2.0);
    print_domain("After Set_Length(2.0)", from_bounds);

    from_bounds.Set_Bounds(1.0, 6.0);
    print_domain("After Set_Bounds(1.0, 6.0)", from_bounds);

    std::cout << "\nInvalid updates are rejected and preserve the current interval:\n";
    try
    {
        from_bounds.set_End_Coord(0.0);  // The end must be greater than the start.
    }
    catch (const std::invalid_argument& error)
    {
        std::cout << "Rejected update: " << error.what() << '\n';
        print_domain("Interval after rejection", from_bounds);
    }

    try
    {
        from_bounds.Set_Length(std::numeric_limits<double>::infinity());
    }
    catch (const std::invalid_argument& error)
    {
        std::cout << "Rejected non-finite length: " << error.what() << '\n';
        print_domain("Interval after rejection", from_bounds);
    }

    std::cout << "\nDomain1D limitations:\n"
              << "  - It stores only one finite, increasing interval on the real line.\n"
              << "  - It does not create cells, faces, or cell centers.\n"
              << "  - It does not store boundary conditions, field values, or\n"
              << "    cross-sectional area. Those belong to mesh/solver objects.\n";
    std::cin.get();
    return 0;
}
