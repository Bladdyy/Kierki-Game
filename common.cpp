#include "common.h"

#include <iostream>

// Creates port.
uint16_t read_port(string string, bool *error) {
    char *endptr;
    unsigned long port = strtoul(string.c_str(), &endptr, 10);
    if ((port == ULONG_MAX && errno == ERANGE) || *endptr != 0 || port == 0 || port > UINT16_MAX) {
        fprintf(stderr,"ERROR: Not a valid port number.\n");
        *error = true;
    }
    return (uint16_t) port;
}



// Receives messages using TCP protocol.
string tcp_read(const int socket_fd, bool onr, uint8_t *ret){
    int got;
    if (onr) {
        got = 1;
    }
    else {
        got = 0;
    }
    char let;
    string msg;
    ssize_t read_new;
    do {
        read_new = read(socket_fd, &let, 1);
        if (read_new < 0) {
            *ret = 2;
            got = 2;
        }
        else if (let == '\r') {
            got = 1;
        }
        else if (let == '\n' && got == 1) {
            got = 2;
            *ret = 1;
        }
        else {
            msg += let;
            got = 0;
        }
    } while (got < 2 && read_new > 0);
    return msg;
}