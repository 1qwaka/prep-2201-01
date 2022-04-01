#include <stdio.h>
#include <stddef.h>
#include <malloc.h>
#include "utils.h"
#include "prompts.h"
#include "types.h"
#include "enters.h"
#include "scans.h"
#include "writes.h"

#define RECORD_FILENAME "record.dat"

void black_record(FILE *data_file, FILE *transaction_file,
                  FILE *blackrecord, data_t client_data, data_t transfer);

void process_choice(int choice);

int main(void) {
    int choice = 0;

    // clients data, transactions, backup
    FILE *Ptr = NULL, *Ptr_2, *blackrecord;
    data_t client_data = { 0 }, transfer = { 0 };

    choice_input_prompt();

    while (scanf("%d", &choice) == 1) {
        switch (choice) {
            case 1: {
                Ptr = fopen("record.dat", "r+");
                if (Ptr == NULL) {
                    puts("Not access");
                } else {
                    enter_client_data(Ptr, client_data);
                    fclose(Ptr);
                }
                break;
            }

            case 2: {
                Ptr = fopen(filename, "r+");
                if (Ptr == NULL) {
                    puts("Not acess");
                } else {
                    enter_transaction(Ptr, transfer);
                    fclose(Ptr);
                }
                break;
            }

            case 3: {
                Ptr = fopen("record.dat", "r");
                Ptr_2 = fopen("transaction.dat", "r");
                blackrecord = fopen("blackrecord.dat", "w");

                if (Ptr == NULL || Ptr_2 == NULL || blackrecord == NULL) {
                    puts("exit");
                } else {
                    black_record(Ptr, Ptr_2, blackrecord, client_data, transfer);
                    fclose(Ptr);
                    fclose(Ptr_2);
                    fclose(blackrecord);
                }
                break;
            }

            default: {
                puts("error");
                break;
            }
        }

        choice_input_prompt();
    }
    return 0;
}

// void process_choice(int choice) {
//             switch (choice) {
//             case 1: {
//                 Ptr = fopen("record.dat", "r+");
//                 if (Ptr == NULL) {
//                     puts("Not access");
//                 } else {
//                     write_data(Ptr, client_data);
//                     fclose(Ptr);
//                 }
//                 break;
//             }

//             case 2: {
//                 Ptr = fopen(filename, "r+");
//                 if (Ptr == NULL) {
//                     puts("Not acess");
//                 } else {
//                     write_transaction(Ptr, transfer);
//                     fclose(Ptr);
//                 }
//                 break;
//             }

//             case 3: {
//                 Ptr = fopen("record.dat", "r");
//                 Ptr_2 = fopen("transaction.dat", "r");
//                 blackrecord = fopen("blackrecord.dat", "w");

//                 if (Ptr == NULL || Ptr_2 == NULL || blackrecord == NULL) {
//                     puts("exit");
//                 } else {
//                     black_record(Ptr, Ptr_2, blackrecord, client_data, transfer);
//                     fclose(Ptr);
//                     fclose(Ptr_2);
//                     fclose(blackrecord);
//                 }
//                 break;
//             }

//             default: {
//                 puts("error");
//                 break;
//             }
//         }
// }



void black_record(FILE *data_file, FILE *transaction_file,
                  FILE *blackrecord, data_t client_data, data_t transfer) {
    while (fscan_client_data(data_file, &client_data)) {
        while (fscan_transaction(transaction_file, &transfer)) {
            if (client_data.number == transfer.number && transfer.cash_payments != 0) {
                client_data.credit_limit += transfer.cash_payments;
            }
        }
        write_client_data(blackrecord, client_data);
        rewind(transaction_file);
    }
}
