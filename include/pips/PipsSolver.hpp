#ifndef PIPS_SOLVER_HPP
#define PIPS_SOLVER_HPP

#include "pips/PipsState.hpp"
#include <iostream>
#include <memory>
#include <vector>

// Declarations

template <int Width, int Height> class PipsSolver {
  PipsState<Width, Height> bestFound;
  Grid<Width, Height> grid;
  std::vector<Domino> dominos;
  int iterations = 0;

public:
  std::shared_ptr<const std::vector<std::shared_ptr<Constraint>>> constraints;
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
    : bestFound(startState), dominos(startState.dominos),
      constraints(startState.constraints),
      grid(startState.grid) {
  srand(time(0));
}

template <int Width, int Height>
PipsState<Width, Height> PipsSolver<Width, Height>::iterate() {
  iterations++;
  std::cout << "Iterating" << '\n';
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
  std::cout << "Entering solve while loop" << '\n';
  std::cout << "Constraints: " << constraints << '\n';
  std::cout << "Bestfound Constraints: " << bestFound.constraints << '\n';
  while (!bestFound.isSolved()) {
    std::cout << "in loop" << '\n';
    iterate();
  }
  return bestFound;
}

#endif
