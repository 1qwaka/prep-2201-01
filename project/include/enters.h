#ifndef PROJECT_INCLUDE_ENTERS_H_
#define PROJECT_INCLUDE_ENTERS_H_

#include <stdio.h>
#include "types.h"

void enter_client_data(FILE *client_data_file, data_t client);

void enter_transaction(FILE *transaction_file, data_t transaction);

#endif  // PROJECT_INCLUDE_ENTERS_H_
