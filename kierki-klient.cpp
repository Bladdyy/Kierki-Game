#include <iostream>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>

#include "common.h"
using namespace std;
string term("\r\n");

static struct sockaddr_in get_server_address(string host, uint16_t port, bool* error, int fam, int sock, int prot) {
    // Creating hints.
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = fam;
    hints.ai_socktype = sock;
    hints.ai_protocol = prot;

    // Getting address information.
    struct addrinfo *address_result;
    int errcode = getaddrinfo(host.c_str(), nullptr, &hints, &address_result);
    if (errcode != 0) {
        *error = true;
    }

    // Updating socket address information.
    struct sockaddr_in send_address;
    send_address.sin_family = fam;
    send_address.sin_addr.s_addr =
            ((struct sockaddr_in *) (address_result->ai_addr))->sin_addr.s_addr;
    send_address.sin_port = htons(port); // Port from the command line
    freeaddrinfo(address_result);
    return send_address;
}

int sit_down(const int socketfd, const char dir) {
    string iam("IAM");
    iam += dir;
    iam += term;
    tcp_write(socketfd, iam);
    return 0;
}


int main(int const argc, char* argv[]) {
    string host;           // Host address.
    string port_arg;       // Port number.
    int version = 0;       // Version of IP.
    char dir = 'X';        // Direction of place at the table
    bool automat = false;  // Enables automatic player.

    char get;
    while ((get = getopt(argc, argv, ":h:p:46NESWa")) != -1){
        switch (get){
            case 'h':
                host = optarg;
                break;
            case 'p':
                port_arg = optarg;
                break;
            case '4':
                version = 4;
                break;
            case '6':
                version = 6;
                break;
            case 'N':
                dir = 'N';
                break;
            case 'E':
                dir = 'E';
                break;
            case 'S':
                dir = 'S';
                break;
            case 'W':
                dir = 'W';
                break;
            case 'a':
                automat = true;
                break;
            default:
                break;
        }
    }
    if (!host.empty() && !port_arg.empty() && dir != 'X'){
        bool error = false;
        uint16_t port = read_port(port_arg, &error);
        if (error){  // There was an error getting port.
            return 1;
        }
        error = false;
        struct sockaddr_in server_address = get_server_address(host, port, &error, AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (error){  // There was an error getting server address.
            fprintf(stderr, "ERROR: Couldn't get address information.\n");
            return 1;
        }

        int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (socket_fd < 0) {  // There was an error creating a socket.
            fprintf(stderr,"ERROR: Couldn't create a socket\n");
            return 1;
        }
        if (connect(socket_fd, (struct sockaddr *) &server_address, (socklen_t) sizeof(server_address)) < 0) {
            fprintf(stderr, "ERROR: Couldn't connect to the server.");
            return 1;
        }
        sit_down(socket_fd, dir);
        close(socket_fd);
    }
    else{
        cout << "fuck off\n";
    }
    return 0;
}