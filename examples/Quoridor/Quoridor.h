#ifndef MCTS_QUORIDOR_H
#define MCTS_QUORIDOR_H

#include "../../mcts/include/state.h"


struct Quoridor_move : public MCTS_move {
    short int x, y;
    char player;
    char type;         // 'h'/'v' -> horizontal/vertical wall, ' ' or other for move
    Quoridor_move(short int x, short int y, char p, char t) : x(x), y(y), player(p), type(t) { }
    bool operator==(const MCTS_move& other) const override {
        const Quoridor_move &o = (const Quoridor_move &) other;
        return x == o.x && y == o.y && player == o.player && type == o.type;
    }
    string sprint() const {
        string movetype = (type == 'h') ? "places horizontal wall at" : (type == 'v') ? "places vertical wall at" : "moves to";
        string playerstr = (player == 'W') ? "White" : "Black";
        return playerstr + " " + movetype + " " + string(1, (char) ('A' + y)) + to_string(x + 1);
    }
};


class Quoridor_state : public MCTS_state {
    /** white's and black's coordinates on the board */
    short int wx, wy, bx, by;
    /** white's and black's remaining number of walls */
    short int wwallsno, bwallsno;
    /** Effectively a look-up table:
     * ' ', 'h', 'v', 'b' for empty, horizontal, vertical and both walls in the right and bottom part of a cell */
    char walls[9][9]{};
    bool wall_connections[8][8]{};    // TODO: can be a bitmap to save up space (bools are 1 byte = 8 bits)
    /** Whose turn it is to play: 'W' or 'B' */
    char turn;
    /** Keep track of the distance from each player to each square while
     * taking account for any walls */
    short int **wdists;
    short int **bdists;
    //////////////////////////////////////////
    char change_turn() { turn = (turn == 'W') ? 'B' : 'W'; return turn; }
    bool horizontal_wall(short int x, short int y) const { return walls[x][y] == 'h' || walls[x][y] == 'b'; }
    bool vertical_wall(short int x, short int y) const { return walls[x][y] == 'v' || walls[x][y] == 'b'; }
    void add_wall(short int x, short int y, bool horizontal);
    void remove_wall(short int x, short int y, bool horizontal);
    bool legal_step(short int x, short int y, char p) const;
    bool legal_wall(short int x, short int y, char p, bool horizontal);
    short int **calculate_dists_from(short int x, short int y);
    static void reset_dists(short int **&dists);
public:
    Quoridor_state();
    Quoridor_state(const Quoridor_state &other);
    ~Quoridor_state() override;
    char whose_turn() const { return turn; }
    char check_winner() const;
    bool legal_move(const Quoridor_move *move);
    bool play_move(const Quoridor_move *move);
    int get_shortest_path(char player);
    /** Heuristics **/
    queue<MCTS_move *> * generate_good_moves(int min_wall_enc) const;
    /** Overrides: **/
    bool is_terminal() const override;
    MCTS_state *next_state(const MCTS_move *move) const override;
    queue<MCTS_move *> *actions_to_try() const override;
    double rollout() const override;                        // the rollout simulation in MCTS
    void print() const override;
};


#endif
