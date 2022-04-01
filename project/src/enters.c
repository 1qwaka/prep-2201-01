#include <stdio.h>
#include "types.h"
#include "prompts.h"
#include "writes.h"
#include "scans.h"
#include "enters.h"


void enter_client_data(FILE *data_file, data_t client) {
    client_data_input_prompt();

    while (scan_client_data(&client)) {
        write_client_data(data_file, client);
        client_data_input_prompt();
    }
}

void enter_transaction(FILE *transaction_file, data_t transfer) {
    transaction_input_prompt();
    while (scan_transaction(&transfer)) {
        write_transaction(transaction_file, transfer);
        transaction_input_prompt();
    }
}
