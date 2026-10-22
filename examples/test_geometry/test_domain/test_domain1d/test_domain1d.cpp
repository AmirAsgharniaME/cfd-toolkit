#include <cfd/geometry/domain/domain1d.hpp>

#include <iomanip>
#include <iostream>
#include <limits>
#include <stdexcept>

namespace
{

void print_domain(const char* label, const cfd::geometry::Domain1D& domain)
{
    std::cout << label << ": [" << domain.x_start() << ", " << domain.x_end()
              << "], length = " << domain.length_x() << '\n';
}

}  // namespace

int main()
{
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
