#ifndef PIPS_SOLVER_HPP
#define PIPS_SOLVER_HPP

#include "pips/PipsState.hpp"
#include <vector>

class PipsSolver {
  std::vector<std::vector<Tile>> grid;
  std::shared_ptr<std::vector<Constraint>> constraints;
  std::vector<Domino> dominos;
  PipsState bestFound;

public:
	PipsSolver(int width, int height, std::vector<Position> disabledTiles,
            std::vector<Domino> dominos,
            std::shared_ptr<std::vector<Constraint>> constraints);
	PipsState iterate();
	PipsState solve();
};

#endif
