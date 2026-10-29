
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

| Canonical (Official) Name | Shorthand In Code | What Was Dropped? |
| :--- | :--- | :--- |
| `signed int` | `int` | `signed` is implicit default |
| `unsigned int` | `unsigned` or `unsigned int` | `int` can be omitted |
| `signed short int` | `short` | Both `signed` and `int` dropped |
| `unsigned short int` | `unsigned short` | `int` dropped |
| `signed long int` | `long` | Both `signed` and `int` dropped |
| `unsigned long int` | `unsigned long` | `int` dropped |
| `signed long long int` | `long long` | Both `signed` and `int` dropped |
| `unsigned long long int` | `unsigned long long` | `int` dropped |

> **Key Takeaway:** There is no distinct fundamental type named `long long` by itself—it is technically the width modifier `long long` modifying the base `int`. Writing `long long` is 100% valid and universally accepted standard practice.

---

### Floating-Point Anatomy
Floating-point numbers do **not** accept `signed` or `unsigned` modifiers because the sign bit is an intrinsic, permanent component of the IEEE 754 hardware standard.

* **Base Types:** `float` and `double` are two completely independent, distinct base types.
* **Width Modifier:** Only `long` is available, and it applies exclusively to `double`:

| Official Type Name | Shorthand | Size (Linux x86_64) | Precision |
| :--- | :--- | :---: | :--- |
| `float` | *(None - complete as is)* | 4 Bytes | Single Precision (~6–7 decimal digits) |
| `double` | *(None - complete as is)* | 8 Bytes | Double Precision (~15–17 decimal digits) |
| `long double` | *(None - both words required)* | 16 Bytes (GCC LP64) | Extended Precision (80-bit padded to 128-bit) |

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

| Category | Canonical Container Type | Standard Code Shorthand | Size (Bytes) | Literal Suffix | Code Declaration & Literal Example |
| :--- | :--- | :--- | :---: | :---: | :--- |
| **Character / Byte** | `signed char` | `char` | 1 | None | `char c = 'A';` |
| **Unsigned Byte** | `unsigned char` | `unsigned char` | 1 | None | `unsigned char b = 255;` |
| **Small Integer** | `signed short int` | `short` | 2 | None *(auto-promoted)* | `short s = 32000;` |
| **Small Unsigned** | `unsigned short int` | `unsigned short` | 2 | None *(auto-promoted)* | `unsigned short us = 65000U;` |
| **Standard Integer** | `signed int` | `int` | 4 | None *(default)* | `int count = -42;` |
| **Standard Unsigned** | `unsigned int` | `unsigned int` / `unsigned` | 4 | `u` / `U` | `unsigned int u = 42U;` |
| **Wide Integer** | `signed long int` | `long` | 8 | `l` / `L` | `long idx = 1000000L;` |
| **Wide Unsigned** | `unsigned long int` | `unsigned long` | 8 | `ul` / `UL` | `unsigned long n = 1000000UL;` |
| **Very Wide Integer** | `signed long long int` | `long long` | 8 | `ll` / `LL` | `long long large = 9000000000LL;` |
| **Very Wide Unsigned**| `unsigned long long int`| `unsigned long long` | 8 | `ull` / `ULL` | `unsigned long long max = 18000000000ULL;` |
| **Single Precision** | `float` | `float` | 4 | `f` / `F` | `float dt = 0.001f;` |
| **Double Precision** | `double` | `double` | 8 | None *(default)* | `double viscosity = 1.002e-3;` |
| **Extended Precision**| `long double` | `long double` | 16 | `l` / `L` | `long double exact = 3.141592653589793238L;` |

---

## 5. Engineering Guidelines for Scientific & CFD Codes

1. **Physical Field Variables ($U, V, W, P, T$):**
   * Use **`double`** exclusively. Single-precision `float` accumulates catastrophic round-off error over millions of iterative sweeps and non-linear solver cycles.
2. **Mesh Indices, Cell Counts, and Addressing:**
   * Prefer **`std::size_t`** (an alias for `unsigned long` on 64-bit Linux) for array indices, boundary cell counts, and memory sizing to prevent sign-comparison warnings and prevent 32-bit integer overflow on large grids.
3. **Scientific Literals:**
   * Write floating constants cleanly using exponential notation with standard `double` precision: `1.0e-5` rather than `0.00001`.
```