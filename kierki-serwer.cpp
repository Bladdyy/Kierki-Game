#include <iostream>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fstream>
#include <list>
#include "common.h"
using namespace std;
string term("\r\n");
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
int read_file(string file, list<string> *modes, list<string> *hands) {
    ifstream inputFile(file);
    // Check if the file is successfully opened
    if (!inputFile.is_open()) {
        return 1;
    }
    string line;
    int new_line = 0;
    while (getline(inputFile, line)) {
        if (!line.empty()) {
            new_line++;
            if (new_line == 1) {
                (*modes).emplace_back(line);
            }
            else {
                (*hands).emplace_back(line);
                if (new_line == 5) {
                    new_line = 0;
                }
            }
        }
    }
    // Close the file
    inputFile.close();
    return 0;
}


// Creates server with specified protocol.
int main(int argc, char *argv[]) {
    list<string> modes;
    list<string> hands;
    string file;      // Host address.
    string port_arg;  // Port number.
    int timeout = 5;  // Version of IP.
    char get;

    // Looking for invalid arguments that aren't an option.
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] != '-' && (strcmp(argv[i - 1], "-f") != 0 && strcmp(argv[i - 1], "-p") != 0 && strcmp(argv[i - 1], "-t") != 0)) {
            fprintf(stderr, "ERROR: Invalid additional argument.\n");
            return 1;
        }
    }

    while ((get = getopt(argc, argv, ":p:f:t:")) != -1) {
        switch (get){
            case 'f':
                file = optarg;
                break;
            case 'p':
                port_arg = optarg;
                break;
            case 't':
                timeout = atoi(optarg);
                break;
            default:
                fprintf(stderr, "ERROR: Invalid option argument.\n");
                return 1;
        }
    }
    if (!file.empty() && timeout > 0) {
        if (!port_arg.empty()) {
            bool error = false;
            uint16_t port = read_port(port_arg, &error);
            if (error){  // There was an error getting port.
                return 1;
            }

            if (read_file(file, &modes, &hands) == 1){
                fprintf(stderr, "Couldn't open the file.\n");
                return 1;
            }

            // Create socket.
            int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
            if (socket_fd < 0) {  // There was an error creating a socket.
                fprintf(stderr,"ERROR: Couldn't create a socket\n");
                return 1;
            }

            // Bind the socket to an address.
            struct sockaddr_in server_address;
            server_address.sin_family = AF_INET;
            server_address.sin_addr.s_addr = htonl(INADDR_ANY);
            server_address.sin_port = htons(port);

            // Binding.
            if (bind(socket_fd, (struct sockaddr *) &server_address, (socklen_t) sizeof(server_address)) < 0) {
                fprintf(stderr, "ERROR: Couldn't bind the socket.\n");
                return 1;
            }
            // Listening.
            if (listen(socket_fd, MAX_QUEUE) < 0){
                fprintf(stderr, "ERROR: Couldn't listen.\n");
                return 1;
            }
            socklen_t length = (socklen_t) sizeof(server_address);
            if (getsockname(socket_fd, (struct sockaddr*) &server_address, &length) < 0){
                fprintf(stderr, "ERROR: Couldn't find the port to listen on.\n");
                return 1;
            }
            struct sockaddr_in client_address;
            socklen_t cli_size = sizeof(client_address);
            int client_fd = accept(socket_fd, (struct sockaddr *) &client_address,  &cli_size);
            if (client_fd < 0){
                fprintf(stderr, "ERROR: Couldn't connect with the client.\n");
                return 1;
            }
            close(socket_fd);
        }
    }
    else {
        fprintf(stderr, "ERROR: Wrong arguments received.\n");
        return 1;
    }
    return 0;
}