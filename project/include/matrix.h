#pragma once // NOLINT

#include <vector>
#include <istream>

namespace prep {
class Matrix {
 private:
  std::vector<double> table;
  size_t rows;
  size_t cols;

 public:
  explicit Matrix(size_t rows = 0, size_t cols = 0);
  explicit Matrix(std::istream& is);
  Matrix(const Matrix& rhs) = default;
  Matrix& operator=(const Matrix& rhs) = default;
  ~Matrix() = default;

  size_t getRows() const;
  size_t getCols() const;

  double operator()(size_t i, size_t j) const;
  double& operator()(size_t i, size_t j);

  bool operator==(const Matrix& rhs) const;
  bool operator!=(const Matrix& rhs) const;

  Matrix operator+(const Matrix& rhs) const;
  Matrix operator-(const Matrix& rhs) const;
  Matrix operator*(const Matrix& rhs) const;

  Matrix operator*(double val) const;

  friend
  Matrix operator*(double val, const Matrix& matrix);
  friend
  std::ostream& operator<<(std::ostream& os, const Matrix& matrix);

  Matrix transp() const;
  double det() const;
  Matrix adj() const;
  Matrix inv() const;

  bool sameSize(const Matrix& rhs) const;
  Matrix pairElementProcess(const Matrix& rhs, double processor(double, double)) const;
  Matrix* getMinor(size_t row, size_t col) const;
  void swith_rows(size_t row1, size_t row2);
  void sum_rows(size_t row1, size_t row2, double multiplier);
  Matrix get_upper_triangular() const;
};

Matrix operator*(double val, const Matrix& matrix);
std::ostream& operator<<(std::ostream& os, const Matrix& matrix);
}  // namespace prep
