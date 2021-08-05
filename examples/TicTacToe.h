#ifndef MCTS_TICTACTOE_H
#define MCTS_TICTACTOE_H

#include "../mcts/include/state.h"


// TODO-IDEA: Instead of a FIFO Queue use a Priority Queue
// with priority on most probable (better) actions to be explored first
// TODO: or maybe this should just be an iterable and we let the implementation decide!


class TicTacToe_state : public MCTS_state {
    char board[3][3];
    bool player_won(char player);
    char turn, winner;
    void change_turn();
public:
    TicTacToe_state();
    TicTacToe_state(const TicTacToe_state &other);
    char get_turn() const;
    char get_winner(bool test = true);
    bool is_terminal() override;
    MCTS_state *next_state(MCTS_move *move) override;
    queue<MCTS_move *> *actions_to_try() const override;
    double rollout() override;                        // the rollout simulation in MCTS
    void print() override;
};


struct TicTacToe_move : public MCTS_move {
    int x, y;
    char player;
    TicTacToe_move(int x, int y, char p) : x(x), y(y), player(p) {}
    bool operator==(const MCTS_move& other) const override;
};

#endif
