#include <string.h>
#include <stdio.h>
#include <math.h>
#include "scans.h"
#include "writes.h"
#include "types.h"


int equals_datas_t(data_t data1, data_t data2) {
    const double compare_float_error = 1e-2;
    return
        data1.number == data2.number &&
        strcmp(data1.name, data2.name) == 0 &&
        strcmp(data1.surname, data2.surname) == 0 &&
        strcmp(data1.address, data2.address) == 0 &&
        strcmp(data1.tel_number, data2.tel_number) == 0 &&
        fabs(data1.indebtedness - data2.indebtedness) < compare_float_error &&
        fabs(data1.credit_limit - data2.credit_limit) < compare_float_error &&
        fabs(data1.cash_payments - data2.cash_payments) < compare_float_error;
}

void test_write_to_file() {
    const char *filename = "test_record.dat";

    FILE *file = fopen(filename, "w+");
    if (file == NULL) {
        printf("\nTEST FAILED (CAN NOT CREATE TEST FILE)\n");
        return;
    }

    data_t expected_data = {
        11,
        "Jonathan",
        "Joestar",
        "Russia",
        "+79999999999",
        0,
        10,
        5
    };
    write_client_data(file, expected_data);
    rewind(file);

    data_t got_data = { 0 };
    if (fscan_client_data(file, &got_data)) {
        if (equals_datas_t(expected_data, got_data)) {
            printf("TEST PASSED\n");
        } else {
            printf("TEST FAILED (READED DATA NOT EQUALS EXPECTED)\n");
        }
    } else {
        printf("TEST FAILED (CAN NOT READ DATA FROM FILE)\n");
    }
    fclose(file);
}
