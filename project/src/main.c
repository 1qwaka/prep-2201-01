#include <stdio.h>
#include "matrix.h"


int main(void) {
    Matrix* m = create_matrix(10, 10);
    // my_function(30, NULL);
    free_matrix(m);

    return 0;
}
