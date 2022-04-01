#include <stdio.h>
#include "types.h"
#include "scans.h"
#include "formats.h"
#include "writes.h"

void write_client_data(FILE *client_data_file, data_t client) {
    char format[FORMAT_STRING_SIZE] = { 0 };
    write_client_data_format(format);

    fprintf(client_data_file,
            format,
            client.number,
            client.name,
            client.surname,
            client.address,
            client.tel_number,
            client.indebtedness,
            client.credit_limit,
            client.cash_payments);
}

void write_transaction(FILE *transaction_file, data_t transaction) {
    char format[FORMAT_STRING_SIZE] = { 0 };
    write_transaction_format(format);
    fprintf(transaction_file,
            format,
            transaction.number,
            transaction.cash_payments);
}
