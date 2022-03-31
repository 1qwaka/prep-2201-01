#include <stdio.h>
#include <stddef.h>
#include <malloc.h>
#include "utils.h"
#include "prompts.h"
#include "types.h"

#define RECORD_FILENAME "record.dat"

void write_data(FILE *ofPTR, data_t Client);

void write_transaction(FILE *ofPTR, data_t transfer);

void black_record(FILE *ofPTR, FILE *ofPTR_2, FILE *blackrecord, data_t client_data, data_t transfer);

void enter_data_client();

void enter_data_transaction();

void update_base();

void process_choice(int choice);

int main(void) {
    int choice = 0;

    //clients data, transactions, backup
    FILE *Ptr = NULL, *Ptr_2, *blackrecord;
    data_t client_data, transfer;

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
                    write_transaction(Ptr, transfer);
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

void write_client_data(FILE *data_file, data_t client) {
    char format[100] = { 0 };
    snprintf(format,
             sizeof(format),
             "%%-%dd%%-%ds%%-%ds%%-%ds%%%ds%%%d.2f%%%d.2f%%%d.2f\n",
             NUMBER_WRITE_W,
             NAME_WRITE_W,
             SURNAME_WRITE_W,
             ADRESS_WRITE_W,
             TEL_NUMBER_WRITE_W,
             INDEBTEDNESS_WRITE_W,
             CREDIT_LIMIT_WRITE_W,
             CASH_PAYMENTS_WRITE_W);
    
    fprintf(data_file, 
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

void enter_client_data(FILE *data_file, data_t client) {
    client_data_input_prompt();
    // char format[60];

    while (scanf("%d%s%s%s%s%lf%lf%lf",
                &client.number,
                client.name,
                client.surname,
                client.address,
                client.tel_number,
                &client.indebtedness,
                &client.credit_limit, 
                &client.cash_payments) == 8) {
        write_client_data(data_file, client);
        client_data_input_prompt();
    }
}

void write_transaction(FILE *transaction_file, data_t transfer) {
    char format[30] = { 0 };
    snprintf(format,
            sizeof(format),
            "%%-%dd%%-%d.2f\n",
            TRS_NUMBER_WRITE_W,
            TRS_CASH_PAYMENTS_W);
    fprintf(transaction_file,
            format,
            transfer.number, 
            transfer.cash_payments);
}

void enter_transaction(FILE *transaction_file, data_t transfer) {
    transaction_input_prompt();
    while (scanf("%d %lf", 
            &transfer.number, 
            &transfer.cash_payments) == 2) {
        write_transaction(transaction_file, transfer);
        transaction_input_prompt();
    }
}

void black_record(FILE *ofPTR, FILE *ofPTR_2, FILE *blackrecord, data_t client_data, data_t transfer) {
    while (fscanf(ofPTR, 
                  "%d%s%s%s%s%lf%lf%lf", 
                  &client_data.number, 
                  client_data.name, 
                  client_data.surname,
                  client_data.address, 
                  client_data.tel_number,
                  &client_data.indebtedness, 
                  &client_data.credit_limit,
                  &client_data.cash_payments) != 8) {

        while (fscanf(ofPTR_2, "%d %lf", &transfer.number, &transfer.cash_payments) != -1) {
            if (client_data.number == transfer.number && transfer.cash_payments != 0) {
                client_data.credit_limit += transfer.cash_payments;
            }
        }
        write_client_data(blackrecord, client_data);
        rewind(ofPTR_2);
        
    }
}
