#include <stdio.h>
#include <stddef.h>
#include <malloc.h>
#include "prompts.h"
#include "types.h"
#include "enters.h"
#include "scans.h"
#include "writes.h"
#include "test.h"

#define CLIENTS_DATA_FILENAME "record.dat"
#define TRANSACTIONS_FILENAME "transaction.dat"
#define DATABASE_FILENAME "blackrecord.dat"


void process_choice(int choice);

void update_database(FILE *client_data_file, FILE *transaction_file,
                  FILE *database_file, data_t client, data_t transaction);


int main(void) {
    test_write_to_file();

    int choice = 0;

    choice_input_prompt();

    while (scanf("%d", &choice) == 1) {
        process_choice(choice);
        choice_input_prompt();
    }
    return 0;
}

void process_choice(int choice) {
    FILE *client_data_file = NULL;
    FILE *transactions_file = NULL;
    FILE *database_file = NULL;

    data_t client_data = { 0 };
    data_t transaction = { 0 };

    switch (choice) {
        case 1: {
            client_data_file = fopen(CLIENTS_DATA_FILENAME, "r+");
            if (client_data_file == NULL) {
                puts("Not access");
            } else {
                enter_client_data(client_data_file, client_data);
                fclose(client_data_file);
            }
            break;
        }
        case 2: {
            transactions_file = fopen(TRANSACTIONS_FILENAME, "r+");
            if (transactions_file == NULL) {
                puts("Not acess");
            } else {
                enter_transaction(transactions_file, transaction);
                fclose(transactions_file);
            }
            break;
        }
        case 3: {
            if ((client_data_file = fopen(CLIENTS_DATA_FILENAME, "r")) == NULL) {
                puts("exit");
            } else if ((transactions_file = fopen(TRANSACTIONS_FILENAME, "r")) == NULL) {
                puts("exit");
                fclose(client_data_file);
            } else if ((database_file = fopen(DATABASE_FILENAME, "w")) == NULL) {
                puts("exit");
                fclose(client_data_file);
                fclose(transactions_file);
            } else {
                update_database(client_data_file, transactions_file, database_file, client_data, transaction);
                fclose(client_data_file);
                fclose(transactions_file);
                fclose(database_file);
            }
            break;
        }
        default: {
            puts("error");
            break;
        }
    }
}

void update_database(FILE *client_data_file, FILE *transaction_file,
                  FILE *database_file, data_t client, data_t transaction) {
    if (client_data_file == NULL || transaction_file == NULL || database_file == NULL)
        return;

    while (fscan_client_data(client_data_file, &client)) {
        while (fscan_transaction(transaction_file, &transaction)) {
            if (client.number == transaction.number && transaction.cash_payments != 0) {
                client.credit_limit += transaction.cash_payments;
            }
        }
        write_client_data(database_file, client);
        rewind(transaction_file);
    }
}
