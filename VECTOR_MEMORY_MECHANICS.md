
# std::vector Memory Mechanics & Allocation Guidelines for High-Performance CFD

> **Scope:** Guidelines for container memory management, heap allocations, and zero-overhead data structures in scientific computing and Computational Fluid Dynamics (CFD) using Modern C++ (C++17/20, Linux x86_64, GCC).

---

## 1. Executive Summary & The Golden Rule

In high-performance CFD simulations, `std::vector<double>` is the primary building block for spatial discretizations (cell volumes, face centroids, flux fields, and pressure-velocity couplings). A naive mental model treats a vector as an automatically resizing array; a hardware-aware engineer treats it as a **strict, contiguous block of heap memory managed by three raw pointers**.

```
+-----------------------------------------------------------------------+
|                           THE GOLDEN RULE                             |
|                                                                       |
|  * SIZE means LIFE:      The number of constructed, active objects.   |
|  * CAPACITY means LAND:  The total allocated heap buffer.             |
|                                                                       |
|  -> Indexing (v[i]) is ONLY legal within [0, size - 1].               |
|  -> push_back() ALWAYS appends at index == size, NEVER inside holes.  |
+-----------------------------------------------------------------------+
```

---

## 2. Low-Level Memory Model: The Three-Pointer Architecture

On Linux x86_64 (LP64 data model), `sizeof(std::vector<T>)` is exactly **24 bytes** on the stack, regardless of how many millions of elements are stored on the heap. Internally (GCC `libstdc++`), it consists of:

```
Stack Memory (24 bytes)                Heap Memory (Contiguous Storage)
+-------------------------+            +-------+-------+-------+-------+-------+
|  _M_start               | ---------> |  [0]  |  [1]  |  [2]  |  ...  |  ...  |
+-------------------------+            +-------+-------+-------+-------+-------+
|  _M_finish              | ---------------------------------> | (end) |       |
+-------------------------+                                    +-------+-------+
|  _M_end_of_storage      | -------------------------------------------------> |
+-------------------------+

* size()     = _M_finish - _M_start
* capacity() = _M_end_of_storage - _M_start
```

- **`size()`**: Number of valid, alive, accessible elements.
- **`capacity()`**: Total elements that can fit before the memory manager is forced to allocate a new heap block.

---

## 3. The Big Three: `resize()`, `reserve()`, and `assign()`

| Operation | Affects `size`? | Affects `capacity`? | Value-Initializes Elements? | Direct Indexing `v[i]` Legal? | `push_back()` Appends At: | Primary CFD Use-Case |
| :--- | :---: | :---: | :---: | :---: | :---: | :--- |
| **`resize(N)`** | **Yes** ($\to N$) | If $N > \text{cap}$, grows | **Yes** (zeroes for `double`) | **Yes** for $0 \le i < N$ | Index $N$ | Known mesh topology (Cells, Faces, Volumes) |
| **`reserve(N)`** | **No** (unchanged) | If $N > \text{cap}$, grows | **No** (raw uninitialized bytes) | **No** (out of range!) | Current `size()` | Dynamic boundary cell filtering, sparse lists |
| **`assign(N, val)`** | **Yes** ($\to N$) | If $N > \text{cap}$, grows | **Yes** (overwrites all with `val`) | **Yes** for $0 \le i < N$ | Index $N$ | Timestep resets, field re-initialization |

---

### Detailed Operational Semantics

#### A. `resize(size_type count, const T& value = T())`
- **What it does:** Changes the number of active elements.
- **Expansion:** If `count > size()`, appends `(count - size())` new elements. For fundamental types like `double`, these are value-initialized to `0.0`. Previous elements are strictly preserved in their original indices.
- **Contraction:** If `count < size()`, destructs elements past `count`. Memory capacity is **not** released back to the OS.
- **Correct Pattern:**
  ```cpp
  // Mesh face coordinates: 1D mesh with N cells has (N + 1) faces
  std::vector<double> face_coords;
  face_coords.resize(n_cells + 1UL); // Immediately creates (n_cells + 1) zeroes

  // Safe to use random-access index assignment or front/back
  face_coords.front() = x_min;
  face_coords.back()  = x_max;
  for (std::size_t f = 1; f < n_cells; ++f) {
      face_coords[f] = calculate_face_x(f);
  }
  ```

#### B. `reserve(size_type new_cap)`
- **What it does:** Pre-allocates raw heap storage without constructing a single object.
- **Rules:** `size()` remains **strictly identical**. Reading or writing via `operator[]` into reserved, unconstructed space is **Undefined Behavior (UB)**.
- **Correct Pattern:**
  ```cpp
  // Extracting boundary cell indices (unknown count upfront, upper bounded by total cells)
  std::vector<CellId> inlet_cells;
  inlet_cells.reserve(mesh.n_boundary_cells()); // Allocates memory chunk, size is still 0

  for (CellId p = 0; p < mesh.n_cells(); ++p) {
      if (mesh.is_inlet(p)) {
          inlet_cells.push_back(p); // No reallocation occurs; fills pre-allocated slots
      }
  }
  ```

#### C. `assign(size_type count, const T& value)`
- **What it does:** Completely replaces existing contents with `count` copies of `value`.
- **Destructive nature:** It wipes all previous elements. If capacity is sufficient, it reuses existing heap memory without triggering a new `malloc`/heap allocation.
- **Correct Pattern:**
  ```cpp
  // Resetting linear system source terms or pressure corrections at timestep n+1
  std::vector<double> source_vector(n_cells, 100.0);

  // Next iteration: completely overwrite with zeroes
  source_vector.assign(n_cells, 0.0); // Size = n_cells, all elements = 0.0
  ```

---

## 4. Hardware Mechanics: What Reallocation ("Moving House") Costs

When elements are appended (`push_back` / `emplace_back`) beyond current `capacity()`, the vector cannot simply expand in place because adjacent virtual memory in the heap is likely occupied.

### The 4-Step Reallocation Pipeline:
1. **Acquire New Land:** The runtime allocates a larger contiguous memory chunk on the heap (GCC `libstdc++` uses a $2\times$ growth factor: $1 \to 2 \to 4 \to 8 \dots$).
2. **Move Old Elements:** Every existing element is copied or moved (`std::move_if_noexcept`) from the old buffer to the new buffer.
3. **Insert New Element:** The requested element is written at `_M_finish`.
4. **Demolish Old House:** The old buffer is deallocated (`free`), and all internal pointers are updated.

```
Old Heap:  [ 1.0 | 2.0 | 3.0 | 4.0 ] (FULL: size=4, cap=4)
                                     |
                User calls push_back(5.0)
                                     v
New Heap:  [ 1.0 | 2.0 | 3.0 | 4.0 | 5.0 |  .  |  .  |  .  ] (size=5, cap=8)
             ^--- moved from old ---^
Old buffer deallocated! Any pointers/iterators to old buffer are now INVALID.
```

### The Performance Penalty in CFD:
- **Cache Pollution (L1/L2 Thrashing):** Moving megabytes of scalar field arrays flushes useful CFD stencil coefficients out of the CPU cache.
- **Memory Fragmentation:** Constant allocations and deallocations strain the kernel memory allocator (`ptmalloc`).
- **Pipeline Stalls:** Arithmetic logic units (ALU) sit idle waiting for memory bus round-trips.

---

## 5. Critical Pitfalls & Anti-Patterns

### Anti-Pattern 1: The "Accidental Inflation" (`resize` + `push_back`)
*Mistake:* Resizing to $N$, then attempting to populate using `push_back`.
```cpp
// BUGGY CODE
std::vector<double> cell_volumes;
cell_volumes.resize(1000); // 1,000 zeros created! size = 1000, cap = 1000

for (CellId p = 0; p < 1000; ++p) {
    cell_volumes.push_back(compute_volume(p)); // Appends AFTER index 999!
}
// RESULT:
// size() is now 2000!
// Indices [0..999] are useless 0.0 values.
// Real data sits at indices [1000..1999].
// Triggered costly reallocations from 1000 -> 2000 capacity!
```
*Fix:*
```cpp
// CORRECT APPROACH A (Direct indexing):
std::vector<double> cell_volumes(n_cells); // Or .resize(n_cells)
for (CellId p = 0; p < n_cells; ++p) {
    cell_volumes[p] = compute_volume(p);
}

// CORRECT APPROACH B (Reserve & Append):
std::vector<double> cell_volumes;
cell_volumes.reserve(n_cells);
for (CellId p = 0; p < n_cells; ++p) {
    cell_volumes.push_back(compute_volume(p));
}
```

---

### Anti-Pattern 2: The "Ghost Memory" Trap (`reserve` + direct indexing)
*Mistake:* Reserving memory and immediately indexing via `[]`.
```cpp
// DANGEROUS / UNDEFINED BEHAVIOR
std::vector<double> pressure;
pressure.reserve(1000); // size = 0, capacity = 1000

pressure[0] = 101325.0; // UNDEFINED BEHAVIOR: Object not alive!
// pressure.size() is still 0! 
// pressure.back() or range-for loops will see ZERO elements.
// GDB or AddressSanitizer (ASan) will flag out-of-bounds container access.
```

---

### Anti-Pattern 3: Reallocation Inside the Time-Stepping Loop
*Mistake:* Instantiating or expanding vectors inside inner solver iterations.
```cpp
// SLOW PATTERN
for (int step = 0; step < total_timesteps; ++step) {
    std::vector<double> flux;
    for (FaceId f = 0; f < n_faces; ++f) {
        flux.push_back(compute_flux(f)); // Reallocates on every single timestep!
    }
    solve(flux);
}

// OPTIMAL PATTERN
std::vector<double> flux(n_faces); // Allocated ONCE outside the loop
for (int step = 0; step < total_timesteps; ++step) {
    for (FaceId f = 0; f < n_faces; ++f) {
        flux[f] = compute_flux(f);   // Zero heap allocations inside time-loop
    }
    solve(flux);
}
```

---

## 6. Decision Matrix for ApexFVM Developers

```
When setting up vector storage:

                    Are the number of elements known in advance?
                                   /           \
                                 YES            NO
                                 /                \
         Will you fill via indexing?          Can you estimate an upper bound?
              /               \                         /             \
            YES                NO                     YES              NO
            /                   \                     /                 \
  Use constructor            Use reserve()      Use reserve(max)    Use push_back()
  `v(N)` or `resize(N)`     + push_back()       + push_back()       (Accept amortized
  then assign `v[p] = ...`                                           2x growth)
```

---

## 7. Key Best Practices in Numerical Code

1. **Prefer Paren-Constructor or `resize` for Mesh Geometries:** Face coordinates, cell volumes, and centroid locations have known counts at mesh construction time. Always size them upfront and fill via direct indexing.
2. **Reserve Before Collecting:** If elements are selected conditionally (e.g., non-conforming interfaces, boundary patches, hanging nodes), calculate or estimate capacity and `reserve()` prior to the insertion loop.
3. **Use `assign` to Reset Uniform Fields:** When clearing and refilling scalar variables across timesteps, avoid `clear() + resize()`; use `v.assign(N, 0.0)` which guarantees the exact state in a single clear operation without deallocating buffer memory.
4. **Compile with Sanitize Flags During Development:**
   ```bash
   g++ -std=c++20 -O2 -fsanitize=address,undefined -Wall -Wextra ...
   ```
   AddressSanitizer (ASan) immediately catches illegal writes into reserved (but un-sized) vector buffers.

