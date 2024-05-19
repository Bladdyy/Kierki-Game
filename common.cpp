#include "common.h"

#include <string>
#include <vector>


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


// Sends messages using TCP protocol.
int tcp_write(const int socket_fd, string data){
    uint32_t to_write = data.size();  // Size to write.
    uint32_t written = 0;      // Already sent bytes.
    while (to_write > 0){
        ssize_t done = write(socket_fd, &data[written], to_write);
        if (done <= 0){  // Error while sending.
            return 1;
        }
        written += done;
        to_write -= done;
    }
    return 0;
}


// Receives messages using TCP protocol.
string tcp_read(const int socket_fd){
    int got = 0;
    char let;
    string msg;
    while (got < 2) {
        ssize_t read_new = read(socket_fd, &let, 1);
        if (read_new <= 0) {
            return "";
        }
        if (let == '\r') {
            got = 1;
        }
        else if (let == '\n' && got == 1) {
            got = 2;
        }
        else {
            msg += let;
            got = 0;
        }
    }
    return msg;
}