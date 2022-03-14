#include "utils.h"

size_t timer_from(int from) {
    size_t counter = 0;
    for (short i = from; i > 0; --i) {
        ++counter;
        printf("%d ", i);
    }
    printf("0");
    return counter;
}

int custom_pow(int base, int power) {
    int res = 1;
    for (int i = 0; i < power; ++i) {
        res *= base;
    }
    return res;
}

