#include <stdio.h>
#include "types.h"
#include "formats.h"


void scan_client_data_format(char *format_str) {
    if (format_str == NULL)
        return;

    snprintf(format_str,
             FORMAT_STRING_SIZE,
             "%%%dd%%%ds%%%ds%%%ds%%%ds%%%dlf%%%dlf%%%dlf",
             NUMBER_WRITE_W,
             NAME_WRITE_W,
             SURNAME_WRITE_W,
             ADRESS_WRITE_W,
             TEL_NUMBER_WRITE_W,
             INDEBTEDNESS_WRITE_W,
             CREDIT_LIMIT_WRITE_W,
             CASH_PAYMENTS_WRITE_W);
}

void write_client_data_format(char *format_str) {
    if (format_str == NULL)
            return;

    snprintf(format_str,
             FORMAT_STRING_SIZE,
             "%%-%dd%%-%ds%%-%ds%%-%ds%%%ds%%%d.2f%%%d.2f%%%d.2f\n",
             NUMBER_WRITE_W,
             NAME_WRITE_W,
             SURNAME_WRITE_W,
             ADRESS_WRITE_W,
             TEL_NUMBER_WRITE_W,
             INDEBTEDNESS_WRITE_W,
             CREDIT_LIMIT_WRITE_W,
             CASH_PAYMENTS_WRITE_W);
}

void write_transaction_format(char *format_str) {
    if (format_str == NULL)
        return;

    snprintf(format_str,
             FORMAT_STRING_SIZE,
             "%%-%dd%%-%d.2f\n",
             TRS_NUMBER_WRITE_W,
             TRS_CASH_PAYMENTS_W);
}
