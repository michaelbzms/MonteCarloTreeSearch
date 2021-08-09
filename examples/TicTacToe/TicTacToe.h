#ifndef MCTS_TICTACTOE_H
#define MCTS_TICTACTOE_H

#include "../../mcts/include/state.h"


class TicTacToe_state : public MCTS_state {
    char board[3][3]{};
    bool player_won(char player) const;
    char calculate_winner() const;
    char turn, winner;
    void change_turn();
public:
    TicTacToe_state();
    TicTacToe_state(const TicTacToe_state &other);
    char get_turn() const;
    char get_winner() const;
    bool is_terminal() const override;
    MCTS_state *next_state(const MCTS_move *move) const override;
    queue<MCTS_move *> *actions_to_try() const override;
    double rollout() const override;                        // the rollout simulation in MCTS
    void print() const override;
    bool player1_turn() const override { return turn == 'x'; }
};


struct TicTacToe_move : public MCTS_move {
    int x, y;
    char player;
    TicTacToe_move(int x, int y, char p) : x(x), y(y), player(p) {}
    bool operator==(const MCTS_move& other) const override;
};

#endif
