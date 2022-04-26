#include "matrix.h"
#include "exceptions.h"
#include <limits>
#include <iomanip>
#include <iostream>
#include <math.h>

#define EPSILON 1e-7


namespace prep {

Matrix::Matrix(size_t rows, size_t cols) {
    table.reserve(rows * cols);
    table.resize(rows * cols);
    this->rows = rows;
    this->cols = cols;
}


Matrix::Matrix(std::istream& is) {
    if (is.eof() || is.bad()) {
        throw InvalidMatrixStream();
    }
    if (is >> rows >> cols) {
        table.reserve(rows * cols);
        table.resize(rows * cols);

        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < cols; ++j) {
                if (!(is >> table[i * cols + j])) {
                    throw InvalidMatrixStream();
                }
            }
        }
    } else {
        throw InvalidMatrixStream();
    }
}


size_t Matrix::getRows() const {
    return rows;
}


size_t Matrix::getCols() const {
    return cols;
}


double& Matrix::operator()(size_t i, size_t j) {
    if (i >= rows || j >= cols) {
        throw OutOfRange(i, j, *this);
    }
    return table[i * cols + j];
}


double Matrix::operator()(size_t i, size_t j) const {
    if (i >= rows || j >= cols) {
        throw OutOfRange(i, j, *this);
    }
    return table[i * cols + j];
}


bool Matrix::operator==(const Matrix& rhs) const {
    if (rhs.getRows() != rows || rhs.getCols() != cols) {
        return false;
    }

    bool equals = true;
    for (size_t i = 0; i < rows && equals; ++i) {
        for (size_t j = 0; j < cols && equals; ++j) {
            equals = std::fabs(rhs(i, j) - (*this)(i, j)) < EPSILON;
        }
    }

    return equals;
}


bool Matrix::operator!=(const Matrix& rhs) const {
    return !(*this == rhs);
}


Matrix Matrix::operator+(const Matrix& rhs) const {
    return pairElementProcess(rhs, [](double a, double b){ return a + b; });
}


Matrix Matrix::operator-(const Matrix& rhs) const {
    return pairElementProcess(rhs, [](double a, double b){ return a - b; });
}


Matrix Matrix::operator*(double val) const {
    Matrix return_mat(rows, cols);

    for (size_t i = 0; i < rows * cols; ++i) {
        return_mat.table[i] = table[i] * val;
    }

    return return_mat;
}


Matrix operator*(double val, const Matrix& matrix) {
    Matrix return_mat(matrix.rows, matrix.cols);

    for (size_t i = 0; i < matrix.rows * matrix.cols; ++i) {
        return_mat.table[i] = matrix.table[i] * val;
    }

    return return_mat;
}


Matrix Matrix::operator*(const Matrix& rhs) const {
    if (cols != rhs.getRows()) {
        throw DimensionMismatch(*this, rhs);
    }

    size_t cols_ = rhs.getCols();
    Matrix return_mat(rows, cols_);

    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols_; ++j) {
            return_mat.table[i * cols_ + j] = 0;

            for (size_t k = 0; k < cols; ++k) {
                return_mat.table[i * cols_ + j] +=
                    table[i * cols + k] *
                    rhs.table[k * cols_ + j];
            }
        }
    }

    return return_mat;
}


Matrix Matrix::transp() const {
    Matrix return_mat(cols, rows);

    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols; ++j) {
            return_mat.table[j * rows + i] = table[i * cols + j];
        }
    }

    return return_mat;
}


double Matrix::det() const {
    if (rows != cols) {
        throw DimensionMismatch(*this, *this);
    }

    if (rows == 3) {
        return table[0] * table[4] * table[8] +
               table[2] * table[3] * table[7] +
               table[1] * table[5] * table[6] -
               table[2] * table[4] * table[6] -
               table[0] * table[5] * table[7] -
               table[1] * table[3] * table[8];
    }

    if (rows == 2) {
        return table[0] * table[3] - table[1] * table[2];
    }

    if (rows == 1) {
        return table[0];
    }

    double determinant;
    Matrix upper_triangular = getUpperTriangular();

    determinant = 1;
    for (size_t i = 0; i < rows; ++i) {
        determinant *= upper_triangular.table[i * cols + i];
    }

    return determinant;
}


Matrix Matrix::adj() const {
    if (rows != cols) {
        throw DimensionMismatch(*this, *this);
    }

    Matrix adj_mat(rows, cols);

    Matrix* minor = NULL;
    double minor_det;

    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols; ++j) {
            minor = getMinor(i, j);
            minor_det = minor->det();

            adj_mat.table[i * rows + j] = ((i + j) % 2 == 0 ? 1 : -1) * minor_det;

            delete minor;
        }
    }

    Matrix tmp = adj_mat.transp();
    adj_mat = tmp;

    return adj_mat;
}


Matrix Matrix::inv() const {
    double determinant = det();

    if (std::fabs(determinant) < EPSILON) {
        throw SingularMatrix();
    }

    Matrix inv_matrix = adj();

    inv_matrix = inv_matrix * (1 / determinant);

    return inv_matrix;
}


bool Matrix::sameSize(const Matrix& rhs) const {
    return rows == rhs.getRows() && cols == rhs.getCols();
}


void Matrix::swithRows(size_t row1, size_t row2) {
    if (row1 == row2) {
        return;
    }

    for (size_t i = 0; i < cols; ++i) {
        double tmp = table[row1 * cols + i];
        table[row1 * cols + i] = table[row2 * cols + i];
        table[row2 * cols + i] = tmp;
    }
}


void Matrix::sumRows(size_t row1, size_t row2, double multiplier) {
    if (row1 == row2) {
        return;
    }

    for (size_t i = 0; i < cols; ++i) {
        table[row2 * cols + i] += table[row1 * cols + i] * multiplier;
    }
}


Matrix Matrix::getUpperTriangular() const {
    Matrix upper_triangular(rows, rows);
    for (size_t i = 0; i < rows * rows; ++i) {
        upper_triangular.table[i] = table[i];
    }

    for (size_t i = 0; i < rows; ++i) {
        ssize_t row = -1;
        for (size_t j = i; j < rows; ++j) {
            if (std::fabs(table[j * cols + i]) > EPSILON) {
                row = j;
                break;
            }
        }

        if (row == -1) {
            continue;
        }

        upper_triangular.swithRows(i, row);

        for (size_t j = i + 1; j < rows; ++j) {
            double multiplier = upper_triangular.table[j * rows + i] /
                                upper_triangular.table[i * rows + i] * -1;
            upper_triangular.sumRows(i, j, multiplier);
        }
    }

    return upper_triangular;
}


Matrix* Matrix::getMinor(size_t row, size_t col) const {
    Matrix* minor = new Matrix(rows - 1, cols - 1);

    size_t flag_i = 0, flag_j;

    for (size_t i = 0; i < rows; ++i) {
        if (i == row) {
            flag_i = 1;
            continue;
        }

        flag_j = 0;
        for (size_t j = 0; j < cols; ++j) {
            if (j == col) {
                flag_j = 1;
                continue;
            }

            minor->table[(i - flag_i) * minor->cols + j - flag_j] = table[i * cols + j];
        }
    }

    return minor;
}


Matrix Matrix::pairElementProcess(const Matrix& rhs, double processor(double, double)) const {
    if (!sameSize(rhs)) {
        throw DimensionMismatch(*this, rhs);
    }
    Matrix return_mat(rows, cols);

    for (size_t i = 0; i < rows * cols; ++i) {
        return_mat.table[i] = processor(table[i], rhs.table[i]);
    }

    return return_mat;
}


std::ostream& operator<<(std::ostream& os, const Matrix& matrix) {
    os << matrix.getRows() << " " << matrix.getCols() << std::endl;
    for (size_t i = 0; i < matrix.getRows(); ++i) {
        for (size_t j = 0; j < matrix.getCols(); ++j) {
            os << std::setprecision(std::numeric_limits<double>::max_digits10)
            << matrix(i, j) << " ";
        }
        os << std::endl;
    }

    return os;
}

}  // namespace prep
