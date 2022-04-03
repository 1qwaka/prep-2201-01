#ifndef PROJECT_INCLUDE_SCANS_H_
#define PROJECT_INCLUDE_SCANS_H_

#include <stdio.h>
#include "types.h"

int scan_client_data(data_t *client);

int scan_transaction(data_t *transaction);

int fscan_client_data(FILE *client_data_file, data_t *client);

int fscan_transaction(FILE *transaction_file, data_t *transaction);

#endif  // PROJECT_INCLUDE_SCANS_H_
