#include <iostream>
#include <iomanip>
#include "Quoridor.h"


using namespace std;


Quoridor_state::Quoridor_state()
    : wx(0), wy(4), bx(8), by(4), wwallsno(10), bwallsno(10), turn('W'), wdists(NULL), bdists(NULL) {
    for (int i = 0 ; i < 81 ; i++) {
        walls[i / 9][i % 9] = ' ';
        if (i < 64) wall_connections[i / 8][i % 8] = false;
    }
}

Quoridor_state::Quoridor_state(const Quoridor_state &other)
    : wx(other.wx), wy(other.wy), bx(other.bx), by(other.by),
      wwallsno(other.wwallsno), bwallsno(other.bwallsno), turn(other.turn),
      wdists(NULL), bdists(NULL) {    // TODO: Is it cheaper to copy dists than to potentially recalculate them?
    for (int i = 0 ; i < 81 ; i++) {
        walls[i / 9][i % 9] = other.walls[i / 8][i % 8];
        if (i < 64) wall_connections[i / 8][i % 8] = other.wall_connections[i / 8][i % 8];
    }
}

Quoridor_state::~Quoridor_state() {
    reset_dists(wdists);
    reset_dists(bdists);
}

char Quoridor_state::check_winner() const {
    if (wx == 8) return 'W';
    if (bx == 0) return 'B';
    return ' ';
}

short int **Quoridor_state::calculate_dists_from(short int x, short int y) {
    // allocate space for result
    short int **dists = new short int *[9];
    for (int i = 0 ; i < 9 ; i++) {
        dists[i] = new short int[9];
        for (int j = 0 ; j < 9 ; j++) {
            dists[i][j] = -1;   // < 0 signifies unreachable squares
        }
    }
    // perform bfs on the board
    struct Node {
        short int x, y;
        int dist;
        Node(short int x, short int y, int dist) : x(x), y(y), dist(dist) {}
        bool operator==(const Node& other) const { return x == other.x && y == other.y; }
    };
    queue<Node> Q;
    Q.push(Node(x, y, 0));       // TODO: does this work? Like allocate to stack?
    dists[x][y] = 0;
    while(!Q.empty()) {
        // get new node
        Node n = Q.front();
        Q.pop();
        // add neighbours to queue if not already explored
        if (n.x - 1 >= 0 && !horizontal_wall(n.x - 1, n.y) && dists[n.x - 1][n.y] < 0) {          // up
            dists[n.x - 1][n.y] = n.dist + 1;
            Q.push(Node(n.x - 1, n.y, n.dist + 1));
        }
        if (n.x + 1 < 9 && !horizontal_wall(n.x, n.y) && dists[n.x + 1][n.y] < 0) {                  // down
            dists[n.x + 1][n.y] = n.dist + 1;
            Q.push(Node(n.x + 1, n.y, n.dist + 1));
        }
        if (n.y - 1 >= 0 && !horizontal_wall(n.x, n.y - 1) && dists[n.x][n.y - 1] < 0) {          // left
            dists[n.x][n.y - 1] = n.dist + 1;
            Q.push(Node(n.x, n.y - 1, n.dist + 1));
        }
        if (n.y + 1 < 9 && !horizontal_wall(n.x, n.y)  && dists[n.x][n.y + 1] < 0) {                 // right
            dists[n.x][n.y + 1] = n.dist + 1;
            Q.push(Node(n.x, n.y + 1, n.dist + 1));
        }
    }
    return dists;
}

int Quoridor_state::get_shortest_path(char player, const Quoridor_move *extra_wall_move) {
    int endzone;
    short int posx, posy;
    short int **dists = NULL;
    if (player == 'W') {
        // if not already calculated on a previous call
        if (wdists == NULL && extra_wall_move == NULL) {
            // calculate dists to every square using BFS (expensive)
            wdists = calculate_dists_from(wx, wy);
        }
        endzone = 8;
        posx = wx;
        posy = wy;
        dists = wdists;
    } else if (player == 'B') {
        // if not already calculated on a previous call
        if (bdists == NULL && extra_wall_move == NULL) {
            // calculate dists to every square using BFS (expensive)
            bdists = calculate_dists_from(bx, by);
        }
        endzone = 0;
        posx = bx;
        posy = by;
        dists = bdists;
    } else {
        cerr << "Invalid player arg" << endl;   // should not happen
        return -1;
    }
    // if dists is NULL then we need to re-calculate dists separately (disregarding previous value)
    if (extra_wall_move != NULL) {
        // should not happen:
        if (extra_wall_move->type != 'h' && extra_wall_move->type != 'v') { cerr << "Error: extra_wall_move is not a wall move!" << endl; return -1; }
        // TODO: the move must be legal but can we assume this or should we check? Do we check it twice this way?
        if (!legal_wall(extra_wall_move->x, extra_wall_move->y, extra_wall_move->player, extra_wall_move->type == 'h', false)) {   // (!) check_blocking = false to prevent infinite loop
            cerr << "Error: extra_wall_move is illegal!" << endl;
            return -1;
        }
        // temporarily play the wall move
        bool horizontal = extra_wall_move->type == 'h';
        add_wall(extra_wall_move->x, extra_wall_move->y, horizontal);
        add_wall(extra_wall_move->x + ((int) !horizontal), extra_wall_move->y + ((int) horizontal), horizontal);
        // calc dists
        dists = calculate_dists_from(posx, posy);
        // remove wall
        remove_wall(extra_wall_move->x, extra_wall_move->y, horizontal);
        remove_wall(extra_wall_move->x + ((int) !horizontal), extra_wall_move->y + ((int) horizontal), horizontal);
    }
    // scan the end-zone and keep the minimum
    #define BIGNUM 9999999
    int min = BIGNUM;
    for (int i = 0 ; i < 9 ; i++) {
        if (dists[endzone][i] >= 0 && dists[endzone][i] < min) {
            min = dists[endzone][i];
        }
    }
    if (min == BIGNUM) min = -1;     // something < 0  ->  no path exists
    if (extra_wall_move != NULL) {
        reset_dists(dists);      // delete from heap
    }
    return min;
}

void Quoridor_state::reset_dists(short int **&dists) {
    if (dists == NULL) return;
    for (int i = 0 ; i < 9 ; i++) {
        delete[] dists[i];
    }
    delete[] dists;
    dists = NULL;   // (!) has to change that's why we use a reference
}

void Quoridor_state::add_wall(short int x, short int y, bool horizontal) {
    // Note: this low-level function does NOT reset dists calculated so we need to do that outside if we wish so
    char put = horizontal ? 'h' : 'v', other = horizontal ? 'v' : 'h';
    if (walls[x][y] == ' ') walls[x][y] = put;
    else if (walls[x][y] == other) walls[x][y] = 'b';
    else { cerr << "Warning: Illegal wall let through!" << endl; }   // should not happen
}

void Quoridor_state::remove_wall(short x, short y, bool horizontal) {
    char put = horizontal ? 'h' : 'v', other = horizontal ? 'v' : 'h';
    if (walls[x][y] == put) walls[x][y] = ' ';
    else if (walls[x][y] == 'b') walls[x][y] = other;
    else { cerr << "Warning: Illegal wall let through!" << endl; }   // should not happen
}

bool Quoridor_state::legal_step(short int x, short int y, char p) const {
    // TODO: Double Check
    // check if our turn
    if (p != turn) return false;
    // check if out-of-bouds
    if (x < 0 || x >= 9 || y < 0 || y >= 9) return false;
    // check if move falls into an occupied square
    if ((x == bx && y == by) || (x == wx && y == wy)) return false;
    // determine two player's pos
    short int posx, posy, enemy_posx, enemy_posy;
    if (p == 'W') {
        posx = wx;
        posy = wy;
        enemy_posx = bx;
        enemy_posy = by;
    } else if (p == 'B') {
        posx = bx;
        posy = by;
        enemy_posx = wx;
        enemy_posy = wy;
    } else return false;
    // check all possible legal moves one-by-one
    if (y == posy) {
        if (x >= 0 && !horizontal_wall(posx - 1, posy)) {
            if (x == posx - 1) return true;                                     // up
            if (x == posx - 2 && !horizontal_wall(posx - 2, posy) &&
                enemy_posy == posy && enemy_posx == posx - 1) return true;      // up-up
        }
        if (x < 9 && !horizontal_wall(posx, posy)) {
            if (x == posx + 1) return true;                                     // down
            if (x == posx + 2 && !horizontal_wall(posx + 1, posy) &&
                enemy_posy == posy && enemy_posx == posx + 1) return true;      // down-down
        }
    }
    if (x == posx) {
        if (y >= 0 && !vertical_wall(posx, posy - 1)) {
            if (y == posy - 1) return true;                                     // left
            if (y == posy - 2 && !vertical_wall(posx, posy - 2) &&
                enemy_posx == posx && enemy_posy == posy - 1) return true;      // left-left
        }
        if (y < 9 && !vertical_wall(posx, posy)) {
            if (y == posy + 1) return true;                                     // right
            if (y == posy + 2 && !vertical_wall(posx, posy + 1) &&
                enemy_posx == posx && enemy_posy == posy + 1) return true;      // right-right
        }
    }
    // check diagonal moves
    if (x >= 0 && y >= 0 && x == posx - 1 && y == posy - 1) {                   // up-left
        if (enemy_posx == posx - 1 && enemy_posy == posy &&
            (posx - 2 < 0 || horizontal_wall(posx - 2, posy))) return true;
        if (enemy_posx == posx && enemy_posy == posy - 1 &&
            (posy - 2 < 0 || vertical_wall(posx, posy - 2))) return true;
    }
    if (x >= 0 && y < 9 && x == posx - 1 && y == posy + 1) {                    // up-right
        if (enemy_posx == posx - 1 && enemy_posy == posy &&
            (posx - 2 < 0 || horizontal_wall(posx - 2, posy))) return true;
        if (enemy_posx == posx && enemy_posy == posy + 1 &&
            (posy + 2 >= 9 || vertical_wall(posx, posy + 1))) return true;
    }
    if (x < 9 && y >= 0 && x == posx + 1 && y == posy - 1) {                   // down-left
        if (enemy_posx == posx + 1 && enemy_posy == posy &&
            (posx + 2 >= 9 || horizontal_wall(posx + 1, posy))) return true;
        if (enemy_posx == posx && enemy_posy == posy - 1 &&
            (posy - 2 < 0 || vertical_wall(posx, posy - 2))) return true;
    }
    if (x < 9 && y < 9 && x == posx + 1 && y == posy + 1) {                   // down-right
        if (enemy_posx == posx + 1 && enemy_posy == posy &&
            (posx + 2 >= 9 || horizontal_wall(posx + 1, posy))) return true;
        if (enemy_posx == posx && enemy_posy == posy + 1 &&
            (posy + 2 >= 9 || vertical_wall(posx, posy + 1))) return true;
    }
    return false;
}

bool Quoridor_state::legal_wall(short int x, short int y, char p, bool horizontal, bool check_blocking) {
    // TODO: Double-check
    // check if our turn
    if (p != turn) return false;
    // check out-of-bounds
    if (x < 0 || y < 0 || x >= 8 || y >= 8) return false;
    // check if out of walls
    if (p == 'W' && wwallsno <= 0) return false;
    if (p == 'B' && bwallsno <= 0) return false;
    // check if blocked by the same wall or by an opposite wall at the same exact spot
    char type = (horizontal) ? 'h' : 'v';
    char opposite_type = (horizontal) ? 'v' : 'h';
    if (walls[x][y] == type || walls[x][y] == 'b' || (walls[x][y] == opposite_type && wall_connections[x][y])) return false;
    // check if the second part of the wall is blocked
    if (horizontal && horizontal_wall(x, y + 1)) return false;
    if (!horizontal && vertical_wall(x + 1, y)) return false;
    // check if playing this wall blocks a pawn's path (expensive, avoid when possible)
    if (check_blocking && 20 - wwallsno - bwallsno >= 5) {    // (!) there need to be played at least 5 walls already for this to be possible
        bool blocked = false;
        Quoridor_move wallmove(x, y, p, horizontal ? 'h' : 'v');
        int white_path = get_shortest_path('W', &wallmove);
        if (white_path < 0) return false;           // white is blocked
        int black_path = get_shortest_path('B', &wallmove);
        if (black_path < 0) return false;           // black is blocked
    }
    // if passed all the checks, it's legal
    return true;
}

bool Quoridor_state::legal_move(const Quoridor_move *move) {
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

bool Quoridor_state::play_move(const Quoridor_move *move) {
    if (!legal_move(move)) {
        cout << "Invalid command: Illegal move: " << move->sprint() << endl << endl;
        return false;
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
        // reset all dists
        reset_dists(wdists);
        reset_dists(bdists);
    } else {                                        // pawn move
        // play legal move
        switch (move->player) {
            case 'W':
                wx = move->x;
                wy = move->y;
                // reset his dists
                reset_dists(wdists);
                break;
            case 'B':
                bx = move->x;
                by = move->y;
                // reset his dists
                reset_dists(bdists);
                break;
        }
    }
    // change turn
    change_turn();
    return true;
}

void Quoridor_state::print() const {
#define VWALL "║"
#define BOTH "╬"
    cout << endl << "  ";
    for (int i = 0 ; i < 9 ; i++) {
        cout << "     " << (char) ('A' + i);
    }
    cout << "           Player turn: " << (turn == 'W' ? "White" : "Black") << endl;
    cout << "    +";
    for (int i = 0 ; i < 9 ; i++) {
        cout << " ━━━ +";
    }
    cout << endl;
    for (int row = 0 ; row < 9 ; row++) {
        printf(" %d  ┃", row+1);
        for (int col = 0 ; col < 9 ; col++) {
            printf("  %c  %s",
                   (bx == row && by == col) ? 'B' : (wx == row && wy == col) ? 'W' : ' ',
                   (vertical_wall(row, col) ? VWALL : (col < 8 ? "|" : "┃")));
        }
        printf("  %d", row+1);
        // print wall counts
        if (row < 2) {
            printf("     %s walls: %d", (row == 0) ? "White" : "Black", (row == 0) ? wwallsno : bwallsno);
        }
        cout << endl << "    +";
        for (int col = 0 ; col < 9 ; col++) {
            if (horizontal_wall(row, col)) {
                printf("═════%s", wall_connections[row][col] ? ((row < 8 && vertical_wall(row, col) && wall_connections[row][col] && vertical_wall(row + 1, col)) ? VWALL : "═") : "+");
            } else if (row < 8) {
                printf("-----%s", (vertical_wall(row, col) && wall_connections[row][col] && vertical_wall(row + 1, col)) ? VWALL : "+");
            } else {
                printf(" ━━━ +");
            }
        }
        cout << endl;
    }
    cout << "  ";
    for (int i = 0 ; i < 9 ; i++) {
        cout << "     " << (char) ('A' + i);
    }
    cout << endl << endl;
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
queue<MCTS_move *> *Quoridor_state::generate_good_moves(int min_wall_enc) const {
    char p = turn;
    short int posx = (turn == 'W') ? wx : bx;
    short int posy = (turn == 'W') ? wy : by;
    short int enemy_posx = (turn == 'W') ? bx : wx;
    short int enemy_posy = (turn == 'W') ? by : wy;
    queue<MCTS_move *> *Q = new queue<MCTS_move *>();
    // First consider all legal step moves
    if (legal_step(posx - 1, posy, p)) Q->push(new Quoridor_move(posx - 1, posy, p, ' '));
    if (legal_step(posx - 2, posy, p)) Q->push(new Quoridor_move(posx - 2, posy, p, ' '));
    if (legal_step(posx + 1, posy, p)) Q->push(new Quoridor_move(posx + 1, posy, p, ' '));
    if (legal_step(posx + 2, posy, p)) Q->push(new Quoridor_move(posx + 2, posy, p, ' '));
    if (legal_step(posx, posy - 1, p)) Q->push(new Quoridor_move(posx, posy - 1, p, ' '));
    if (legal_step(posx, posy - 2, p)) Q->push(new Quoridor_move(posx, posy - 2, p, ' '));
    if (legal_step(posx, posy + 1, p)) Q->push(new Quoridor_move(posx, posy + 1, p, ' '));
    if (legal_step(posx, posy + 2, p)) Q->push(new Quoridor_move(posx, posy + 2, p, ' '));
    if (legal_step(posx - 1, posy - 1, p)) Q->push(new Quoridor_move(posx - 1, posy - 1, p, ' '));
    if (legal_step(posx - 1, posy + 1, p)) Q->push(new Quoridor_move(posx - 1, posy + 1, p, ' '));
    if (legal_step(posx + 1, posy - 1, p)) Q->push(new Quoridor_move(posx + 1, posy - 1, p, ' '));
    if (legal_step(posx + 1, posy + 1, p)) Q->push(new Quoridor_move(posx + 1, posy + 1, p, ' '));
    // Then consider good wall moves
    // TODO: our encumbrance, their encumbrance, their difference
    // TODO: BUT we shouldn't devote a number of expensive BFSs on every possible move! --> too expensive probably
    return Q;
}

queue<MCTS_move *> *Quoridor_state::actions_to_try() const {
    #define MIN_WALL_ENCUMBRANCE 2
    return generate_good_moves(MIN_WALL_ENCUMBRANCE);
}

double Quoridor_state::rollout() const {
    // TODO
    return 0;
}
