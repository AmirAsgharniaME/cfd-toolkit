#include <cmath>
#include <cstddef>
#include <iostream>
#include <span>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

#include <cfd/field/field1d/field1d.hpp>
#include <cfd/field/field1d/field1d_operators.hpp>
#include <cfd/geometry/domain/domain1d.hpp>
#include <cfd/mesh/mesh1d/fv_mesh1d.hpp>
#include <cfd/vector/vector1d.hpp>

namespace
{

template<class L, class R>
concept Addable = requires(L lhs, R rhs) { lhs + rhs; };

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

        // Volume fields allocate one value per cell.
        cfd::field::Field1D<double, cfd::field::CellLocation> temperature("temperature", mesh);
        require(temperature.size() == mesh.N_Cells(),
                "A cell field should have one entry per mesh cell.");
        require_near(temperature[0], 0.0, "The default initial value should be zero.");
        require(temperature.Name() == "temperature", "The field name should be retained.");

        // Surface fields contain one value per mesh face.
        cfd::field::SurfaceScalarField1D face_flux("face_flux", mesh, 2.0);
        require(face_flux.size() == mesh.N_Faces(),
                "A face field should have one entry per mesh face.");
        require_near(face_flux[0], 2.0, "The supplied initial value should fill the face field.");

        temperature.Fill(3.0);
        temperature[1] = 4.0;
        require_near(temperature.at(1), 4.0, "Indexed writes should update field values.");

        const auto shifted = temperature + temperature;
        require_near(shifted[0], 6.0, "Field addition should operate entry by entry.");
        const auto difference = shifted - temperature;
        require_near(difference[1], 4.0, "Field subtraction should operate entry by entry.");

        cfd::field::VolScalarField1D factors("factors", mesh, 2.0);
        const auto product = temperature * factors;
        require_near(product[1], 8.0, "Scalar field multiplication should operate entry by entry.");
        const auto quotient = product / factors;
        require_near(quotient[1], 4.0, "Scalar field division should operate entry by entry.");

        const auto scaled = 2.0 * temperature;
        require_near(scaled[1], 8.0, "Scalar multiplication should scale every entry.");
        const auto divided = scaled / 2.0;
        require_near(divided[1], 4.0, "Scalar division should scale every entry.");

        cfd::field::VolScalarField1D scalar_copy(temperature);
        require(&scalar_copy.Mesh() == &mesh && scalar_copy.Name() == temperature.Name(),
                "Copy construction should preserve the mesh reference and field name.");
        cfd::field::VolScalarField1D scalar_moved(std::move(scalar_copy));
        require(&scalar_moved.Mesh() == &mesh && scalar_moved.size() == temperature.size(),
                "Move construction should preserve the mesh reference and stored values.");
        cfd::field::VolScalarField1D scalar_assigned("assigned", mesh);
        scalar_assigned = temperature;
        require(scalar_assigned.Name() == temperature.Name() && scalar_assigned[1] == temperature[1],
                "Copy assignment should copy the name and values.");
        scalar_assigned = std::move(scalar_moved);
        require(scalar_assigned.Name() == temperature.Name() && scalar_assigned[1] == temperature[1],
                "Move assignment should copy the name and transfer the values.");

        static_assert(std::is_copy_assignable_v<cfd::field::VolScalarField1D>);
        static_assert(std::is_move_assignable_v<cfd::field::VolScalarField1D>);
        using Field = cfd::field::VolScalarField1D;
        using Mesh = cfd::mesh::FvMesh1D;
        static_assert(std::is_constructible_v<Field, std::string, Mesh&>);
        static_assert(!std::is_constructible_v<Field, std::string, Mesh&&>);

        using FaceField = cfd::field::SurfaceScalarField1D;
        static_assert(std::is_same_v<Field, cfd::field::Field1D<double, cfd::field::CellLocation>>);
        static_assert(std::is_same_v<FaceField, cfd::field::Field1D<double, cfd::field::FaceLocation>>);
        static_assert(!std::is_constructible_v<FaceField, std::string, Mesh&&>);
        static_assert(!Addable<Field, FaceField>);
        static_assert(!std::is_assignable_v<Field&, FaceField>);
        const auto face_sum = face_flux + face_flux;
        require(face_sum.N_Faces() == mesh.N_Faces(), "Face count should be retained.");
        require_near(face_sum.at(0), 4.0, "Face field arithmetic should work.");

        using VolVectorField = cfd::field::VolVectorField1D;
        using SurfaceVectorField = cfd::field::SurfaceVectorField1D;
        static_assert(std::is_same_v<VolVectorField,
                                     cfd::field::Field1D<cfd::vector::Vector1D, cfd::field::CellLocation>>);
        static_assert(std::is_same_v<SurfaceVectorField,
                                     cfd::field::Field1D<cfd::vector::Vector1D, cfd::field::FaceLocation>>);
        static_assert(!Addable<VolVectorField, FaceField>);

        VolVectorField cell_velocity("cell_velocity", mesh, cfd::vector::Vector1D{1.5});
        require(cell_velocity.size() == mesh.N_Cells(),
                "A vector cell field should allocate one value per cell.");
        require(cell_velocity[0] == cfd::vector::Vector1D{1.5},
                "The vector cell field should retain its initial value.");
        cell_velocity.Fill(cfd::vector::Vector1D{2.0});
        cell_velocity[1] = cfd::vector::Vector1D{3.0};
        const auto vector_sum = cell_velocity + cell_velocity;
        require(vector_sum[1] == cfd::vector::Vector1D{6.0},
                "Vector field addition should operate entry by entry.");
        const auto vector_difference = vector_sum - cell_velocity;
        require(vector_difference.at(1) == cfd::vector::Vector1D{3.0},
                "Vector field subtraction should operate entry by entry.");
        const auto vector_scaled = cell_velocity * 2.0;
        require(vector_scaled[1] == cfd::vector::Vector1D{6.0},
                "Vector field scalar multiplication should scale each component.");
        const auto vector_divided = vector_scaled / 2.0;
        require(vector_divided[1] == cfd::vector::Vector1D{3.0},
                "Vector field scalar division should scale each component.");

        SurfaceVectorField face_velocity("face_velocity", mesh, cfd::vector::Vector1D{4.0});
        require(face_velocity.size() == mesh.N_Faces() && face_velocity.N_Faces() == mesh.N_Faces(),
                "A vector face field should expose one value per face.");
        require(face_velocity[mesh.N_Faces() - 1] == cfd::vector::Vector1D{4.0},
                "The vector face field should initialize every face.");
        face_velocity.Fill(cfd::vector::Vector1D{5.0});
        require((face_velocity + face_velocity).at(0) == cfd::vector::Vector1D{10.0},
                "Vector face field arithmetic should work.");

        auto vector_view = cell_velocity.values();
        vector_view[2] = cfd::vector::Vector1D{7.0};
        require(cell_velocity[2] == cfd::vector::Vector1D{7.0},
                "Vector values() should alias the field storage.");
        const auto& const_cell_velocity = cell_velocity;
        const std::span<const cfd::vector::Vector1D> const_vector_span = const_cell_velocity.span();
        require(const_vector_span.size() == cell_velocity.size() &&
                    const_cell_velocity.Values()[2] == cfd::vector::Vector1D{7.0},
                "Const span accessors should expose read-only field storage.");

        const cfd::mesh::FvMesh1D graded(domain, 4, cfd::mesh::GradingType::Forward, 2.0);
        const cfd::field::VolScalarField1D graded_field("graded", graded);
        require(graded_field.size() == graded.N_Cells(),
                "A field should bind to a non-uniform base mesh.");
        require(&graded_field.Mesh() == &graded, "Mesh() should return the bound mesh.");

        const cfd::mesh::FvMesh1D twin(domain, 4);
        const cfd::field::VolScalarField1D twin_field("twin", twin);
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

        FaceField twin_face("twin face", twin);
        bool face_identity_checked = false;
        try
        {
            face_flux = twin_face;
        }
        catch (const std::invalid_argument&)
        {
            face_identity_checked = true;
        }
        require(face_identity_checked, "Face field assignment should check mesh identity.");

        auto view = temperature.Values();
        view[2] = 9.0;
        require_near(temperature[2], 9.0, "values() should alias the field storage.");
        require(view.size() == temperature.size(), "Values() should span all entries.");

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
            (void)(temperature + twin_field);
        }
        catch (const std::invalid_argument&)
        {
            size_checked = true;
        }
        require(size_checked, "Incompatible fields should throw.");

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

        factors[2] = 0.0;
        bool field_division_checked = false;
        try
        {
            temperature /= factors;
        }
        catch (const std::invalid_argument&)
        {
            field_division_checked = true;
        }
        require(field_division_checked, "Division by a field containing zero should throw.");

        std::cout << "Field1D example checks passed.\n"
                  << "  Cell entries: " << temperature.size() << '\n'
                  << "  Face entries: " << face_flux.size() << '\n'
                  << "  Vector cell entries: " << cell_velocity.size() << '\n'
                  << "  Vector face entries: " << face_velocity.size() << '\n';

        std::cin.get();
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Field1D example check failed: " << error.what() << '\n';
        return 1;
    }
}
