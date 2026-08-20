#include "library.h"
#include <stdio.h>
#include <string.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/un.h>
#include <stdint.h>
#include <sys/socket.h>

#define SOCK_PATH "/tmp/daemon.sock"

uint8_t sendNewBlock(char *id, uint8_t *secret, uint32_t data_length, void *data) {
    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock < 0) return 1;

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCK_PATH, sizeof(addr.sun_path) -1 );

    if (connect(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        close(sock);
        return 1;
    }

    if  (write(sock, "S", 1) != 1 ||
        write(sock, id, 64) != 64 ||
        write(sock, secret, 16) != 16 ||
        write(sock, &data_length, sizeof(data_length)) != sizeof(data_length) ||
        write(sock, data, data_length) != data_length){
        close(sock);
        return 1;
        }

    close(sock);
    return 0;
}

uint8_t getBlock(char *id, uint8_t *secret, uint8_t buffer_length, void *buffer) {
    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock < 0) return 1;

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, SOCK_PATH);

    if (connect(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        close(sock);
        return 1;
    }
    if (write(sock, "G", 1) != 1 ||
        write(sock, id, 64) != 64 ||
        write(sock, secret, 16) != 16) {
        close(sock);
        return 1;
        }
    uint32_t data_length;
    if (read(sock, &data_length, sizeof(data_length)) != sizeof(data_length)) {
        close(sock);
        return 1;
    }
    if (data_length > buffer_length) {
        close(sock);
        return 1;
    }
    if (read(sock, buffer, data_length) != data_length) {
        close(sock);
        return 1;

    }
    close(sock);
    return 0;
}




