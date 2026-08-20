#ifndef LIBRARY_H
#define LIBRARY_H

#include <stdint.h>

#define SOCKET_PATH "/tmp/socket"

//id = unique string identifier for block
//secret = 16 byte authentication string
//data_length = length of data in bytes
//data = pointer to blocks data
//buffer_length = how much data was copied from memory
//buffer = pointer to pre allocated memory

uint8_t sendNewBlock(char *id, uint8_t *secret, uint32_t data_length, void *data);
uint8_t getBlock(char *id, uint8_t *secret, uint8_t buffer_length, void *buffer);

#endif //LIBRARY_H
