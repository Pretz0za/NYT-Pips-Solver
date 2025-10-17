#ifndef PIPS_SOLVER_HPP
#define PIPS_SOLVER_HPP

#include "pips/PipsState.hpp"
#include <memory>
#include <vector>

// Declarations

template <int Width, int Height> class PipsSolver {
  PipsState<Width, Height> bestFound;
  Grid<Width, Height> grid;
  std::shared_ptr<const std::vector<std::shared_ptr<Constraint>>> constraints;
  std::vector<Domino> dominos;

public:
  PipsSolver(
      int width, int height, std::vector<Position> disabledTiles,
      std::vector<Domino> dominos,
      std::shared_ptr<std::vector<std::shared_ptr<Constraint>>> constraints);
  PipsSolver(PipsState<Width, Height> startState);
  PipsState<Width, Height> iterate();
  PipsState<Width, Height> solve();
};

// Definitions

template <int Width, int Height>
PipsSolver<Width, Height>::PipsSolver(
    int width, int height, std::vector<Position> disabledTiles,
    std::vector<Domino> dominos,
    std::shared_ptr<std::vector<std::shared_ptr<Constraint>>> constraints)
    : dominos(std::move(dominos)), constraints(std::move(constraints)),
      grid(disabledTiles), bestFound{disabledTiles, dominos, constraints} {
  srand(time(0));
}

template <int Width, int Height>
PipsSolver<Width, Height>::PipsSolver(PipsState<Width, Height> startState)
    : bestFound(std::move(startState)), dominos(std::move(startState.dominos)),
      constraints(std::move(bestFound.constraints)),
      grid(std::move(startState.grid)) {
  srand(time(0));
}

template <int Width, int Height>
PipsState<Width, Height> PipsSolver<Width, Height>::iterate() {
  auto actions = bestFound.availableActions();
  PipsState curr = bestFound;
  for (const auto &action : actions) {
    curr = bestFound.stateAfterAction(action);
    if (curr.objective() < bestFound.objective())
      bestFound = curr;
    else if (rand() % 100 == 0)
      bestFound = curr;
  }
  return bestFound;
}

template <int Width, int Height>
PipsState<Width, Height> PipsSolver<Width, Height>::solve() {
  while (!bestFound.isSolved())
    iterate();
  return bestFound;
}

#endif
