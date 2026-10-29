
# C++ Numeric Types & Literal Suffixes Cheatsheet

A precise technical guide to fundamental numeric types, type anatomy, and literal suffixes in Modern C++ (C++17/20), configured for the LP64 data model on Linux x86_64 (`g++`).

---

## 1. The Core Philosophy: Container vs. Literal

The single biggest source of confusion in numeric declarations is mixing up two fundamentally distinct concepts:

1. **The Container Type (Variable Declaration):** The data type of the storage location in memory, constructed via keywords and modifiers (`unsigned long long int`).
2. **The Literal (The Raw Value in Code):** The raw number written directly in source code (`100ULL` or `3.14f`), whose type is inferred by the compiler or explicitly dictated via a **Literal Suffix**.

> **Golden Rule:** Suffixes (`u`, `l`, `ll`, `f`, etc.) **never** attach to variable names or type definitions. They attach exclusively to raw literal values in source code to override the compiler's default assumptions.

---

## 2. The Golden Formula of C++ Type Anatomy

In C++, variable types are not arbitrary strings; they follow a strict taxonomic formula:

$$\text{[Sign Modifier]} + \text{[Width Modifier]} + \text{[Base Type]}$$

### Integer Anatomy
* **Base Type:** Only `int` (and `char` for 1-byte integral units).
* **Sign Modifiers:** `signed` (default) or `unsigned`.
* **Width Modifiers:** `short`, `long`, or `long long`.

Because C++ allows shorthand alias forms by dropping the default `signed` and the implied base `int`, we get:

| Canonical (Official) Name | Shorthand In Code | What Was Dropped? | Value Range (Linux x86_64 / GCC LP64) |
| :--- | :--- | :--- | :--- |
| `signed int` | `int` | `signed` is implicit default | [-2,147,483,648 to 2,147,483,647] (~ -2.14 x 10^9 to +2.14 x 10^9) |
| `unsigned int` | `unsigned` or `unsigned int` | `int` can be omitted | [0 to 4,294,967,295] (~ 0 to 4.29 x 10^9) |
| `signed short int` | `short` | Both `signed` and `int` dropped | [-32,768 to 32,767] |
| `unsigned short int` | `unsigned short` | `int` dropped | [0 to 65,535] |
| `signed long int` | `long` | Both `signed` and `int` dropped | [-9,223,372,036,854,775,808 to 9,223,372,036,854,775,807] (~ -9.22 x 10^18 to +9.22 x 10^18) |
| `unsigned long int` | `unsigned long` | `int` dropped | [0 to 18,446,744,073,709,551,615] (~ 0 to 1.84 x 10^19) |
| `signed long long int` | `long long` | Both `signed` and `int` dropped | [-9,223,372,036,854,775,808 to 9,223,372,036,854,775,807] |
| `unsigned long long int` | `unsigned long long` | `int` dropped | [0 to 18,446,744,073,709,551,615] |

> **Key Takeaway:** There is no distinct fundamental type named `long long` by itself—it is technically the width modifier `long long` modifying the base `int`. Writing `long long` is 100% valid and universally accepted standard practice.

---

### Floating-Point Anatomy
Floating-point numbers do **not** accept `signed` or `unsigned` modifiers because the sign bit is an intrinsic, permanent component of the IEEE 754 hardware standard.

* **Base Types:** `float` and `double` are two completely independent, distinct base types.
* **Width Modifier:** Only `long` is available, and it applies exclusively to `double`:

| Official Type Name | Shorthand | Size (Linux x86_64) | Precision | Approximate Normalized Positive Range |
| :--- | :--- | :---: | :--- | :--- |
| `float` | *(None - complete as is)* | 4 Bytes | Single Precision (~6–7 decimal digits) | ~ [1.18 x 10^-38 to 3.40 x 10^38] |
| `double` | *(None - complete as is)* | 8 Bytes | Double Precision (~15–17 decimal digits) | ~ [2.23 x 10^-308 to 1.79 x 10^308] |
| `long double` | *(None - both words required)* | 16 Bytes (GCC LP64) | Extended Precision (80-bit padded to 128-bit) | ~ [3.36 x 10^-4932 to 1.18 x 10^4932] (~18–19 decimal digits, x87 80-bit extended layout padded to 16 bytes) |

---

## 3. Literal Suffixes: Overriding Compiler Defaults

When a raw number appears in source code, the compiler applies strict default type assignments before any assignment or operation occurs:

* Any raw integer without a decimal point (e.g., `42`) defaults to **`int`**.
* Any raw floating-point number with a decimal point (e.g., `3.14`) defaults to **`double`**.

To prevent unwanted implicit conversions or truncation, attach literal suffixes to the value:

```cpp
auto a = 100;     // Type: int (4 bytes)
auto b = 100U;    // Type: unsigned int (4 bytes)
auto c = 100L;    // Type: long (8 bytes on Linux x86_64)
auto d = 100ULL;  // Type: unsigned long long (8 bytes)

auto x = 3.14;    // Type: double (8 bytes)
auto y = 3.14f;   // Type: float (4 bytes)
auto z = 3.14L;   // Type: long double (16 bytes on GCC x86_64)
```

> **Performance Note:** Writing `float pressure = 101325.0;` causes the compiler to treat `101325.0` as an 8-byte `double` and then convert/truncate it to a 4-byte `float`. Use `101325.0f` to instantiate directly as single precision.

---

## 4. Master Reference Table (Linux x86_64 / GCC LP64)

The definitive mapping of container types to hardware size and corresponding literal suffixes:

| Category | Canonical Container Type | Standard Code Shorthand | Size (Bytes) | Value / Dynamic Range (Linux x86_64 / GCC LP64) | Literal Suffix | Code Declaration & Literal Example |
| :--- | :--- | :--- | :---: | :--- | :---: | :--- |
| **Character / Byte** | `signed char` | `char` | 1 | [-128 to 127] | None | `char c = 'A';` |
| **Unsigned Byte** | `unsigned char` | `unsigned char` | 1 | [0 to 255] | None | `unsigned char b = 255;` |
| **Small Integer** | `signed short int` | `short` | 2 | [-32,768 to 32,767] | None *(auto-promoted)* | `short s = 32000;` |
| **Small Unsigned** | `unsigned short int` | `unsigned short` | 2 | [0 to 65,535] | None *(auto-promoted)* | `unsigned short us = 65000U;` |
| **Standard Integer** | `signed int` | `int` | 4 | [-2,147,483,648 to 2,147,483,647] (~ -2.14 x 10^9 to +2.14 x 10^9) | None *(default)* | `int count = -42;` |
| **Standard Unsigned** | `unsigned int` | `unsigned int` / `unsigned` | 4 | [0 to 4,294,967,295] (~ 0 to 4.29 x 10^9) | `u` / `U` | `unsigned int u = 42U;` |
| **Wide Integer** | `signed long int` | `long` | 8 | [-9,223,372,036,854,775,808 to 9,223,372,036,854,775,807] (~ -9.22 x 10^18 to +9.22 x 10^18) | `l` / `L` | `long idx = 1000000L;` |
| **Wide Unsigned** | `unsigned long int` | `unsigned long` | 8 | [0 to 18,446,744,073,709,551,615] (~ 0 to 1.84 x 10^19; std::size_t has this range) | `ul` / `UL` | `unsigned long n = 1000000UL;` |
| **Very Wide Integer** | `signed long long int` | `long long` | 8 | [-9,223,372,036,854,775,808 to 9,223,372,036,854,775,807] | `ll` / `LL` | `long long large = 9000000000LL;` |
| **Very Wide Unsigned**| `unsigned long long int`| `unsigned long long` | 8 | [0 to 18,446,744,073,709,551,615] | `ull` / `ULL` | `unsigned long long max = 18000000000ULL;` |
| **Single Precision** | `float` | `float` | 4 | ~ [1.18 x 10^-38 to 3.40 x 10^38] (~6-7 decimal digits) | `f` / `F` | `float dt = 0.001f;` |
| **Double Precision** | `double` | `double` | 8 | ~ [2.23 x 10^-308 to 1.79 x 10^308] (~15-17 decimal digits) | None *(default)* | `double viscosity = 1.002e-3;` |
| **Extended Precision**| `long double` | `long double` | 16 | ~ [3.36 x 10^-4932 to 1.18 x 10^4932] (~18-19 decimal digits, x87 80-bit extended layout padded to 16 bytes) | `l` / `L` | `long double exact = 3.141592653589793238L;` |

---

## 5. Engineering Guidelines for Scientific & CFD Codes

1. **Physical Field Variables ($U, V, W, P, T$, Fluxes):**
   * Use **`double`** exclusively. Single-precision `float` accumulates catastrophic round-off error over millions of iterative sweeps and non-linear solver cycles (e.g., SIMPLE / PISO loops).
   * Avoid unintended precision demotion: never append `f` to floating constants in double-precision equations (e.g., write `0.5 * rho * U * U`, never `0.5f * ...`).
   * When evaluating truncation/round-off errors in experimental solvers using `long double`, always explicitly suffix raw literals with `L` (e.g., `1.0e-12L`), otherwise the compiler defaults the constant to a 64-bit `double` before assigning.

2. **Mesh Indices, Cell Counts, and Addressing:**
   * Prefer **`std::size_t`** (an alias for `unsigned long` on 64-bit Linux LP64) for array indices, global cell IDs, face addressing, and linear algebra matrix offsets.
   * **Prevent 32-bit Mesh Overflow:** When multiplying grid dimensions to calculate global volume/cell buffers ($N_x \times N_y \times N_z$), raw numeric literals must use the `UL` or `ULL` suffix:
     ```cpp
     // DANGEROUS: Evaluates as 32-bit signed int, causing catastrophic overflow (> 2.14B cells)
     std::size_t total_cells = 3000 * 3000 * 3000; 

     // ROBUST: Promotes the entire arithmetic expression to 64-bit unsigned arithmetic
     std::size_t total_cells = 3000UL * 3000UL * 3000UL;
     ```

3. **Template Deductions & Limiters (Minmod, Superbee, Courant Number):**
   * Algorithms involving flux limiters or adaptive time-stepping (`std::min`, `std::max`, `std::clamp`) require strict matching of operand types.
   * Suffix literals to prevent compilation template deduction failures:
     ```cpp
     std::size_t iterations = 50;
     auto max_iter = std::min(iterations, 100UL); // Matches std::size_t exactly
     ```

4. **Conditionals & Loop Counters (Readability vs. Over-engineering):**
   * For plain loop iteration and simple assertion/precondition checks against small numbers (e.g., checking minimum required boundary layers or grading resolution), keep the code clean and free of visual noise:
     ```cpp
     if (n_cells < 2) { ... } // Idiomatic, clean, and zero-cost on x86_64
     ```
   * The compiler's optimizer generates identical assembly (`cmp` immediate) regardless of whether `2`, `2U`, or `2UL` is used. Reserve suffixes for expressions where overflow or template deduction is genuinely at stake.

5. **Scientific Literals & Vector Math:**
   * Write tiny or massive physical constants using exponential scientific notation directly as standard `double`: write `1.0e-6` rather than `0.000001`, and `1.0e5` instead of `100000.0`.
   * Ensure division expressions preserve floating-point semantics: write `1.0 / 3.0` (or `1. / 3.`), never `1 / 3` which yields integer zero.