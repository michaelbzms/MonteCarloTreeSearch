#ifndef MCTS_H
#define MCTS_H

#include "state.h"
#include <vector>
#include <queue>
#include <iomanip>

#define STARTING_NUMBER_OF_CHILDREN 4   // expected number so that we can preallocate this many pointers


using namespace std;

/** TODOs
 * - state, move, parent should probably be const
 */


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
    MCTS_move *get_move() const;
    unsigned int get_size() const;
    void expand();
    void rollout();
    MCTS_node *select_best_child(double c) const;
    MCTS_node *advance_tree(MCTS_move *m);
    const MCTS_state *get_current_state() const;
    void print_stats() const;
};



class MCTS_tree {
    MCTS_node *root;
public:
    MCTS_tree(MCTS_state *starting_state);
    ~MCTS_tree();
    MCTS_node *select(double c=1.41);        // select child node to expand according to tree policy (UCT)
    MCTS_node *select_best_child();          // select the most promising child of the root node
    void grow_tree(int max_iter, double max_time_in_seconds);
    void advance_tree(MCTS_move *move);      // if the move is applicable advance the tree, else start over
    unsigned int get_size() const;
    const MCTS_state *get_current_state() const;
    void print_stats() const;
};


class MCTS_agent {
    MCTS_tree *tree;
    int max_iter, max_seconds;
public:
    MCTS_agent(MCTS_state *starting_state, int max_iter = 100000, int max_seconds = 30);
    ~MCTS_agent();
    MCTS_move *genmove(MCTS_move *enemy_move);
    const MCTS_state *get_current_state() const;
    void feedback() const { tree->print_stats(); }
};


#endif
