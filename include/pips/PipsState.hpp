#ifndef PIPS_STATE_HPP
#define PIPS_STATE_HPP

#include "helpers.hpp"
#include <algorithm>
#include <array>
#include <cstdlib>
#include <initializer_list>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

// Declarations

struct Position {
  int x;
  int y;
};

namespace Orientation {
inline std::pair Right{false, false};
inline std::pair Left{false, true};
inline std::pair Down{true, false};
inline std::pair Up{true, true};
} // namespace Orientation

class Domino {
  std::pair<int, int> value;
  Position position;
  // 4 possible orientations give us:
  std::pair<bool, bool> orientation;

public:
  Domino(std::pair<int, int> value);
  Domino(std::pair<int, int> value, Position position,
         std::pair<bool, bool> orientation);

  void setPosition(const Position &newPosition);
  void setOrientation(const std::pair<bool, bool> &newOrientation);

  void rotate();
  void swap(Domino &other);

  // Returns the values of the domino
  std::pair<int, int> getValues() const;

  // Returns the positions of both faces in a pair based on orientation
  std::pair<Position, Position> getPosition() const;

  ~Domino() = default;
};

class Tile {
  int value;

public:
  Tile(int value = -1);

  // gets/sets the value of the tile and errors if the tile is out of play
  int getValue() const;
  void setValue(int newValue);

  ~Tile() = default;
};

template <int Width, int Height> class Grid {
  std::array<std::array<Tile, Width>, Height> grid;

public:
  Grid(const std::vector<Position> &disabledTiles);
  std::array<std::array<Tile, Width>, Height> getGrid() const;

  Tile &operator[](const Position &pos);
  const Tile &operator[](const Position &pos) const;

  ~Grid() = default;
};

class Constraint {
protected:
  std::vector<Position> tiles;

public:
  Constraint(std::vector<Position> tiles);

  std::vector<Position> getTiles() const;
  virtual int evaluate(std::vector<int> values) const = 0;

  virtual ~Constraint() = default;
};

class EqualConstraint : public Constraint {
public:
  EqualConstraint(std::vector<Position> tiles);
  int evaluate(std::vector<int> values) const override;
  ~EqualConstraint() = default;
};

class UniqueConstraint : public Constraint {
public:
  UniqueConstraint(std::vector<Position> tiles);
  int evaluate(std::vector<int> values) const override;
  ~UniqueConstraint() = default;
};

class LessThanConstraint : public Constraint {
  int limit;

public:
  LessThanConstraint(std::vector<Position> tiles, int limit);
  int evaluate(std::vector<int> values) const override;
  ~LessThanConstraint() = default;
};

class GreaterThanConstraint : public Constraint {
  int limit;

public:
  GreaterThanConstraint(std::vector<Position> tiles, int limit);
  int evaluate(std::vector<int> values) const override;
  ~GreaterThanConstraint() = default;
};

class ExactSumConstraint : public Constraint {
  int target;

public:
  ExactSumConstraint(std::vector<Position> tiles, int target);
  int evaluate(std::vector<int> values) const override;
  ~ExactSumConstraint() = default;
};

enum PipsActionType { Rotate, Swap };

struct PipsAction {
  PipsActionType action;
  int first;
  int second;
};

template <int Width, int Height> class PipsSolver;

template <int Width, int Height> class PipsState {
  Grid<Width, Height> grid;
  std::vector<Domino> dominos;
  std::shared_ptr<const std::vector<std::shared_ptr<Constraint>>> constraints;

  void rotateDomino(int index);
  void swapDominos(int first, int second);
  void placeDomino(const Domino &domino);

public:
  PipsState(
      std::vector<Position> disabledTiles, std::vector<Domino> dominos,
      std::shared_ptr<std::vector<std::shared_ptr<Constraint>>> constraints);

  std::vector<int> getValues(const std::vector<Position> &positions) const;

  std::vector<PipsAction> availableActions() const;
  PipsState stateAfterAction(const PipsAction &action) const;

  bool isSolved() const;
  int objective() const;

  ~PipsState() = default;

  friend PipsSolver<Width, Height>;
};

// Definitions

// Grid Class ------------------------------------------------------------------
template <int Width, int Height>
Grid<Width, Height>::Grid(const std::vector<Position> &disabledTiles) {
  for (const auto &pos : disabledTiles) {
    if (pos.x >= Width || pos.y >= Height)
      throw std::runtime_error("Disabled tiles out of bounds");
    (*this)[pos] = -2;
  }
}

template <int Width, int Height>
Tile &Grid<Width, Height>::operator[](const Position &pos) {
  if (pos.x >= Width || pos.y >= Height)
    throw std::runtime_error("Indexed position out of bounds");
  return grid[pos.x][pos.y];
}

template <int Width, int Height>
const Tile &Grid<Width, Height>::operator[](const Position &pos) const {
  if (pos.x >= Width || pos.y >= Height)
    throw std::runtime_error("Indexed position out of bounds");
  return grid[pos.x][pos.y];
}

template <int Width, int Height>
std::array<std::array<Tile, Width>, Height>
Grid<Width, Height>::getGrid() const {
  return grid;
}

// End Grid Class --------------------------------------------------------------

// PipsState Class -------------------------------------------------------------

template <int Width, int Height>
PipsState<Width, Height>::PipsState(
    std::vector<Position> disabledTiles, std::vector<Domino> dominos,
    std::shared_ptr<std::vector<std::shared_ptr<Constraint>>> constraints)
    : constraints(std::move(constraints)), dominos(std::move(dominos)),
      grid(disabledTiles) {}

template <int Width, int Height>
void PipsState<Width, Height>::rotateDomino(int index) {
  dominos[index].rotate();
  const auto [pos1, pos2] = dominos[index].getPosition();
  std::swap(grid[pos1], grid[pos2]);
}

template <int Width, int Height>
void PipsState<Width, Height>::placeDomino(const Domino &domino) {
  auto [pos1, pos2] = domino.getPosition();
  auto [val1, val2] = domino.getValues();
  grid[pos1] = val1;
  grid[pos2] = val2;
}

template <int Width, int Height>
void PipsState<Width, Height>::swapDominos(int first, int second) {
  dominos[first].swap(dominos[second]);
  placeDomino(dominos[first]);
  placeDomino(dominos[second]);
}

template <int Width, int Height>
std::vector<int> PipsState<Width, Height>::getValues(
    const std::vector<Position> &positions) const {
  std::vector<int> values{};
  std::transform(positions.begin(), positions.end(), values.begin(),
                 [&](const Position &pos) { return grid[pos].getValue(); });
  return values;
}

template <int Width, int Height>
bool PipsState<Width, Height>::isSolved() const {
  for (const auto &constraint : *constraints) {
    std::vector<int> values{};
    std::vector<Position> positions = constraint->getTiles();
    std::transform(
        positions.begin(), positions.end(), values.begin(),
        [&](Position &position) { return grid[position].getValue(); });
    if (constraint->evaluate(values) != 0)
      return false;
  }
  return true;
}

template <int Width, int Height>
std::vector<PipsAction> PipsState<Width, Height>::availableActions() const {
  std::vector<PipsAction> output{};
  for (int i = 0; i < dominos.size(); i++) {
    output.push_back({PipsActionType::Rotate, i, i});
    for (int j = i + 1; j < dominos.size(); j++) {
      output.push_back({PipsActionType::Swap, i, j});
    }
  }
  return output;
}

template <int Width, int Height>
PipsState<Width, Height>
PipsState<Width, Height>::stateAfterAction(const PipsAction &action) const {
  PipsState output{*this};
  if (action.action == PipsActionType::Swap) {
    output.swapDominos(action.first, action.second);
  } else if (action.action == PipsActionType::Rotate) {
    output.rotateDomino(action.first);
  }
  return output;
}

template <int Width, int Height>
int PipsState<Width, Height>::objective() const {
  int error = 0;
  for (const auto &constraint : *constraints) {
    error += constraint->evaluate(getValues(constraint->getTiles()));
  }
  return error;
}

#endif
