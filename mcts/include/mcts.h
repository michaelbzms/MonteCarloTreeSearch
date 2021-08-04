#ifndef MCTS_H
#define MCTS_H

#include "state.h"
#include <vector>
#include <queue>

#define STARTING_NUMBER_OF_CHILDREN 4   // expected number so that we can preallocate this many pointers


using namespace std;


class MCTS_node {
    bool terminal;
    unsigned int size;
    unsigned int number_of_simulations;
    double score;                       // e.g. number of wins (could be int but double is more general if we use evaluation functions)
    MCTS_state *state;                  // current state
    MCTS_move *move;                    // move to get here from parent node's state
    vector<MCTS_node *> *children;
    MCTS_node *parent;
    queue<MCTS_move *> *untried_actions;
    void backpropagate(double w, int n);
public:
    MCTS_node(MCTS_node *parent, MCTS_state *state, MCTS_move *move);
    ~MCTS_node();
    bool is_fully_expanded() const;
    bool is_terminal() const;
    unsigned int get_size() const;
    void expand();
    void rollout();
    MCTS_node *select_best_child(double c);
    MCTS_node *advance_tree(MCTS_move *move);
    MCTS_node *
};



class MCTS_tree {
    MCTS_node *root;
public:
    MCTS_tree(MCTS_state *starting_state);
    ~MCTS_tree();
    MCTS_node *select(double c=1.41);        // select child node according to tree policy (UCT)
    void grow_tree(int max_iter, double max_time_in_seconds);
    void advance_tree(MCTS_move *move);      // if the move is applicable advance the tree, else start over
    unsigned int get_size() const;
};


#endif
