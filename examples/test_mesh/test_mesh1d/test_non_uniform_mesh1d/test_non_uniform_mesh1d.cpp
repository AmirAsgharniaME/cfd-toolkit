#include <cmath>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>

#include <cfd/geometry/domain/domain1d.hpp>
#include <cfd/mesh/grading_type.hpp>
#include <cfd/mesh/mesh1d/fv_mesh1d.hpp>

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
    using cfd::mesh::FvMesh1D;
    using cfd::mesh::GradingType;

    Domain1D domain(-2.0, 4.0);
    const FvMesh1D forward(domain, 4, GradingType::Forward, 2.0);
    require(forward.Start_Coord() == -2.0 && forward.End_Coord() == 4.0 &&
            forward.Length() == 6.0, "Graded mesh bounds do not match the domain");
    require(forward.N_Cells() == 4 && forward.N_Faces() == 5, "Incorrect graded mesh counts");
    for (std::size_t cell = 0; cell + 1 < forward.N_Cells(); ++cell)
    {
        const double ratio = forward.Cell_Volume(cell + 1) / forward.Cell_Volume(cell);
        require(std::fabs(ratio - 2.0) < 1e-12, "Forward grading ratio is not honored");
    }

    const FvMesh1D backward(domain, 4, GradingType::Backward, 2.0);
    for (std::size_t cell = 0; cell < backward.N_Cells(); ++cell)
        require(std::fabs(backward.Cell_Volume(cell) -
                          forward.Cell_Volume(backward.N_Cells() - 1 - cell)) < 1e-12,
                "Backward grading is not the mirror of Forward");

    const FvMesh1D both(domain, 5, GradingType::BothEnds, 3.0);
    require(both.Cell_Volume(0) < both.Cell_Volume(2) && both.Cell_Volume(4) < both.Cell_Volume(2),
            "BothEnds grading does not refine both boundaries");
    const FvMesh1D center(domain, 5, GradingType::Center, 3.0);
    require(center.Cell_Volume(2) < center.Cell_Volume(0), "Center grading does not refine the center");

    Domain1D mutable_domain(0.0, 1.0);
    const FvMesh1D snapshot(mutable_domain, 3, GradingType::Forward, 2.0);
    mutable_domain.Set_Bounds(10.0, 20.0);
    require(snapshot.Start_Coord() == 0.0 && snapshot.End_Coord() == 1.0,
            "Domain mutation changed the mesh snapshot");

    const double cell_volume_sum = [&] {
        double sum = 0.0;
        for (const double volume : forward.Cell_Volumes())
            sum += volume;
        return sum;
    }();
    require(std::fabs(cell_volume_sum - forward.Length()) < 1e-12,
            "Cell volumes do not sum to the mesh length");

    const FvMesh1D single_uniform(domain, 1);
    for (const GradingType type : {GradingType::Uniform, GradingType::Forward,
                                   GradingType::Backward, GradingType::BothEnds,
                                   GradingType::Center})
    {
        const FvMesh1D single(domain, 1, type, 1.0);
        require(single.N_Cells() == 1 && single.Face_Coord(0) == single_uniform.Face_Coord(0) &&
                single.Face_Coord(1) == single_uniform.Face_Coord(1),
                "Single-cell graded mesh differs from the uniform single-cell mesh");
    }

    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double inf = std::numeric_limits<double>::infinity();
    require_exception<std::invalid_argument>([&] { (void)FvMesh1D(domain, 0, GradingType::Forward, 2.0); });
    require_exception<std::invalid_argument>([&] { (void)FvMesh1D(domain, 4, GradingType::Forward, 0.5); });
    require_exception<std::invalid_argument>([&] { (void)FvMesh1D(domain, 4, GradingType::Forward, nan); });
    require_exception<std::invalid_argument>([&] { (void)FvMesh1D(domain, 4, GradingType::Forward, inf); });
    require_exception<std::invalid_argument>([&] {
        (void)FvMesh1D(domain, 4, static_cast<GradingType>(255U), 1.0);
    });

    const Domain1D adjacent_bounds(1.0, std::nextafter(1.0, inf));
    require_exception<std::invalid_argument>([&] {
        (void)FvMesh1D(adjacent_bounds, 2, GradingType::Forward, 1.0);
    });

    std::ostringstream details;
    const FvMesh1D steep(Domain1D(0.0, 1.0), 12, GradingType::Forward, 10.0);
    steep.Print_Details(details);
    require(details.str().find("e-") != std::string::npos,
            "Print_Details should show small graded cell volumes in scientific notation");
    std::cout << "FvMesh1D contract checks passed\n";
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
