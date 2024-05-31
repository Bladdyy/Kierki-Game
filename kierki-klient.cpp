// TODO Raport z rozgrywki klient automatyczny
// TODO obsługa nadmiernych argumentów.
#include <fcntl.h>
#include <iostream>
#include <cstdio>
#include <cstring>
#include <list>
#include <set>
#include <map>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>

#include "common.h"
using namespace std;

string term("\r\n");  // End of the message symbols.
string dirs("NESW");  // Often used to check if directions of the table are correct.


// Gets address of the server for IPv4 and connects to it.
int get_server_address_and_connect_4(string host, uint16_t port) {
    int socket_fd;  // Socket number.
    // Creating hints.
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    // Getting address information.
    struct addrinfo *address_result;
    int errcode = getaddrinfo(host.c_str(), nullptr, &hints, &address_result);
    if (errcode != 0) {
        fprintf(stderr, "ERROR: Couldn't get address information.\n");
        socket_fd = -1;
    }
    else {
        // Updating socket address information.
        struct sockaddr_in send_address;
        memset(&send_address, 0, sizeof(sockaddr_in));
        send_address.sin_family = address_result->ai_family;
        send_address.sin_addr.s_addr =
                ((struct sockaddr_in *) (address_result->ai_addr))->sin_addr.s_addr;
        send_address.sin_port = htons(port); // Port from the command line
        freeaddrinfo(address_result);
        // Creating new socket.
        socket_fd = socket(send_address.sin_family, SOCK_STREAM, 0);
        if (socket_fd < 0) {  // There was an error creating a socket.
            fprintf(stderr,"ERROR: Couldn't create a socket\n");
        }
        else {
            // Connecting to the server.
            if (connect(socket_fd, (const struct sockaddr *) &send_address, sizeof(send_address)) < 0) {
                close(socket_fd);
                socket_fd = -1;
                fprintf(stderr, "ERROR: Couldn't connect to the server.\n");
            }
        }
    }
    return socket_fd;
}


// Gets address of the server for IPv6 and connects to it.
int get_server_address_and_connect_6(string host, uint16_t port) {
    int socket_fd;  // Socket value.
    // Creating hints.
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Getting address information.
    struct addrinfo *address_result;
    int errcode = getaddrinfo(host.c_str(), nullptr, &hints, &address_result);
    if (errcode != 0) {
        fprintf(stderr, "ERROR: Couldn't get address information.\n");
        socket_fd = -1;
    }
    else {
        // Updating socket address information.
        struct sockaddr_in6 send_address;
        memset(&send_address, 0, sizeof(sockaddr_in6));
        send_address = *((struct sockaddr_in6*)(address_result->ai_addr));
        send_address.sin6_port = htons(port); // Port from the command line
        freeaddrinfo(address_result);

        // Creating new socket.
        socket_fd = socket(AF_INET6, SOCK_STREAM, 0);
        if (socket_fd < 0) {  // There was an error creating a socket.
            fprintf(stderr,"ERROR: Couldn't create a socket\n");
        }
        else {
            // Connecting to the server.
            if (connect(socket_fd, (struct sockaddr *) &send_address, sizeof(send_address)) < 0) {
                close(socket_fd);
                socket_fd = -1;
                fprintf(stderr, "ERROR: Couldn't connect to the server.\n");
            }
        }
    }
    return socket_fd;
}


// Gets server address with IP version not specified. Connects to the server.
int get_any_address(string host, uint16_t port) {
    struct addrinfo hints;  // Creating hints.
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;

    char ports[6];
    snprintf(ports, sizeof(ports), "%d", port);  // Port value to string.
    int socket_fd;  // Socket value.

    // Getting address information.
    struct addrinfo *address_result;
    int errcode = getaddrinfo(host.c_str(), ports, &hints, &address_result);
    if (errcode != 0) {
        fprintf(stderr, "ERROR: Couldn't get address information.\n");
        socket_fd = -1;
    }
    else {
        struct addrinfo *many;
        // Get the right address.
        for (many = address_result; many != nullptr; many = many->ai_next) {
            socket_fd = socket(many->ai_family, many->ai_socktype, many->ai_protocol);
            if (socket_fd >= 0) {
                // Connecting to the server.
                if (connect(socket_fd, many->ai_addr, many->ai_addrlen) < 0) {
                    close(socket_fd);
                }
                else {
                    break;
                }
            }
        }
        freeaddrinfo(address_result);
        if (many == nullptr) {  // Couldn't find good address.
            fprintf(stderr, "ERROR: Couldn't connect to server.\n");
            socket_fd = -1;
        }
    }
    return socket_fd;
}

// Prints list of all the cards.
void print_cards(list<string> cards) {
    if (!cards.empty()) {  // If there are any cards in the list.
        auto card = cards.begin();
        cout << *card << flush;
        for (int i = 0; i < cards.size() - 1; i++) {
            ++card;
            cout << ", " << *card << flush;
        }
    }
}


// Checks if there is exactly one card from client's hand in taken cards and erases it from hand.
// real_hand - list of cards in hand, taken - list of taken cards, played - card played in current trick,
// action - type of next message to handle.
uint8_t in_hand(list<string> *real_hand, list<string> taken, string played, uint8_t action) {
    string card;  //
    uint8_t in_hand = 0;
    list<string>::iterator del;
    set<string> temp;
    for (string el: taken) {
        temp.insert(el);
    }
    for (auto it = real_hand->begin(); it != real_hand->end(); ++it) {
        pair<set<string>::iterator, bool> ret = temp.insert(*it);  // Trying to add to hand.
        if (!ret.second) {  // If this card already was in hand.
            in_hand++;
            card = *it;
            del = it;
        }
    }
    if (in_hand == 1 && (action == 6 || card == played)) {
        real_hand->erase(del);
    }
    else {
        in_hand = 0;
    }
    return in_hand;
}

// Extracts card numbers from 'answer',
int get_cards(list<string> *cards, string answer) {
    string card;       // Currently extracted card.
    bool num = false;  // Number got.
    for (char el: answer) {
        // Getting grade of card.
        if (!num && (el == 'J' || el == 'Q' || el == 'K' || el == 'A' || (el >= '1' && el <='9'))) {
            card = el;  // Saving grade of the card.
            if (el != '1') {  // If grade of the card has only one symbol.
                num = true;
            }
        }
        else if (el == '0' && card == "1") {  // If grade of the card is equal to '10'.
            card = "10";
            num = true;
        }
        // Getting the color of the card if grade is extracted.
        else if ((el == 'C' || el == 'D' || el == 'H' || el == 'S') && num) {
            card += el;
            for (string got: *cards) {
                if (card == got) {  // If server gave a duplicate of the same card.
                    return 1;
                }
            }
            cards->emplace_back(card);  // Adding to the hand.
            num = false;
        }
        else {  // If there was an illegal symbol.
            return 1;
        }
    }
    return 0;
}


// Checks if number of turn is proper.
uint8_t get_lewa(string answer, uint8_t lewa) {
    string str_lewa = to_string(lewa);
    // If turn number is double digits.
    if (lewa > 9 && answer.size() >= 2 && str_lewa[0] == answer[0] && str_lewa[1] == answer[1]) {
        return 0;
    }
    // If turn number is one digit.
    if (lewa <= 9 && !answer.empty() && str_lewa[0] == answer[0]) {
        return 0;
    }
    return 1;
}


// Handles 'TRICK'. Forces program to pick a card and send it to the server.
// Displays trick info. Picks a card to play if 'automat' is true.
// Hand is the list of client's cards, automat - automatically picks card to play, answer - message received from server,
// action - type of next message to handle, played - ID of card to play, waiting - wait for a card pick from user.
// offset - number of digits in the number of turn, lewa - number of the turn.
uint8_t play_card(list<string> hand, bool automat, string answer, uint8_t* action, string *played, bool* waiting, uint8_t offset, uint8_t *lewa) {
    list<string> cards;  // Cards played in current turn by other players.
    int get = get_cards(&cards, answer.substr(5 + offset, answer.size() - 5 - offset));  // Extracts list of card from message.
    if (get == 0 && cards.size() < 4) {  // If there are max 3 cards.
        cout << "Trick: (" << to_string(*lewa) << ") " << flush;
        print_cards(cards);
        cout << "\n" << flush;
        cout << "Available: " << flush;
        print_cards(hand);
        cout << "\n" << flush;

        *action = 4;  // Next - receive 'WRONG' or 'TAKEN'.

        string play;  // Played card.
        // Picks card to play automatically.
        if (automat) {
            play = *hand.begin();
            if (!cards.empty()) {
                string match = *cards.begin();
                for (string card: hand) {
                    if (card.back() == match.back()) {
                        play = card;
                        break;
                    }
                }
            }
            *played = play;
            return 2;
        }
        *waiting = true;  // Waiting for user to pick a card to play.
    }
    return 0;
}


// Handles 'WRONG' and 'TAKEN'. Displays communicate info.
// Answer - message received from server, lewa - number of the turn, offset - length of received turn number,
// real_hand - list of client's cards, played - last played card, action - type of next message to handle,
// tricks list of tricks taken, dir - ID of seat occupied by client.
void trick_answer(string answer, uint8_t *lewa, uint8_t offset, list<string> *real_hand,
        string *played, uint8_t *action, list<list<string>> *tricks, char dir) {
    // Correct 'WRONG'.
    if (answer.substr(0, 5) == "WRONG") {
        cout << "Wrong message received in trick " << to_string(*lewa) << ".\n" << flush;
        *action = 2;  // Next - receive 'TRICK'.
    }
    // Correct 'TAKEN'.
    else if (answer.size() > 6 + offset && answer.substr(0, 5) == "TAKEN"
            && dirs.find(answer.back()) != string::npos) {
        list<string> taken;  // Taken cards.
        int got = get_cards(&taken, answer.substr(5 + offset, answer.size() - 6 - offset));
        if (got == 0 && taken.size() == 4 && in_hand(real_hand, taken, *played, *action) == 1) {  // If there are cards.
            // Add trick to the list, if taken by client.
            if (answer.back() == dir) {
                tricks->emplace_back(taken);
            }
            cout << "A trick " << to_string(*lewa) << " is taken by " << answer.back() << ", cards " << flush;
            print_cards(taken);
            cout << ".\n" << flush;
            if (*lewa == 13) {
                *lewa = 0;
                *action = 5;  // Next - receive 'SCORE' or 'TOTAL',
            }
            else {
                if (*action == 4) {
                    *action = 2;  // Next - receive 'TRICK'.
                }
                *lewa = *lewa + 1;
            }
        }
    }
}



// Handles 'SCORE' and 'TOTAL'. Displays communicate info.
// Answer - message received from server, action - type of next message to handle.
// Tricks - List of all taken tricks, total - last received message was 'TOTAL'.
void scoring(string answer, uint8_t *action, list<list<string>> *tricks, bool *total) {
    set<char> places;                      // Checks for four different seats.
    list<pair<char, string>> point_pairs;  // List of scores.
    bool miss = false;                     // Error in the data.
    string points;                         // Points of currently checked player.
    if (dirs.find(answer[5]) != string::npos) {  // If first value is a valid seat number.
        char new_dir = answer[5];
        places.insert(new_dir); // Player with first seat saved.

        // Extracs points of each player.
        for (char el: answer.substr(6, answer.size() - 6)) {
            if (dirs.find(el) != string::npos && !points.empty()) {  // New player and last player's score is not empty.
                // Checks for new player.
                pair<set<char>::iterator, bool> ret = places.insert(el);
                if (!ret.second) {
                    miss = true;
                    break;
                }
                // Saves points of last player.
                pair<char, string> pair;
                pair.first = new_dir;
                pair.second = points;
                point_pairs.emplace_back(pair);

                // New player.
                new_dir = el;
                points = "";
            }
            else if (el >= '0' && el <='9'){  // Reading score.
                points += el;
            }
            else {  // Invalid symbol.
                miss = true;
                break;
            }
        }
        if (points.empty()) {  // Score of the last player is empty.
            miss = true;
        }
        else {  // Saving score of the last player.
            pair<char, string> pair;
            pair.first = new_dir;
            pair.second = points;
            point_pairs.emplace_back(pair);
        }

        if (!miss && places.size() == 4) {  // There are four player scores and no errors.
            // Displays values.
            if (answer.substr(0, 5) == "SCORE") {
                cout << "The scores are:\n" << flush;
                *action = 7;  // Next - receive 'TOTAL'.
                tricks->clear();
            }
            else {
                cout << "The total scores are:\n" << flush;
                *action = 3;
                *total = true;
            }
            for (pair<char, string> el: point_pairs) {
                cout << el.first << " | " << el.second << "\n" << flush;
            }
        }
    }
}


// Handles 'DEAL'. Displays deal's info.
// Answer - message received from server, real_hand - list of client's cards, action - type of next message to handle,
// lewa - number of the turn, total - last received message was 'TOTAL'.
void deal(string answer, list<string> *real_hand, uint8_t *action, uint8_t *lewa, bool* total) {
    list<string> hand;  // Cards given in the deal.
    int get = get_cards(&hand, answer.substr(6, answer.size() - 6));
    if (get == 0 && hand.size() == 13) {  // If received 13 cards.
        *total = false;  // Received something other than total.
        *real_hand = hand;
        cout << "New deal " << answer[4] << " staring place " << answer[5] << ", your cards: " << flush;
        print_cards(hand);
        cout<< ".\n" << flush;
        if (*action == 1) {
            *action = 6;  // Next - receive 'TRICK' or possible 'TAKEN'.
        }
        else {
            *action = 2;  // Next - receive 'TRICK'.
        }
        *lewa = 1;    // First turn in the round.
    }
}


// Handles 'BUSY. Displays communicate info. Forces program to quit.
// Answer - message received from server, dir - ID of seat occupied by client.
int busy(string answer, char dir) {
    set<char> places;   // Seats taken.
    int code = 0;       // Return value.
    bool miss = false;  // Wrong values of seats taken.
    for (char el: answer.substr(4, answer.size() - 4)) {
        if (dirs.find(el) != string::npos) {
            pair<set<char>::iterator, bool> ret = places.insert(el);
            if (!ret.second) {
                miss = true;
            }
        }
        else {
            miss = true;
        }
    }
    pair<set<char>::iterator, bool> ret = places.insert(dir);
    if (ret.second) {  // Seat which client wanted to take isn't taken.
        miss = true;
    }
    if (!miss) {  // If received message is correct.
        string response("Place busy, list of busy places received:");
        for (char el: answer.substr(4, answer.size() - 4)) {
            response += " ";
            response += el;
        }
        cout << response << ".\n" << flush;
        code = 1;  // Force quit.
    }
    return code;
}


// Redirects to correct function based on given variables. Action - type of next message to handle
// Answer - message received from server, real_hand - list of client's cards, lewa - number of the turn,
// played - last played card, automat - automatically picks card to play, waiting - waits for user input,
// tricks list of tricks taken, total - last received message was 'TOTAL'. dir - ID of seat occupied by client
// by any player when client arrives to a round already in play.
uint8_t determine_action(uint8_t *action, string answer, list<string> *real_hand, uint8_t *lewa, string *played,
    bool automat, bool *waiting, list<list<string>> *tricks, bool *total, char dir) {
    int code = 0;    // Return value.
    uint8_t offset;  // Length of number of current turn.
    if (*lewa > 9) {
        offset = 2;
    }
    else {
        offset = 1;
    }
    // Proper trick received was received.
    if (*action == 2 && answer.size() >= 5 + offset && get_lewa(answer.substr(5, offset), *lewa) == 0
            && answer.substr(0, 5) == "TRICK") {
        code = play_card(*real_hand, automat, answer, action, played, waiting, offset, lewa);
    }
    // Proper answer for playing card received was received.
    else if (*action == 4 && answer.size() >= 5 + offset && get_lewa(answer.substr(5, offset), *lewa) == 0) {
        trick_answer(answer, lewa, offset, real_hand, played, action, tricks, dir);
    }
    // Proper round/game ending message was received.
    else if (answer.size() >= 7 && ((answer.substr(0, 5) == "SCORE" && (*action == 5 || (*action == 2 && *lewa > 1)))
        || (answer.substr(0, 5) == "TOTAL" && *action == 7))) {
        scoring(answer, action, tricks, total);
    }
    // Proper deal message was received.
    else if ((*action == 1 || *action == 3) && answer.size() >= 6 && answer.substr(0, 4) == "DEAL" &&
            answer[4] <= '7' && answer[4] >= '1' && dirs.find(answer[5]) != string::npos) {
        deal(answer, real_hand, action, lewa, total);
    }
    // Proper busy message was received.
    else if (*action == 1 && answer.size() >= 4 && answer.substr(0, 4) == "BUSY") {
        code = busy(answer, dir);
    }
    else if (*action == 6 && answer.size() >= 5 + offset && get_lewa(answer.substr(5, offset), *lewa) == 0) {
        if (answer.substr(0, 5) == "TAKEN") {
            trick_answer(answer, lewa, offset, real_hand, played, action, tricks, dir);
        }
        else if (answer.substr(0, 5) == "TRICK") {
            code = play_card(*real_hand, automat, answer, action, played, waiting, offset, lewa);
        }
    }
    return code;
}


// Setting up client. Getting arguments. Handling all messages.
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

        // Choosing IP version.
        int socket_fd;  // Socket descriptor.
        if (version == 4) {  // Getting server address IPv4.
            socket_fd = get_server_address_and_connect_4(host, port);
        }
        else if (version == 6) {  // Getting server address IPv6.
            socket_fd = get_server_address_and_connect_6(host, port);
        }
        else {  // Pick IP.
            socket_fd = get_any_address(host, port);
        }
        if (socket_fd < 0){  // There was an error while connecting.
            return 1;
        }

        // Setting non blocking socket.
        if (fcntl(socket_fd, F_SETFL, O_NONBLOCK) == -1) {
            close(socket_fd);
            fprintf(stderr, "ERROR: Couldn't set non-blocking socket.\n");
            return 1;
        }

        // Creating first message to send to the server.
        string message("IAM");
        message += dir;
        message += term;
        uint8_t to_send = message.size();  // Size of the message.

        string receiever;           // String holding received message.
        string played;              // Played card,
        uint8_t action = 1;         // Which message to get.
        uint8_t lewa = 0;           // Number of played turn.
        list<list<string>> tricks;  // Taken tricks.
        list<string> hand;          // Cards in client's hand.
        bool waiting = false;       // Waiting for user input.
        bool total = false;         // Last recieved message was total.

        int poll_act;                           // Value returned from poll.
        struct pollfd poll_descriptors[2];      // Array of descriptors used in poll.
        poll_descriptors[0].fd = socket_fd;     // Descriptor used to contact server.
        poll_descriptors[0].events = POLLOUT;   // Looking for space to write to the server.
        poll_descriptors[1].fd = STDIN_FILENO;  // STDIN descriptor.
        poll_descriptors[1].events = POLLIN;    // Waiting for message from STDIN.

        do {
            for (int i = 0; i < 2; ++i) {
                poll_descriptors[i].revents = 0;
            }
            if (waiting) {  // Waiting for user's STDIN input.
                poll_act = poll(&poll_descriptors[1], 1, -1);
            }
            else {  // Looking for certain events on both descriptors.
                poll_act = poll(poll_descriptors, 2, -1);
            }
            if (poll_act == -1) {  // Poll error.
                close(socket_fd);
                fprintf(stderr, "ERROR: There was an error while polling.\n");
            }
            else if (poll_act > 0) {  // Any event detected.
                if (poll_descriptors[0].revents & POLLOUT) {  // Client wants and is able to write.
                    ssize_t done = write(socket_fd, message.c_str(), to_send);  // Writing as much as possible.
                    if (done <= 0) {  // If there was an error while writing.
                        close(socket_fd);
                        fprintf(stderr, "ERROR: There was an error while writing to server.\n");
                        return 1;
                    }
                    to_send -= done;  // Amount of sent message.
                    if (to_send == 0) {  // Whole message was sent.
                        poll_descriptors[0].events = POLLIN;  // Looking for response
                    }
                    else {  // Couldn't send whole message.
                        message = message.substr(done, message.size() - done);
                    }
                }
                else if (poll_descriptors[0].revents & POLLIN) {  // Client wants and is able to read.
                    uint8_t ret = read_byte(socket_fd, &receiever);  // Checks if message was read whole or was there an error while reading.
                    if (ret == 2) {  // If there was an error while reading.
                        close(socket_fd);
                        fprintf(stderr, "ERROR: Couldn't read message.\n");
                        return 1;
                    }
                    if (ret == 1) {  // If whole message was read.
                        // Determines which action to do.
                        uint8_t code = determine_action(&action, receiever, &hand, &lewa, &played, automat, &waiting, &tricks, &total, dir);
                        receiever = "";
                        if (code == 1) {  // Action forced quit.
                            close(socket_fd);
                            return 1;
                        }
                        if (code == 2) {  // Send automatically played card.
                            // Preparing the message to send.
                            message = "TRICK";
                            message += to_string(lewa);
                            message += played;
                            message += term;
                            to_send = message.size();
                            poll_descriptors[0].events = POLLOUT;  // Send message.
                        }
                    }
                    else if (ret == 3) {
                        close(socket_fd);
                        if (total) {
                            return 0;
                        }
                        fprintf(stderr, "ERROR: Server dismissed the connection.\n");
                        return 1;
                    }
                }
                if (poll_descriptors[1].revents & POLLIN) {  // There was an user input.
                    string recv;               // Message received from user.
                    getline(cin, recv);  // Getting the message.
                    if (recv == "cards") {    // User requested display of cards.
                        print_cards(hand);
                        cout << "\n" << flush;
                    }
                    else if (recv == "tricks") {  // User requested display of taken cards.
                        for (list<string> trick: tricks) {  // Printing every trick.
                            auto iter = trick.begin();
                            cout << *iter << flush;
                            for (int i = 1; i < 4; i++) {
                                ++iter;
                                cout << ", " << *iter << flush;
                            }
                            cout << "\n" << flush;
                        }
                    }
                    else if(recv[0] == '!' && waiting) {  // User wanted to put card when requested.
                        // Preparing the message to send.
                        played = recv.substr(1, recv.size() - 1);
                        message = "TRICK";
                        message += to_string(lewa);
                        message += played;
                        message += term;
                        to_send = message.size();
                        waiting = false;  // Stop waiting for user input.
                        poll_descriptors[0].events = POLLOUT;  // Send message.
                    }
                }
            }
        } while (true);
    }
    cout << "ERROR: Invalid arguments.\n" << flush;
    return 1;
}