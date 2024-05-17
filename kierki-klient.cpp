#include <iostream>
#include <stdio.h>
#include <unistd.h>
using namespace std;

int main(int argc, char* argv[]) {
    char *host = NULL;     // Host address.
    char *port = NULL;     // Port number.
    int version = 0;       // Version of IP.
    int dir = 0;           // Direction of place at the table
    bool automat = false;  // Enables automatic player.

    char get;
    while ((get = getopt(argc, argv, ":h:p:46NESWa")) != -1){
        switch (get){
            case 'h':
                host = optarg;
            break;
            case 'p':
                port = optarg;
            break;
            case '4':
                version = 4;
            break;
            case '6':
                version = 6;
            break;
            case 'N':
                dir = 1;
            break;
            case 'E':
                dir = 2;
            break;
            case 'S':
                dir = 3;
            break;
            case 'W':
                dir = 4;
            break;
            case 'a':
                automat = true;
            break;
        }
    }
    if (host != NULL && port != NULL && dir != 0){
        cout << "host: " << host << " port: " << port << " version: " << version << " direction: " << dir << " automat: " << automat << "\n";
    }
    else{
        cout << "fuck off\n";
    }
    return 0;
}