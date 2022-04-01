#include <stdio.h>
#include "types.h"
#include "formats.h"
#include "scans.h"

int scan_client_data(data_t *client) {
    char format[FORMAT_STRING_SIZE] = { 0 };
    scan_client_data_format(format);
    int rc = scanf(format,
                   &client->number,
                   client->name,
                   client->surname,
                   client->address,
                   client->tel_number,
                   &client->indebtedness,
                   &client->credit_limit,
                   &client->cash_payments);
    return rc == 8;
}

int scan_transaction(data_t *transaction) {
    int rc = scanf("%d %lf",
                   &transaction->number,
                   &transaction->cash_payments);
    return rc == 2;
}

int fscan_client_data(FILE *client_data_file, data_t *client) {
    char format[FORMAT_STRING_SIZE] = { 0 };
    scan_client_data_format(format);
    int rc = fscanf(client_data_file,
                    format,
                    &client->number,
                    client->name,
                    client->surname,
                    client->address,
                    client->tel_number,
                    &client->indebtedness,
                    &client->credit_limit,
                    &client->cash_payments);
    return rc == 8;
}

int fscan_transaction(FILE *transaction_file, data_t *transaction) {
    int rc = fscanf(transaction_file,
                    "%d %lf",
                    &transaction->number,
                    &transaction->cash_payments);
    return rc == 2;
}
