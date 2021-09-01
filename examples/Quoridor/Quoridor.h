#ifndef MCTS_QUORIDOR_H
#define MCTS_QUORIDOR_H

#include "../../mcts/include/state.h"
#include <forward_list>
#include <random>


/** TODOs-Ideas:
 * - If all I need is the shortest path to a goal state then A* search could be faster than BFS with a heuristic
 * that prioritizes moves towards the endzone (i.e. up for black, down for white)?
 */


struct Quoridor_move : public MCTS_move {
    short int x, y;
    char player;
    char type;         // 'h'/'v' -> horizontal/vertical wall, ' ' or other for move
    Quoridor_move(short int x, short int y, char p, char t) : x(x), y(y), player(p), type(t) { }
    bool operator==(const MCTS_move& other) const override {
        const Quoridor_move &o = (const Quoridor_move &) other;
        return x == o.x && y == o.y && player == o.player && type == o.type;
    }
    string sprint() const override {
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
    bool wall_connections[8][8]{};    // Note: can be a bitmap to save up space (bools are 1 byte = 8 bits)
    /** Whose turn it is to play: 'W' or 'B' */
    char turn;
    /** Keep track of the distance from each player to each square while
     * taking account for any walls */
    short int **wdists;
    short int **bdists;
    /** moves played */
    unsigned int move_counter;
    /** randomness for rollouts */
    static default_random_engine generator;
    //////////////////////////////////////////
    char change_turn() { turn = (turn == 'W') ? 'B' : 'W'; return turn; }
    bool horizontal_wall(short int x, short int y) const { return walls[x][y] == 'h' || walls[x][y] == 'b'; }
    bool vertical_wall(short int x, short int y) const { return walls[x][y] == 'v' || walls[x][y] == 'b'; }
    void add_wall(short int x, short int y, bool horizontal);
    void remove_wall(short int x, short int y, bool horizontal);
    bool legal_step(short int x, short int y, char p) const;
    bool legal_wall(short int x, short int y, char p, bool horizontal, bool check_blocking = true);
    short int **calculate_dists_from(short int x, short int y, bool stop_at_goal, char player);
    static void reset_dists(short int **&dists);
public:
    Quoridor_state();
    Quoridor_state(const Quoridor_state &other);
    ~Quoridor_state() override;
    char whose_turn() const { return turn; }
    unsigned int get_number_of_turns() const { return move_counter; }
    char check_winner() const;
    short int remaining_walls(char p) const { return (p == 'W') ? wwallsno : bwallsno; }
    bool legal_move(const Quoridor_move *move);
    bool play_move(const Quoridor_move *move);
    int get_shortest_path(char player, const Quoridor_move *extra_wall_move = NULL, short int posx = -1, short int posy = -1);
    forward_list<MCTS_move *> get_legal_step_moves(char p) const;
    vector<MCTS_move *> get_legal_step_moves2(char p) const;
    Quoridor_move *get_best_step_move(char player);
    /** Heuristics **/
    queue<MCTS_move *> *generate_good_moves();
    queue<MCTS_move *> *generate_all_moves();
    friend bool force_playwall(Quoridor_state &s);
    friend Quoridor_move *pick_semirandom_move(Quoridor_state &s, std::uniform_real_distribution<double> &dist, std::default_random_engine &gen);
    friend double evaluate_position(Quoridor_state &s, bool cheap);
    /** Overrides: **/
    bool is_terminal() const override;
    MCTS_state *next_state(const MCTS_move *move) const override;
    queue<MCTS_move *> *actions_to_try() const override;
    double rollout() const override;                        // the rollout simulation in MCTS
    void print() const override;
    bool player1_turn() const override { return turn == 'W'; }
};


#endif
