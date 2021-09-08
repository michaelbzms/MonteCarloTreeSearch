#include <iostream>
#include "Quoridor.h"
#include "../../mcts/include/mcts.h"

/** AI PARAMETERS **/
#define MAXITER 20000
#define MAXSECONDS 15

#define PROMPT "> "

const string commands = R"(Valid commands are:
  quit or q                         -- exits program
  help                              -- lists commands
  autoprint                         -- toggles automatic printing of the board after every command (initially true)
  winner                            -- check if there is a winner yet and who
  showboard or print                -- prints board
  playmove or m <col><row>          -- plays a move for current player
  playwall or w <type> <col><row>   -- places a wall for current player
  genmove                           -- generates move for current player using MCTS
  clearboard or reset               -- resets the board)";


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
    /** Important to set seed once. Quoridor_state won't do this! (TODO) **/
    srand(time(NULL));

    cout << "============================================================" << endl
         << "===============╣    Welcome to Quoridor!    ╠===============" << endl
         << "============================================================" << endl << endl;
    cout << commands << endl << endl;

    Quoridor_state *state = new Quoridor_state();
    string command;
    char winner = ' ';
    bool auto_print = true;
    if (auto_print) {
        state->print();
    }
    /** Game Tree for AI (works for both sides) **/
    MCTS_tree *game_tree = new MCTS_tree(new Quoridor_state());    // Important: do not use the same state that we change in main loop

    cout << (state->whose_turn() == 'W' ? "White's move:" : "Black's move:") << endl << PROMPT;
    flush(cout);
    while (cin >> command) {
        if (command == "q" || command == "quit"){
            cout << "Exiting..." << endl;
            break;
        }
        else if (command == "listcommands" || command == "help") {
            cout << commands << endl;
        }
        else if (command == "autoprint") {
            auto_print = !auto_print;        // toggle
        }
        else if (command == "winner") {
            winner = state->check_winner();
            cout << ((winner != ' ') ? "TRUE " : "FALSE ") << winner << endl;
        }
        else if (command == "showboard" || command == "print") {
            state->print();
        }
        else if (command == "playmove" || command == "m") {
            if (winner != ' ') {
                cin.ignore(512, '\n');
                cout << "Game has already finished." << endl << endl;
            } else {
                string coords;
                cin >> coords;
                int x, y;
                char p = state->whose_turn();
                bool succ = parse_coords(coords, x, y);
                if (!succ) {
                    cout << "Invalid command: Invalid arguments" << endl << endl;
                } else {
                    // play the move
                    Quoridor_move move(x, y, p, ' ');
                    succ = state->play_move(&move);
                    if (succ) {
                        cout << move.sprint() << endl << endl;
                        // check if winning move
                        winner = state->check_winner();
                        // advance game tree
                        game_tree->advance_tree(&move);
                    }
                }
            }
        }
        else if (command == "playwall" || command == "w") {
            if (winner != ' ') {
                cin.ignore(512, '\n');
                cout << "Game has already finished." << endl << endl;
            } else {
                string type, coords;
                cin >> type >> coords;
                char p = state->whose_turn();
                char t = parse_type(type);
                int x, y;
                bool succ = parse_coords(coords, x, y);
                if (t == 0x00 || !succ) {
                    cout << "Invalid command: Invalid arguments" << endl << endl;
                } else {
                    // play the move
                    Quoridor_move move(x, y, p, t);
                    succ = state->play_move(&move);
                    if (succ) {
                        cout << move.sprint() << endl << endl;
                        // advance game tree
                        game_tree->advance_tree(&move);
                    }
                }
            }
        }
        else if (command == "genmove") {    // generate AI move
            if (winner != ' ') {
                cin.ignore(512, '\n');
                cout << "Game has already finished." << endl << endl;
            } else {
                // determine "thinking time" required
                double max_seconds = MAXSECONDS;
                if (state->remaining_walls(state->whose_turn()) == 0) {
                    // if can't play a wall then there isn't much to think about
                    max_seconds = (max_seconds >= 5) ? 2.5 : max_seconds / 2;
                } else if (state->remaining_walls(state->whose_turn() == 'W' ? 'B' : 'W') == 0) {
                    // if enemy can't play a wall then less thinking is probably required as well
                    max_seconds = 0.75 * max_seconds;
                }

                // grow tree by thinking ahead and sampling monte carlo rollouts
                game_tree->grow_tree(MAXITER, max_seconds);
                game_tree->print_stats();   // debug

                // select best child node at root level
                MCTS_node *best_child = game_tree->select_best_child();
                if (best_child == NULL) {
                    cerr << "Warning: Could not find best child. Tree has no children? Possible terminal node" << endl << endl;
                }
                const Quoridor_move *best_move = (const Quoridor_move *) best_child->get_move();

                // advance the tree so the selected child node is now the root
                game_tree->advance_tree(best_move);

                // play AI move
                bool succ = state->play_move(best_move);
                if (!succ) {
                    cerr << "Warning: AI generated illegal move: " <<  best_move->sprint() << endl << endl;
                } else {
                    // print AI's move
                    cout << best_move->sprint() << endl << endl;
                }

                // check if winning move
                winner = state->check_winner();
            }
        } else if (command == "stats") {
            game_tree->print_stats();
        }
        else if (command == "clearboard" || command == "reset") {
            delete state;
            state = new Quoridor_state();
            delete game_tree;
            game_tree = new MCTS_tree(new Quoridor_state());
        }
        else if (command == "rollout") {   // for debug
            double res = 0.0;
            int num;
            cin >> num;
            for (int i = 0 ; i < num ; i++) {
                res += state->rollout();
            }
            double score = res / num;
            cout << "Rollout average score: " << setprecision(4) << 100.0 * score << endl;
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
        cout << (state->whose_turn() == 'W' ? "White's move:" : "Black's move:") << endl << PROMPT;
        flush(cout);
    }
    delete state;
    delete game_tree;
    return 0;
}

