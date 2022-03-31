#include <stdio.h>
#include "prompts.h"


void choice_input_prompt() {
    printf("%s", "please enter action\n"
                 "1 enter data client:\n"
                 "2 enter data transaction:\n"
                 "3 update base\n");
}


void client_data_input_prompt() {
    printf("%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n\n",
           "1 number account: ",
           "2 Client name: ",
           "3 surname: ",
           "4 Addres client: ",
           "5 Client Telnum: ",
           "6 Client indebtedness: ",
           "7 Client credit limit: ",
           "8 Client cash payments: ");
}

void transaction_input_prompt() {
    printf("%s\n%s\n",
           "1 number account: ",
           "2 Client cash payments: ");
}
