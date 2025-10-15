#include "pips/PipsSolver.hpp"
#include <cstdlib>
#include <ctime>

PipsSolver::PipsSolver(int width, int height,
                       std::vector<Position> disabledTiles,
                       std::vector<Domino> dominos,
                       std::shared_ptr<std::vector<Constraint>> constraints)
    : dominos(std::move(dominos)), constraints(std::move(constraints)),
      grid(height, std::vector<Tile>(width, Tile(-1))),
      bestFound{width, height, disabledTiles, dominos, constraints} {
  srand(time(0));
}

PipsState PipsSolver::iterate() {
  auto actions = bestFound.availableActions();
  PipsState curr = bestFound;
  for (const auto &action : actions) {
    curr = bestFound.stateAfterAction(action);
    if (curr.objective() > bestFound.objective())
      bestFound = curr;
    else if (rand() % 100 == 0)
      bestFound = curr;
  }
  return bestFound;
}
