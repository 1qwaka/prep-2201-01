#include <stdio.h>
#include "types.h"
#include "formats.h"
#include "scans.h"

int scan_client_data(data_t *client_data) {
    char format[FORMAT_STRING_SIZE] = { 0 };
    scan_client_data_format(format);
    int rc = scanf(format,
                   &client_data->number,
                   client_data->name,
                   client_data->surname,
                   client_data->address,
                   client_data->tel_number,
                   &client_data->indebtedness,
                   &client_data->credit_limit,
                   &client_data->cash_payments);
    return rc == 8;
}

int scan_transaction(data_t *transaction) {
    int rc = scanf("%d %lf",
                   &transaction->number,
                   &transaction->cash_payments);
    return rc == 2;
}

int fscan_client_data(FILE *data_file, data_t *client_data) {
    char format[FORMAT_STRING_SIZE] = { 0 };
    scan_client_data_format(format);
    int rc = fscanf(data_file,
                    format,
                    &client_data->number,
                    client_data->name,
                    client_data->surname,
                    client_data->address,
                    client_data->tel_number,
                    &client_data->indebtedness,
                    &client_data->credit_limit,
                    &client_data->cash_payments);
    return rc == 8;
}

int fscan_transaction(FILE *transaction_file, data_t *transaction) {
    int rc = fscanf(transaction_file,
                    "%d %lf",
                    &transaction->number,
                    &transaction->cash_payments);
    return rc == 2;
}
