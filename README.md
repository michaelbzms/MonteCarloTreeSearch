## Monte Carlo Tree Search C++ implementation & application to Quoridor

**Monte Carlo Tree Search (MCTS)** is a probabilistic search algorithm that uses random simulations to selectively (i.e. in an imbalanced manner) grow a game tree.
MCTS has been particularly successful in domains with vast search spaces (i.e. game trees with high branching factor) 
where deterministic algorithms such as minimax (or alpha-beta pruning) have struggled.

This project contains a fast **C++** implementation of the vanilla MCTS algorithm. 
This implementation (mcts.h/.cpp) uses abstract (aka pure virtual) classes for the concept of states and moves (state.h).
The user of this implementation should extend these classes and implement their virtual methods according to his desired application.
He can then use tree class provided in mcts.h to use this algorithm.

Two usage examples are provided along with said implementation (the Makefile compiles both): 
1. The first is a simple Tic-Tac-Toe application of MCTS that I used to mainly debug and develop the MCTS algorithm.
2. The second is its application to **Quoridor**, a zero-sum two-player strategy game with a notoriously high average branching factor 
(more than Chess but less than Go). After some experimenting (especially with the rollouts) I believe I ended up with a fairly competent agent although there is definitely room for further tuning: 
   a trade-off must be made between lightweight random rollouts and heavier rollouts with smarter actions taken.


## Implementation Details

### MCTS basics

The MCTS algorithm selectively builds up a game tree where each node stores:
1. its associated game state
2. the move that led to this state from the parent node's state
3. the number of simulations performed starting from this node as well as any of its descendants
4. the accumulated score from all said simulations

The score of a simulation must be in the range [0, 1] (e.g. 1 for P1 win, 0 for P1 loss, 0.5 for draw).
When divided the latter two give us an estimate of the average winning probability or win rate for P1 (Player 1) in this node's state.

MCTS consists of 4 different phases:
1. **Selection:** traverse the tree using a policy that balances *exploration vs exploitation* (e.g. UCT) and select a promising node that is not fully expanded yet.
2. **Expansion:** expand the selected node by adding a child node to it based on an untried move on that state.
3. **Simulation or rollout:** simulate a game starting from the new child node either to the end or until a point where an evaluation function can be used.
4. **Backpropagation:** recursively backpropagate the results of the simulation to the parent node all the way up to the root node.

![MCTS phases from wikipedia](https://i.stack.imgur.com/wZAqy.png "MCTS phases")

These 4 phases are repeated for an arbitrary amount of times (e.g. based on how long we want to "think") in order to grow the tree.

In order to select the most promising move for the state of the root node we simply examine all the children of the root node and their win rates (or winning chances) 
and choose the move from the child-node with the highest win rate for the player whose turn it is to play (that is we have to use w for P1 and 1-w for P2). 

After that point we can "advance" the tree and make that child-node the new root node of the tree.

All phases other than the rollout itself - which has to be supplied by the user - are implemented.

### Parallel rollouts

One way to take advantage of modern multi-core CPUs and the fact that different simulations are independent and thus embarrassingly parallel is to use a thread pool (allocated once at the beginning)
and, instead of 1, perform multiple (e.g. as many cores as we have available) rollouts at the Simulation phase. To that end, I employ the JobScheduler.h/.cpp which uses posix threads, mutexes and
conditional variables to implement a thread pool for embarrassingly parallel tasks. Its use is optional through a #defined variable in mcts.h.


## References

1. Max Magnuson. (2015). Monte Carlo Tree Search and Its Applications, https://digitalcommons.morris.umn.edu/horizons/vol2/iss2/4/

2. Massagué Respall, Victor & Brown, Joseph & Aslam, Hamna. (2018). Monte Carlo Tree Search for Quoridor, https://www.researchgate.net/publication/327679826_Monte_Carlo_Tree_Search_for_Quoridor
