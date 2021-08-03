#include <iostream>
#include <cassert>
#include <cmath>
#include <ctime>
#include "../include/mcts.h"

#define DEBUG


using namespace std;


/*** MCTS NODE ***/
MCTS_node::MCTS_node(MCTS_node *parent, MCTS_state *state, MCTS_move *move)
        : parent(parent), state(state), move(move), score(0.0), number_of_simulations(0), size(0) {
    children = new vector<MCTS_node>();
    children->reserve(STARTING_NUMBER_OF_CHILDREN);
    untried_actions = state->actions_to_try();
    terminal = state->is_terminal();
}

MCTS_node::~MCTS_node() {
    delete children;
}

void MCTS_node::expand() {
    if (is_fully_expanded()) {
        cerr << "Warning: Cannot expanded this node any more (or it is terminal)!" << endl;
        return;
    }
    // get next untried action
    MCTS_move next_move = untried_actions->front();     // get value
    untried_actions->pop();                             // remove it
    MCTS_state *next_state = state->next_state(next_move);
    // build a new MCTS node from it
    MCTS_node new_node(this, next_state, &next_move);
    // rollout, updating its stats
    new_node.rollout();
    // add new node to tree
    children->push_back(new_node);                      // TODO: copy by value (right?)
}

void MCTS_node::rollout() {
    // TODO: do more than one simulations? Perhaps in parallel?
    double w = state->rollout();
    backpropagate(w, 1);
}

void MCTS_node::backpropagate(double w, int n) {
    score += w;
    number_of_simulations++;
    size++;   // Note: number of sims can differ from size if we do more than one per node expanded.
    if (parent != NULL) {
        parent->backpropagate(w, n);
    }
}

bool MCTS_node::is_fully_expanded() const {
    return is_terminal() || untried_actions->empty();
}

bool MCTS_node::is_terminal() const {
    return terminal;
}

unsigned int MCTS_node::get_size() const {
    return size;
}

MCTS_node *MCTS_node::select_best_child(double c) {
    if (children->empty()) return NULL;
    else if (children->size() == 1) return &children->at(0);
    else {
        double uct, max = -1;
        MCTS_node *argmax = NULL;
        for (auto& child : *children) {
            if (c > 0) {
                uct = child.score / ((double) child.number_of_simulations) +
                      c * sqrt(log((double) this->number_of_simulations) / ((double) child.number_of_simulations));
            } else {
                uct = child.score / child.number_of_simulations;
            }
            if (uct > max) {
                max = uct;
                argmax = &child;
            }
        }
        return argmax;
    }
}


/*** MCTS TREE ***/
MCTS_node *MCTS_tree::select(double c) {
    MCTS_node *node = root;
    while (!node->is_terminal()) {
        if (!node->is_fully_expanded()) {
            return node;
        } else {
            node = node->select_best_child(c);
        }
    }
    return node;
}

MCTS_tree::MCTS_tree(MCTS_state *starting_state) : root(NULL) {
    assert(starting_state != NULL);
    root = new MCTS_node(NULL, starting_state, NULL);
}

MCTS_tree::~MCTS_tree() {
    delete root;
}

void MCTS_tree::grow_tree(int max_iter, double max_time_in_seconds) {
    MCTS_node *node;
    double dt;
    time_t start_t, now_t;
    time(&start_t);
    for (int i = 0 ; i < max_iter ; i++){
        // select node to expand according to tree policy
        node = select();
        // expand it (this will perform a rollout and backpropagate the results)
        node->expand();
        // check if we need to stop
        time(&now_t);
        dt = difftime(now_t, start_t);
        if (dt > max_time_in_seconds) {
            #ifdef DEBUG
            cout << "Made " << i << " iterations in " << dt << " seconds!" << endl;
            #endif
            break;
        }
    }
}

unsigned int MCTS_tree::get_size() const {
    return root != NULL && root->get_size();
}
