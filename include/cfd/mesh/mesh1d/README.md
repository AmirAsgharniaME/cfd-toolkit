# 1D Mesh Geometry

`FvMesh1D` is the concrete, copyable and movable mesh value type. Construct it
from an explicit face vector, or use `create_uniform(x_min, x_max, n)` and
`create_graded(x_min, x_max, n, ratio)`. The graded factory uses the Forward
distribution from `grading::make_1d_faces`; callers needing another
`GradingType` can call that helper and pass its result to the face constructor.

Construction validates finite, strictly increasing faces and a finite,
positive represented interval. It stores the face coordinates, cell centers,
and cell lengths (volumes assume unit cross-sectional area). A center must be
representable strictly inside its cell. Indexed geometry access checks bounds;
spans expose read-only views. Meshes own their geometry and do not track later
changes to any source domain.

The global bounds and represented extent are available through
`Start_Coord()`, `End_Coord()`, and `Length()`. The legacy `start()` and `end()`
aliases remain available for the bounds.

`grading1d.hpp` and `grading1d.cpp` provide the independent face-generation
functions `grading::make_1d_faces` and `grading::ratio_from_total_ratio`.
Fields bind to a mesh by object reference, so the mesh must outlive each field;
do not move or assign it while a field or view depends on it.
