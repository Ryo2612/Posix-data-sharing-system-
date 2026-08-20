#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "library.h"

int main() {
    char id[64] = {0};
    strncpy(id, "Block1", sizeof(id)-1);
    uint8_t secret [16] = {0};
    char data[] = "Hello";
    uint32_t data_length = strlen(data) + 1;

    uint8_t clientdata = sendNewBlock(id, secret, data_length, data);

    if (clientdata == 0) {
        printf("stored block \n");
    }
    else {
        printf("storing block failed\n");
    }

    return 0;
}


