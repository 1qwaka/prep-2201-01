#include <stdlib.h>
#include <stdio.h>
#include "utils.h"
#include "nums.h"
#include "print_rec.h"

#define ERR_ARGS_COUNT (-1)
#define ERR_WRONG_FLG (-2)
#define ERR_NOT_A_NUMBER (-3)

#define TST_FOO_FIX     1
#define TST_FOO_IMPL    2
#define TST_MOD_IMPL    3
#define TST_PRINT_RECURSIVE 4


int main(int argc, const char** argv) {
    if (argc < 3) {
        return ERR_ARGS_COUNT;
    }
    char* end = NULL;

    int Test_case = (int) strtol(argv[1], &end, 0);
    if (*end != '\0') {
        return ERR_NOT_A_NUMBER;
    }

    const char* data;
    data = argv[2];

    switch (Test_case) {
        case TST_FOO_FIX: {
            int to = (int) strtol(data, &end, 0);
            if (*end != '\0') {
                return ERR_NOT_A_NUMBER;
            }
            size_t ticks_count = timer_from(to);
            if (ticks_count != 0) {
                printf("\n%zu", ticks_count + 1);
            }
            break;
        }
        case TST_FOO_IMPL: {
            if (argc == 4) {
                int base = (int) strtol(data, &end, 0);
                if (*end != '\0') {
                    return ERR_NOT_A_NUMBER;
                }
                int pow = (int) strtol(argv[3], &end, 0);
                if (*end != '\0') {
                    return ERR_NOT_A_NUMBER;
                }

                int res = custom_pow(base, pow);

                printf("%i\n", res);
            } else {
                return ERR_ARGS_COUNT;
            }
            break;
        }
        case TST_MOD_IMPL: {
           int num = (int) strtol(data, &end, 0);
            if (*end != '\0') {
                return ERR_NOT_A_NUMBER;
            }

            int res = is_prime(num);

            printf("%d", res);

            break;
        }
        case TST_PRINT_RECURSIVE: {
            int num = (int) strtol(data, &end, 0);
            if (*end != '\0') {
                return ERR_NOT_A_NUMBER;
            }

            print_recursive(num);

            break;
        }
        default: {
            return ERR_WRONG_FLG;
        }
    }

    return 0;
}
