#include <iostream>
#include "Quoridor.h"
#include "../../mcts/include/mcts.h"

//TODO: make this prettier
#define COMMANDS "Valid commands are:\n - quit or q\n - help\n - winner\n - showboard or print\n - playmove <row><col>\n - playwall <type> <row><col>\n - genmove\n - clearboard or reset\n"


int main() {
    cout << "==============================" << endl
         << "===╣ Welcome to Quoridor! ╠===" << endl
         << "==============================" << endl << endl;
    cout << COMMANDS << endl << endl;

    Quoridor_state *state = new Quoridor_state();
    string command;
    char winner = ' ';
    while (true) {
        if (winner == 'W' || winner == 'B') {
            cout << endl << ((winner == 'W') ? "White" : "Black") << " has won the game!" << endl << endl;
        }
        cout << "> ";
        flush(cout);
        cin >> command;
        if (command == "q" || command == "quit"){
            cout << "Exiting..." << endl;
            break;
        }
        else if (command == "listcommands" || command == "help") {
            cout << COMMANDS << endl;
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
                string coords;
                cin >> coords;

                // check if winning move
                winner = state->check_winner();
            }
        }
        else if (command == "playwall") {
            if (winner != ' ') {
                cout << "Game has already finished." << endl << endl;
            } else {
                string type, coords;
                cin >> type >> coords;

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
    }
    delete state;
    return 0;
}

