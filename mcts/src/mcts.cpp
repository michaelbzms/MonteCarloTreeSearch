#include <iostream>
#include <cassert>
#include <cmath>
#include <ctime>
#include <algorithm>
#include "../include/mcts.h"

#define DEBUG


using namespace std;


/*** MCTS NODE ***/
MCTS_node::MCTS_node(MCTS_node *parent, MCTS_state *state, const MCTS_move *move)
        : parent(parent), state(state), move(move), score(0.0), number_of_simulations(0), size(0) {
    children = new vector<MCTS_node *>();
    children->reserve(STARTING_NUMBER_OF_CHILDREN);
    untried_actions = state->actions_to_try();
    terminal = state->is_terminal();
}

MCTS_node::~MCTS_node() {
    delete state;
    delete move;
    for (auto *child : *children) {
        delete child;
    }
    delete children;
    while (!untried_actions->empty()) {
        delete untried_actions->front();    // if a move is here then it is not a part of a child node and needs to be deleted here
        untried_actions->pop();
    }
    delete untried_actions;
}

void MCTS_node::expand() {
    if (is_terminal()) {              // can legitimately happen in end-game situations
        rollout();                    // keep rolling out, eventually causing UCT to pick another node to expand due to exploration
        return;
    } else if (is_fully_expanded()) {
        cerr << "Warning: Cannot expanded this node any more!" << endl;
        return;
    }
    // get next untried action
    MCTS_move *next_move = untried_actions->front();     // get value
    untried_actions->pop();                              // remove it
    MCTS_state *next_state = state->next_state(next_move);
    // build a new MCTS node from it
    MCTS_node *new_node = new MCTS_node(this, next_state, next_move);
    // rollout, updating its stats
    new_node->rollout();
    // add new node to tree
    children->push_back(new_node);
}

void MCTS_node::rollout() {
#ifdef PARALLEL_ROLLOUTS
    // schedule Jobs
    static JobScheduler scheduler;               // static so that we don't create new threads every time (!)
    double results[NUMBER_OF_THREADS]{-1};
    for (int i = 0 ; i < NUMBER_OF_THREADS ; i++) {
        scheduler.schedule(new RolloutJob(state, &results[i]));
    }
    // wait for all simulations to finish
    scheduler.waitUntilJobsHaveFinished();
    // aggregate results
    double score_sum = 0.0;
    for (int i = 0 ; i < NUMBER_OF_THREADS ; i++) {
        if (results[i] >= 0.0 && results[i] <= 1.0){
            score_sum += results[i];
        } else {    // should not happen
            cerr << "Warning: Invalid result when aggregating parallel rollouts" << endl;
        }
    }
    backpropagate(score_sum, NUMBER_OF_THREADS);
#else
    double w = state->rollout();
    backpropagate(w, 1);
#endif
}

void MCTS_node::backpropagate(double w, int n) {
    score += w;
    number_of_simulations += n;
    if (parent != NULL) {
        parent->size++;
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

MCTS_node *MCTS_node::select_best_child(double c) const {
    /** selects best child based on the winrate of whose turn it is to play */
    if (children->empty()) return NULL;
    else if (children->size() == 1) return children->at(0);
    else {
        double uct, max = -1;
        MCTS_node *argmax = NULL;
        for (auto *child : *children) {
            double winrate = child->score / ((double) child->number_of_simulations);
            // If its the opponent's move apply UCT based on his winrate i.e. our loss rate.   <-------
            if (!state->player1_turn()){
                winrate = 1.0 - winrate;
            }
            if (c > 0) {
                uct = winrate +
                      c * sqrt(log((double) this->number_of_simulations) / ((double) child->number_of_simulations));
            } else {
                uct = winrate;
            }
            if (uct > max) {
                max = uct;
                argmax = child;
            }
        }
        return argmax;
    }
}

MCTS_node *MCTS_node::advance_tree(const MCTS_move *m) {
    // Find child with this m and delete all others
    MCTS_node *next = NULL;
    for (auto *child: *children) {
        if (*(child->move) == *(m)) {
            next = child;
        } else {
            delete child;
        }
    }
    // remove children from queue so that they won't be re-deleted by the destructor when this node dies (!)
    this->children->clear();
    // if not found then we have to create a new node
    if (next == NULL) {
        // Note: UCT may lead to not fully explored tree even for short-term children due to terminal nodes being chosen
        cout << "INFO: Didn't find child node. Had to start over." << endl;
        MCTS_state *next_state = state->next_state(m);
        next = new MCTS_node(NULL, next_state, NULL);
    } else {
        next->parent = NULL;     // make parent NULL
        // IMPORTANT: m and next->move can be the same here if we pass the move from select_best_child()
        // (which is what we will typically be doing). If not then it's the caller's responsibility to delete m (!)
    }
    // return the next root
    return next;
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

MCTS_tree::MCTS_tree(MCTS_state *starting_state) {
    assert(starting_state != NULL);
    root = new MCTS_node(NULL, starting_state, NULL);
}

MCTS_tree::~MCTS_tree() {
    delete root;
}

void MCTS_tree::grow_tree(int max_iter, double max_time_in_seconds) {
    MCTS_node *node;
    double dt;
    #ifdef DEBUG
    cout << "Growing tree..." << endl;
    #endif
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
            cout << "Early stopping: Made " << (i + 1) << " iterations in " << dt << " seconds." << endl;
            #endif
            break;
        }
    }
    #ifdef DEBUG
    time(&now_t);
    dt = difftime(now_t, start_t);
    cout << "Finished in " << dt << " seconds." << endl;
    #endif
}

unsigned int MCTS_tree::get_size() const {
    return root->get_size();
}

const MCTS_move *MCTS_node::get_move() const {
    return move;
}

const MCTS_state *MCTS_node::get_current_state() const { return state; }

void MCTS_node::print_stats() const {
    #define TOPK 10
    if (number_of_simulations == 0) {
        cout << "Tree not expanded yet" << endl;
        return;
    }
    cout << "___ INFO _______________________" << endl
         << "Tree size: " << size << endl
         << "Number of simulations: " << number_of_simulations << endl
         << "Branching factor at root: " << children->size() << endl
         << "Chances of P1 winning: " << setprecision(4) << 100.0 * (score / number_of_simulations) << "%" << endl;
    // sort children based on winrate of player's turn for this node (!)
    if (state->player1_turn()) {
        std::sort(children->begin(), children->end(), [](const MCTS_node *n1, const MCTS_node *n2){
            return n1->calculate_winrate(true) > n2->calculate_winrate(true);
        });
    } else {
        std::sort(children->begin(), children->end(), [](const MCTS_node *n1, const MCTS_node *n2){
            return n1->calculate_winrate(false) > n2->calculate_winrate(false);
        });
    }
    // print TOPK of them along with their winrates
    cout << "Best moves:" << endl;
    for (int i = 0 ; i < children->size() && i < TOPK ; i++) {
        cout << "  " << i + 1 << ". " << children->at(i)->move->sprint() << "  -->  "
             << setprecision(4) << 100.0 * children->at(i)->calculate_winrate(state->player1_turn()) << "%" << endl;
    }
    cout << "________________________________" << endl;
}

double MCTS_node::calculate_winrate(bool player1turn) const {
    if (player1turn) {
        return score / number_of_simulations;
    } else {
        return 1.0 - score / number_of_simulations;
    }
}

void MCTS_tree::advance_tree(const MCTS_move *move) {
    MCTS_node *old_root = root;
    root = root->advance_tree(move);
    delete old_root;       // this won't delete the new root since we have emptied old_root's children
}

const MCTS_state *MCTS_tree::get_current_state() const { return root->get_current_state(); }

MCTS_node *MCTS_tree::select_best_child() {
    return root->select_best_child(0.0);
}

void MCTS_tree::print_stats() const { root->print_stats(); }


/*** MCTS agent ***/
MCTS_agent::MCTS_agent(MCTS_state *starting_state, int max_iter, int max_seconds)
: max_iter(max_iter), max_seconds(max_seconds) {
    tree = new MCTS_tree(starting_state);
}

const MCTS_move *MCTS_agent::genmove(const MCTS_move *enemy_move) {
    if (enemy_move != NULL) {
        tree->advance_tree(enemy_move);
    }
    // If game ended from opponent move, we can't do anything
    if (tree->get_current_state()->is_terminal()) {
        return NULL;
    }
    #ifdef DEBUG
    cout << "___ DEBUG ______________________" << endl
         << "Growing tree..." << endl;
    #endif
    tree->grow_tree(max_iter, max_seconds);
    #ifdef DEBUG
    cout << "Tree size: " << tree->get_size() << endl
         << "________________________________" << endl;
    #endif
    MCTS_node *best_child = tree->select_best_child();
    if (best_child == NULL) {
        cerr << "Warning: Tree root has no children! Possibly terminal node!" << endl;
        return NULL;
    }
    const MCTS_move *best_move = best_child->get_move();
    tree->advance_tree(best_move);
    return best_move;
}

MCTS_agent::~MCTS_agent() {
    delete tree;
}

const MCTS_state *MCTS_agent::get_current_state() const { return tree->get_current_state(); }
