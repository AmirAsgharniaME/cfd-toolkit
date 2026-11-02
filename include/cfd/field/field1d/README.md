# 1D fields

`Field1D<Type, LocationTag>` stores `double` or `Vector1D` values at mesh
cells or faces. The aliases `VolScalarField1D`, `SurfaceScalarField1D`,
`VolVectorField1D`, and `SurfaceVectorField1D` cover these combinations.
Include `field1d.hpp` to use the fields and `field1d_operators.hpp` for
non-member binary arithmetic operators.

Construct a field with a name, an `FvMesh1D` reference, and an optional initial
value. The mesh must outlive the field and remain unmoved while the field or
its views exist. Non-const temporary meshes are rejected. `name()`, `mesh()`,
`size()`, and `values()` expose the field, and `operator[]` and `at()` provide
unchecked and checked access, respectively.

Copy and move construction preserve the mesh binding. Assignment and field
arithmetic require the same mesh object; arithmetic also checks storage size.
Both value types support field addition and subtraction and scaling by a
scalar. Scalar fields also support elementwise multiplication and division.
Division by zero throws `std::invalid_argument`.
