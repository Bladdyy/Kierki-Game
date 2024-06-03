
// Max package size.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <arpa/inet.h>
#include <netinet/in.h>

#define MAX_QUEUE 50

using namespace std;


// Creates port, port_string - port number, error - checks for errors.
uint16_t read_port(string string, bool *error);


// Receives messages using TCP protocol, socket_fd - socket descriptor, message - message to read from.
uint8_t read_byte(int socket_fd, string *message);


// Writes new message as report line, message - message send/received, send_port - local port value,
// send_address - local IP address, dest_port - destination port value, dest_address - destination ip address.
void to_report(string message, string send_port, string send_address, string dest_port, string dest_address);


// Gets IPv6 address and port of both server and client, socket_fd - socket descriptor, local_port - local port number,
// local - local IP address, dest_port - destination port number, dest - destination IP address.
int get_data_6(int socket_fd, int *local_port, string *local_ip, int *dest_port, string *dest_ip);


// Gets IPv4 address and port of both server and client, socket_fd - socket descriptor, local_port - local port number,
// local - local IP address, dest_port - destination port number, dest - destination IP address.
int get_data_4(int socket_fd, int *local_port, string *local_ip, int *dest_port, string *dest_ip);