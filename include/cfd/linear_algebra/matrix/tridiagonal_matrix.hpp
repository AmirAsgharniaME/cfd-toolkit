#pragma once

#include <cstddef>
#include <vector>

namespace cfd::linear_algebra {

// Stores an N x N tridiagonal system and its right-hand-side vector.
// All four vectors use row indexing and have exactly size() entries.
class TridiagonalMatrix {
public:
    explicit TridiagonalMatrix(std::size_t size);

    [[nodiscard]] std::size_t size() const noexcept;

    [[nodiscard]] double diag(std::size_t row) const;
    void set_diag(std::size_t row, double value);
    void add_diag(std::size_t row, double value);

    [[nodiscard]] double lower(std::size_t row) const;
    void set_lower(std::size_t row, double value);
    void add_lower(std::size_t row, double value);

    [[nodiscard]] double upper(std::size_t row) const;
    void set_upper(std::size_t row, double value);
    void add_upper(std::size_t row, double value);

    [[nodiscard]] double source(std::size_t row) const;
    void set_source(std::size_t row, double value);
    void add_source(std::size_t row, double value);

    [[nodiscard]] std::vector<double>& diag() noexcept;
    [[nodiscard]] const std::vector<double>& diag() const noexcept;
    [[nodiscard]] std::vector<double>& lower() noexcept;
    [[nodiscard]] const std::vector<double>& lower() const noexcept;
    [[nodiscard]] std::vector<double>& upper() noexcept;
    [[nodiscard]] const std::vector<double>& upper() const noexcept;
    [[nodiscard]] std::vector<double>& source() noexcept;
    [[nodiscard]] const std::vector<double>& source() const noexcept;

    // Reset values in place, preserving vector sizes and capacities.
    void reset() noexcept;

private:
    std::vector<double> diag_;
    std::vector<double> lower_;
    std::vector<double> upper_;
    std::vector<double> source_;
};

} // namespace cfd::linear_algebra
