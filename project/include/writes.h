#ifndef PROJECT_INCLUDE_WRITES_H_
#define PROJECT_INCLUDE_WRITES_H_

#include <stdio.h>
#include "types.h"

void write_client_data(FILE *data_file, data_t client);

void write_transaction(FILE *transaction_file, data_t transaction);

#endif  // PROJECT_INCLUDE_WRITES_H_
