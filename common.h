
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
#define MAX_QUEUE 50

using namespace std;



// Creates port.
uint16_t read_port(string string, bool *error);


// Reading while tcp.
uint8_t read_byte(int socket_fd, string *message);