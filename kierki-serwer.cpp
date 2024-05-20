#include <iostream>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <vector>
#include <fstream>
#include <list>
#include "common.h"
using namespace std;
string term("\r\n");

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
                break;
        }
    }
    if (!file.empty() && timeout > 0) {
        // TODO if (!port_arg.empty()) {
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
        }
        struct sockaddr_in client_address;
        socklen_t cli_size = sizeof(client_address);
        int client_fd = accept(socket_fd, (struct sockaddr *) &client_address,  &cli_size);
        if (client_fd < 0){
            fprintf(stderr, "ERROR: Couldn't connect with the client.\n");
        }
        string x;
        x = tcp_read(client_fd);
        cout << x << " od klienta\n";
        tcp_write(client_fd, "DEAL6N2H3H4H5H6D2DQSKSAS10H10S10D10C\r\n");
        tcp_write(client_fd, "TRICK110H\r\n");

        x = tcp_read(client_fd);

        cout << x << " " << x.size() << " od klienta\n";
        tcp_write(client_fd, "TAKEN110C4C2C3CN\r\n");

        tcp_write(client_fd, "TRICK210H\r\n");
        x = tcp_read(client_fd);
        tcp_write(client_fd, "WRONG2\r\n");
        cout << x << " " << x.size() << " od klienta\n";


        tcp_write(client_fd, "TRICK210H\r\n");
        x = tcp_read(client_fd);
        cout << x << " " << x.size() << " od klienta\n";
        tcp_write(client_fd, "TAKEN210C4C2C3CN\r\n");



        tcp_write(client_fd, "TRICK310H\r\n");

        x = tcp_read(client_fd);
        cout << x << " " << x.size() << " od klienta\n";
        tcp_write(client_fd, "TAKEN310C4C2C3CN\r\n");

        tcp_write(client_fd, "TRICK410H\r\n");
        x = tcp_read(client_fd);
        cout << x << " " << x.size() << " od klienta\n";
        tcp_write(client_fd, "TAKEN410C4C2C3CN\r\n");

        tcp_write(client_fd, "TRICK10C\r\n");

        tcp_write(client_fd, "TRICK5QH\r\n");
        x = tcp_read(client_fd);

        cout << x << " " << x.size() << " od klienta\n";
        tcp_write(client_fd, "TAKEN510C4C2C3CN\r\n");

        close(socket_fd);
    }
    else {
        cout << "fuck offff\n";
    }





    return 0;
}