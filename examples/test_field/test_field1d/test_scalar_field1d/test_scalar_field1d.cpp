#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>

#include <cfd/field/field1d/vol_scalar_field1d.hpp>
#include <cfd/geometry/domain/domain1d.hpp>
#include <cfd/mesh/mesh1d/fv_mesh1d.hpp>

#include <type_traits>

namespace
{

void require(bool condition, const std::string& message)
{
    if (!condition)
    {
        throw std::runtime_error(message);
    }
}

void require_near(double actual, double expected, const std::string& message)
{
    constexpr double tolerance = 1.0e-12;
    require(std::abs(actual - expected) <= tolerance, message);
}

}  // namespace

int main()
{
    try
    {
        const cfd::geometry::Domain1D domain(-1.0, 1.0);
        const cfd::mesh::FvMesh1D mesh(domain, 4);

        // Cell location is the default and allocates one value per cell.
        cfd::field::field1d::ScalarField1D temperature("temperature", mesh);
        require(temperature.location() == cfd::field::FieldLocation::Cell,
                "The default field location should be Cell.");
        require(temperature.size() == mesh.N_Cells(),
                "A cell field should have one entry per mesh cell.");
        require_near(temperature[0], 0.0, "The default initial value should be zero.");
        require(temperature.name() == "temperature", "The field name should be retained.");

        // Face fields contain one more value than cell fields in a 1D mesh.
        cfd::field::field1d::ScalarField1D face_flux(
            "face_flux", mesh, cfd::field::FieldLocation::Face, 2.0);
        require(face_flux.size() == mesh.N_Faces(),
                "A face field should have one entry per mesh face.");
        require(face_flux.location() == cfd::field::FieldLocation::Face,
                "The requested Face location should be retained.");
        require_near(face_flux[0], 2.0, "The supplied initial value should fill the face field.");

        temperature.fill(3.0);
        temperature[1] = 4.0;
        require_near(temperature.at(1), 4.0, "Indexed writes should update field values.");

        const auto shifted = temperature + temperature;
        require_near(shifted[0], 6.0, "Field addition should operate entry by entry.");
        const auto difference = shifted - temperature;
        require_near(difference[1], 4.0, "Field subtraction should operate entry by entry.");

        const auto scaled = 2.0 * temperature;
        require_near(scaled[1], 8.0, "Scalar multiplication should scale every entry.");
        const auto divided = scaled / 2.0;
        require_near(divided[1], 4.0, "Scalar division should scale every entry.");

        static_assert(!std::is_copy_assignable_v<cfd::field::field1d::VolScalarField1D>);
        static_assert(!std::is_move_assignable_v<cfd::field::field1d::VolScalarField1D>);

        const cfd::mesh::FvMesh1D graded(domain, 4, cfd::mesh::GradingType::Forward, 2.0);
        const cfd::field::field1d::VolScalarField1D graded_field("graded", graded);
        require(graded_field.size() == graded.N_Cells(),
                "A field should bind to a non-uniform base mesh.");
        require(&graded_field.mesh() == &graded, "mesh() should return the bound mesh.");

        const cfd::mesh::FvMesh1D twin(domain, 4);
        const cfd::field::field1d::VolScalarField1D twin_field("twin", twin);
        bool identity_checked = false;
        try
        {
            (void)(temperature + twin_field);
        }
        catch (const std::invalid_argument&)
        {
            identity_checked = true;
        }
        require(identity_checked, "Equal geometry on distinct meshes should still be rejected.");

        auto view = temperature.values();
        view[2] = 9.0;
        require_near(temperature[2], 9.0, "values() should alias the field storage.");
        require(view.size() == temperature.size(), "values() should span all entries.");

        bool location_checked = false;
        try
        {
            const cfd::field::field1d::VolScalarField1D bad(
                "bad", mesh, static_cast<cfd::field::FieldLocation>(7));
        }
        catch (const std::invalid_argument&)
        {
            location_checked = true;
        }
        require(location_checked, "An invalid field location should be rejected.");

        bool bounds_checked = false;
        try
        {
            (void)temperature.at(temperature.size());
        }
        catch (const std::out_of_range&)
        {
            bounds_checked = true;
        }
        require(bounds_checked, "at() should reject an index equal to size().");

        bool size_checked = false;
        try
        {
            temperature += face_flux;
        }
        catch (const std::invalid_argument&)
        {
            size_checked = true;
        }
        require(size_checked, "Adding fields with different sizes should throw.");

        bool division_checked = false;
        try
        {
            temperature /= 0.0;
        }
        catch (const std::invalid_argument&)
        {
            division_checked = true;
        }
        require(division_checked, "Division by zero should throw.");

        std::cout << "ScalarField1D example checks passed.\n"
                  << "  Cell entries: " << temperature.size() << '\n'
                  << "  Face entries: " << face_flux.size() << '\n';

        std::cin.get();
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "ScalarField1D example check failed: " << error.what() << '\n';
        return 1;
    }
}
