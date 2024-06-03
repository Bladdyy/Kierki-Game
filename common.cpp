#include "common.h"



// Creates port, port_string - port number, error - checks for errors.
uint16_t read_port(string port_string, bool *error) {
    char *endptr;
    unsigned long port = strtoul(port_string.c_str(), &endptr, 10);
    if ((port == ULONG_MAX && errno == ERANGE) || *endptr != 0 || port == 0 || port > UINT16_MAX) {
        fprintf(stderr,"ERROR: Not a valid port number.\n");
        *error = true;
    }
    return (uint16_t) port;
}


// Receives messages using TCP protocol, socket_fd - socket descriptor, message - message to read from.
uint8_t read_byte(const int socket_fd, string *message){
    char let;
    ssize_t read_new = read(socket_fd, &let, 1);
    if (read_new < 0) {
        return 2;
    }
    if (read_new == 0) {
        return 3;
    }
    *message += let;
    uint8_t size = message->size();
    if (size >= 2 && message->substr(size - 2, 2) == "\r\n") {
        *message = message->substr(0, size - 2);
        return 1;
    }
    return 0;
}


// Writes new message as report line, message - message send/received, send_port - local port value,
// send_address - local IP address, dest_port - destination port value, dest_address - destination ip address.
void to_report(string message, string send_port, string send_address, string dest_port, string dest_address) {
    auto now = chrono::system_clock::now();
    time_t current = chrono::system_clock::to_time_t(now);
    tm local;
    localtime_r(&current, &local);
    char date[11];
    char time[10];
    strftime(date, sizeof(date), "%Y-%m-%d", &local);  // Creating current date.
    strftime(time, sizeof(time), "%H:%M:%S", &local);  // Creating current time.
    // Creating miliseconds.
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    cout << "[" << send_address << ":" << send_port << "," <<  dest_address << ":" << dest_port << "," << date << "T"
    << time << '.' << setfill('0') << setw(3) << milliseconds.count()  << "] " << message<< "\\r\\n" << endl;
}


// Gets IPv6 address and port of both server and client, socket_fd - socket descriptor, local_port - local port number,
// local - local IP address, dest_port - destination port number, dest - destination IP address.
int get_data_6(int socket_fd, int *local_port, string *local, int *dest_port, string *dest) {
    char local_ip[INET6_ADDRSTRLEN];  // Local IP buffer.
    char dest_ip[INET6_ADDRSTRLEN];   // Destination IP buffer.
    struct sockaddr_in6 local_addr = {};  // Local server address.
    socklen_t server_len = sizeof(local_addr);  // Server length.
    if (getsockname(socket_fd, (struct sockaddr *)&local_addr, &server_len) < 0) {  // Getting local information.
        fprintf(stderr, "ERROR: Couldn't get local address.\n");
        return 1;
    }
    if (inet_ntop(AF_INET6, &local_addr.sin6_addr, local_ip, sizeof(local_ip)) == nullptr) {  // Converting IP address.
        fprintf(stderr, "ERROR: Couldn't get local address.\n");
        return 1;
    }
    *local_port = ntohs(local_addr.sin6_port);  // Local port convert.
    *local = local_ip;  // Local IP save.

    struct sockaddr_in6 dest_addr = {};  // Destination server address.
    socklen_t client_len = sizeof(dest_addr);  // Client address length.
    if (getpeername(socket_fd, (struct sockaddr *)&dest_addr, &client_len) < 0) {  // Getting destinaton information.
        fprintf(stderr, "ERROR: Couldn't get destination address.\n");
        return 1;
    }
    if (inet_ntop(AF_INET6, &dest_addr.sin6_addr, dest_ip, sizeof(dest_ip)) == nullptr) {  // Converting IP address.
        fprintf(stderr, "ERROR: Couldn't get destination address.\n");
        return 1;
    }
    *dest_port = ntohs(dest_addr.sin6_port);  // Destination port convert.
    *dest = dest_ip;  // Destination IP save.
    return 0;
}


// Gets IPv4 address and port of both server and client, socket_fd - socket descriptor, local_port - local port number,
// local - local IP address, dest_port - destination port number, dest - destination IP address.
int get_data_4(int socket_fd, int *local_port, string *local, int *dest_port, string *dest) {
    char local_ip[INET_ADDRSTRLEN];  // Local IP buffer.
    char dest_ip[INET_ADDRSTRLEN];   // Destination IP buffer.
    struct sockaddr_in local_addr = {};  // Local server address.
    socklen_t server_len = sizeof(local_addr);  // Server length.
    if (getsockname(socket_fd, (struct sockaddr *)&local_addr, &server_len) < 0) {  // Getting local information.
        fprintf(stderr, "ERROR: Couldn't get local address.\n");
        return 1;
    }
    if (inet_ntop(AF_INET, &local_addr.sin_addr, local_ip, sizeof(local_ip)) == nullptr) {  // Converting IP address.
        fprintf(stderr, "ERROR: Couldn't get local address.\n");
        return 1;
    }
    *local_port = ntohs(local_addr.sin_port);  // Local port convert.
    *local = local_ip;  // Local IP save.

    struct sockaddr_in dest_addr = {};  // Destination server address.
    socklen_t client_len = sizeof(dest_addr);  // Client address length.
    if (getpeername(socket_fd, (struct sockaddr *)&dest_addr, &client_len) < 0) {  // Getting destinaton information.
        fprintf(stderr, "ERROR: Couldn't get destination address.\n");
        return 1;
    }
    if (inet_ntop(AF_INET, &dest_addr.sin_addr, dest_ip, sizeof(dest_ip)) == nullptr) {  // Converting IP address.
        fprintf(stderr, "ERROR: Couldn't get destination address.\n");
        return 1;
    }
    *dest_port = ntohs(dest_addr.sin_port);  // Destination port convert.
    *dest = dest_ip;  // Destination IP save.
    return 0;
}