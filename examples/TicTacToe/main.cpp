#include <iostream>
#include "TicTacToe.h"
#include "../../mcts/include/mcts.h"


MCTS_move *select_random_move(const MCTS_state *state);


int main() {
    // This is just to test the example, not ideally what one should be doing... TODO
    bool done;
    char winner;
    MCTS_state *state = new TicTacToe_state();
    state->print();                           // IMPORTANT: state will be garbage after advance_tree()
    MCTS_agent agent(state, 1000);
    MCTS_move *enemy_move = NULL;
    do {
        agent.feedback();
        // TODO: This way we don't check if the enemy move ends the game but it's our responsibility to check that, not the tree's...
        agent.genmove(enemy_move);   // Note: we ignore the move because the MCTS tree has played it as well and we can get it from there.
        const MCTS_state *new_state = agent.get_current_state();
        new_state->print();
        if (new_state->is_terminal()) {
            winner = ((const TicTacToe_state *) new_state)->get_winner();
            break;
        }
        enemy_move = select_random_move(new_state);
        done = new_state->is_terminal();
        winner = ((const TicTacToe_state *) new_state)->get_winner();
    } while (!done);
    delete enemy_move;
    cout << "\nWinner is: " << winner << endl;
    return 0;
}


MCTS_move *select_random_move(const MCTS_state *state){
    MCTS_move *move = NULL, *m = NULL;
    queue<MCTS_move *> *actions = state->actions_to_try();
    int r = rand() % actions->size();
    while ((r--) > 0) {
        m = actions->front();
        delete m;
        actions->pop();
    }
    move = actions->front();
    actions->pop();
    while (!actions->empty()) {
        m = actions->front();
        delete m;
        actions->pop();
    }
    delete actions;
    return move;
}
