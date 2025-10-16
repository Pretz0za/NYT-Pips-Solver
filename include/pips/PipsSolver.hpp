#ifndef PIPS_SOLVER_HPP
#define PIPS_SOLVER_HPP

#include "pips/PipsState.hpp"
#include <vector>

template <int Width, int Height> class PipsSolver {
  std::vector<std::vector<Tile>> grid;
  std::shared_ptr<std::vector<Constraint>> constraints;
  std::vector<Domino> dominos;
  PipsState<Width, Height> bestFound;

public:
  PipsSolver(int width, int height, std::vector<Position> disabledTiles,
             std::vector<Domino> dominos,
             std::shared_ptr<std::vector<Constraint>> constraints);
  PipsState<Width, Height> iterate();
  PipsState<Width, Height> solve();
};

#endif
