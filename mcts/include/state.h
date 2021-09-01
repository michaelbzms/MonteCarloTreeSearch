#ifndef MCTS_STATE_H
#define MCTS_STATE_H

#include <stdexcept>
#include <queue>


using namespace std;


struct MCTS_move {
    virtual ~MCTS_move() = default;
    virtual bool operator==(const MCTS_move& other) const = 0;             // implement this!
    virtual string sprint() const { return "Not implemented"; }   // and optionally this
};


/** Implement all pure virtual methods. Notes:
 * - rollout() must return something in [0, 1] for UCT to work as intended and specifically
 * the winning chance of player1.
 * - player1 is determined by player1_turn()
 */
class MCTS_state {
public:
    // Implement these:
    virtual ~MCTS_state() = default;
    virtual queue<MCTS_move *> *actions_to_try() const = 0;
    virtual MCTS_state *next_state(const MCTS_move *move) const = 0;
    virtual double rollout() const = 0;
    virtual bool is_terminal() const = 0;
    virtual void print() const {
        cout << "Printing not implemented" << endl;
    }
    virtual bool player1_turn() const = 0;     // MCTS is for two-player games mostly -> (keeps win rate)
};


#endif
