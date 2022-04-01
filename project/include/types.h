#ifndef PROJECT_INCLUDE_TYPES_H_
#define PROJECT_INCLUDE_TYPES_H_

typedef enum {
    NAME_SIZE = 20,
    SURNAME_SIZE = 20,
    ADRESS_SIZE = 30,
    TEL_NUMBER_SIZE = 15
} arr_sizes_t;

typedef enum {
    NUMBER_WRITE_W = 12,
    NAME_WRITE_W = 11,
    SURNAME_WRITE_W = 11,
    ADRESS_WRITE_W = 16,
    TEL_NUMBER_WRITE_W = 20,
    INDEBTEDNESS_WRITE_W = 12,
    CREDIT_LIMIT_WRITE_W = 12,
    CASH_PAYMENTS_WRITE_W = 12
} client_data_write_width_t;

typedef enum {
    TRS_NUMBER_WRITE_W = 3,
    TRS_CASH_PAYMENTS_W = 6,
} transaction_write_width_t;

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

#endif  // PROJECT_INCLUDE_TYPES_H_
