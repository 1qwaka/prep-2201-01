#include <stdio.h>
#include <stddef.h>
#include <malloc.h>
#include "utils.h"

#define RECORD_FILENAME "record.dat"

typedef enum {
    NAME_SIZE = 20,
    SURNAME_SIZE = 20,
    ADRESS_SIZE = 30,
    TEL_NUMBER_SIZE = 15
} arr_sizes_t;

typedef struct {
    int      number;
    char     name[NAME_SIZE];
    char     surname[SURNAME_SIZE];
    char     address[ADRESS_SIZE];
    char     tel_number[TEL_NUMBER_SIZE];
    double   indebtedness;
    double   credit_limit;
    double   cash_payments;
} data_t;

void write_data(FILE *ofPTR, data_t Client);

void write_transaction(FILE *ofPTR, data_t transfer);

void black_record(FILE *ofPTR, FILE *ofPTR_2, FILE *blackrecord, data_t client_data, data_t transfer);

void enter_data_client();

void enter_data_transaction();

void update_base();

int main(void) {
    int choice = 0;

    //clients data, transactions, backup
    FILE *Ptr = NULL, *Ptr_2, *blackrecord;
    data_t client_data, transfer;

    printf("%s", "please enter action\n"
                 "1 enter data client:\n"
                 "2 enter data transaction:\n"
                 "3 update base\n");

    while (scanf("%d", &choice) == 1) {
        switch (choice) {
            case 1: {
                Ptr = fopen("record.dat", "r+");
                if (Ptr == NULL) {
                    puts("Not access");
                } else {
                    write_data(Ptr, client_data);
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

        printf("%s", "please enter action\n1 enter data client:\n2 enter data transaction:\n3 update base\n");
    }
    return 0;
}

void write_data(FILE *ofPTR, data_t Client) {
    printf("%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n\n",
           "1 number account: ",
           "2 Client name: ",
           "3 surname: ",
           "4 Addres client: ",
           "5 Client Telnum: ",
           "6 Client indebtedness: ",
           "7 Client credit limit: ",
           "8 Client cash payments: ");
    while (scanf("%d%s%s%s%s%lf%lf%lf", &Client.number, Client.name, Client.surname, Client.address, Client.tel_number,
                 &Client.indebtedness, &Client.credit_limit, &Client.cash_payments) != -1) {
        fprintf(ofPTR, "%-12d%-11s%-11s%-16s%20s%12.2f%12.2f%12.2f\n", Client.number, Client.name, Client.surname,
                Client.address, Client.tel_number, Client.indebtedness, Client.credit_limit, Client.cash_payments);
        printf("%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n\n",
               "1 number account: ",
               "2 Client name: ",
               "3 surname: ",
               "4 Addres client: ",
               "5 Client Telnum: ",
               "6 Client indebtedness: ",
               "7 Client credit limit: ",
               "9 Client cash payments:");
    }
}

void write_transaction(FILE *ofPTR, data_t transfer) {
    printf("%s\n%s\n",
           "1 number account: ",
           "2 Client cash payments: ");
    while (scanf("%d %lf", &transfer.number, &transfer.cash_payments) != -1) {
        fprintf(ofPTR, "%-3d%-6.2f\n", transfer.number, transfer.cash_payments);
        printf("%s\n%s\n",
               "1 number account:",
               "2 Client cash payments: ");
    }
}

void black_record(FILE *ofPTR, FILE *ofPTR_2, FILE *blackrecord, data_t client_data, data_t transfer) {

    while (fscanf(ofPTR, "%d%s%s%s%s%lf%lf%lf", &client_data.number, client_data.name, client_data.surname,
                  client_data.address, client_data.tel_number, &client_data.indebtedness, &client_data.credit_limit,
                  &client_data.cash_payments) != -1) {

        while (fscanf(ofPTR_2, "%d %lf", &transfer.number, &transfer.cash_payments) != -1) {
            if (client_data.number == transfer.number && transfer.cash_payments != 0) {
                client_data.credit_limit += transfer.cash_payments;
            }
        }
        fprintf(blackrecord, "%-12d%-11s%-11s%-16s%20s%12.2f%12.2f%12.2f\n", client_data.number, client_data.name,
                client_data.surname, client_data.address, client_data.tel_number, client_data.indebtedness,
                client_data.credit_limit, client_data.cash_payments);
        rewind(ofPTR_2);
        
    }
}
