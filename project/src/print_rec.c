#include "print_rec.h"
#include "utils.h"

void print_recursive(int n) {
    if (n == 1) {
        printf("1");
        return;
    }

    int step = n > 1 ? -1 : 1;
    print_recursive(n + step);

    printf(" %d", n);
}
