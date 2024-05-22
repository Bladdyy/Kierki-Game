#include <iostream>
#include <cstdio>
#include <cstring>
#include <list>
#include <set>
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

void print_cards(set<string> cards) {
    if (!cards.empty()) {
        auto card = cards.begin();
        cout << *card;
        for (int i = 0; i < cards.size() - 1; i++) {
            card++;
            cout << ", " << *card;
        }
    }
}

int get_cards(set<string> *cards, string answer) {
    string card;
    bool num = false;
    for (char el: answer) {
        if (!num && (el == 'J' || el == 'Q' || el == 'K' || el == 'A' || (el >= '1' && el <='9'))) {
            card = el;
            if (el != '1') {
                num = true;
            }
        }
        else if (el == '0' && card == "1") {
            card = "10";
            num = true;
        }
        else if ((el == 'C' || el == 'D' || el == 'H' || el == 'S') && num) {
            card += el;
            pair<set<string>::iterator, bool> ret = (*cards).insert(card);
            if (!ret.second) {
                return 1;
            }
            num = false;
        }
        else {
            return 1;
        }
    }
    return 0;
}
int sit_down(const int socketfd, const char dir, set<string> *real_hand) {
    bool recieved = false;
    int code = -1;
    while (!recieved) {
        set<string> hand;
        string answer(tcp_read(socketfd));
        if (answer.size() >= 4 && answer.substr(0, 4) == "BUSY") {
            set<char> places;
            bool miss = false;
            for (char el: answer.substr(4, answer.size() - 4)) {
                int tab = -1;
                if (el == 'N' || el == 'S' || el == 'E' || el == 'S') {
                    pair<set<char>::iterator, bool> ret = places.insert(el);
                    if (!ret.second) {
                        miss = true;
                    }
                }
            }
            if (!miss) {
                string response("Place busy, list of busy places received:");
                for (const char el: places) {
                    response += " ";
                    response += el;
                }
                recieved = true;
                code = 1;
                cout << response << ".\n";
            }
        }
        else if (answer.size() >= 6 && answer.substr(0, 4) == "DEAL" && answer[4] <= '7' && answer[4] >= '1'
            && (answer[5] == 'N' || answer[5] == 'E' || answer[5] == 'S' || answer[5] == 'W')) {
            int get = get_cards(&hand, answer.substr(6, answer.size() - 6));
            if (get == 0 && hand.size() == 13) {
                recieved = true;
                code = 0;
                *real_hand = hand;
                cout << "New deal " << answer[4] << " staring place " << answer[5] << ", your cards: ";
                print_cards(hand);
                cout<< ".\n";
            }
        }
    }
    return code;
}


int play_turn(int socketfd, set<string> hand, bool automat) {
    int lewa = 1;
    bool repeat = false;
    string answer;
    while (lewa < 6){
        if (!repeat) {
            answer = tcp_read(socketfd);
        }

        set<string> cards;
        if (answer.size() >= 6 && answer.substr(0, 5) == "TRICK" && answer[5] == lewa + '0') {
            int get = get_cards(&cards, answer.substr(6, answer.size() - 6));
            if (get == 0 && cards.size() < 4) {
                lewa++;
                cout << "Trick: " << answer[5] << " ";
                print_cards(cards);
                cout << "\n";
                cout << "Available: ";
                print_cards(hand);
                cout << "\n";
                string play;
                if (automat) {
                    if (!cards.empty()) {
                        string match = *cards.begin();
                        for (string card: hand) {
                            if (card.back() == match.back()) {
                                play = card;
                                break;
                            }
                        }
                    }
                    if (play.empty()) {
                        play = *hand.begin();
                    }
                }
                else {
                    bool okay = false;
                    while (!okay) {
                        cin >> play;
                        // TODO Zawsze można skorzystać z cards i tricks (nie blokuje komunikacja z serwerem.)
                        if (play == "cards") {
                            print_cards(hand);
                            cout << "\n";
                        }
                        else if (play == "tricks") {
                            cout << "tricks\n";
                        }
                        else if(play[0] == '!') {
                            play = play.substr(1, play.size() - 1);
                            set<string> card;
                            int get = get_cards(&card, play);
                            if (get == 0 && card.size() == 1 &&  hand.find(*card.begin()) != hand.end()) {
                                okay = true;
                            }
                        }
                    }
                }
                string msg("TRICK");
                msg += (answer[5] + play + term);
                tcp_write(socketfd, msg);
                bool passed = false;
                while(!passed) {
                    string accept = tcp_read(socketfd);
                    // TODO popraw accept.size() z 6, bo lewa nie musi być jednocyfrowa.
                    if (accept.size() == 6 && accept.substr(0, 5) == "WRONG" && accept[5] == lewa - 1 + '0') {
                        lewa--;
                        repeat = true;
                        passed = true;
                        cout << "Wrong message received in trick " << accept[5] << ".\n";
                    }
                    else if (accept.size() > 7 && accept.substr(0, 5) == "TAKEN" && accept[5] == lewa - 1 + '0' && (accept.back() == 'N' || accept.back() == 'E' || accept.back() == 'S' || accept.back() == 'W')) {
                        set<string> taken;
                        int got = get_cards(&taken, accept.substr(6, accept.size() - 7));
                        if (got == 0 && taken.size() == 4) {
                            hand.erase(play);
                            passed = true;
                            repeat = false;
                            cout << "A trick " << accept[5] << " is taken by " << accept.back()<< ", cards ";
                            print_cards(taken);
                            cout << ".\n";
                        }
                    }
                }
            }
        }
    }
    return 0;
}

int main(int const argc, char* argv[]) {
    string host;           // Host address.
    string port_arg;       // Port number.
    int version = 0;       // Version of IP.
    char dir = 'X';        // Direction of place at the table
    bool automat = false;  // Enables automatic player.

    // Getting option arguments.
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

    // If requested arguments were given.
    if (!host.empty() && !port_arg.empty() && dir != 'X'){
        bool error = false;  // Checks for error in functions.
        uint16_t port = read_port(port_arg, &error); // Reading port.
        if (error){  // There was an error getting port.
            return 1;
        }
        error = false;
        struct sockaddr_in server_address = get_server_address(host, port, &error, AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (error){  // There was an error getting server address.
            fprintf(stderr, "ERROR: Couldn't get address information.\n");
            return 1;
        }
        int socket_fd = socket(AF_INET, SOCK_STREAM, 0);  // TODO ogarnięcie jak robić ipv4 lub ipv6
        if (socket_fd < 0) {  // There was an error creating a socket.
            fprintf(stderr,"ERROR: Couldn't create a socket\n");
            return 1;
        }
        if (fcntl(socket_fd, F_SETFL, O_NONBLOCK) == -1) {  // Setting socket to be non-blocking.
            fprintf(stderr, "ERROR: Couldn't set non-blocking socket.\n");
            return 1;
        }

        // Connecting to the server.
        // if (connect(socket_fd, (struct sockaddr *) &server_address, (socklen_t) sizeof(server_address)) < 0) {
        //     fprintf(stderr, "ERROR: Couldn't connect to the server.\n");
        //     return 1;
        // }

        string message("IAM");  // First message to send to the server.
        message += dir;
        message += term;
        int to_send = message.size();  // Size of the message.

        set<string> hand;  // Cards in hand.
        struct pollfd poll_descriptors[2];
        poll_descriptors[0].fd = socket_fd;     // Descriptoe used to contact server.
        poll_descriptors[0].events = POLLOUT;   // Looking for space to write to the server.
        poll_descriptors[1].fd = STDIN_FILENO;  // STDIN descriptor.
        poll_descriptors[1].events = POLLIN;    // Waiting for message from STDIN.
        int check = 0;  // TODO: Wywalenie check.
        do {
            for (int i = 0; i < 2; ++i) {  // Setting revents to zero.
                poll_descriptors[i].revents = 0;
            }
            int actions = poll(poll_descriptors, 2, -1);
            if (actions == -1) {  // Poll error.
                fprintf(stderr, "ERROR: There was an error while polling.\n");
            }
            else if (actions > 0) {  // Any event detected.
                if (poll_descriptors[0].revents & POLLOUT) {  // Client is able to write.
                    int written = tcp_write(socket_fd, message);
                    if (written <= 0) {  // TODO: ERROR HANDLING?.
                        fprintf(stderr, "ERROR: Writing\n");
                    }
                    else {
                        to_send -= written;
                    }
                    if (to_send == 0) {
                        poll_descriptors[0].events = POLLIN;
                    }
                }


                if (poll_descriptors[1].revents & POLLIN) {
                    string recv;
                    size_t size = 0;
                    getline(cin, recv);
                    cout << "this " << recv << "\n";
                    check++;
                }
            }
        } while (check < 3);
        // sit_down(socket_fd, dir, &hand);
        // play_turn(socket_fd, hand, automat);
        close(socket_fd);
    }
    else{
        cout << "fuck off\n";
    }
    return 0;
}