#include "Quoridor.h"


Quoridor_state::Quoridor_state()
    : wx(4), wy(0), bx(4), by(8), wwallsno(10), bwallsno(10), turn('W') {
    for (int i = 0 ; i < 64 ; i++) {
        walls[i / 8][i % 8] = ' ';
        wall_connections[i / 8][i % 8] = false;
    }
}

Quoridor_state::Quoridor_state(const Quoridor_state &other)
    : wx(other.wx), wy(other.wy), bx(other.bx), by(other.by),
      wwallsno(other.wwallsno), bwallsno(other.bwallsno), turn(other.turn) {
    for (int i = 0 ; i < 64 ; i++) {
        walls[i / 8][i % 8] = other.walls[i / 8][i % 8];
        wall_connections[i / 8][i % 8] = other.wall_connections[i / 8][i % 8];
    }
}
