# field1d

`VolScalarField1D` is a scalar field sampled at either the cells or the faces
of a 1D finite-volume mesh (`FieldLocation::Cell` or `FieldLocation::Face`).
`ScalarField1D` is a compatibility alias.

- Binds by reference to the base `cfd::mesh::FvMesh1D`, so uniform and
  non-uniform meshes are both accepted. The mesh must outlive the field.
- Copy and move construction keep the same mesh binding; assignment is deleted.
- Storage is exposed as `values()` spans, so callers cannot resize it.
- An invalid `FieldLocation` throws `std::invalid_argument`.
- Field arithmetic requires the same mesh object (pointer identity; equal
  geometry is not enough), the same location, and the same size, otherwise
  `std::invalid_argument` is thrown.
- Scalar division by zero throws `std::invalid_argument`.
