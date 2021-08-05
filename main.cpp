#include <iostream>
#include "examples/TicTacToe.h"
#include "mcts/include/mcts.h"


MCTS_move *select_random_move(const MCTS_state *state);


int main() {
//    MCTS_state *state = new TicTacToe_state();
//    state = state->next_state(new TicTacToe_move(0, 0, 'x'))
//        ->next_state(new TicTacToe_move(0, 1, 'x'))
//        ->next_state(new TicTacToe_move(1, 0, 'x'))
//        ->next_state(new TicTacToe_move(1, 1, 'x'))
//        ->next_state(new TicTacToe_move(2, 0, 'o'))
//        ->next_state(new TicTacToe_move(2, 1, 'o'))
//        ->next_state(new TicTacToe_move(0, 2, 'o'))
//        ->next_state(new TicTacToe_move(1, 2, 'o'))
//        ->next_state(new TicTacToe_move(2, 2, 'o'));
//    state->print();
//    cout << "Terminal: " << state->is_terminal() << endl;


//    TicTacToe_state state;
//    state.print();
//    TicTacToe_move move(0, 0, 'x');
//    MCTS_state *next_state = state.next_state(&move);
//    next_state->print();
////    queue<MCTS_move *> *actions = next_state->actions_to_try();
//    double res = next_state->rollout();
//    cout << "Rollout result: " << res << endl;
//    delete next_state;


    MCTS_state *state = new TicTacToe_state();
    MCTS_agent agent(state);
    MCTS_move *enemy_move = NULL;
    while (!state->is_terminal()) {
        agent.genmove(enemy_move);
        const MCTS_state *new_state = agent.get_current_state();
        enemy_move = select_random_move(new_state);
    }

    return 0;
}


MCTS_move *select_random_move(const MCTS_state *state){
    MCTS_move *move = NULL, *m = NULL;
    queue<MCTS_move *> *actions = state->actions_to_try();
    int r = rand() % actions->size();
    while ((r--) > 0) {
        m = actions->front();
        actions->pop();
        delete m;
    }
    move = actions->front();
    actions->pop();
    while (!actions->empty()) {
        m = actions->front();
        delete m;
    }
    return move;
}