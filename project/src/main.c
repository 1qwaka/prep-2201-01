#include <stdio.h>
#include "matrix.h"


int main(void) {
    Matrix* l = create_matrix_from_file("in.txt");

    double deter = 0;
    int rc = det(l, &deter);

    // for (size_t i = 0; i < res->rows; i++) {
    //     for (size_t j = 0; j < res->cols; j++) {
    //         printf("%10.4lf", res->table[i * res->cols + j]);
    //     }
    //     printf("\n");
    // }
    printf("det = %lf\nrc = %d\n", deter, rc);

    free_matrix(l);

    return 0;
}
