#ifndef MCTS_QUORIDOR_H
#define MCTS_QUORIDOR_H

#include "../../mcts/include/state.h"


class Quoridor_state : public MCTS_state {
    /** white's and black's coordinates on the board */
    short int wx, wy, bx, by;
    /** white's and black's remaining number of walls */
    short int wwallsno, bwallsno;
    /** Effectively a look-up table:
     * 'h', 'v', 'b' for horizontal, vertical and both walls in the right and bottom part of a cell */
    char walls[8][8]{};
    bool wall_connections[8][8]{};    // TODO: can be a bitmap to save up space (bools are 1 byte = 8 bits)
    /** Whose turn it is to play: 'W' or 'B' */
    char turn;

    //////////////////////////////////////////
    char change_turn() { turn = (turn == 'W') ? 'B' : 'W'; return turn; }
public:
    Quoridor_state();
    Quoridor_state(const Quoridor_state &other);

};


#endif
