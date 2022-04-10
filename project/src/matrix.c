#include "matrix.h"
#include <stdlib.h>
#include <stdio.h>

#define valid_matrix(mat) (mat != NULL && mat->rows != 0 && mat->cols != 0)

static double* alloc_table(size_t rows, size_t cols) {
    double *table = calloc(rows * cols, sizeof(double));
    // раньше тут было больше кода и данная функция имела смысл
    return table;
}

static Matrix* alloc_matrix(int rows, int cols) {
    Matrix *matrix = NULL;

    if (rows != 0 && cols != 0) {
        matrix = malloc(sizeof(Matrix));

        if (matrix != NULL) {
            matrix->table = alloc_table(rows, cols);

            if (matrix->table == NULL) {
                free(matrix);
                matrix = NULL;
            }
        }
    }

    return matrix;
}

static void free_table(double *table) {
    // раньше тут было больше кода и данная функция имела смысл
    free(table);
}

void free_matrix(Matrix* matrix) {
    if (matrix != NULL) {
        if (matrix->table != NULL) {
            free_table(matrix->table);
        }
        free(matrix);
    }
}

Matrix* create_matrix(size_t rows, size_t cols) {
    Matrix* matrix = NULL;

    if (rows != 0 && cols != 0) {
        matrix = alloc_matrix(rows, cols);

        if (matrix != NULL) {
            matrix->rows = rows;
            matrix->cols = cols;

            for (size_t i = 0; i < rows; ++i) {
                for (size_t j = 0; j < cols; ++j) {
                    matrix->table[i * cols + j] = 0;
                }
            }
        }
    }

    return matrix;
}

Matrix* create_matrix_from_file(const char* path_file) {
    Matrix* ret_matrix = NULL;
    FILE* matrix_file = fopen(path_file, "r");
    long long rows = 0, cols = 0;

    if (matrix_file != NULL &&
        fscanf(matrix_file, "%lld%lld", &rows, &cols) == 2 &&
        rows > 0 && cols > 0) {
            ret_matrix = create_matrix((size_t)rows, (size_t)cols);

            int rc = 1;
            for (size_t i = 0; i < (size_t)rows && rc == 1; ++i) {
                for (size_t j = 0; j < (size_t)cols && rc == 1; ++j) {
                    rc = fscanf(matrix_file, "%lf", &ret_matrix->table[i * cols + j]);
                }
            }

            if (rc != 1) {
                free_matrix(ret_matrix);
                ret_matrix = NULL;
            }

            fclose(matrix_file);
    }

    return ret_matrix;
}

int get_rows(const Matrix* matrix, size_t* rows) {
    int rc = 0;
    if (valid_matrix(matrix)) {
        *rows = matrix->rows;
    } else {
        rc = 1;
    }

    return rc;
}

int get_cols(const Matrix* matrix, size_t* cols) {
    int rc = 0;
    if (valid_matrix(matrix)) {
        *cols = matrix->cols;
    } else {
        rc = 1;
    }

    return rc;
}

int get_elem(const Matrix* matrix, size_t row, size_t col, double* val) {
    int rc = 0;
    if (valid_matrix(matrix) &&
        val != NULL &&
        row < matrix->rows &&
        col < matrix->cols) {
        *val = matrix->table[row * matrix->cols + col];
    } else {
        rc = 1;
    }

    return rc;
}

int set_elem(Matrix* matrix, size_t row, size_t col, double val) {
    int rc = 0;
    if (valid_matrix(matrix) &&
        row < matrix->rows &&
        col < matrix->cols) {
        matrix->table[row * matrix->cols + col] = val;
    } else {
        rc = 1;
    }

    return rc;
}

Matrix* mul_scalar(const Matrix* matrix, double val) {
    Matrix *ret_matrix = NULL;

    if (valid_matrix(matrix)) {
        ret_matrix = create_matrix(matrix->rows, matrix->cols);

        if (ret_matrix != NULL) {
            for (size_t i = 0; i < matrix->rows; ++i) {
                for (size_t j = 0; j < matrix->cols; ++j) {
                    matrix->table[i * matrix->cols + j] = matrix->table[i * matrix->cols + j] * val;
                }
            }
        }
    }

    return ret_matrix;
}

Matrix* transp(const Matrix* matrix) {
    Matrix *ret_matrix = NULL;

    if (valid_matrix(matrix)) {
        ret_matrix = create_matrix(matrix->cols, matrix->rows);

        if (ret_matrix != NULL) {
            for (size_t i = 0; i < matrix->rows; ++i) {
                for (size_t j = 0; j < matrix->cols; ++j) {
                    ret_matrix->table[i * ret_matrix->cols + j] = matrix->table[i * matrix->cols + j];
                }
            }
        }
    }

    return ret_matrix;
}

static Matrix* pair_elements_process(const Matrix* l,
                                     const Matrix* r,
                                     double (*processor)(double l, double r)) {
    Matrix *ret_matrix = NULL;

    if (valid_matrix(l) &&
        valid_matrix(r) &&
        l->rows == r->rows &&
        l->cols == r->cols) {
        ret_matrix = create_matrix(l->rows, l->cols);

        if (ret_matrix != NULL) {
            for (size_t i = 0; i < l->rows; ++i) {
                for (size_t j = 0; j < l->cols; ++j) {
                    ret_matrix->table[i * ret_matrix->cols + j] =
                        (*processor)(l->table[i * l->cols + j], r->table[i * r->cols + j]);
                }
            }
        }
    }

    return ret_matrix;
}

static double _double_sum(double l, double r) {
    return l + r;
}

static double _double_sub(double l, double r) {
    return l - r;
}

Matrix* sum(const Matrix* l, const Matrix* r) {
    // не знаю, нужна ли тут проверка, ведь я ее в pair_elements_process делаю и так
    Matrix *ret_matrix = NULL;
    ret_matrix = pair_elements_process(l, r, _double_sum);
    return ret_matrix;
}

Matrix* sub(const Matrix* l, const Matrix* r) {
    // не знаю, нужна ли тут проверка, ведь я ее в pair_elements_process делаю и так
    Matrix *ret_matrix = NULL;
    ret_matrix = pair_elements_process(l, r, _double_sub);
    return ret_matrix;
}

Matrix* mul(const Matrix* l, const Matrix* r) {
    Matrix *ret_matrix = NULL;

    if (valid_matrix(l) && valid_matrix(r) && l->cols == r->rows) {
        ret_matrix = create_matrix(l->rows, r->cols);

        for (size_t i = 0; i < ret_matrix->rows; ++i) {
            for (size_t j = 0; j < ret_matrix->cols; ++j) {
                ret_matrix->table[i * ret_matrix->cols + j] = 0;

                for (size_t k = 0; k < l->rows; ++k) {
                    ret_matrix->table[i * ret_matrix->cols + j] +=
                        l->table[i * l->cols + k] *
                        r->table[k * r->cols + i];
                }
            }
        }
    }

    return ret_matrix;
}

static Matrix* get_minor(const Matrix* matrix, size_t row, size_t col) {
    Matrix *minor = create_matrix(matrix->rows - 1, matrix->cols - 1);

    if (minor != NULL) {
        size_t flag_i = 0, flag_j;

        for (size_t i = 0; i < matrix->rows; ++i) {
            if (i != row) {
                flag_j = 0;
                for (size_t j = 0; j < matrix->cols; ++j) {
                    if (j != col) {
                        minor[(i - flag_i) * minor->cols + j - flag_j] = matrix[i * matrix->cols + j];
                    } else {
                        flag_j = 1;
                    }
                }
            } else {
                flag_i = 1;
            }
        }
    }

    return minor;
}

int det(const Matrix* matrix, double* val) {
    int rc = 0;

    if (valid_matrix(matrix) && matrix->rows == matrix->cols) {
        if (matrix->rows == 2) {
            *val = matrix->table[0] * matrix->table[3] - matrix->table[1] * matrix->table[2];

        } else if (matrix->rows == 1) {
            *val = matrix->table[0];

        } else {
            double minor_det = 0;
            int sign = 1;
            Matrix* minor = NULL;

            int flag = 0;

            for (size_t i = 0; i < matrix->cols && flag == 0; ++i) {
                if (matrix->table[i] != 0) {
                    minor = get_minor(matrix, 0, i);

                    if (minor != NULL) {
                        flag = det(minor, &minor_det);
                        if (flag == 0) {
                            *val += sign * matrix->table[i] * minor_det;
                        }

                        free_matrix(minor);
                    } else {
                        flag = 1;
                    }
                }
                sign *= -1;
            }

            if (flag != 0) {
                rc = 2;
            }
        }
    } else {
        rc = 1;
    }

    return rc;
}

Matrix* adj(const Matrix* matrix) {
    Matrix* adj_mat = NULL;

    if (valid_matrix(matrix) && matrix->rows == matrix->cols) {
        adj_mat = create_matrix(matrix->rows, matrix->cols);
        Matrix* minor = NULL;
        double minor_det = 0;

        if (adj_mat != NULL) {
            int flag = 0;

            for (size_t i = 0; i < matrix->rows && flag == 0; ++i) {
                for (size_t j = 0; j < matrix->cols && flag == 0; ++j) {
                    minor = get_minor(matrix, i, j);

                    if (minor != NULL) {
                        flag = det(minor, &minor_det);

                        if (flag == 0) {
                            adj_mat->table[i * adj_mat->rows + j] = ((i + j) % 2 == 0 ? 1 : -1) * minor_det;
                        }

                        free_matrix(minor);
                    } else {
                        flag = 1;
                    }
                }
            }

            if (flag == 0) {
                Matrix *tmp = transp(adj_mat);
                if (tmp != NULL) {
                    free_matrix(adj_mat);
                    adj_mat = tmp;
                } else {
                    free_matrix(adj_mat);
                    adj_mat = NULL;
                }
            } else {
                free_matrix(adj_mat);
                adj_mat = NULL;
            }
        }
    }

    return adj_mat;
}

Matrix* inv(const Matrix* matrix) {
    Matrix *inv_matrix = NULL;

    if (valid_matrix(matrix)) {
        inv_matrix = adj(inv_matrix);

        if (inv_matrix != NULL) {
            double determinant = 0;

            if (det(matrix, &determinant) == 0) {
                Matrix *tmp = mul_scalar(inv_matrix, 1/determinant);

                if (tmp != NULL) {
                    free_matrix(inv_matrix);
                    inv_matrix = tmp;
                } else {
                    free_matrix(inv_matrix);
                    inv_matrix = NULL;
                }
            }
        }
    }

    return inv_matrix;
}
