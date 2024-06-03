
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



// Creates port.
uint16_t read_port(string string, bool *error);


// Reading while tcp.
uint8_t read_byte(int socket_fd, string *message);


// Writes report line.
void to_report(string message, string send_port, string send_address, string dest_port, string dest_address);

//
int get_data_6(int socket_fd, int *local_port, string *local_ip, int *dest_port, string *dest_ip);

int get_data_4(int socket_fd, int *local_port, string *local_ip, int *dest_port, string *dest_ip);