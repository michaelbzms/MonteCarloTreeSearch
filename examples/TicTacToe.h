#ifndef MCTS_TICTACTOE_H
#define MCTS_TICTACTOE_H

#include "../mcts/include/state.h"

class TicTacToe_state : MCTS_state {
    char board[3][3];
    bool player_won(char player);
public:
    TicTacToe_state();
    TicTacToe_state(const TicTacToe_state &other);
    bool is_terminal() override;
    MCTS_state *next_state(MCTS_move *move) override;
    queue<MCTS_move> *actions_to_try() override;
    double rollout() override;                        // the rollout simulation in MCTS
};


struct TicTacToe_move : MCTS_move {
    int x, y;
    char player;
};

#endif
