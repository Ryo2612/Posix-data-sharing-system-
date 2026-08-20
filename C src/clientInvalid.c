#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "library.h"

int main() {
    char id[] = "block1";
    uint8_t secret [16] = {99, 99, 99, 99, 99, 99 , 99, 99, 99,99,99,99,99,99,99,99,99,99};
    char buffer [128];

    uint8_t client2data = getBlock(id, secret, sizeof(buffer), buffer);

    if (getBlock(id, secret, sizeof(buffer), buffer) == 0) {
        printf("success\n");
    }
    else {
        printf("failed\n");
    }

    return 0;

}


