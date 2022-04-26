#include "matrix.h"
#include "exceptions.h"
#include <limits>
#include <iomanip>
#include <iostream>
#include <math.h>

#define EPSILON 1e-7

using std::cout;
using std::cin;
using std::endl;

// int main(void) {

//     prep::Matrix mat(cin);

//     prep::Matrix up = mat.get_upper_triangular();
//     cout << up << endl;

//    // cout << "rows: " << mat.getRows() << endl;
//    // cout << "cols: " << mat.getCols() << endl;
//     //// cout << "mat[1, 1]: " << mat(1, 1);

//     return 0;
// }

namespace prep {

Matrix::Matrix(size_t rows, size_t cols) {
    // cout << "constructor 1" << endl;
    table.reserve(rows * cols);
    table.resize(rows * cols);
    this->rows = rows;
    this->cols = cols;

    // cout << "rows: " << rows << "   cols: " << cols << endl;
    // cout << "size: " << table.size() << endl;
}

Matrix::Matrix(std::istream& is) {
//    cout << "constructor 2" << endl;
    if (is.eof() || is.bad()) {
        // cout << "bad" << endl;
        throw InvalidMatrixStream();
    }
    if (is >> rows >> cols) {
        table.reserve(rows * cols);
        table.resize(rows * cols);

        // cout << "rows: " << rows << "   cols: " << cols << endl;
        // cout << "size: " << table.size() << endl;

        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < cols; ++j) {
                // cout << "==READ " << i * cols + j << endl;
                if (!(is >> table[i * cols + j])) {
                    // cout << "ERROR" << endl;
                    throw InvalidMatrixStream();
                }
            }
        }
    } else {
        throw InvalidMatrixStream();
    }
}

size_t Matrix::getRows() const {
    // cout << "get rows" << endl;
    return rows;
}

size_t Matrix::getCols() const {
    // cout << "get cols" << endl;
    return cols;
}

double& Matrix::operator()(size_t i, size_t j) {
    // cout << "operator ()" << endl;
    // cout << "i: " << i << "   j: " << j << endl;
    // cout << "size: " << table.size() << endl;
    if (i >= rows || j >= cols) {
        throw OutOfRange(i, j, *this);
    }
    return table[i * cols + j];
}

double Matrix::operator()(size_t i, size_t j) const {
    // cout << "operator () const" << endl;
    // cout << "i: " << i << "   j: " << j << endl;
    // cout << "size: " << table.size() << endl;
    if (i >= rows || j >= cols) {
        throw OutOfRange(i, j, *this);
    }
    return table[i * cols + j];
}

bool Matrix::operator==(const Matrix& rhs) const {
    // cout << "operator ==" << endl;
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
    // cout << "operator !=" << endl;
    return !(*this == rhs);
}

Matrix Matrix::pairElementProcess(const Matrix& rhs, double processor(double, double)) const {
    if (!sameSize(rhs)) {
        throw DimensionMismatch(*this, rhs);
    }
    Matrix returnMat(rows, cols);

    for (size_t i = 0; i < rows * cols; ++i) {
        returnMat.table[i] = processor(table[i], rhs.table[i]);
    }

    return returnMat;
}

bool Matrix::sameSize(const Matrix& rhs) const {
    return rows == rhs.getRows() && cols == rhs.getCols();
}

Matrix Matrix::operator+(const Matrix& rhs) const {
    // cout << "operator +" << endl;
    return pairElementProcess(rhs, [](double a, double b){ return a + b; });
}

Matrix Matrix::operator-(const Matrix& rhs) const {
    // cout << "operator -" << endl;
    return pairElementProcess(rhs, [](double a, double b){ return a - b; });
}

Matrix Matrix::operator*(double val) const {
    // cout << "operator * (double val)" << endl;
    Matrix returnMat(rows, cols);

    for (size_t i = 0; i < rows * cols; ++i) {
        returnMat.table[i] = table[i] * val;
    }

    return returnMat;
}

Matrix operator*(double val, const Matrix& matrix) {
    // cout << "operator * (double val, matrix)" << endl;
    Matrix returnMat(matrix.rows, matrix.cols);

    for (size_t i = 0; i < matrix.rows * matrix.cols; ++i) {
        returnMat.table[i] = matrix.table[i] * val;
    }

    return returnMat;
}

Matrix Matrix::operator*(const Matrix& rhs) const {
    // cout << "operator * (Matrix rhs)" << endl;
    if (cols != rhs.getRows()) {
        throw DimensionMismatch(*this, rhs);
    }

    size_t cols_ = rhs.getCols();
    Matrix returnMat(rows, cols_);

    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols_; ++j) {
            returnMat.table[i * cols_ + j] = 0;

            // printf("=== MUL MAT ELEM i=%zu j=%zu ===\n", i, j);
            for (size_t k = 0; k < cols; ++k) {
                // printf("left  == %zu %zu == %lf\n", i, k, l->table[i * l->cols + k]);
                // printf("right == %zu %zu == %lf\n", k, j, r->table[k * r->cols + j]);

                returnMat.table[i * cols_ + j] +=
                    table[i * cols + k] *
                    rhs.table[k * cols_ + j];
            }
        }
    }

    return returnMat;
}

Matrix Matrix::transp() const {
    // cout << "transp " << endl;
    Matrix returnMat(cols, rows);

    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols; ++j) {
            returnMat.table[j * rows + i] = table[i * cols + j];
        }
    }

    return returnMat;
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

            // printf("minor [%zu]x[%zu],  i=%zu j=%zu\n",
            //     minor->rows, minor->cols, i - flag_i, j - flag_j);
            minor->table[(i - flag_i) * minor->cols + j - flag_j] = table[i * cols + j];
        }
    }

    return minor;
}

void Matrix::swith_rows(size_t row1, size_t row2) {
    if (row1 == row2) {
        return;
    }

    for (size_t i = 0; i < cols; ++i) {
        double tmp = table[row1 * cols + i];
        table[row1 * cols + i] = table[row2 * cols + i];
        table[row2 * cols + i] = tmp;
    }
}

void Matrix::sum_rows(size_t row1, size_t row2, double multiplier) {
    if (row1 == row2) {
        return;
    }

    for (size_t i = 0; i < cols; ++i) {
        table[row2 * cols + i] += table[row1 * cols + i] * multiplier;
    }
}

Matrix Matrix::get_upper_triangular() const {
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

        upper_triangular.swith_rows(i, row);

        for (size_t j = i + 1; j < rows; ++j) {
            double multiplier = upper_triangular.table[j * rows + i] /
                                upper_triangular.table[i * rows + i] * -1;
            upper_triangular.sum_rows(i, j, multiplier);

            // cout << "added: " << i << " to " << j << "/ mul = " << multiplier <<endl;
        }
    }

    // cout << upper_triangular << endl;

    return upper_triangular;
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
    Matrix upper_triangular = get_upper_triangular();

    determinant = 1;
    for (size_t i = 0; i < rows; ++i) {
        determinant *= upper_triangular.table[i * cols + i];
    }

    return determinant;
}

// int count = 0;
// double Matrix::det() const {
//     if (count < 60) {
//         cout << "==DET== " << rows << " " << cols << endl;
//         count++;
//     }
//     if (rows != cols) {
//         throw DimensionMismatch(*this, *this);
//     }

//     if (rows == 3) {
//         return table[0] * table[4] * table[8] +
//                table[2] * table[3] * table[7] +
//                table[1] * table[5] * table[6] -
//                table[2] * table[4] * table[6] -
//                table[0] * table[5] * table[7] -
//                table[1] * table[3] * table[8];
//     }
//     //todo: сгененировать код для определителя больших порядков

//     if (rows == 2) {
//         return table[0] * table[3] - table[1] * table[2];
//     }

//     if (rows == 1) {
//         return table[0];
//     }

//     double determinant = 0;
//     double minor_det = 0;
//     int sign = 1;
//     Matrix* minor = nullptr;

//     for (size_t i = 0; i < cols; ++i) {
//         if (table[i] == 0) {
//             continue;
//         }

//         minor = getMinor(0, i);
//         minor_det = minor->det();

//         determinant += sign * table[i] * minor_det;

//         delete minor;

//         sign *= -1;
//     }

//     return determinant;
// }

// int a_count = 0;
Matrix Matrix::adj() const {
    // return (*this);
    // if (a_count < 50) {
    //     cout << "==ADJ== " << rows << " " << cols << endl;
    //     count++;
    // }
    if (rows != cols) {
        throw DimensionMismatch(*this, *this);
    }

    Matrix adj_mat(rows, cols);

    Matrix* minor = nullptr;
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
    // delete adj_mat; ?
    adj_mat = tmp;

    return adj_mat;
}

// int i_count = 0;
Matrix Matrix::inv() const {
    // return (*this);
    // if (i_count < 50) {
    //     cout << "==INV== " << rows << " " << cols << endl;
    //     count++;
    // }
    // cout << "inv " << endl;
    double determinant = det();

    if (std::fabs(determinant) < EPSILON) {
        throw SingularMatrix();
    }

    Matrix inv_matrix = adj();

    inv_matrix = inv_matrix * (1 / determinant);

    return inv_matrix;
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
