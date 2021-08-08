#include <iostream>
#include "Quoridor.h"
#include "../../mcts/include/mcts.h"

//TODO: make this prettier
#define COMMANDS "Valid commands are:\n - quit or q\n - help\n - autoprint\n - winner\n - showboard or print\n - playmove <player> <col><row>\n - playwall <player> <type> <col><row>\n - genmove\n - clearboard or reset\n"
#define PROMPT "> "


char parse_player(const string &s) {
    if (s == "W" || s == "w" || s == "White" || s == "white" || s == "WHITE") return 'W';
    if (s == "B" || s == "b" || s == "Black" || s == "black" || s == "BLACK") return 'B';
    return 0x00;
}

char parse_type(const string &s) {
    if (s == "H" || s == "h" || s == "Horizontal" || s == "horizontal" || s == "HORIZONTAL") return 'h';
    if (s == "V" || s == "v" || s == "Vertical" || s == "vertical" || s == "VERTICAL") return 'v';
    return 0x00;
}

bool parse_coords(const string &s, int &x, int &y) {
    if (s.size() == 2) {
        y = toupper((char) s[0]) - 'A';
        x = ((char) s[1]) - '1';
        if (x >= 0 && x < 9 && y >= 0 && y < 9) return true;
    }
    return false;
}


int main() {
    cout << "============================================================" << endl
         << "===============╣    Welcome to Quoridor!    ╠===============" << endl
         << "============================================================" << endl << endl;
    cout << COMMANDS << endl << endl;

    Quoridor_state *state = new Quoridor_state();
    string command;
    char winner = ' ';
    bool auto_print = true;
    if (auto_print) {
        state->print();
    }
    cout << PROMPT;
    flush(cout);
    while (cin >> command) {
        if (command == "q" || command == "quit"){
            cout << "Exiting..." << endl;
            break;
        }
        else if (command == "listcommands" || command == "help") {
            cout << COMMANDS << endl;
        }
        else if (command == "autoprint") {
            auto_print = !auto_print;        // toggle
        }
        else if (command == "winner") {
            winner = state->check_winner();
            cout << ((winner != ' ') ? "TRUE" : "FALSE") << winner << endl;
        }
        else if (command == "showboard" || command == "print") {
            state->print();
        }
        else if (command == "playmove") {
            if (winner != ' ') {
                cout << "Game has already finished." << endl << endl;
            } else {
                string player, coords;
                cin >> player >> coords;
                char p = parse_player(player);
                int x, y;
                bool succ = parse_coords(coords, x, y);
                if (p == 0x00 || !succ) {
                    cout << player << coords << endl;
                    cout << "Invalid command: Invalid arguments" << endl << endl;
                } else if (p != state->whose_turn()) {
                    cout << "Invalid command: Not this player's turn" << endl << endl;
                }
                else {
                    // play the move
                    Quoridor_move move(x, y, p, ' ');
                    succ = state->play_move(&move);
                    if (succ) {
                        cout << move.sprint() << endl << endl;
                        // check if winning move
                        winner = state->check_winner();
                    }
                }
            }
        }
        else if (command == "playwall") {
            if (winner != ' ') {
                cout << "Game has already finished." << endl << endl;
            } else {
                string player, type, coords;
                cin >> player >> type >> coords;
                char p = parse_player(player);
                char t = parse_type(type);
                int x, y;
                bool succ = parse_coords(coords, x, y);
                if (p == 0x00 || t == 0x00 || !succ) {
                    cout << "Invalid command: Invalid arguments" << endl << endl;
                } else if (p != state->whose_turn()) {
                    cout << "Invalid command: Not this player's turn" << endl << endl;
                } else {
                    // play the move
                    Quoridor_move move(x, y, p, t);
                    succ = state->play_move(&move);
                    if (succ) cout << move.sprint() << endl << endl;
                }
            }
        }
        else if (command == "genmove") {
            if (winner != ' ') {
                cout << "Game has already finished." << endl << endl;
            } else {
                // TODO
                cout << "Not implemented yet" << endl << endl;
                // check if winning move
                winner = state->check_winner();
            }
        }
        else if (command == "clearboard" || command == "reset") {
            delete state;
            state = new Quoridor_state();
        }
        else {
            cout << "? unknown command" << endl << endl;
        }
        // before reading next command
        if (auto_print) {
            state->print();
        }
        if (winner == 'W' || winner == 'B') {
            cout << endl << ((winner == 'W') ? "White" : "Black") << " has won the game!" << endl << endl;
        }
        cout << PROMPT;
        flush(cout);
    }
    delete state;
    return 0;
}

