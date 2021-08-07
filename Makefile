FLAGS = -O2 -g3 -pedantic -std=c++11 # -Wall -Wextra
TICTACTOE_EXE = tictactoe

all: TicTacToe


mcts.o: mcts/src/mcts.cpp mcts/include/mcts.h mcts/include/state.h
	g++ -c $(FLAGS) mcts/src/mcts.cpp


TicTacToe: mcts.o examples/TicTacToe/main.cpp examples/TicTacToe/TicTacToe.cpp examples/TicTacToe/TicTacToe.h
	g++ -o $(TICTACTOE_EXE) $(FLAGS) examples/TicTacToe/main.cpp examples/TicTacToe/TicTacToe.cpp mcts.o


clean:
	rm -f *.o $(TICTACTOE_EXE)
