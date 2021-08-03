#include <iostream>
#include "TicTacToe.h"


using namespace std;


TicTacToe_state::TicTacToe_state() : MCTS_state(false) {
    // initialize board as empty
    for (int i = 0 ; i < 9 ; i++) {
        board[i / 3][i % 3] = ' ';
    }
}

bool TicTacToe_state::player_won(char player) {
    if (player != 'x' and player != 'o') {
        cerr << "Warning: check winner for unknown player" << endl;
    }
    for (int i = 0 ; i < 3 ; i++) {
        if (board[i][0] == player && board[i][1] == player && board[i][2] == player) return true;
        if (board[0][i] == player && board[1][i] == player && board[2][i] == player) return true;
    }
    return (board[0][0] == player && board[1][1] == player && board[2][2] == player) ||
           (board[0][2] == player && board[1][1] == player && board[2][0] == player);
}

bool TicTacToe_state::is_terminal() {
    return player_won('x') || player_won('o');
}

TicTacToe_state::TicTacToe_state(const TicTacToe_state &other)  : MCTS_state(false) {
    // copy board
    for (int i = 0 ; i < 9 ; i++) {
        board[i / 3][i % 3] = other.board[i / 3][i % 3];
    }
}

MCTS_state *TicTacToe_state::next_state(MCTS_move *move) {
    // TODO: We have to manually cast it to its correct type
    TicTacToe_move *m = (TicTacToe_move *) move;
    TicTacToe_state *new_state = new TicTacToe_state(*this);  // create new state from current
    if (new_state->board[m->x][m->y] == ' ')
        new_state->board[m->x][m->y] = m->player;                 // play move
    else
        cerr << "Warning: Illegal move (" << m->x  << ", " << m->y << ")" << endl;
    return new_state;
}

queue<MCTS_move> *TicTacToe_state::actions_to_try() {
    return nullptr;
}

double TicTacToe_state::rollout() {
    return 0;
}


