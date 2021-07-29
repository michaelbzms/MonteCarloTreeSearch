#ifndef MCTS_STATE_H
#define MCTS_STATE_H

#include <stdexcept>
#include <queue>


using namespace std;


class MCTS_move {
    // TODO: ???
    // TODO: Queue iteration:  for (auto it=queue.begin(); it!=int_queue.end(); it++)
public:
    virtual ~MCTS_move() = default;
};


class MCTS_state {
    bool perform_eval;
public:
    explicit MCTS_state(bool use_eval_function = false) : perform_eval(use_eval_function) {}
    virtual ~MCTS_state() = default;
    // rollout must return something in [0, 1] for UCT to work as intended
    virtual MCTS_state *next_state(MCTS_move move) = 0;   // implement this
    virtual double rollout() = 0;                          // implement this (the rollout simulation in MCTS)
    virtual double evaluate() {                            // optionally implement this as well (if use_eval_function is true)
        throw std::runtime_error("Not Implemented");
    }
    virtual queue<MCTS_move> *actions_to_try() = 0;
    virtual bool is_terminal() = 0;
};


#endif
