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

    [[nodiscard]] double Diag(std::size_t row) const;
    void Set_Diag(std::size_t row, double value);
    void Add_Diag(std::size_t row, double value);

    [[nodiscard]] double Lower(std::size_t row) const;
    void Set_Lower(std::size_t row, double value);
    void Add_Lower(std::size_t row, double value);

    [[nodiscard]] double Upper(std::size_t row) const;
    void Set_Upper(std::size_t row, double value);
    void Add_Upper(std::size_t row, double value);

    [[nodiscard]] double Source(std::size_t row) const;
    void Set_Source(std::size_t row, double value);
    void Add_Source(std::size_t row, double value);

    [[nodiscard]] std::vector<double>& Diag() noexcept;
    [[nodiscard]] const std::vector<double>& Diag() const noexcept;
    [[nodiscard]] std::vector<double>& Lower() noexcept;
    [[nodiscard]] const std::vector<double>& Lower() const noexcept;
    [[nodiscard]] std::vector<double>& Upper() noexcept;
    [[nodiscard]] const std::vector<double>& Upper() const noexcept;
    [[nodiscard]] std::vector<double>& Source() noexcept;
    [[nodiscard]] const std::vector<double>& Source() const noexcept;

    // Reset values in place, preserving vector sizes and capacities.
    void Reset() noexcept;

private:
    std::vector<double> diag_;
    std::vector<double> lower_;
    std::vector<double> upper_;
    std::vector<double> source_;
};

} // namespace cfd::linear_algebra
