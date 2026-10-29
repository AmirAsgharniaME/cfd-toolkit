# C++ Numeric Types Cheatsheet for CFD

A quick reference for GCC and Clang on 64-bit Linux (x86-64, LP64), using C++17 or C++20. Byte sizes and ranges in the tables are for this target; the C++ standard leaves several type widths implementation-defined, so use `sizeof`, `<limits>`, and fixed-width types when portability matters.

## 1. Architecture Overview: LP64 on 64-bit Linux

| Data model / target | `int` | `long` | Pointer | Typical `std::size_t` |
|---|---:|---:|---:|---:|
| 32-bit Linux (ILP32) | 4 bytes | 4 bytes | 4 bytes | 4 bytes |
| 64-bit Linux x86-64 (LP64) | 4 bytes | 8 bytes | 8 bytes | 8 bytes |
| 64-bit Windows (LLP64) | 4 bytes | 4 bytes | 8 bytes | 8 bytes |

LP64 means `long` and pointers are 64-bit; LLP64 keeps `long` 32-bit and widens pointers. `std::size_t` is an unsigned type suitable for object sizes and indexing; its width follows the platform's addressable object model in these environments. Do not use `long` as a portable synonym for a 64-bit integer: use `std::int64_t` / `std::uint64_t` when the exact width is part of an interface or file format.

## 2. Taxonomy: Base Types and Modifiers

The fundamental integer type families are `bool`, `char`, `wchar_t`, `char8_t` (C++20), `char16_t`, `char32_t`, and the signed/unsigned integer types based on `short`, `int`, `long`, and `long long`. The common floating types are `float`, `double`, and `long double`.

- **Base type** names the family, such as `int` or `double`.
- **Sign modifiers**: `signed` and `unsigned` apply to integer types. Plain `char` is a distinct type whose signedness is implementation-defined; on the stated GCC/Clang x86-64 target it is normally signed.
- **Width modifiers**: `short`, `long`, and `long long` modify `int` (the `int` may be omitted: `long` means `long int`). `long` also modifies `double` to form `long double`.
- `signed`/`unsigned` cannot modify floating-point types. `short` and `long long` do not modify `float` or `double`.
- `char` has `signed char` and `unsigned char` variants, but `short char` and `long char` are not types.
- Integer types narrower than `int` (notably `char` and `short`) are generally promoted to `int` or `unsigned int` before arithmetic.

## 3. Master Reference Tables

### Integer types

Ranges shown are exact for the stated Linux x86-64 target, using the conventional two's-complement ranges for signed types. Sizes are object sizes; padding bits are not expected for these target types.

| Base type | Full semantic type | Size (bytes / bits) | Exact value range | Valid literal suffixes | No-suffix / invalid suffix notes |
|---|---|---:|---|---|---|
| `char` | `char` | 1 / 8 | normally −128 to 127 on target (plain `char` signedness is implementation-defined) | none | Character literals such as `'A'` are `int` in ordinary narrow form; no `c` suffix |
| `signed char` | `signed char` | 1 / 8 | −128 to 127 | none | No `s` suffix |
| `unsigned char` | `unsigned char` | 1 / 8 | 0 to 255 | `u`/`U` can select an unsigned integer literal, but cannot force `unsigned char` | No `uc` suffix; convert/store explicitly |
| `short` | `short int` | 2 / 16 | −32,768 to 32,767 | none specific; `u` can make an unsigned literal, not a short literal | No `s` suffix. `short x = 1;` initializes from `int` |
| `unsigned short` | `unsigned short int` | 2 / 16 | 0 to 65,535 | `u`/`U` selects unsigned integer candidate type, not specifically `unsigned short` | No `us` suffix |
| `int` | `int` | 4 / 32 | −2,147,483,648 to 2,147,483,647 | none | Unsuffixed decimal integer is `int` when representable |
| `unsigned int` | `unsigned int` | 4 / 32 | 0 to 4,294,967,295 | `u`/`U` | No suffix is guaranteed to mean unsigned; non-decimal literals have additional candidate rules |
| `long` | `long int` | 8 / 64 | −9,223,372,036,854,775,808 to 9,223,372,036,854,775,807 | `l`/`L` | `L` is the long suffix; it does not mean “long long” |
| `unsigned long` | `unsigned long int` | 8 / 64 | 0 to 18,446,744,073,709,551,615 | `ul`, `lu` (any case, e.g. `UL`, `lU`) | `u` alone does not request `unsigned long` specifically |
| `long long` | `long long int` | 8 / 64 | −9,223,372,036,854,775,808 to 9,223,372,036,854,775,807 | `ll`/`LL` | `ll` is two L characters; mixed-case `lL`/`Ll` is not a valid long-long suffix |
| `unsigned long long` | `unsigned long long int` | 8 / 64 | 0 to 18,446,744,073,709,551,615 | `ull`, `llu` (case variants, but the two Ls must match case) | No `s`, `short`, or `i64` standard suffix |

For integer literals, suffix letters are case-insensitive except that the two letters of the `ll` suffix must both be lowercase or both uppercase. `u` and `l` may appear in either order (`UL`/`LU`); likewise for `u` and `ll` (`ULL`/`LLU`). A suffix chooses a candidate type list, then the first type that represents the literal value is used. Thus the suffix does not always force exactly one type. In C++17/20, `z`/`uz` are **not** standard size suffixes; those arrive in C++23. Prefer `std::size_t{...}` for values in code rather than relying on a size literal suffix.

Fixed-width types such as `std::int32_t` and `std::uint64_t` are provided by `<cstdint>` when the implementation has an integer type of exactly that width. Their literal suffixes remain the suffixes of their underlying built-in types; there are no `i32` or `u64` standard suffixes in C++17/20.

### Floating-point types

The following are the usual GCC/Clang x86-64 Linux representations. `long double` ABI/format varies by target and compiler options.

| Type | Precision classification | Size (bytes / bits) | Approx. decimal precision | Approx. finite range | Literal suffixes |
|---|---|---:|---:|---|---|
| `float` | IEEE 754 binary32 | 4 / 32 | 6–9 significant decimal digits (about 7) | max ≈ `3.4028235e38`; min positive normal ≈ `1.1754944e-38` | `f` or `F`; without suffix the literal is `double` then conversion may occur |
| `double` | IEEE 754 binary64 | 8 / 64 | 15–17 significant decimal digits (about 16) | max ≈ `1.7976931e308`; min positive normal ≈ `2.2250739e-308` | no suffix (default); `d` is not a standard suffix |
| `long double` | x86 extended precision (80-bit format stored in 16-byte object) | 16 / 128 storage (typically 80 value bits) | about 18–19 significant decimal digits | max ≈ `1.1897315e4932`; min positive normal ≈ `3.3621031e-4932` | `l` or `L`; the literal has type `long double` |

“Min positive normal” excludes subnormal values. Subnormals may extend the range closer to zero with reduced precision. Query `std::numeric_limits<T>::min()`, `lowest()`, `max()`, `epsilon()`, `digits10`, and `max_digits10` for the actual build. `sizeof(long double) == 16` does not mean 128 bits of precision: the common x86 format stores an 80-bit extended value with padding. Some platforms make `long double` equivalent to `double`.

## 4. Practical Literal Rules and Caveats

### Defaults and suffixes

- An unsuffixed integer literal is selected from `int`, `long`, and `long long` (with candidate selection also depending on base); for a small ordinary decimal value such as `42`, its type is `int`.
- An unsuffixed floating literal such as `0.1` or `1e-6` has type `double`. Append `f`/`F` for `float`; append `l`/`L` for `long double`.
- Integer suffixes describe the literal expression, not the destination variable. `1000U` is unsigned int where representable; assigning it to `short` still converts to `short`.
- A cast or list initialization can make conversions visible: `float a{0.1};` is ill-formed due to narrowing, while `float a = 0.1;` is allowed but converts a `double` value to `float`.
- For large decimal integers, make the intended type explicit (`1'000'000'000LL`, `1ULL << 40`) and ensure the literal itself is representable before using it.

### Arithmetic pitfalls

```cpp
int q = 1 / 2;             // 0: integer division, then assigned to int
int half = 1.0 / 2.0;       // 0: floating result is converted to int
float a = 1.0f / 2.0f;     // 0.5f: float arithmetic
float b = 1.0 / 2.0;       // 0.5 converted from double; usually harmless, sometimes needless
```

A suffix mismatch does not universally cause a bug: usual arithmetic conversions choose a common operand type, then the result converts to the destination. Bugs arise when that conversion loses range or precision, or when integer arithmetic happens before conversion. For example, `float ratio = count / total;` performs integer division if both operands are integral. Use `static_cast<double>(count) / total` or otherwise make one operand floating before division.

Unsigned arithmetic wraps modulo 2^N; signed overflow is undefined behavior. Mixing signed and unsigned operands can convert a negative signed value to a large unsigned value. Avoid casual signed/unsigned comparisons, and do not use an unsigned index if decrementing below zero is meaningful.

## 5. Practical CFD and HPC Guidelines (ApexFVM / OpenFOAM Context)

### a. Physical fields: velocity, pressure, temperature, density

- Use a clear project-wide field scalar alias, commonly `double` for CFD solvers where numerical accuracy is prioritized. OpenFOAM commonly calls its configurable scalar type `scalar`; ApexFVM should make its own precision choice explicit rather than assume OpenFOAM's build configuration.
- Store velocity components (`U`, `V`, `W`), pressure (`P`), temperature (`T`), and density in the same scalar precision unless there is a measured reason to use mixed precision. Keep units and reference/scaling choices in the field documentation.
- `float` halves scalar storage and memory traffic versus `double`, which can help bandwidth-bound workloads and GPU memory, but reduces mantissa precision and dynamic range. Validate stability, conservation, and solver convergence before adopting it.
- Use `long double` selectively for diagnostics or sensitive reductions only after measuring: its performance and representation vary, and it is not a portable serialized format.

### b. Cell, face, and node indexing

- **Local arrays and container sizes:** use `std::size_t` for `vector.size()`, allocation counts, and nonnegative container indexing. Check conversions at API boundaries; subtracting unsigned indices can underflow.
- **Bounded local indices:** use `std::uint32_t` or `std::int32_t` only when the bound and storage format are deliberately fixed and validated. Plain `int` (32-bit here) is often convenient for loop counters, but cannot index more than `INT_MAX` entries safely.
- **Massive/distributed global IDs:** use `std::uint64_t` for nonnegative global cell/face/node identifiers when counts can exceed 2^31−1; use `std::int64_t` if negative sentinel values are genuinely required. Define MPI counts/displacements and serialization widths explicitly, since a 64-bit ID does not imply every MPI API count is 64-bit.
- Avoid storing pointers in integer types. Use pointer types or `std::uintptr_t` only for low-level address work.

### c. Boundary tags and small flags

- Use `enum class` with a deliberate underlying type (often `std::uint8_t`) for a compact, stable boundary-condition tag when the set of values is small. Validate incoming serialized values before converting them to the enum.
- `bool` is clear for in-memory flags, but `std::vector<bool>` is a packed proxy specialization and behaves differently from ordinary vectors. For dense flags, compare `std::vector<std::uint8_t>` (one byte per flag) with bitsets/bit vectors based on workload and memory needs.
- A typical CPU cache line is 64 bytes, so 64 one-byte tags occupy approximately one cache line before alignment/layout effects; 64 32-bit flags occupy 256 bytes. Structure-of-arrays layouts can help when kernels consume one property across many cells. Measure actual cache behavior and padding rather than assuming a particular struct layout.

### d. Accumulation error and iterative solver convergence

- Floating-point addition is not associative. Summing cell contributions in different thread or MPI reduction orders can change low-order bits and, near tolerance thresholds, the iteration count or stopping decision.
- Use `double` for residual norms, dot products, mass/energy balances, and global reductions even when field storage is `float`, unless tests establish adequate behavior otherwise. Consider pairwise/tree or compensated summation for long or cancellation-heavy sums.
- CG requires a symmetric positive-definite operator in exact arithmetic; BiCGStab is used for broader nonsymmetric systems. Roundoff, preconditioning, scaling, boundary treatment, and stopping criteria all affect observed convergence. Track both absolute and relative residuals, use consistent norms, and check conservation/physical residuals alongside algebraic residuals.
- SIMPLE/PISO pressure–velocity coupling and repeated sparse solves can amplify small numerical differences. Set tolerances relative to problem scale, avoid demanding precision beyond discretization/model error, and compare convergence and physical outputs when changing precision, compiler, parallel reduction strategy, or solver.

---

### Quick checks in code

```cpp
#include <cstddef>
#include <cstdint>
#include <limits>

static_assert(sizeof(void*) == 8);  // expected for this target, not portable
static_assert(sizeof(long) == 8);   // LP64; false on 64-bit Windows LLP64

using Scalar = double;
using GlobalCellId = std::uint64_t;

constexpr auto scalar_digits = std::numeric_limits<Scalar>::digits;
constexpr auto scalar_max = std::numeric_limits<Scalar>::max();
```

For a reproducible build record, inspect compiler macros and `std::numeric_limits` on the actual compiler, target flags, and runtime environment.

## References

- [C++ working draft: literals](https://eel.is/c++draft/lex.literal) — integer and floating literal suffix rules.
- [GCC documentation: x86 data models](https://gcc.gnu.org/onlinedocs/gcc-15.2.0/gcc.pdf) — LP64 model description.
- [Microsoft: Abstract Data Models](https://learn.microsoft.com/en-us/windows/win32/winprog64/abstract-data-models) — ILP32 and LLP64 comparison.
- [OpenFOAM scalar forward declarations](https://api.openfoam.com/2412/scalarFwd_8H.html) — `scalar` precision depends on build precision macros.
