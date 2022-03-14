#include "utils.h"
#include "nums.h"

int is_prime(int n) {
    if (n == 0 || n == 1)
        return 0;

    if (n < 0)
        return 0;

    int root = sqrt((double)n);
    for (int i = 2; i < root + 1; ++i) {
        if (n % i == 0)
            return 0;
    }
    return 1;
}
