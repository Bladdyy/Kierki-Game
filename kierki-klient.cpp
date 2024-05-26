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
uint8_t get_lewa(string answer, uint8_t lewa) {
    string str_lewa = to_string(lewa);
    if (lewa > 9 && answer.size() >= 2 && str_lewa[0] == answer[0] && str_lewa[1] == answer[1]) {
        return 0;
    }
    if (lewa <= 9 && !answer.empty() && str_lewa[0] == answer[0]) {
        return 0;
    }
    return 1;
}

// TODO popraw accept.size() z 6, bo lewa nie musi być jednocyfrowa.

// Odbiera komunikat 'TRICK' oraz jeśli automat ma wartość true, to zagrywa kartę. hand - ręka gracza,
// automat - gracz automatyczny, answer - komunikat odebrany od serwera, action - ID następnego spodziewanego komunikatu,
// played - zagrana karta, waiting - oczekiwanie na kartę od gracza nieautomatycznego.
void play_card(set<string> hand, bool automat, string answer, uint8_t* action, string *played, bool* waiting) {
    set<string> cards;
    int get = get_cards(&cards, answer.substr(6, answer.size() - 6));  // Odbiera
    if (get == 0 && cards.size() < 4) {
        cout << "Trick: " << answer[5] << " ";
        print_cards(cards);
        cout << "\n";
        cout << "Available: ";
        print_cards(hand);
        cout << "\n";
        string play;
        if (automat) {
            if (!cards.empty()) {
                play = *hand.begin();
                string match = *cards.begin();
                for (string card: hand) {
                    if (card.back() == match.back()) {
                        play = card;
                        break;
                    }
                }
            }
            *played = play;
        }
        else {
            *waiting = true;
        }
        *action = 4;
    }
}


uint8_t determine_action(uint8_t *action, string answer, set<string> *real_hand, uint8_t *lewa, string *played, bool automat, bool *waiting) {
    int code = 0;
    char dirs[4] = {'N', 'E', 'S', 'W'};
    uint8_t offset;
    if (*lewa > 9) {
        offset = 2;
    }
    else {
        offset = 1;
    }
    if (*action == 2 && answer.size() >= 5 + offset && get_lewa(answer.substr(5, offset), *lewa) == 0
            && answer.substr(0, 5) == "TRICK") {
        play_card(*real_hand, automat, answer, action, played, waiting);
    }
    else if (*action == 4 &&  get_lewa(answer.substr(5, offset), *lewa) == 0) {
        if (answer.size() == 5 + offset && answer.substr(0, 5) == "WRONG") {
            cout << "Wrong message received in trick " << *lewa << ".\n";
            *waiting = false;
        }
        else if (answer.size() = 6 + offset && answer.substr(0, 5) == "TAKEN"
                && find(begin(dirs), end(dirs), answer.back())) {
            set<string> taken;
            int got = get_cards(&taken, answer.substr(6, answer.size() - 7));
            if (got == 0 && taken.size() == 4) {
                (*real_hand).erase(*played);
                cout << "A trick " << answer[5] << " is taken by " << answer.back()<< ", cards ";
                print_cards(taken);
                cout << ".\n";
                if (*lewa == 13) {
                    *action = 3;
                    *lewa = 0;
                }
                else {
                    *action = 2;
                    *lewa = *lewa + 1;
                }
            }
        }
    }
    else if ((*action == 1 || *action == 3) && answer.size() >= 6 && answer.substr(0, 4) == "DEAL" && answer[4] <= '7' && answer[4] >= '1'
        && find(begin(dirs), end(dirs), answer[5])) {
        set<string> hand;
        int get = get_cards(&hand, answer.substr(6, answer.size() - 6));
        if (get == 0 && hand.size() == 13) {
            *real_hand = hand;
            cout << "New deal " << answer[4] << " staring place " << answer[5] << ", your cards: ";
            print_cards(hand);
            cout<< ".\n";
            *action = 2;
            *lewa = 1;
        }
    }
    else if (*action == 1 && answer.size() >= 4 && answer.substr(0, 4) == "BUSY") {
        set<char> places;
        bool miss = false;
        for (char el: answer.substr(4, answer.size() - 4)) {
            if (el == 'N' || el == 'S' || el == 'E' || el == 'S') {
                pair<set<char>::iterator, bool> ret = places.insert(el);
                if (!ret.second) {
                    miss = true;
                }
            }
            else {
                miss = true;
            }
        }
        if (!miss) {
            string response("Place busy, list of busy places received:");
            for (const char el: places) {
                response += " ";
                response += el;
            }
            code = 1;
            cout << response << ".\n";
        }
    }
    return code;
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

        // TODO Nieblokujący socket nie działa ;((
        // if (fcntl(socket_fd, F_SETFL, O_NONBLOCK) == -1) {  // Setting socket to be non-blocking.
        //     fprintf(stderr, "ERROR: Couldn't set non-blocking socket.\n");
        //     return 1;
        // }
        // Connecting to the server.
        if (connect(socket_fd, (struct sockaddr *) &server_address, (socklen_t) sizeof(server_address)) < 0) {
            fprintf(stderr, "ERROR: Couldn't connect to the server.\n");
            return 1;
        }

        string message("IAM");  // First message to send to the server.
        message += dir;
        message += term;
        uint8_t to_send = message.size();  // Size of the message.
        string reciever;
        string played;
        uint8_t action = 1;
        int poll_act;
        uint8_t lewa = 0;
        set<string> hand;  // Cards in hand.
        struct pollfd poll_descriptors[2];
        poll_descriptors[0].fd = socket_fd;     // Descriptoe used to contact server.
        poll_descriptors[0].events = POLLOUT;   // Looking for space to write to the server.
        poll_descriptors[1].fd = STDIN_FILENO;  // STDIN descriptor.
        poll_descriptors[1].events = POLLIN;    // Waiting for message from STDIN.
        bool force_quit = false;  // TODO: Wywalenie check.
        bool waiting = false;
        do {
            if (waiting) {
                poll_descriptors[1].revents = 0;
                poll_act = poll(&poll_descriptors[1], 1, -1);
            }
            else {
                for (int i = 0; i < 2; ++i) {  // Setting revents to zero.
                    poll_descriptors[i].revents = 0;
                }
                poll_act = poll(poll_descriptors, 2, -1);
            }
            if (poll_act == -1) {  // Poll error.
                fprintf(stderr, "ERROR: There was an error while polling.\n");
            }
            else if (poll_act > 0) {  // Any event detected.
                if (poll_descriptors[0].revents & POLLOUT) {  // Client is able to write.
                    ssize_t done = write(socket_fd, &message, to_send);
                    if (done <= 0) {
                        fprintf(stderr, "ERROR: Writing\n");
                        return 1;
                    }
                    to_send -= done;
                    if (to_send == 0) {
                        poll_descriptors[0].events = POLLIN;
                    }
                    else {
                        message = message.substr(done, message.size() - done);
                    }
                }
                else if (poll_descriptors[0].revents & POLLIN) {
                    uint8_t ret = 0;
                    if (!reciever.empty() && reciever.back() == '\r') {
                        string recv = tcp_read(socket_fd, true, &ret);
                        reciever += recv;
                    }
                    if (ret == 2) {
                        fprintf(stderr, "ERROR: Couldn't read message.\n");
                        return 1;
                    }
                    if (ret == 1) {
                        uint8_t code = determine_action(&action, reciever, &hand, &lewa, &played, automat, &waiting);
                        if (code == 1) {
                            force_quit = true;
                        }
                    }
                }
                if (poll_descriptors[1].revents & POLLIN) {
                    string recv;
                    size_t size = 0;
                    getline(cin, recv);
                    if (recv == "cards") {
                        print_cards(hand);
                        force_quit = true;
                        cout << "\n";
                    }
                    else if (recv == "tricks") {
                        cout << "tricks\n";
                    }
                    else if(recv == "!" && waiting) {
                        message = recv.substr(1, recv.size() - 1);
                        played = message;
                        message += term;
                        waiting = false;
                        poll_descriptors[0].revents = POLLOUT;
                    }
                }
            }
        } while (!force_quit);
        close(socket_fd);
    }
    else{
        cout << "fuck off\n";
    }
    return 0;
}