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
uint8_t read_byte(const int socket_fd, string *message){
    char let;
    ssize_t read_new = read(socket_fd, &let, 1);
    if (read_new < 0) {
        return 2;
    }
    *message += let;
    uint8_t size = message->size();
    if (size >= 2 && message->substr(size - 2, 2) == "\r\n") {
        *message = message->substr(0, size - 2);
        return 1;
    }
    return 0;
}