#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cmath>
#include "Quoridor.h"

#define TEST_ALL_MOVES                          // test all moves vs just some found good by a heuristic (increases branching factor of tree but could find unexpectedly good moves)
#define MAX(A, B) (((A) > (B)) ? A : B)


using namespace std;


default_random_engine Quoridor_state::generator = default_random_engine(time(NULL));


Quoridor_state::Quoridor_state()
    : move_counter(0), wx(0), wy(4), bx(8), by(4), wwallsno(10), bwallsno(10), turn('W'), wdists(NULL), bdists(NULL) {
    for (int i = 0 ; i < 81 ; i++) {
        walls[i / 9][i % 9] = ' ';
        if (i < 64) wall_connections[i / 8][i % 8] = false;
    }
}

Quoridor_state::Quoridor_state(const Quoridor_state &other)
    : move_counter(other.move_counter), wx(other.wx), wy(other.wy), bx(other.bx), by(other.by),
      wwallsno(other.wwallsno), bwallsno(other.bwallsno), turn(other.turn),
      wdists(NULL), bdists(NULL) {    // TODO: Is it cheaper to copy dists than to potentially recalculate them?
    for (int i = 0 ; i < 81 ; i++) {
        walls[i / 9][i % 9] = other.walls[i / 9][i % 9];
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

short int **Quoridor_state::calculate_dists_from(short int x, short int y, bool stop_at_goal, char player) {
    /** Important note:
     * - stop_at_goal=true should be used when we only care about 1 square of the endzone (the minimum one)
     * being calculated and we don't really care for the rest of the board (which is typically the case)
     * - In that case we also need the player parameter. Otherwise it won't be used.
     */
    if (x < 0 || x >= 9 || y < 0 || y >= 9) {
        cerr << "Error: Invalid coordinates in calculate_dists_from()" << endl;   // should not happen
        return NULL;
    }
    // allocate space for result
    short int **dists = new short int *[9];
    for (int i = 0 ; i < 9 ; i++) {
        dists[i] = new short int[9];
        for (int j = 0 ; j < 9 ; j++) {
            dists[i][j] = -1;   // < 0 signifies unreachable squares
        }
    }
    // lambda for goal node check
    auto is_goal_state = [](char p, short int x) -> bool {
        if (p == 'W') return x == 8;
        if (p == 'B') return x == 0;
        return false;
    };
    // perform bfs on the board
    struct Node {
        short int x, y;
        int dist;
        Node(short int x, short int y, int dist) : x(x), y(y), dist(dist) {}
        bool operator==(const Node& other) const { return x == other.x && y == other.y; }
    };
    queue<Node> Q;
    Q.push(Node(x, y, 0));
    dists[x][y] = 0;
    while(!Q.empty()) {
        // get new node
        Node n = Q.front();
        Q.pop();
        if (stop_at_goal && is_goal_state(player, n.x)) break;
        // add neighbours to queue if not already explored
        if (n.x - 1 >= 0 && !horizontal_wall(n.x - 1, n.y) && dists[n.x - 1][n.y] < 0) {             // up
            dists[n.x - 1][n.y] = n.dist + 1;
            if (stop_at_goal && is_goal_state(player, n.x - 1)) break;
            Q.push(Node(n.x - 1, n.y, n.dist + 1));
        }
        if (n.x + 1 < 9 && !horizontal_wall(n.x, n.y) && dists[n.x + 1][n.y] < 0) {                  // down
            dists[n.x + 1][n.y] = n.dist + 1;
            if (stop_at_goal && is_goal_state(player, n.x + 1)) break;
            Q.push(Node(n.x + 1, n.y, n.dist + 1));
        }
        if (n.y - 1 >= 0 && !vertical_wall(n.x, n.y - 1) && dists[n.x][n.y - 1] < 0) {              // left
            dists[n.x][n.y - 1] = n.dist + 1;
            Q.push(Node(n.x, n.y - 1, n.dist + 1));
        }
        if (n.y + 1 < 9 && !vertical_wall(n.x, n.y) && dists[n.x][n.y + 1] < 0) {                 // right
            dists[n.x][n.y + 1] = n.dist + 1;
            Q.push(Node(n.x, n.y + 1, n.dist + 1));
        }
    }
    return dists;
}

int Quoridor_state::get_shortest_path(char player, const Quoridor_move *extra_wall_move, short int posx, short int posy) {
    int endzone = (player == 'W') ? 8 : 0;
    short int **dists = NULL;
    if (posx == -1 || posy == -1) {
        if (player == 'W') {
            // if not already calculated on a previous call
            if (wdists == NULL && extra_wall_move == NULL) {
                // calculate dists to every square using BFS (expensive)
                wdists = calculate_dists_from(wx, wy, true, 'W');
            } else if (extra_wall_move != NULL) {
                posx = wx;
                posy = wy;
            }
            dists = wdists;
        } else if (player == 'B') {
            // if not already calculated on a previous call
            if (bdists == NULL && extra_wall_move == NULL) {
                // calculate dists to every square using BFS (expensive)
                bdists = calculate_dists_from(bx, by, true, 'B');
            } else if (extra_wall_move != NULL) {
                posx = bx;
                posy = by;
            }
            dists = bdists;
        } else {
            cerr << "Invalid player arg" << endl;   // should not happen
            return -1;
        }
    }
    // if dists is NULL then we need to re-calculate dists separately (disregarding previous value)
    if (extra_wall_move != NULL) {
        // should not happen:
        if (extra_wall_move->type != 'h' && extra_wall_move->type != 'v') {
            cerr << "Error: extra_wall_move is not a wall move!" << endl;
            return -1;
        }
        if (!legal_wall(extra_wall_move->x, extra_wall_move->y, extra_wall_move->player, extra_wall_move->type == 'h', false)) {   // (!) check_blocking = false to prevent infinite loop
            cerr << "Error: extra_wall_move is illegal!" << endl;
            return -1;
        }
        // temporarily play the wall move
        bool horizontal = extra_wall_move->type == 'h';
        add_wall(extra_wall_move->x, extra_wall_move->y, horizontal);
        add_wall(extra_wall_move->x + ((int) !horizontal), extra_wall_move->y + ((int) horizontal), horizontal);
        // calc dists
        dists = calculate_dists_from(posx, posy, true, player);
        // remove wall
        remove_wall(extra_wall_move->x, extra_wall_move->y, horizontal);
        remove_wall(extra_wall_move->x + ((int) !horizontal), extra_wall_move->y + ((int) horizontal), horizontal);
    } else if (extra_wall_move == NULL && posx != -1 && posy != -1){
        // calc dists from custom posx, posy
        dists = calculate_dists_from(posx, posy, true, player);
    }
    // scan the end-zone and keep the minimum
    #define BIGNUM 9999999
    int min = BIGNUM;
    for (int i = 0 ; i < 9 ; i++) {
        if (dists[endzone][i] >= 0 && dists[endzone][i] < min) {
            min = dists[endzone][i];
        }
    }
    if (min == BIGNUM) min = -1;         // something < 0  ->  no path exists
    if (extra_wall_move != NULL || !(posx == -1 || posy == -1)) {
        reset_dists(dists);          // delete from heap
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
            (posx - 2 < 0 || horizontal_wall(posx - 2, posy)) &&
            !vertical_wall(posx - 1, posy - 1) &&
            !horizontal_wall(posx - 1, posy)) return true;
        if (enemy_posx == posx && enemy_posy == posy - 1 &&
            (posy - 2 < 0 || vertical_wall(posx, posy - 2)) &&
            !horizontal_wall(posx - 1, posy - 1) &&
            !vertical_wall(posx, posy - 1)) return true;
    }
    if (x >= 0 && y < 9 && x == posx - 1 && y == posy + 1) {                    // up-right
        if (enemy_posx == posx - 1 && enemy_posy == posy &&
            (posx - 2 < 0 || horizontal_wall(posx - 2, posy)) &&
            !vertical_wall(posx - 1, posy) &&
            !horizontal_wall(posx - 1, posy)) return true;
        if (enemy_posx == posx && enemy_posy == posy + 1 &&
            (posy + 2 >= 9 || vertical_wall(posx, posy + 1)) &&
            !horizontal_wall(posx - 1, posy + 1) &&
            !vertical_wall(posx, posy)) return true;
    }
    if (x < 9 && y >= 0 && x == posx + 1 && y == posy - 1) {                   // down-left
        if (enemy_posx == posx + 1 && enemy_posy == posy &&
            (posx + 2 >= 9 || horizontal_wall(posx + 1, posy)) &&
            !vertical_wall(posx + 1, posy - 1) &&
            !horizontal_wall(posx, posy)) return true;
        if (enemy_posx == posx && enemy_posy == posy - 1 &&
            (posy - 2 < 0 || vertical_wall(posx, posy - 2)) &&
            !horizontal_wall(posx, posy - 1) &&
            !vertical_wall(posx, posy - 1)) return true;
    }
    if (x < 9 && y < 9 && x == posx + 1 && y == posy + 1) {                   // down-right
        if (enemy_posx == posx + 1 && enemy_posy == posy &&
            (posx + 2 >= 9 || horizontal_wall(posx + 1, posy)) &&
            !vertical_wall(posx + 1, posy) &&
            !horizontal_wall(posx, posy)) return true;
        if (enemy_posx == posx && enemy_posy == posy + 1 &&
            (posy + 2 >= 9 || vertical_wall(posx, posy + 1)) &&
            !horizontal_wall(posx, posy + 1) &&
            !vertical_wall(posx, posy)) return true;
    }
    return false;
}

bool Quoridor_state::legal_wall(short int x, short int y, char p, bool horizontal, bool check_blocking) {
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
    if (check_blocking) {
        // TODO: if there are no walls/edges in both sides then there is no way this wall closed any paths.. -> don't bfs
        // But that is very hard to check so instead just check for completely isolated ones:
        bool isolated = true;
        for (int i = x - 1 ; i <= x + 1 + ((int) !horizontal) ; i++) {
            if (i < 0 || i >= 9) continue;       // ignore out-of-bounds areas
            for (int j = y - 1 ; j <= y + 1 + ((int) horizontal) ; j++) {
                if (j < 0 || j >= 9) continue;   // ignore out-of-bounds areas
                if (walls[i][j] != ' ') {
                    isolated = false;
                    break;
                }
            }
        }
        if (!isolated) {           // skip this check for isolated walls
            Quoridor_move wallmove(x, y, p, horizontal ? 'h' : 'v');
            int white_path = get_shortest_path('W', &wallmove);
            if (white_path < 0) return false;           // white is blocked
            int black_path = get_shortest_path('B', &wallmove);
            if (black_path < 0) return false;           // black is blocked
        }
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
        return legal_wall(move->x, move->y, move->player, move->type == 'h', true);
    } else {                                        // pawn move
        return legal_step(move->x, move->y, move->player);
    }
}

bool Quoridor_state::play_move(const Quoridor_move *move) {
    if (move == NULL || !legal_move(move)) {
        cout << "Invalid command: Illegal move: " << ((move != NULL) ? move->sprint() : "NULL") << endl << endl;
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
    // add to move counter
    move_counter++;
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

forward_list<MCTS_move *> Quoridor_state::get_legal_step_moves(char p) const {
    forward_list<MCTS_move *> Q;
    short int posx = (turn == 'W') ? wx : bx;
    short int posy = (turn == 'W') ? wy : by;
    if (legal_step(posx - 1, posy, p)) Q.push_front(new Quoridor_move(posx - 1, posy, p, ' '));
    if (legal_step(posx - 2, posy, p)) Q.push_front(new Quoridor_move(posx - 2, posy, p, ' '));
    if (legal_step(posx + 1, posy, p)) Q.push_front(new Quoridor_move(posx + 1, posy, p, ' '));
    if (legal_step(posx + 2, posy, p)) Q.push_front(new Quoridor_move(posx + 2, posy, p, ' '));
    if (legal_step(posx, posy - 1, p)) Q.push_front(new Quoridor_move(posx, posy - 1, p, ' '));
    if (legal_step(posx, posy - 2, p)) Q.push_front(new Quoridor_move(posx, posy - 2, p, ' '));
    if (legal_step(posx, posy + 1, p)) Q.push_front(new Quoridor_move(posx, posy + 1, p, ' '));
    if (legal_step(posx, posy + 2, p)) Q.push_front(new Quoridor_move(posx, posy + 2, p, ' '));
    if (legal_step(posx - 1, posy - 1, p)) Q.push_front(new Quoridor_move(posx - 1, posy - 1, p, ' '));
    if (legal_step(posx - 1, posy + 1, p)) Q.push_front(new Quoridor_move(posx - 1, posy + 1, p, ' '));
    if (legal_step(posx + 1, posy - 1, p)) Q.push_front(new Quoridor_move(posx + 1, posy - 1, p, ' '));
    if (legal_step(posx + 1, posy + 1, p)) Q.push_front(new Quoridor_move(posx + 1, posy + 1, p, ' '));
    return Q;
}

vector<MCTS_move *> Quoridor_state::get_legal_step_moves2(char p) const {
    vector<MCTS_move *> Q;
    short int posx = (turn == 'W') ? wx : bx;
    short int posy = (turn == 'W') ? wy : by;
    if (legal_step(posx - 1, posy, p)) Q.push_back(new Quoridor_move(posx - 1, posy, p, ' '));
    if (legal_step(posx - 2, posy, p)) Q.push_back(new Quoridor_move(posx - 2, posy, p, ' '));
    if (legal_step(posx + 1, posy, p)) Q.push_back(new Quoridor_move(posx + 1, posy, p, ' '));
    if (legal_step(posx + 2, posy, p)) Q.push_back(new Quoridor_move(posx + 2, posy, p, ' '));
    if (legal_step(posx, posy - 1, p)) Q.push_back(new Quoridor_move(posx, posy - 1, p, ' '));
    if (legal_step(posx, posy - 2, p)) Q.push_back(new Quoridor_move(posx, posy - 2, p, ' '));
    if (legal_step(posx, posy + 1, p)) Q.push_back(new Quoridor_move(posx, posy + 1, p, ' '));
    if (legal_step(posx, posy + 2, p)) Q.push_back(new Quoridor_move(posx, posy + 2, p, ' '));
    if (legal_step(posx - 1, posy - 1, p)) Q.push_back(new Quoridor_move(posx - 1, posy - 1, p, ' '));
    if (legal_step(posx - 1, posy + 1, p)) Q.push_back(new Quoridor_move(posx - 1, posy + 1, p, ' '));
    if (legal_step(posx + 1, posy - 1, p)) Q.push_back(new Quoridor_move(posx + 1, posy - 1, p, ' '));
    if (legal_step(posx + 1, posy + 1, p)) Q.push_back(new Quoridor_move(posx + 1, posy + 1, p, ' '));
    return Q;
}

Quoridor_move *Quoridor_state::get_best_step_move(char player) {
    int min = 9999999;
    Quoridor_move *argmin = NULL;
    forward_list<MCTS_move *> list = get_legal_step_moves(player);
    for (auto *move : list) {
        Quoridor_move *m = (Quoridor_move *) move;
        int path = get_shortest_path(player, NULL, m->x, m->y);
        if (path >= 0 && path < min) {
            min = path;
            delete argmin;     // delete previous (NULL is ignored)
            argmin = m;
        } else {
            delete m;          // delete all except argmin
        }
    }
    if (argmin == NULL) {
        cerr << "Warning: Could not find best move in this state:" << endl;
        this->print();
    }
    list.clear();
    return argmin;
}

///////////////////////////////////////////////////////////////////////////

bool Quoridor_state::is_terminal() const {
    char winner = check_winner();
    return winner == 'W' || winner == 'B';
}

MCTS_state *Quoridor_state::next_state(const MCTS_move *move) const {
    Quoridor_state *new_state = new Quoridor_state(*this);
    new_state->play_move((const Quoridor_move *) move);
    return new_state;
}

/** It is very important to decide which actions we will be considering.
 *  We would like to mostly consider good moves by using appropriate heuristics.
 *  This minimizes the branching factor of the search tree while also not investing
 *  in subtrees caused by bad enemy (and also ours) moves, where we would probably be better anyway.
 *  Although, that is addressed by UCT as well.
 */
queue<MCTS_move *> *Quoridor_state::generate_good_moves() {
    #define MIN_ENC_FOR_STOPPING_ENEMY_WALLS 3

    char p = turn, enemy = (turn == 'W') ? 'B' : 'W';
    queue<MCTS_move *> *Q = new queue<MCTS_move *>();
    // First consider all legal step moves
    forward_list<MCTS_move *> list = get_legal_step_moves(p);
    for (auto &move : list) {
        Q->push(move);
    }
    // Then consider good wall moves
    if (remaining_walls(p) > 0) {
        int our_path = get_shortest_path(p);
        int enemy_path = get_shortest_path(enemy);
        bool already_used[8][8][2]{false};
        for (short int i = 0; i < 8; i++) {
            for (short int j = 0; j < 8; j++) {
                for (short int k = 0; k < 2; k++) {                                  // orientation
                    if (legal_wall(i, j, p, k == 0, false)) {   // cheap version (don't double count)
                        // First (!), check if this walls encumbers our enemy more than us
                        Quoridor_move *wallmove = new Quoridor_move(i, j, p, (k == 0) ? 'h' : 'v');
                        int enemy_path_with_wall = get_shortest_path(enemy, wallmove);
                        int enemy_enc = enemy_path_with_wall - enemy_path;
                        int our_path_with_wall = get_shortest_path(p, wallmove);
                        int our_enc = our_path_with_wall - our_path;
                        // must annoy the enemy more than us (and be legal when it comes to blocking)
                        if (!already_used[i][j][k] && enemy_enc > our_enc &&
                                enemy_path_with_wall >= 0 && our_path_with_wall >= 0) {
                            already_used[i][j][k] = true;
                            Q->push(wallmove);
                        } else {
                            delete wallmove;
                        }
                        // Then, if this encumbers us significantly more than the enemy check for counter-walls
                        if (remaining_walls(enemy) > 0 && our_enc - enemy_enc >= MIN_ENC_FOR_STOPPING_ENEMY_WALLS) {
                            // same pos, opposite orientation (!)
                            Quoridor_move *countermove = new Quoridor_move(i, j,  p, (k == 0) ? 'v' : 'h');
                            if (!already_used[i][j][1 - k] && legal_move(countermove)) {         // also checks blocking
                                already_used[i][j][1 - k] = true;
                                Q->push(countermove);
                            } else {
                                delete countermove;
                            }
                            // same orientation, moved by 1 square on each side
                            if (k == 0) {
                                if (j - 1 >= 0 && !already_used[i][j-1][k] && legal_wall(i, j - 1, p, true)) {
                                    countermove = new Quoridor_move(i, j - 1, p, 'h');
                                    already_used[i][j-1][k] = true;
                                    Q->push(countermove);
                                }
                                if (j + 1 < 8 && !already_used[i][j+1][k] && legal_wall(i, j + 1, p, true)) {
                                    countermove = new Quoridor_move(i, j + 1, p, 'h');
                                    already_used[i][j+1][k] = true;
                                    Q->push(countermove);
                                }
                            } else if (k == 1) {
                                if (i - 1 >= 0 && !already_used[i-1][j][k] && legal_wall(i - 1, j, p, false)) {
                                    countermove = new Quoridor_move(i - 1, j, p, 'v');
                                    already_used[i-1][j][k] = true;
                                    Q->push(countermove);
                                }
                                if (i + 1 < 8 && !already_used[i+1][j][k] && legal_wall(i + 1, j, p, false)) {
                                    countermove = new Quoridor_move(i + 1, j, p, 'v');
                                    already_used[i+1][j][k] = true;
                                    Q->push(countermove);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return Q;
}

queue<MCTS_move *> *Quoridor_state::generate_all_moves() {
    char p = turn, enemy = (turn == 'W') ? 'B' : 'W';
    queue<MCTS_move *> *Q = new queue<MCTS_move *>();
    // First consider all legal step moves
    forward_list<MCTS_move *> list = get_legal_step_moves(p);
    for (auto &move : list) {
        Q->push(move);
    }
    // Second consider all wall moves
    if (remaining_walls(p) > 0) {
        for (short int i = 0; i < 8; i++) {
            for (short int j = 0; j < 8; j++) {
                for (short int k = 0; k < 2; k++) {
                    if (legal_wall(i, j, p, k == 0, true)) {
                        Quoridor_move *wallmove = new Quoridor_move(i, j, p, (k == 0) ? 'h' : 'v');
                        Q->push(wallmove);
                    }
                }
            }
        }
    }
    return Q;
}

queue<MCTS_move *> *Quoridor_state::actions_to_try() const {
    /** Note: actions_to_try() should probably be const in superclass but it would be very inefficient
     * to be so here because we would need to recalculate paths every time!
     * This is a hack to avoid const error in this specific case. */
#ifdef TEST_ALL_MOVES
    return const_cast<Quoridor_state *>(this)->generate_all_moves();
#else
    return const_cast<Quoridor_state *>(this)->generate_good_moves();
#endif
}

double evaluate_position(Quoridor_state &s, bool cheap) {
    #define GUESS_WIN_CONF 0.95
    #define ROOM_FOR_ERROR 1            // Note: Allow more room for error? path doesn't take "jumping" moves into account...

    int white_path = s.get_shortest_path('W');
    int black_path = s.get_shortest_path('B');

    // if opponent is out of walls and we have the shortest path then we are almost guaranteed to win
    if (s.bwallsno <= 0 && white_path + ((int) (s.whose_turn() != 'W')) <= black_path - ROOM_FOR_ERROR) {
        return GUESS_WIN_CONF;
    }
    if (s.wwallsno <= 0 && black_path + ((int) (s.whose_turn() != 'B')) <= white_path - ROOM_FOR_ERROR) {
        return 1.0 - GUESS_WIN_CONF;
    }
    // if opponent is almost out of walls and we have walls to stop him then we are probably going to win
    if (s.bwallsno <= 1 && s.wwallsno >= 2 && white_path + ((int) (s.whose_turn() != 'W')) <= black_path - ROOM_FOR_ERROR) {
        return GUESS_WIN_CONF - 0.1;
    }
    if (s.wwallsno <= 1 && s.bwallsno >= 2 && black_path + ((int) (s.whose_turn() != 'B')) <= white_path - ROOM_FOR_ERROR) {
        return 1.0 - (GUESS_WIN_CONF - 0.1);
    }

    /** Heuristic metric for difference in walls
     * - In [0, 1]. 0 when equal walls, 1 when enemy has 0 walls and we have > 0. */
    double wallsdiff_metric = 0.0;
    double max = s.wwallsno > s.bwallsno ? s.wwallsno : s.bwallsno;
    if (max > 0)
        wallsdiff_metric = ((s.wwallsno > s.bwallsno) ? +1 : -1) * (((double) pow(s.wwallsno - s.bwallsno, 2)) / ((double) pow(max, 2)));

    /** Shortest distance heuristic
     * - After some lead it doesn't matter if we get even more ahead, we get the full bonus --> keep your walls? */
    double path_diff = black_path - white_path + (s.whose_turn() == 'W' ? +0.5 : -0.5);    // bonus for whose turn it is to play
    double distance_metric = ((double) MAX(path_diff, 10.0)) / 10.0;

    return 0.5 + 0.2 * wallsdiff_metric + 0.2 * distance_metric;   // in [0.1, 0.9]
}

bool force_playwall(Quoridor_state &s) {
    char p = s.whose_turn();
    int our_path = s.get_shortest_path(p);
    short int our_walls = s.remaining_walls(p);
    int enemy_path = s.get_shortest_path(p == 'W' ? 'B' : 'W');
    short int enemy_walls = s.remaining_walls(p == 'W' ? 'B' : 'W');
    // enemy is about to win
    if (enemy_path <= 1 && our_path > enemy_path) return true;
    // enemy is much closer to winning than us
    if (enemy_path <= 2 && our_path > enemy_path + 1 && our_walls >= enemy_walls) return true;
    if (enemy_path <= 3 && our_path > enemy_path + 2 && our_walls > enemy_walls) return true;
    return false;
}

Quoridor_move *pick_semirandom_move(Quoridor_state &s, uniform_real_distribution<double> &dist, default_random_engine &gen) {
    #define WALL_VS_MOVE_CHANCE 0.4
    #define BEST_VS_RANDOM_MOVE 0.8
    #define BEST_WALLMOVE 0.1                   // this is much more expensive
    #define GUIDED_RANDOM_WALL 0.75

    // TODO: simulations need to play smarter walls because otherwise the AI don't think of them as a threat
    // although they are in the branching factor so they really really should think of them...

    char p = s.turn;
    char enemy = (s.turn == 'W') ? 'B' : 'W';

    // avoid walls in the first few moves of the game
    double wall_vs_move_prob = (s.get_number_of_turns() <= 2) ? 0.0 :
                               (s.get_number_of_turns() <= 6) ? (WALL_VS_MOVE_CHANCE / 2) : WALL_VS_MOVE_CHANCE;

    if (s.remaining_walls(s.whose_turn() > 0 && (force_playwall(s) || dist(gen) < wall_vs_move_prob))) {
        /** Idea: Avoid finding all good walls to then just pick one at random
         * - Put all moves we can't immediately reject in a pool (cheap -> no bfs)
         * - Shuffle pool and sample without replacement by iterating
         * - One-by-one check if current move is:
         *      1. legal (with bfs)
         *      2. good enough (with bfs)
         *   and if so play it, else continue searching. If no good move was found return random one or step move.
         */
        // TODO: A wall could be good in other ways as well e.g. blocks an enemy good wall. How do we consider those cheaply?
        // play wall
        vector<Quoridor_move *> pool;
        pool.reserve(128);
        for (short int i = 0; i < 8; i++) {
            for (short int j = 0; j < 8; j++) {
                if (s.legal_wall(i, j, p, true, false)) {    // cheap checks (no check for blocking)
                    pool.push_back(new Quoridor_move(i, j, p, 'h'));
                }
                if (s.legal_wall(i, j, p, false, false)) {   // cheap checks (no check for blocking)
                    pool.push_back(new Quoridor_move(i, j, p, 'v'));
                }
            }
        }
        // shuffle pool randomly
        shuffle(begin(pool), end(pool), gen);
        if (dist(gen) < BEST_WALLMOVE) {
            /** Note: random shuffling should make the choice of maximum random in case of multiple equivalents **/
            // check all wallmove's enc and pick the best (as in maximum difference in encumbrances)
            Quoridor_move *bestwallmove = NULL;
            int max_enc_diff = 0.0;
            for (Quoridor_move *move : pool) {
                /** Note: we also check if the wall is legal for blocking manually */
                int enemy_path = s.get_shortest_path(enemy, move);
                int enemy_enc = enemy_path - s.get_shortest_path(enemy);
                if (enemy_path >= 0 && enemy_enc > 0) {
                    int our_path = s.get_shortest_path(p, move);
                    int our_enc = our_path - s.get_shortest_path(p);
                    if (our_path >= 0 && enemy_enc > our_enc) {         // must annoy the enemy more than us
                        if (enemy_enc - our_enc > max_enc_diff) {
                            delete bestwallmove;
                            bestwallmove = move;
                            max_enc_diff = enemy_enc - our_enc;
                        } else {
                            delete move;
                        }
                    } else delete move;
                } else delete move;
            }
            // if found a good move play the first one we found goon enough randomly
            if (bestwallmove != NULL) return bestwallmove;
            // else resort to a step move by not returning here
        } else {
            if (dist(gen) < GUIDED_RANDOM_WALL) {
                // random but at least helpful in an obvious way
                Quoridor_move *wallmove = NULL;
                // examine moves in (random) order
                bool accepted = false;
                for (Quoridor_move *move : pool) {
                    if (!accepted) {
                        /** Note: we also check if the wall is legal for blocking manually */
                        int enemy_path = s.get_shortest_path(enemy, move);
                        int enemy_enc = enemy_path - s.get_shortest_path(enemy);
                        if (enemy_path >= 0 && enemy_enc > 0) {
                            int our_path = s.get_shortest_path(p, move);
                            int our_enc = our_path - s.get_shortest_path(p);
                            if (our_path >= 0 && enemy_enc > our_enc) {         // must annoy the enemy more than us
                                wallmove = move;
                                accepted = true;
                                continue;             // avoids deleting this move
                            }
                        }
                    }
                    delete move;                     // delete all not selected moves
                }
                // if found a good move play the first one we found goon enough randomly
                if (wallmove != NULL) return wallmove;
                // else resort to a step move by not returning here
            } else {
                // completely random wall move
                Quoridor_move *wallmove = NULL;
                bool accepted = false;
                for (Quoridor_move *move : pool) {
                    if (!accepted && s.legal_move(move)) {       // return the first legal move
                        wallmove = move;
                        accepted = true;
                        continue;
                    }
                    delete move;
                }
                // if a wallmove exists
                if (wallmove != NULL) return wallmove;
                // else resort to a step move by not returning here
            }
        }
    }
    // play move
    if (s.remaining_walls(enemy) == 0 || dist(gen) < BEST_VS_RANDOM_MOVE) {
        return s.get_best_step_move(s.whose_turn());
    } else {
        vector<MCTS_move *> v = s.get_legal_step_moves2(s.whose_turn());
        int r = rand() % v.size();
        for (int i = 0 ; i < v.size() ; i++) {
            if (i != r) delete v[i];
        }
        return (Quoridor_move *) v[r];
    }
    cerr << "Warning: could not find a legal move?" << endl << endl;
}

/**
 * Player1's (== white) win chance is returned. If genmove is for black
 * then this is dealt with in select_best_child of mcts!
 */
double Quoridor_state::rollout() const {
    #define MAXSTEPS 50
    #define EVALUATION_THRESHOLD 0.8     // when eval is this skewed then don't simulate any more, return eval
    // #define DDEBUG

    uniform_real_distribution<double> dist(0.0, 1.0);
    Quoridor_state s(*this);     // copy current state (bypasses const restriction and allows to change state)
    bool noerror;
    #ifdef DDEBUG
    queue<Quoridor_move *> hist;
    #endif
    for (int i = 0 ; i < MAXSTEPS ; i++) {
        // first check if terminal state
        if (s.is_terminal()) {
            return (s.check_winner() == 'W') ? 1.0 : 0.0;
        }
        // second check if we can call who is going to win
        double eval = evaluate_position(s, true);
        if (eval <= 1.0 - EVALUATION_THRESHOLD && eval >= EVALUATION_THRESHOLD) {
            break;
        }
        // otherwise keep simulating until we do or reached a certain depth
        Quoridor_move *m = pick_semirandom_move(s, dist, generator);
        if (!s.legal_move(m)) {
            cout << "Picked illegal move: " << ((m != NULL) ? m->sprint() : "NULL" ) << " intentionally! Move history:" << endl;
            #ifdef DDEBUG
            while (!hist.empty()) {
                Quoridor_move *prev = hist.front();
                hist.pop();
                cout << prev->sprint() << endl;
            }
            #endif
        }
        #ifdef DDEBUG
        hist.push(m);
        #endif
        noerror = s.play_move(m);
        if (!noerror) {
            cerr << "Error: in rollouts" << endl;
            break;
        }
        #ifndef DDEBUG
        delete m;
        #endif
    }
    return evaluate_position(s, false);
}
