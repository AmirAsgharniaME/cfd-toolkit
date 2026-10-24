# ScalarField1D example

`cfd::field::field1d::ScalarField1D` stores `double` values associated with a
`cfd::mesh::UniformMesh1D`. A field keeps a reference to its mesh, so the mesh
must remain alive for the entire lifetime of the field.

## Storage locations

The `cfd::field::FieldLocation` enum selects where values are stored:

- `FieldLocation::Cell` stores one value per mesh cell. This is the default.
- `FieldLocation::Face` stores one value per mesh face. A 1D mesh has one more
  face than cells.

All entries are initialized to `0.0` unless an initial value is supplied to the
constructor. For example:

```cpp
cfd::geometry::Domain1D domain(-1.0, 1.0);
cfd::mesh::UniformMesh1D mesh(domain, 4);

// Defaults to a cell field initialized to zero.
cfd::field::field1d::ScalarField1D temperature("temperature", mesh);

// A face field initialized to 2.0.
cfd::field::field1d::ScalarField1D face_flux(
    "face_flux", mesh, cfd::field::FieldLocation::Face, 2.0);
```

`name()`, `mesh()`, `location()`, and `size()` report the field metadata and
number of stored entries. `operator[]` accesses entries without bounds
checking; `at()` checks the index and throws `std::out_of_range` when it is
outside the field. `raw_data()` provides direct vector access, and `fill(value)`
assigns the same value to every entry.

## Arithmetic

Fields support entry-wise `+`, `-`, `+=`, and `-=`, and scalar multiplication
and division using `*`, `/`, `*=`, and `/=`. Field addition and subtraction
require both fields to refer to the same mesh object and have the same
`FieldLocation` and size. A mismatch throws `std::invalid_argument`. Division
by `0.0` also throws `std::invalid_argument`.

## Build and run the example

From the repository root, configure and build the example target:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --target test_scalar_field1d
./build/bin/test_scalar_field1d
```

The executable checks cell and face storage, initial values, field and scalar
arithmetic, bounds checking, incompatible field addition, and division by zero.
It prints a success message and exits with status `0` if all checks pass; a
failed check prints an error and exits with status `1`.
