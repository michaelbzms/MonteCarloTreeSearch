# Monte Carlo Tree Search C++ implementation and its application to Quoridor

**Monte Carlo Tree Search (MCTS)** is a probabilistic search algorithm that uses random simulations to selectively (i.e. in an imbalanced manner) grow a game tree.
MCTS has been particularly successful in domains with vast search spaces (i.e. game trees with high branching factor) 
where deterministic algorithms such as minimax (or alpha-beta pruning) have struggled.

This project contains a fast **C++ implementation** of the vanilla **Monte Carlo Tree Search (MCTS)** algorithm. 
This implementation (mcts.h/.cpp) uses abstract (aka pure virtual) classes for the concept of states and moves (state.h).
The user of this implementation should extend these classes and implement their virtual methods according to his desired application.
He can then use tree class provided in mcts.h to use this algorithm.

Two examples are provided along with said implementation. 
The first is a simple Tic-Tac-Toe application of MCTS that I used to mainly debug and develop the MCTS algorithm.
The second is its application to **Quoridor**, a zero-sum two-player strategy game with a notoriously high average branching factor 
(more than Chess but less than Go).
