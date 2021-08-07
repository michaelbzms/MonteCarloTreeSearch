#include <iostream>
#include "Quoridor.h"


using namespace std;


Quoridor_state::Quoridor_state()
    : wx(4), wy(0), bx(4), by(8), wwallsno(10), bwallsno(10), turn('W') {
    for (int i = 0 ; i < 64 ; i++) {
        walls[i / 8][i % 8] = ' ';
        wall_connections[i / 8][i % 8] = false;
    }
}

Quoridor_state::Quoridor_state(const Quoridor_state &other)
    : wx(other.wx), wy(other.wy), bx(other.bx), by(other.by),
      wwallsno(other.wwallsno), bwallsno(other.bwallsno), turn(other.turn) {
    for (int i = 0 ; i < 64 ; i++) {
        walls[i / 8][i % 8] = other.walls[i / 8][i % 8];
        wall_connections[i / 8][i % 8] = other.wall_connections[i / 8][i % 8];
    }
}

char Quoridor_state::check_winner() const {
    if (wy == 8) return 'W';
    if (by == 0) return 'B';
    return ' ';
}

void Quoridor_state::add_wall(short int x, short int y, bool horizontal) {
    char put = horizontal ? 'h' : 'v', other = horizontal ? 'v' : 'h';
    if (walls[x][y] == ' ') walls[x][y] = put;
    else if (walls[x][y] == other) walls[x][y] = 'b';
    else {
        cerr << "Warning: Illegal wall let through!" << endl;   // should not happen
    }
}

bool Quoridor_state::legal_step(short int x, short int y, char p) const {
    // TODO
    return false;
}

bool Quoridor_state::legal_wall(short int x, short int y, char p, bool horizontal) const {
    // TODO
    return false;
}

bool Quoridor_state::legal_move(const Quoridor_move *move) const {
    if (move == NULL) return false;
    if (move->player != 'W' && move->player != 'B'){
        cerr << "Warning: wrong player argument!" << endl;
        return false;
    }
    if (move->type == 'h' || move->type == 'v') {   // wall move
        return legal_wall(move->x, move->y, move->player, move->type == 'h');
    } else {                                        // pawn move
        return legal_step(move->x, move->y, move->player);
    }
}

void Quoridor_state::play_move(const Quoridor_move *move) {
    if (!legal_move(move)) {
        cerr << "Attempted illegal move:" << move << endl;
        return;
    }
    if (move->type == 'h' || move->type == 'v') {   // wall move
        // play legal wall
        add_wall(move->x, move->y, move->type == 'h');
        if (move->type == 'h'){
            add_wall(move->x, move->y + 1, true);
        } else {
            add_wall(move->x + 1, move->y, false);
        }
        wall_connections[move->x][move->y] = true;
        // reduce walls
        switch (move->player) {
            case 'W':
                wwallsno--;
                break;
            case 'B':
                bwallsno--;
                break;
        }
    } else {                                        // pawn move
        // play legal move
        switch (move->player) {
            case 'W':
                wx = move->x;
                wy = move->y;
                break;
            case 'B':
                bx = move->x;
                by = move->y;
                break;
        }
    }
    // change turn
    change_turn();
}

///////////////////////////////////////////////////////////////////////////

bool Quoridor_state::is_terminal() const {
    char winner = check_winner();
    return winner != 'W' && winner != 'B';
}

MCTS_state *Quoridor_state::next_state(const MCTS_move *move) const {
    Quoridor_state *new_state = new Quoridor_state(*this);
    new_state->play_move((const Quoridor_move *) move);
    return new_state;
}

/** It is very important to decide which actions we will be considering.
 *  We would like to only consider good moves by using appropriate heuristics.
 *  This minimizes the branching factor of the search tree while also not investing
 *  in subtrees caused by bad enemy (and also ours) moves, where we would probably be better anyway.
 */
queue<MCTS_move *> *Quoridor_state::actions_to_try() const {
    // TODO
    return nullptr;
}

double Quoridor_state::rollout() const {
    // TODO
    return 0;
}

void Quoridor_state::print() const {
    // TODO
    MCTS_state::print();
}

