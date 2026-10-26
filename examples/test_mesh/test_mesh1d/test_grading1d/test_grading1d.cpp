#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <initializer_list>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

#include <cfd/geometry/domain/domain1d.hpp>
#include <cfd/mesh/grading_type.hpp>
#include <cfd/mesh/mesh1d/grading1d.hpp>
#include <cfd/mesh/mesh1d/non_uniform_fv_mesh1d.hpp>

namespace
{

using cfd::geometry::Domain1D;
using cfd::mesh::GradingType;
using cfd::mesh::NonUniformFvMesh1D;

constexpr std::array<GradingType, 5> grading_types{
    GradingType::Uniform,
    GradingType::Forward,
    GradingType::Backward,
    GradingType::BothEnds,
    GradingType::Center};

void require(bool condition, const std::string& message)
{
    if (!condition) {
        throw std::runtime_error(message);
    }
}

template<class Exception, class Operation>
void require_exception(Operation operation, const char* message)
{
    try {
        operation();
    }
    catch (const Exception&) {
        return;
    }
    throw std::runtime_error(message);
}

[[nodiscard]] std::string type_name(const GradingType type)
{
    switch (type) {
    case GradingType::Uniform:
        return "Uniform";
    case GradingType::Forward:
        return "Forward";
    case GradingType::Backward:
        return "Backward";
    case GradingType::BothEnds:
        return "BothEnds";
    case GradingType::Center:
        return "Center";
    }
    return "Unknown";
}

void check_generated_geometry(const Domain1D& domain,
                              const GradingType type,
                              const int n_cells,
                              const double ratio,
                              double& maximum_symmetry_error)
{
    const auto faces = cfd::mesh::grading::generate_faces_1d(domain, n_cells, type, ratio);
    const auto cell_count = static_cast<std::size_t>(n_cells);
    const std::string context = type_name(type) + " with N=" + std::to_string(n_cells);

    require(faces.size() == cell_count + 1U, context + ": incorrect face count");
    require(faces.front() == domain.start(), context + ": start face is not exact");
    require(faces.back() == domain.end(), context + ": end face is not exact");
    for (std::size_t cell = 0; cell < cell_count; ++cell) {
        const double width = faces[cell + 1U] - faces[cell];
        const double center = faces[cell] + width * 0.5;
        require(std::isfinite(faces[cell]) && faces[cell] < faces[cell + 1U],
                context + ": faces are not finite and strictly increasing");
        require(std::isfinite(center) && faces[cell] < center &&
                    center < faces[cell + 1U],
                context + ": cell center is not representable inside its faces");
    }

    if (type == GradingType::BothEnds || type == GradingType::Center) {
        const double tolerance = 1e-12 * domain.length();
        for (std::size_t face = 0; face <= cell_count; ++face) {
            const double left_distance = faces[face] - domain.start();
            const double right_distance = domain.end() - faces[cell_count - face];
            const double symmetry_error = std::fabs(left_distance - right_distance);
            maximum_symmetry_error = std::max(maximum_symmetry_error, symmetry_error);
            require(symmetry_error <= tolerance,
                    context + ": symmetric grading drift exceeds tolerance");
        }
    }

    const NonUniformFvMesh1D mesh(domain, faces);
    require(mesh.num_cells() == cell_count && mesh.num_faces() == faces.size(),
            context + ": generated faces are incompatible with NonUniformFvMesh1D");
    for (std::size_t face = 0; face < faces.size(); ++face) {
        require(mesh.face(face) == faces[face],
                context + ": consumer changed a generated face");
    }
}

void check_unit_ratio(const Domain1D& domain, const GradingType type, const int n_cells)
{
    if (type == GradingType::Uniform) {
        return;
    }

    const auto faces = cfd::mesh::grading::generate_faces_1d(domain, n_cells, type, 1.0);
    const auto cell_count = static_cast<std::size_t>(n_cells);
    const double expected_width = domain.length() / static_cast<double>(n_cells);
    const double tolerance = 1e-12 * domain.length();
    for (std::size_t cell = 0; cell < cell_count; ++cell) {
        const double width = faces[cell + 1U] - faces[cell];
        require(std::fabs(width - expected_width) <= tolerance,
                type_name(type) + " ratio 1.0 did not produce uniform spacing");
    }
}

void check_invalid_inputs()
{
    const Domain1D domain(-2.5, 5.5);
    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double inf = std::numeric_limits<double>::infinity();
    const auto generate = [&domain](const int n_cells,
                                    const GradingType type,
                                    const double ratio) {
        return cfd::mesh::grading::generate_faces_1d(domain, n_cells, type, ratio);
    };

    require_exception<std::invalid_argument>(
        [&] { (void)generate(0, GradingType::Uniform, 1.0); },
        "Expected n_cells=0 to throw std::invalid_argument");
    require_exception<std::invalid_argument>(
        [&] { (void)generate(1, GradingType::Uniform, 1.0); },
        "Expected n_cells=1 to throw std::invalid_argument");
    for (const double ratio : {0.0, -1.0, nan, inf, -inf}) {
        require_exception<std::invalid_argument>(
            [&] { (void)generate(4, GradingType::Forward, ratio); },
            "Expected invalid ratio to throw std::invalid_argument");
    }
    require_exception<std::invalid_argument>(
        [&] { (void)generate(4, GradingType::Forward, 0.5); },
        "Expected graded ratio below one to throw std::invalid_argument");
    require_exception<std::invalid_argument>(
        [&] {
            (void)generate(4, static_cast<GradingType>(255U), 1.0);
        },
        "Expected invalid grading type to throw std::invalid_argument");

    const double next = std::nextafter(1.0, inf);
    const Domain1D two_ulp_domain(1.0, std::nextafter(next, inf));
    require_exception<std::invalid_argument>(
        [&] {
            (void)cfd::mesh::grading::generate_faces_1d(
                two_ulp_domain, 2, GradingType::Uniform, 1.0);
        },
        "Expected unrepresentable cell centers to throw std::invalid_argument");
}

void check_contracts()
{
    const Domain1D domain(-2.5, 5.5);
    double maximum_symmetry_error = 0.0;
    for (const int n_cells : {10, 11}) {
        for (const GradingType type : grading_types) {
            check_generated_geometry(domain, type, n_cells, 1.8,
                                     maximum_symmetry_error);
            check_unit_ratio(domain, type, n_cells);
        }
    }
    check_invalid_inputs();
    std::cout << "Maximum symmetric-face distance error: " << std::scientific
              << std::setprecision(3) << maximum_symmetry_error
              << " (tolerance " << 1e-12 * domain.length() << ")\n";
}

}  // namespace

int main()
{
    try {
        check_contracts();
    }
    catch (const std::exception& error) {
        std::cerr << "1D grading contract check failed: " << error.what() << '\n';
        return 1;
    }

    std::cout << "1D grading contract checks passed\n";
    return 0;
}
