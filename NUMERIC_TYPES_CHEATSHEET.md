# C++ Numeric Types and Literal Suffixes

A practical guide for C++17/20 CFD development on Linux x86_64 with GCC. Sizes below describe the usual GCC LP64 target: `int` is 32 bits, `long` is 64 bits, pointers are 64 bits, and `long double` is typically stored in a 16-byte object. The C++ standard does not require all these sizes, so confirm them with `sizeof` and `<limits>` for the build you ship.

## 1. THE GOLDEN FORMULA OF C++ TYPES

For many integer types, think of the spelling as:

> **[Sign Modifier] + [Width Modifier] + [Base Type]**

For example, `unsigned long int` is `unsigned` (sign) + `long` (width) + `int` (base). Some parts can be omitted: `long` is shorthand for `long int`, and `unsigned long` means `unsigned long int`. This formula is a learning aid, not a rule that every C++ type must fit: `char` has its own variants, and floating-point types follow a separate taxonomy.

Keep the declared **container type** separate from the type of a **literal** used to initialize or calculate a value:

```cpp
unsigned long cell_count = 42UL;
// ^ container type             ^^ literal suffix
```

The declaration determines the type used to store `cell_count`. The literal is the raw numeric token `42UL` in the source code; its suffix helps determine the literal expression's type before any conversion to the variable's type. A suffix does not change the type of a variable.

## 2. COMPONENT COUNT & TAXONOMY

### Base types

- **`int`** is the base for the ordinary signed and unsigned integer families. The standard integer types also include `short int`, `long int`, and `long long int` forms.
- **`char`** is a distinct character type. It is not an alias for `signed char` or `unsigned char`; plain `char` has implementation-defined signedness. On common GCC x86_64 Linux builds it is signed.
- **`float`** and **`double`** are floating-point base types. `long double` is the extended-width form of `double`.

### Sign modifiers

There are **two** sign modifiers: `signed` and `unsigned`. They apply to integer types, including `char` (`signed char`, `unsigned char`) and the `int` family. Omitting the sign from `int` means signed `int`; omitting it from plain `char` leaves the implementation-defined `char` type. Sign modifiers do not apply to `float`, `double`, or `long double`.

### Width modifiers

There are **three** integer width modifiers: `short`, `long`, and `long long`. They apply to `int`, whose name can be omitted (`short int` → `short`, `long int` → `long`, `long long int` → `long long`). The standard guarantees the ordering `sizeof(short) <= sizeof(int) <= sizeof(long) <= sizeof(long long)`, not fixed byte counts. On the target described here, the usual sizes are 2, 4, 8, and 8 bytes respectively. `long` also combines with `double` to form `long double`; `short` and `long long` do not modify floating-point types.

### How combinations work

Valid integer spellings include `signed int`, `unsigned short int`, `long int`, `signed long int`, and `unsigned long long int`. Equivalent shorthand spellings include `int`, `unsigned short`, `long`, `long`, and `unsigned long long`. In particular, `signed long int` is commonly written `long`, because plain `long int` is signed by default.

Floating-point types form their own, separate taxonomy: **`float`, `double`, and `long double`**. They do not take `signed` or `unsigned`; `long double` is not an integer type with both modifiers and does not have an integer suffix.

## 3. LITERAL SUFFIXES

A literal suffix is a letter or group of letters written directly after a numeric literal, such as the `U` in `100U`, the `LL` in `100LL`, or the `f` in `0.25f`. Suffixes apply to raw numeric values in the source code, **never to variable names**: `countU` is an identifier, not a suffixed variable.

Suffixes affect the compiler's type selection for a literal. In C++17/20, an unsuffixed floating-point literal such as `0.25` has type `double`; add `f`/`F` for `float` or `l`/`L` for `long double`. An unsuffixed integer literal such as `42` has type `int` when representable. For larger values, C++ tries a standard candidate sequence based on the literal's base and suffix; adding `U`, `L`, or `LL` changes that sequence. The chosen type must still be able to represent the value.

Integer suffixes select among integer types; there are no standard suffixes that directly produce `short`, `char`, `unsigned char`, or a fixed-width type such as `std::uint64_t`. For example, `42` is an `int` literal even when used to initialize a `short`; the value is then converted to the declared container type. Use an explicit cast or type-aware construction where that conversion matters.

```cpp
short local_count = 42;          // 42 is int; its value converts to short
double time_step = 1e-4;          // unsuffixed floating literal is double
float display_value = 1e-4f;      // f selects float literal type
long double accumulated = 1e-4L;  // L selects long double literal type
std::uint64_t global_id = 42ULL;  // ULL selects an unsigned integer candidate
```

Integer suffix letters are case-insensitive in general. The two `l` letters in `ll` must both use the same case (`LL` or `ll`); `u` may be combined before or after the `l`/`ll` part (`ULL` or `LLU`). Prefer uppercase `U`, `L`, and `LL` in code when they make the intended type easier to read. C++17/20 has no standard `z` size suffix (it was added in C++23); use `std::size_t{n}` or an appropriately typed value for container counts.

## 4. THE ULTIMATE REFERENCE TABLE

The sizes are typical `sizeof` results in bytes for GCC on Linux x86_64 (LP64), not portable guarantees. “Literal suffixes” lists suffixes that influence selection in that family; it does not claim every suffixed literal is exactly the row's type. Integer suffixes select a candidate sequence, and the compiler chooses a type in that sequence that can represent the value. `—` means no standard suffix directly selects that type.

| Category | Full/Official Type Name (Container) | Common Shorthand Name | Typical Size on Linux x86_64 (GCC) | Literal Suffixes (if applicable) | Code Example |
|---|---|---|---:|---|---|
| Integer / character | `char` | `char` | 1 byte | `—` | `char marker = 'x';` |
| Integer / character | `signed char` | `signed char` | 1 byte | `—` | `signed char offset = -3;` |
| Integer / character | `unsigned char` | `unsigned char` | 1 byte | `—` | `unsigned char tag = 7;` |
| Signed integer | `short int` | `short` | 2 bytes | `—` | `short layer = 12;` |
| Unsigned integer | `unsigned short int` | `unsigned short` | 2 bytes | `U` (unsigned candidate sequence; does not force `short`) | `unsigned short flags = 7U;` |
| Signed integer | `int` | `int` | 4 bytes | none (default when representable); `U` can select unsigned candidates | `int iterations = 1000;` |
| Unsigned integer | `unsigned int` | `unsigned` / `unsigned int` | 4 bytes | `U` | `unsigned int cells = 1000U;` |
| Signed integer | `long int` | `long` | 8 bytes | `L` | `long offset = -100L;` |
| Unsigned integer | `unsigned long int` | `unsigned long` | 8 bytes | `UL` or `LU` | `unsigned long count = 100UL;` |
| Signed integer | `long long int` | `long long` | 8 bytes | `LL` | `long long wide_value = 100LL;` |
| Unsigned integer | `unsigned long long int` | `unsigned long long` | 8 bytes | `ULL` or `LLU` | `unsigned long long global_cells = 1ULL << 40;` |
| Floating-point | `float` | `float` | 4 bytes | `F` or `f` | `float cfl = 0.5f;` |
| Floating-point | `double` | `double` | 8 bytes | none (default); no standard `D` suffix | `double pressure = 101325.0;` |
| Floating-point | `long double` | `long double` | 16 bytes of storage (typically x87 extended precision) | `L` or `l` | `long double sum = 1.0L;` |

For CFD code, `double` is a common choice for mesh coordinates, field values, and solver arithmetic when accuracy is the priority. Use `std::size_t` for container sizes and nonnegative local indices; use fixed-width types from `<cstdint>` when an interface, file format, or distributed global identifier requires a specific width. Fixed-width types are aliases to available built-in integer types and do not introduce new literal suffixes. Check `std::numeric_limits<T>` when range or precision matters, and avoid assuming that `long double` has more precision on every platform.
